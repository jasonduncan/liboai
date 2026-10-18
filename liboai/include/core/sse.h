#pragma once

#include "request.h"
#include <string_view>

namespace liboai {
// Original data is retained even for unknown events or non-JSON payloads.
struct SseEvent {
    std::string type, data, id;
    nlohmann::json json;
    LIBOAI_EXPORT std::optional<std::string> text_delta() const;
};
using SseCallback = std::function<bool(const SseEvent&)>;

// One parser per stream. feed accepts arbitrary byte boundaries, including UTF-8.
// finish discards a frame without its terminating blank line (the SSE contract).
class SseParser {
public:
    LIBOAI_EXPORT explicit SseParser(SseCallback callback);
    LIBOAI_EXPORT bool feed(std::string_view bytes);
    LIBOAI_EXPORT void finish();
private:
    bool line();
    SseCallback callback_;
    std::string line_, data_, type_, id_;
    bool skip_lf_ = false, first_line_ = true, stopped_ = false, finished_ = false;
};

enum class StreamStatus { ended, completed, failed, incomplete, locally_cancelled };
struct ParsedStreamResult {
    ApiResult result;
    StreamStatus status = StreamStatus::ended;
    std::optional<nlohmann::json> terminal_response;
};
using FutureParsedStreamResult = std::future<ParsedStreamResult>;

namespace detail {
LIBOAI_EXPORT ParsedStreamResult parsed_stream_request(const std::string& root,
    const std::string& method, const std::string& path,
    const std::optional<nlohmann::json>& body, const Query& query,
    const RequestOptions& options, const SseCallback& callback);
LIBOAI_EXPORT ParsedStreamResult parsed_multipart_stream_request(const std::string& root,
    const std::string& path, const netimpl::components::Multipart& parts,
    const RequestOptions& options, const SseCallback& callback);
}
}
