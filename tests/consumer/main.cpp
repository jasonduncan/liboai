#include <liboai.h>
#include <type_traits>

int main() {
    liboai::OpenAI api("http://127.0.0.1:1/v1");
    // Legacy overload and function-pointer compatibility, without network calls.
    using RawCreate = liboai::Response (liboai::Responses::*)(const nlohmann::json&,
        std::optional<liboai::Responses::StreamCallback>) const &;
    RawCreate raw = &liboai::Responses::create;
    auto old_image = &liboai::Images::create;
    auto old_speech = &liboai::Audio::speech;
    auto old_files = &liboai::Files::list;
    (void)raw; (void)old_image; (void)old_speech; (void)old_files;
    liboai::Conversations conversations;
    liboai::Batch batch;
    liboai::Uploads uploads;
    liboai::VectorStores stores;
    liboai::Containers containers;
    liboai::Webhooks webhooks;
    liboai::Decisions decisions;
    liboai::Skills skills;
    if (!liboai::parse_decision_answers(nlohmann::json{{"answers", nlohmann::json::array()}}).empty()) return 1;
    liboai::RequestOptions options;
    liboai::ApiResult result;
    liboai::SseParser parser([](const liboai::SseEvent&) { return true; });
    if (!parser.feed("data: {}\n\n")) return 1;
    parser.finish();
    if (!result.request_id().empty()) return 1;
    if (!liboai::Responses::output_text(nlohmann::json{{"output", nlohmann::json::array()}}).empty()) return 1;
    (void)liboai::webhook_verification_available();
    return 0;
}
