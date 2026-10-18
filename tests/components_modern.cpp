#include "liboai.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace {
using json = nlohmann::json;
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void route(const liboai::ApiResult& result, const char* method, const std::string& path) {
    const auto& echo = result.response.raw_json;
    check(echo.at("method") == method, "component request method");
    const auto actual = echo.at("path").get<std::string>();
    check(actual.size() >= path.size() && actual.compare(actual.size() - path.size(), path.size(), path) == 0,
          "component request route");
}
void echoed(const liboai::ApiResult& result, const json& payload, const std::string& path) {
    route(result, "POST", path);
    check(result.response.raw_json.at("json") == payload, "raw component payload must remain intact");
}
struct TempFile {
    std::filesystem::path path = std::filesystem::temp_directory_path() /
        ("liboai-modern-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    explicit TempFile(const std::string& bytes) {
        std::ofstream file(path, std::ios::binary);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        if (!file) throw std::runtime_error("fixture file creation failed");
    }
    ~TempFile() { std::error_code ignored; std::filesystem::remove(path, ignored); }
};
}

void test_components(const std::string& root) {
    using namespace liboai;
    using namespace netimpl::components;
    ChatCompletion chat(root);
    Images images(root);
    Audio audio(root);
    Embeddings embeddings(root);
    Moderations moderations(root);
    liboai::Files files(root);

    // Real public calls prove that complete tool messages and unknown fields survive.
    const json messages = json::array({
        {{"role", "user"}, {"content", json::array({{{"type", "text"}, {"text", "describe"}},
            {{"type", "image_url"}, {"image_url", {{"url", "data:image/png;base64,AA=="}}}}})}},
        {{"role", "assistant"}, {"content", nullptr}, {"unknown_message_field", {{"retain", true}}},
            {"tool_calls", json::array({{{"id", "call_1"}, {"type", "function"}, {"function", {{"name", "first"}, {"arguments", "{}"}}}},
                                      {{"id", "call_2"}, {"type", "function"}, {"function", {{"name", "second"}, {"arguments", "{}"}}}}})}},
        {{"role", "tool"}, {"tool_call_id", "call_1"}, {"content", "one"}},
        {{"role", "tool"}, {"tool_call_id", "call_2"}, {"content", "two"}}
    });
    json chat_body = {{"model", "fixture"}, {"messages", messages}, {"max_completion_tokens", 200000},
        {"store", true}, {"reasoning_effort", "low"}, {"future_option", {{"nested", 1}}},
        {"response_format", {{"type", "json_object"}}}};
    echoed(chat.create_json(chat_body), chat_body, "/chat/completions");
    auto chat_future = chat.create_json_async(chat_body);
    const auto original = chat_body;
    chat_body["messages"] = json::array();
    echoed(chat_future.get(), original, "/chat/completions");
    route(chat.retrieve("id/ ?"), "GET", "/chat/completions/id%2F%20%3F");
    route(chat.retrieve_async("id").get(), "GET", "/chat/completions/id");
    const json metadata = {{"metadata", {{"project", "blue"}}}};
    echoed(chat.update("id", metadata), metadata, "/chat/completions/id");
    echoed(chat.update_async("id", metadata).get(), metadata, "/chat/completions/id");
    route(chat.remove("id"), "DELETE", "/chat/completions/id");
    route(chat.remove_async("id").get(), "DELETE", "/chat/completions/id");
    const Query query = {{"after", "cursor /+?"}, {"limit", "2"}, {"order", "asc"}};
    auto listed = chat.list(query);
    check(listed.response.raw_json.at("query").at("after").at(0) == "cursor /+?", "chat query encoding");
    route(chat.list_async(query).get(), "GET", "/chat/completions");
    route(chat.list_messages("id", query), "GET", "/chat/completions/id/messages");
    route(chat.list_messages_async("id", query).get(), "GET", "/chat/completions/id/messages");

    for (const auto& input : std::vector<json>{"hello", json::array({"one", "two"}), json::array({1, 2}), json::array({json::array({1, 2}), json::array({3})})}) {
        const json body = {{"model", "fixture"}, {"input", input}, {"dimensions", 256}, {"encoding_format", "base64"}};
        echoed(embeddings.create_json(body), body, "/embeddings");
        echoed(embeddings.create_json_async(body).get(), body, "/embeddings");
    }
    for (const auto& input : std::vector<json>{json::array({"one", "two"}), json::array({{{"type", "text"}, {"text", "hello"}}, {{"type", "image_url"}, {"image_url", {{"url", "data:image/png;base64,AA=="}}}}})}) {
        const json body = {{"model", "fixture"}, {"input", input}};
        echoed(moderations.create_json(body), body, "/moderations");
        echoed(moderations.create_json_async(body).get(), body, "/moderations");
    }
    RequestOptions float_options; float_options.headers["X-Fixture-Component"] = "embedding-float";
    RequestOptions base64_options; base64_options.headers["X-Fixture-Component"] = "embedding-base64";
    const json embedding_body = {{"model", "fixture"}, {"input", "hello"}};
    auto floats = embeddings.create_json(embedding_body, float_options).response.raw_json;
    check(floats.at("data").at(0).at("embedding") == json::array({0.25, -0.5, 1.0}), "float embeddings retained");
    auto encoded = embeddings.create_json(embedding_body, base64_options).response.raw_json;
    check(encoded.at("data").at(0).at("embedding") == "AACAPw==", "base64 embeddings retained");
    check(encoded.at("data").at(0).at("future_field").at("preserve") == true, "unknown embedding fields retained");
    RequestOptions moderation_options; moderation_options.headers["X-Fixture-Component"] = "moderation";
    auto scores = moderations.create_json({{"input", "hello"}}, moderation_options).response.raw_json.at("results").at(0);
    check(scores.at("category_scores").at("future_category") == 0.375 && scores.at("future_field").at("preserve") == true,
          "moderation category scores and unknown fields retained");
    const json image_body = {{"model", "fixture-image"}, {"prompt", "cup"}, {"background", "transparent"},
        {"output_format", "webp"}, {"output_compression", 85}, {"quality", "high"}};
    echoed(images.create_json(image_body), image_body, "/images/generations");
    echoed(images.create_json_async(image_body).get(), image_body, "/images/generations");

    const std::string binary("\0\1\2\xff", 4);
    TempFile input(binary), output("placeholder");
    Multipart edit = {{"model", "fixture-image"}, {"prompt", "combine"},
        {"image[]", File{input.path.string(), "first.png"}}, {"image[]", File{input.path.string(), "second.png"}},
        {"output_format", "png"}};
    auto edit_result = images.create_edit_multipart(edit);
    route(edit_result, "POST", "/images/edits");
    const auto parts = edit_result.response.raw_json.at("multipart");
    check(parts.at(2).at("name") == "image[]" && parts.at(3).at("name") == "image[]", "repeated image fields");
    check(parts.at(2).at("filename") == "first.png" && parts.at(3).at("filename") == "second.png", "image filenames");
    check(parts.at(2).at("data_base64") == "AAEC/w==" && parts.at(3).at("data_base64") == "AAEC/w==", "binary edit inputs");
    route(images.create_edit_multipart_async(edit).get(), "POST", "/images/edits");

    Multipart transcription = {{"file", File{input.path.string()}}, {"model", "fixture-transcribe"},
        {"language", "de"}, {"timestamp_granularities[]", "word"}, {"timestamp_granularities[]", "segment"},
        {"chunking_strategy", "auto"}, {"known_speaker_names[]", "speaker-one"}};
    route(audio.transcribe_multipart(transcription), "POST", "/audio/transcriptions");
    route(audio.transcribe_multipart_async(transcription).get(), "POST", "/audio/transcriptions");
    route(audio.translate_multipart(transcription), "POST", "/audio/translations");
    route(audio.translate_multipart_async(transcription).get(), "POST", "/audio/translations");
    const json speech = {{"model", "fixture"}, {"voice", "alloy"}, {"input", "hello"}, {"instructions", "Speak warmly"}};
    echoed(audio.speech_json(speech), speech, "/audio/speech");
    echoed(audio.speech_json_async(speech).get(), speech, "/audio/speech");
    const auto legacy_speech = audio.speech_async("fixture", "alloy", "legacy-async", "wav", 1.0f).get();
    check(legacy_speech.raw_json.at("json").at("input") == "legacy-async", "legacy speech_async payload");
    check(legacy_speech.raw_json.at("path").get<std::string>().find("/audio/speech") != std::string::npos, "legacy speech_async route regression");
    json binary_speech = speech; binary_speech["input"] = "fixture-binary";
    const auto bytes = audio.speech_json(binary_speech).response.content;
    check(!bytes.empty(), "binary speech response retained");
    std::string streamed;
    audio.speech_bytes(binary_speech, [&streamed](std::string chunk, intptr_t) { streamed += chunk; return true; });
    check(streamed == bytes, "raw speech streaming preserves bytes");
    streamed.clear();
    (void)audio.speech_bytes_async(binary_speech, [&streamed](std::string chunk, intptr_t) { streamed += chunk; return true; }).get();
    check(streamed == bytes, "async raw speech streaming preserves bytes");

    const Query filters = {{"purpose", "user_data"}, {"order", "desc"}, {"limit", "1"}, {"after", "cursor +/?"}};
    auto file_list = files.list_with_options(filters);
    check(file_list.response.raw_json.at("query").at("after").at(0) == "cursor +/?", "file cursor encoding");
    route(files.list_with_options_async(filters).get(), "GET", "/files");
    RequestOptions pages; pages.headers["X-Fixture-Component"] = "files-pagination";
    Query page_query = {{"purpose", "user_data"}, {"limit", "1"}};
    std::vector<std::string> file_ids;
    for (int page = 0; page != 3; ++page) {
        const auto page_result = files.list_with_options(page_query, pages).response.raw_json;
        for (const auto& file : page_result.at("data")) file_ids.push_back(file.at("id").get<std::string>());
        if (!page_result.at("has_more").get<bool>()) break;
        page_query.emplace_back("after", page_result.at("last_id").get<std::string>());
    }
    check(file_ids == std::vector<std::string>{"file /first", "file-second"}, "explicit file pagination traversal");
    Multipart upload = {{"file", File{input.path.string()}}, {"purpose", "user_data"},
        {"expires_after[anchor]", "created_at"}, {"expires_after[seconds]", "3600"}};
    route(files.create_multipart(upload), "POST", "/files");
    route(files.create_multipart_async(upload).get(), "POST", "/files");
    std::vector<unsigned char> buffer = {0, 1, 2, 255};
    Multipart buffered = {{"file", Buffer{buffer.begin(), buffer.end(), "buffer.bin"}}, {"purpose", "user_data"}};
    auto buffered_future = files.create_multipart_async(buffered);
    buffer.assign(4, 42);
    check(buffered_future.get().response.raw_json.at("multipart").at(0).at("data_base64") == "AAEC/w==",
          "async multipart must own buffer snapshot");
    auto content = files.content("file-download");
    check(files.content_async("file-download").get().response.content == content.response.content, "async file content");
    check(files.download("file-download", output.path.string()), "custom-root file download");
    std::ifstream downloaded(output.path, std::ios::binary);
    const std::string downloaded_bytes((std::istreambuf_iterator<char>(downloaded)), {});
    check(downloaded_bytes == content.response.content, "custom-root download content");

    // Generic parser preserves domain-specific partial image/audio event JSON.
    RequestOptions image_stream; image_stream.headers["X-Fixture-Component"] = "image-stream";
    std::vector<SseEvent> events;
    auto callback = [&events](const SseEvent& event) { events.push_back(event); return true; };
    check(images.create_stream(image_body, callback, image_stream).status == StreamStatus::completed, "image completed event status");
    check(!events.empty(), "image parsed streaming emits events");
    bool has_image = false;
    for (const auto& event : events) if (event.json.is_object() && event.json.contains("b64_json")) has_image = true;
    check(has_image, "image base64 event retained");
    (void)images.create_stream_async(image_body, callback, image_stream).get();
    edit.parts.emplace_back("stream", "false");
    edit.parts.emplace_back("stream", "false");
    check(images.create_edit_stream(edit, callback, image_stream).status == StreamStatus::completed, "image edit stream flags replaced");
    (void)images.create_edit_stream_async(edit, callback, image_stream).get();
    RequestOptions audio_stream; audio_stream.headers["X-Fixture-Component"] = "audio-stream";
    events.clear();
    audio.speech_stream(speech, callback, audio_stream);
    check(!events.empty(), "speech parsed streaming emits events");
    (void)audio.speech_stream_async(speech, callback, audio_stream).get();
    transcription.parts.emplace_back("stream", "false");
    check(audio.transcribe_stream(transcription, callback, audio_stream).status == StreamStatus::completed, "transcription completion status");
    (void)audio.transcribe_stream_async(transcription, callback, audio_stream).get();
    RequestOptions chat_stream; chat_stream.headers["X-Fixture-Component"] = "chat-stream";
    events.clear();
    check(chat.create_stream(original, callback, chat_stream).status == StreamStatus::completed, "chat DONE completion");
    check(events.front().json.at("choices").at(0).at("delta").at("tool_calls").at(0).at("id") == "call_one", "streamed chat tool call retained");
    (void)chat.create_stream_async(original, callback, chat_stream).get();
    const auto cancelled = images.create_stream(image_body, [](const SseEvent&) { return false; }, image_stream);
    check(cancelled.status == StreamStatus::locally_cancelled, "image local callback cancellation");
}
