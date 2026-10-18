#include "common.h"

// Four synthetic generations: three GPT-5.2 requests capped at 64 tokens,
// one GPT-5.2-pro request capped at 128. Compaction has no output-token option.
namespace {
void own_response(const std::string& id) {
    own("response", id, [id] {
        Responses api("https://api.openai.com/v1");
        const auto state = api.retrieve(id, {}, opts()).response.raw_json.value("status", "");
        if (state == "queued" || state == "in_progress") api.cancel(id, J::object(), opts());
        check(api.remove(id, opts()).response.raw_json.value("deleted", false), "Response deletion not confirmed");
    });
}
void usage(const std::string& name, const J& value) {
    record(name, "INFO", {{"model", value.value("model", "")}, {"status", value.value("status", "")},
        {"usage", value.value("usage", J())}, {"output", Responses::output_text(value)}});
}
}
int main() {
    return execute("responses", [] {
        Responses api("https://api.openai.com/v1");
        const auto options = opts();
        const J base = {{"model", "gpt-5.2"}, {"input", "Return exactly OK."},
            {"max_output_tokens", 64}, {"reasoning", {{"effort", "none"}}}, {"store", false}};
        run("responses.input_tokens", [&] {
            const auto value = api.count_input_tokens({{"model", "gpt-5.2"}, {"input", "Return exactly OK."}}, options);
            check(value.response.raw_json.at("input_tokens").get<int>() > 0, "Missing input token count");
        });
        std::string stored;
        if (run("responses.create.structured", [&] {
            J body = base;
            body["store"] = true;
            body["input"] = "Return an object whose ok property is true.";
            body["text"] = J::parse(R"({"format":{"type":"json_schema","name":"smoke","strict":true,"schema":{"type":"object","properties":{"ok":{"type":"boolean"}},"required":["ok"],"additionalProperties":false}}})");
            const auto result = api.create_with_options(body, options);
            const auto& value = result.response.raw_json;
            stored = value.at("id").get<std::string>();
            own_response(stored);
            usage("responses.structured.details", value);
            check(value.at("status") == "completed", "Response incomplete");
            check(J::parse(Responses::output_text(value)).at("ok") == true, "Structured output mismatch");
            check(!result.request_id().empty(), "Missing request ID");
        })) {
            run("responses.retrieve_async", [&] {
                check(api.retrieve_async(stored, {}, options).get().response.raw_json.at("id") == stored, "Stored response mismatch");
            });
            run("responses.list_input_items", [&] {
                check(api.list_input_items(stored, {{"limit", "1"}}, options).response.raw_json.at("data").size() == 1, "Missing input items");
            });
        }
        run("responses.compact", [&] {
            const auto result = api.compact({{"model", "gpt-5.2"}, {"input", J::array({
                {{"role", "user"}, {"content", "Remember the synthetic token blue."}},
                {{"role", "assistant"}, {"content", "The token is blue."}}})}}, options);
            check(result.response.raw_json.at("object") == "response.compaction", "Unexpected compact object");
            record("responses.compact.usage", "INFO", {{"usage", result.response.raw_json.value("usage", J())}});
        });
        std::string streamed;
        if (run("responses.create_stream.background", [&] {
            J body = base;
            body["store"] = true;
            body["background"] = true;
            std::string text;
            int events = 0;
            const auto result = api.create_stream(body, [&](const SseEvent& event) {
                ++events;
                if (event.type == "response.created" && event.json.contains("response") && streamed.empty()) {
                    streamed = event.json.at("response").at("id").get<std::string>();
                    own_response(streamed);
                }
                if (auto delta = event.text_delta()) text += *delta;
                return true;
            }, options);
            check(result.status == StreamStatus::completed && result.terminal_response.has_value(), "Streaming incomplete");
            // Track the terminal ID too if a service omitted response.created.
            if (streamed.empty()) {
                streamed = result.terminal_response->at("id").get<std::string>();
                own_response(streamed);
            }
            check(text == Responses::output_text(*result.terminal_response), "Stream/output text mismatch");
            usage("responses.stream.details", *result.terminal_response);
            record("responses.stream.events", "INFO", {{"count", events}});
        })) {
            run("responses.retrieve_stream", [&] {
                int events = 0;
                const auto result = api.retrieve_stream(streamed, [&](const SseEvent&) { ++events; return true; }, {}, options);
                check(events > 0 && result.status == StreamStatus::completed, "Replay incomplete");
            });
        }
        run("responses.pro.async", [&] {
            J body = base;
            body["model"] = "gpt-5.2-pro";
            body["reasoning"] = {{"effort", "medium"}};
            body["max_output_tokens"] = 128;
            const auto result = api.create_with_options_async(body, options).get();
            usage("responses.pro.details", result.response.raw_json);
            check(result.response.raw_json.at("status") == "completed", "Pro output incomplete within token cap");
        });
        std::string background;
        if (run("responses.background.create", [&] {
            J body = base;
            body["background"] = true;
            body["store"] = true;
            const auto result = api.create_with_options(body, options);
            background = result.response.raw_json.at("id").get<std::string>();
            own_response(background);
        })) {
            run("responses.cancel", [&] {
                const auto state = api.retrieve(background, {}, options).response.raw_json.value("status", "");
                if (state == "queued" || state == "in_progress") {
                    const auto cancelled = api.cancel(background, J::object(), options).response.raw_json.value("status", "");
                    check(cancelled == "cancelled", "Cancellation not confirmed");
                } else record("responses.cancel", "SKIPPED", {{"reason", "Background response already terminal"}, {"state", state}});
            });
        }
    });
}
