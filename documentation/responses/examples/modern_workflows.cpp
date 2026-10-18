#include "liboai.h"
#include <chrono>
#include <thread>

using Json = nlohmann::json;

// Explicitly run with a model and workflow. These examples incur API usage.
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: modern_workflows MODEL background|stream|structured|tool|multi-agent\n";
        return 1;
    }
    liboai::OpenAI api;
    if (!api.auth.SetKeyEnv("OPENAI_API_KEY")) return 1;
    const std::string model = argv[1], mode = argv[2];
    liboai::RequestOptions options;
    options.timeout = std::chrono::seconds(60);
    Json request = {{"model", model}, {"input", "Say hello in one sentence."}, {"max_output_tokens", 128}};
    try {
        if (mode == "background") {
            request["background"] = true;
            request["store"] = true;
            auto response = api.Responses->create_with_options(request, options);
            const auto id = response.response.raw_json.at("id").get<std::string>();
            // Polling is deliberately bounded. Local receipt cancellation is separate.
            for (int attempt = 0; attempt < 10; ++attempt) {
                const auto status = response.response.raw_json.at("status").get<std::string>();
                if (status != "queued" && status != "in_progress") break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                response = api.Responses->retrieve(id, {}, options);
            }
            const auto status = response.response.raw_json.at("status").get<std::string>();
            if (status == "queued" || status == "in_progress") response = api.Responses->cancel(id, Json::object(), options);
            std::cout << response.response << '\n';
        } else if (mode == "stream") {
            auto stream = api.Responses->create_stream(request, [](const liboai::SseEvent& event) {
                if (auto delta = event.text_delta()) std::cout << *delta << std::flush;
                return true;
            }, options);
            if (stream.status != liboai::StreamStatus::completed) {
                std::cerr << "\nStream did not complete; inspect terminal_response and status.\n";
                return 2;
            }
        } else if (mode == "structured") {
            request["text"] = {{"format", {{"type", "json_schema"}, {"name", "greeting"}, {"strict", true},
                {"schema", {{"type", "object"}, {"properties", {{"greeting", {{"type", "string"}}}}},
                    {"required", Json::array({"greeting"})}, {"additionalProperties", false}}}}}};
            auto response = api.Responses->create_with_options(request, options);
            std::cout << response.response << '\n'; // Preserve refusals and incomplete status.
        } else if (mode == "tool") {
            request["input"] = "Call synthetic_temperature for the fixture reading.";
            request["tools"] = Json::array({{{"type", "function"}, {"name", "synthetic_temperature"},
                {"description", "Return a synthetic demonstration reading."}, {"strict", true},
                {"parameters", {{"type", "object"}, {"properties", Json::object()}, {"required", Json::array()}, {"additionalProperties", false}}}}});
            auto first = api.Responses->create_with_options(request, options).response.raw_json;
            Json results = Json::array();
            for (const auto& item : first.at("output")) {
                if (item.value("type", "") != "function_call") continue;
                if (item.value("name", "") != "synthetic_temperature" || Json::parse(item.at("arguments").get<std::string>()) != Json::object())
                    throw std::runtime_error("Unexpected tool name or arguments");
                // Application-owned, explicitly allowlisted synthetic action.
                results.push_back({{"type", "function_call_output"}, {"call_id", item.at("call_id")},
                    {"output", R"({"celsius":21,"synthetic":true})"}});
            }
            if (!results.empty()) {
                request["previous_response_id"] = first.at("id");
                request["input"] = results;
                std::cout << api.Responses->create_with_options(request, options).response << '\n';
            } else std::cout << first.dump(2) << '\n';
        } else if (mode == "multi-agent") {
            // Choose a model documented to support this beta; GPT-5.2 is not implied.
            options.headers["OpenAI-Beta"] = "responses_multi_agent=v1";
            request["multi_agent"] = {{"enabled", true}, {"max_concurrent_subagents", 2}};
            request["input"] = "Compare two approaches to organizing a tiny synthetic TODO list.";
            auto response = api.Responses->create_with_options(request, options).response.raw_json;
            std::cout << liboai::Responses::output_text(response, "/root") << '\n';
        } else {
            std::cerr << "Unknown workflow\n";
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
