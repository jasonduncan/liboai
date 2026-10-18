#include "common.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>

namespace {
constexpr auto api_root = "https://api.openai.com/v1";

void details(const std::string& name, const ApiResult& result, J extra = J::object()) {
    extra["http"] = result.response.status_code;
    extra["request_id"] = result.request_id();
    record(name + "_details", "INFO", extra);
}

void write_bytes(const std::filesystem::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    check(bool(output), "Could not write synthetic artifact");
}

std::string decode_image(const std::string& input) {
    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned accumulator = 0;
    int bits = -8;
    std::string output;
    for (unsigned char character : input) {
        if (character == '=') break;
        const auto digit = alphabet.find(character);
        check(digit != std::string::npos, "Invalid image base64");
        accumulator = (accumulator << 6) | static_cast<unsigned>(digit);
        bits += 6;
        if (bits >= 0) {
            output.push_back(static_cast<char>((accumulator >> bits) & 255));
            bits -= 8;
        }
    }
    return output;
}

void chat_checks() {
    ChatCompletion chat(api_root);
    const std::string marker = "liboai-live-" +
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    // At most two generation requests, each explicitly capped at 64 output tokens.
    const J body = {
        {"model", "gpt-5.2"}, {"reasoning_effort", "none"}, {"max_completion_tokens", 64}, {"store", false},
        {"messages", J::array({{{"role", "user"}, {"content", "Reply with the single word hello."}}})}
    };
    J stored_body = body;
    stored_body["store"] = true;
    stored_body["metadata"] = {{"liboai_smoke", marker}};
    std::string stored;
    const bool created = run("chat_create_stored", [&] {
        auto result = chat.create_json(stored_body, opts());
        stored = result.response.raw_json.at("id").get<std::string>();
        // Register before any assertion that could abandon a successfully created resource.
        own("stored_chat", stored, [id = stored] {
            ChatCompletion cleanup_client(api_root);
            // The server can acknowledge creation before the stored object is visible.
            // Only reads are retried; delete is attempted exactly once after readiness.
            wait_for_readiness("chat_cleanup_readiness", [&] {
                const auto found = cleanup_client.retrieve(id, opts());
                check(found.response.raw_json.at("id") == id, "Cleanup chat ID mismatch");
                return true;
            });
            const auto removed = cleanup_client.remove(id, opts());
            check(removed.response.raw_json.value("deleted", false), "Stored chat deletion not confirmed");
            details("chat_remove_cleanup", removed, {{"id", id}});
            release(id);
        });
        check(!result.response.raw_json.at("choices").empty(), "Missing chat choices");
        details("chat_create_stored", result, {
            {"model", result.response.raw_json.value("model", "")}, {"id", stored},
            {"usage", result.response.raw_json.value("usage", J::object())}
        });
    });
    if (!created) {
        record("chat_dependents", "SKIP", {{"reason", "Stored creation failed; no generation retry"}});
        return;
    }
    const bool ready = run("chat_stored_readiness", [&] {
        wait_for_readiness("chat_stored_readiness", [&] {
            auto result = chat.retrieve(stored, opts());
            check(result.response.raw_json.at("id") == stored, "Stored chat ID mismatch");
            details("chat_retrieve", result);
            return true;
        });
    });
    if (ready) {
        run("chat_update", [&] {
            auto result = chat.update(stored,
                {{"metadata", {{"liboai_smoke", marker}, {"stage", "verified"}}}}, opts());
            check(result.response.raw_json.at("metadata").at("stage") == "verified", "Chat metadata mismatch");
            details("chat_update", result);
        });
        run("chat_messages", [&] {
            auto result = chat.list_messages(stored, {{"limit", "2"}}, opts());
            check(!result.response.raw_json.at("data").empty(), "Missing stored messages");
            details("chat_messages", result, {{"count", result.response.raw_json.at("data").size()}});
        });
        run("chat_list_filtered", [&] {
            wait_for_readiness("chat_list_visibility", [&] {
                auto result = chat.list({{"metadata[liboai_smoke]", marker}, {"limit", "1"}}, opts());
                const auto& data = result.response.raw_json.at("data");
                check(data.is_array(), "Missing completion list");
                bool found = false;
                for (const auto& item : data) if (item.at("id") == stored) found = true;
                if (found) details("chat_list_filtered", result, {{"count", data.size()}});
                return found;
            });
        });
    } else {
        record("chat_stored_lifecycle", "SKIP", {{"reason", "Owned stored chat did not become ready"}});
    }
    run("chat_stream", [&] {
        std::size_t events = 0, text_bytes = 0;
        auto result = chat.create_stream(body, [&](const SseEvent& event) {
            ++events;
            if (event.json.is_object() && event.json.contains("choices")) {
                for (const auto& choice : event.json["choices"]) {
                    if (choice.contains("delta") && choice["delta"].value("content", J()).is_string())
                        text_bytes += choice["delta"]["content"].get<std::string>().size();
                }
            }
            return true;
        }, opts());
        check(result.status == StreamStatus::completed && text_bytes > 0, "Chat stream missing DONE or text");
        details("chat_stream", result.result, {{"events", events}, {"text_bytes", text_bytes}});
    });
}

void embedding_and_moderation_checks() {
    Embeddings embeddings(api_root);
    Moderations moderation(api_root);
    run("embeddings_batch_dimensions_base64", [&] {
        auto result = embeddings.create_json({
            {"model", "text-embedding-3-small"}, {"input", J::array({"hello", "world"})},
            {"dimensions", 16}, {"encoding_format", "base64"}
        }, opts());
        const auto& data = result.response.raw_json.at("data");
        check(data.size() == 2 && data[0].at("embedding").is_string(), "Embedding batch/base64 mismatch");
        details("embeddings", result, {{"count", 2}, {"encoding", "base64"}, {"dimensions", 16}});
    });
    run("moderation_text_batch", [&] {
        auto result = moderation.create_json({
            {"model", "omni-moderation-latest"},
            {"input", J::array({"Hello, friend.", "A blue ceramic cup."})}
        }, opts());
        const auto& data = result.response.raw_json.at("results");
        check(data.size() == 2 && data[0].at("category_scores").is_object(), "Moderation category scores missing");
        details("moderation", result, {{"count", 2}});
    });
}

void audio_checks() {
    Audio audio(api_root);
    const std::filesystem::path wav = artifact("synthetic-speech.wav");
    // Three tiny TTS requests maximum, followed by one transcription and translation.
    const bool wav_ready = run("legacy_speech_async_dispatch", [&] {
        auto response = audio.speech_async("tts-1", "alloy", "Hello world.", "wav").get();
        check(response.status_code == 200 && response.content.size() > 44 && response.content.substr(0, 4) == "RIFF",
              "Legacy async speech did not return WAV");
        write_bytes(wav, response.content);
        record("legacy_speech_async_details", "INFO", {
            {"http", response.status_code}, {"bytes", response.content.size()}, {"format", "wav"}
        });
    });
    const bool modern_tts = run("audio_speech_json", [&] {
        auto result = audio.speech_json({
            {"model", "gpt-4o-mini-tts"}, {"voice", "alloy"}, {"input", "Hello world."},
            {"instructions", "Speak clearly."}, {"response_format", "wav"}
        }, opts());
        check(result.response.content.size() > 44, "Speech binary absent");
        details("audio_speech_json", result, {{"bytes", result.response.content.size()}});
    });
    if (modern_tts) {
        run("audio_speech_sse", [&] {
            std::size_t events = 0, audio_bytes = 0;
            std::set<std::string> types;
            auto result = audio.speech_stream({
                {"model", "gpt-4o-mini-tts"}, {"voice", "alloy"}, {"input", "Hello."}, {"response_format", "pcm"}
            }, [&](const SseEvent& event) {
                ++events;
                types.insert(event.type);
                if (event.json.is_object()) {
                    for (const auto* key : {"audio", "delta"})
                        if (event.json.contains(key) && event.json[key].is_string())
                            audio_bytes += event.json[key].get<std::string>().size();
                }
                return true;
            }, opts());
            check(events > 0 && audio_bytes > 0 && result.status == StreamStatus::completed,
                  "Speech SSE lacked audio data or completion");
            details("audio_speech_sse", result.result, {
                {"events", events}, {"event_types", types}, {"encoded_audio_bytes", audio_bytes}
            });
        });
    } else record("audio_speech_sse", "SKIP", {{"reason", "Modern TTS failed; no permission retry"}});
    if (!wav_ready) {
        record("audio_transcription_translation", "SKIP", {{"reason", "Synthetic WAV creation failed"}});
        return;
    }
    using namespace netimpl::components;
    run("audio_transcription_sse", [&] {
        std::size_t events = 0, text_bytes = 0;
        Multipart parts = {{"file", File{wav.string()}}, {"model", "gpt-4o-mini-transcribe"}, {"language", "en"}};
        auto result = audio.transcribe_stream(parts, [&](const SseEvent& event) {
            ++events;
            if (event.json.is_object() && event.json.contains("delta") && event.json["delta"].is_string())
                text_bytes += event.json["delta"].get<std::string>().size();
            return true;
        }, opts());
        check(events > 0 && text_bytes > 0 && result.status == StreamStatus::completed,
              "Transcription missing terminal event or text");
        details("audio_transcription_sse", result.result, {{"events", events}, {"text_bytes", text_bytes}});
    });
    run("audio_translation_multipart", [&] {
        Multipart parts = {{"file", File{wav.string()}}, {"model", "whisper-1"}};
        auto result = audio.translate_multipart(parts, opts());
        check(result.response.raw_json.at("text").is_string(), "Translation text missing");
        details("audio_translation_multipart", result,
            {{"text_bytes", result.response.raw_json.at("text").get<std::string>().size()}});
    });
}

void image_checks() {
    Images images(api_root);
    const std::filesystem::path png = artifact("synthetic-image.png");
    // One generated image plus one edit, both low quality; no paid partial images.
    const bool image_ready = run("image_json_generation", [&] {
        auto result = images.create_json({
            {"model", "gpt-image-1.5"},
            {"prompt", "A small solid blue square centered on a white background. No text."},
            {"n", 1}, {"size", "1024x1024"}, {"quality", "low"}, {"output_format", "png"}
        }, opts());
        const auto encoded = result.response.raw_json.at("data").at(0).at("b64_json").get<std::string>();
        const auto decoded = decode_image(encoded);
        check(decoded.size() > 8 && decoded.substr(0, 8) == std::string("\x89PNG\r\n\x1a\n", 8),
              "Image output is not PNG");
        write_bytes(png, decoded);
        details("image_json_generation", result, {
            {"model", "gpt-image-1.5"}, {"quality", "low"}, {"bytes", decoded.size()}
        });
    });
    if (!image_ready) {
        record("image_edit_multipart_sse", "SKIP", {{"reason", "Generation failed; no image permission retry"}});
        return;
    }
    run("image_edit_multipart_sse", [&] {
        using namespace netimpl::components;
        Multipart parts = {
            {"model", "gpt-image-1.5"}, {"image[]", File{png.string()}},
            {"prompt", "Change the blue square to green. Keep the white background."},
            {"n", "1"}, {"size", "1024x1024"}, {"quality", "low"},
            {"output_format", "png"}, {"partial_images", "0"}
        };
        std::size_t events = 0, encoded_bytes = 0;
        auto result = images.create_edit_stream(parts, [&](const SseEvent& event) {
            ++events;
            if (event.type == "image_edit.completed" && event.json.contains("b64_json"))
                encoded_bytes = event.json["b64_json"].get<std::string>().size();
            return true;
        }, opts());
        check(result.status == StreamStatus::completed && encoded_bytes > 0, "Image edit final image absent");
        details("image_edit_multipart_sse", result.result, {{"events", events}, {"encoded_image_bytes", encoded_bytes}});
    });
}
}

int main() {
    return execute("components", [] {
        Authorization::Authorizer().SetMaxTimeout(45000); // Legacy speech has no RequestOptions argument.
        chat_checks();
        embedding_and_moderation_checks();
        audio_checks();
        image_checks();
        record("offline_only_variants", "INFO", {
            {"methods", J::array({"audio.speech_bytes", "audio.transcribe_multipart non-streaming",
                "images.create_stream generation", "images.create_edit_multipart non-streaming",
                "modern async overload matrix", "legacy Conversation behavior"})},
            {"reason", "Equivalent routes and serialization covered offline; bounded live generation budget"}
        });
    });
}
