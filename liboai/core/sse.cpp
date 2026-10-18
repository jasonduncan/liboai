#include "../include/core/sse.h"
#include <stdexcept>

std::optional<std::string> liboai::SseEvent::text_delta() const {
    if (type == "response.output_text.delta" && json.is_object()) {
        auto it = json.find("delta");
        if (it != json.end() && it->is_string()) return it->get<std::string>();
    }
    return std::nullopt;
}

liboai::SseParser::SseParser(SseCallback callback) : callback_(std::move(callback)) {
    if (!callback_) throw std::invalid_argument("SSE callback must not be empty");
}

bool liboai::SseParser::feed(std::string_view bytes) {
    if (stopped_ || finished_) return false;
    for (char c : bytes) {
        if (skip_lf_) {
            skip_lf_ = false;
            if (c == '\n') continue;
        }
        if (c == '\r' || c == '\n') {
            skip_lf_ = c == '\r';
            if (!line()) return false;
        } else {
            line_ += c;
        }
    }
    return true;
}

bool liboai::SseParser::line() {
    if (first_line_) {
        first_line_ = false;
        if (line_.compare(0, 3, "\xEF\xBB\xBF") == 0) line_.erase(0, 3);
    }
    if (line_.empty()) {
        if (!data_.empty()) {
            data_.pop_back();
            SseEvent event;
            event.type = type_.empty() ? "message" : type_;
            event.data = std::move(data_);
            event.id = id_;
            auto parsed = nlohmann::json::parse(event.data, nullptr, false);
            if (!parsed.is_discarded()) {
                event.json = std::move(parsed);
                if (type_.empty() && event.json.is_object()) {
                    auto type = event.json.find("type");
                    if (type != event.json.end() && type->is_string()) event.type = type->get<std::string>();
                }
            }
            data_.clear();
            type_.clear();
            stopped_ = !callback_(event);
        }
        type_.clear();
    } else if (line_[0] != ':') {
        auto colon = line_.find(':');
        auto field = line_.substr(0, colon);
        auto value = colon == std::string::npos ? std::string{} : line_.substr(colon + 1);
        if (!value.empty() && value[0] == ' ') value.erase(0, 1);
        if (field == "data") data_ += value + '\n';
        else if (field == "event") type_ = std::move(value);
        else if (field == "id" && value.find('\0') == std::string::npos) id_ = std::move(value);
    }
    line_.clear();
    return !stopped_;
}

void liboai::SseParser::finish() {
    line_.clear(); data_.clear(); type_.clear();
    finished_ = true;
}

namespace {
template<class Send>
liboai::ParsedStreamResult parse_request(const liboai::SseCallback& callback, Send send) {
    if (!callback) throw std::invalid_argument("SSE callback must not be empty");
    liboai::ParsedStreamResult result;
    liboai::SseParser parser([&](const liboai::SseEvent& event) {
        if (event.type == "response.completed" || event.type == "image_generation.completed" ||
            event.type == "image_edit.completed" ||
            event.type == "transcript.text.done" || event.data == "[DONE]")
            result.status = liboai::StreamStatus::completed;
        else if (event.type == "response.failed" || event.type == "error") result.status = liboai::StreamStatus::failed;
        else if (event.type == "response.incomplete") result.status = liboai::StreamStatus::incomplete;
        if ((event.type == "response.completed" || event.type == "response.failed" ||
             event.type == "response.incomplete") && event.json.is_object() && event.json.contains("response"))
            result.terminal_response = event.json["response"];
        return callback(event);
    });
    result.result = send(liboai::RawStreamCallback([&](std::string bytes, intptr_t) { return parser.feed(bytes); }));
    parser.finish();
    if (result.result.locally_cancelled) result.status = liboai::StreamStatus::locally_cancelled;
    return result;
}
}

liboai::ParsedStreamResult liboai::detail::parsed_stream_request(const std::string& root,
    const std::string& method, const std::string& path,
    const std::optional<nlohmann::json>& body, const Query& query,
    const RequestOptions& options, const SseCallback& callback) {
    return parse_request(callback, [&](const RawStreamCallback& raw) {
        return request(root, method, path, body, query, options, raw);
    });
}

liboai::ParsedStreamResult liboai::detail::parsed_multipart_stream_request(const std::string& root,
    const std::string& path, const netimpl::components::Multipart& parts,
    const RequestOptions& options, const SseCallback& callback) {
    return parse_request(callback, [&](const RawStreamCallback& raw) {
        return multipart_request(root, path, parts, options, raw);
    });
}
