#include "components/responses.h"
#include <stdexcept>
#include <vector>

namespace {
using Json = nlohmann::json;
void require(bool value) { if (!value) throw std::runtime_error("Responses assertion failed"); }
void route(const liboai::ApiResult& result, const std::string& method, const std::string& path) {
    require(result.response.raw_json.at("method") == method);
    require(result.response.raw_json.at("path") == "/v1" + path);
}
}

void test_responses(const std::string& root) {
    liboai::Responses responses(root);
    const Json body = {{"model", "fixture-model"}, {"input", "synthetic"}, {"future", {{"kept", true}}}};
    auto created = responses.create_with_options(body);
    route(created, "POST", "/responses");
    require(created.response.raw_json.at("json") == body);
    route(responses.create_with_options_async(body).get(), "POST", "/responses");
    const liboai::Query query = {{"include[]", "a/b"}, {"include[]", "c+d"}, {"starting_after", "9007199254740991"}};
    auto retrieved = responses.retrieve("id /?#", query);
    route(retrieved, "GET", "/responses/id%20%2F%3F%23");
    require(retrieved.response.raw_json.at("query").at("include[]") == Json::array({"a/b", "c+d"}));
    require(retrieved.response.raw_json.at("query").at("starting_after") == Json::array({"9007199254740991"}));
    route(responses.retrieve_async("id").get(), "GET", "/responses/id");
    route(responses.remove("id"), "DELETE", "/responses/id");
    route(responses.remove_async("id").get(), "DELETE", "/responses/id");
    route(responses.cancel("id", {{"future", true}}), "POST", "/responses/id/cancel");
    route(responses.cancel_async("id").get(), "POST", "/responses/id/cancel");
    route(responses.list_input_items("id", {{"after", "a+b"}, {"limit", "2"}, {"order", "asc"}}), "GET", "/responses/id/input_items");
    route(responses.list_input_items_async("id").get(), "GET", "/responses/id/input_items");
    route(responses.count_input_tokens(body), "POST", "/responses/input_tokens");
    route(responses.count_input_tokens_async(body).get(), "POST", "/responses/input_tokens");
    route(responses.compact(body), "POST", "/responses/compact");
    route(responses.compact_async(body).get(), "POST", "/responses/compact");
    require(responses.retrieve("fixture-background").response.raw_json.at("status") == "in_progress");
    require(responses.cancel("fixture-background").response.raw_json.at("status") == "cancelled");
    auto first = responses.list_input_items("fixture-pages").response.raw_json;
    auto second = responses.list_input_items("fixture-pages", {{"after", first.at("last_id").get<std::string>()}}).response.raw_json;
    require(first.at("has_more") == true && second.at("has_more") == false);
    require(second.at("data").at(0).at("id") == "item_2");

    std::vector<liboai::SseEvent> events;
    auto callback = [&](const liboai::SseEvent& e) { events.push_back(e); return true; };
    Json streaming = {{"input", "fixture-stream"}, {"stream", false}};
    auto stream = responses.create_stream(streaming, callback);
    require(streaming.at("stream") == false);
    require(stream.status == liboai::StreamStatus::completed && stream.terminal_response->at("status") == "completed");
    require(events.size() == 5 && events[1].json.at("item").at("type") == "function_call");
    require(events[2].text_delta() == "snowman ☃" && events[3].json.at("unknown").at("keep") == true);
    require(stream.result.request_id() == "req_stream");
    // The options/raw path leaves stream selection under caller control.
    std::string raw_body;
    auto raw = responses.create_with_options(streaming, {}, liboai::Responses::StreamCallback(
        [&](std::string bytes, intptr_t) { raw_body += bytes; return true; }));
    require(!raw.locally_cancelled && Json::parse(raw_body).at("json").at("stream") == false);
    require(responses.create_stream_async(streaming, callback).get().status == liboai::StreamStatus::completed);
    require(responses.retrieve_stream("fixture-stream", callback, {{"stream", "false"}, {"starting_after", "1"}}).status == liboai::StreamStatus::completed);
    require(responses.retrieve_stream_async("fixture-stream", callback).get().status == liboai::StreamStatus::completed);
    for (const auto& entry : std::vector<std::pair<std::string, liboai::StreamStatus>>{
            {"fixture-failed", liboai::StreamStatus::failed}, {"fixture-incomplete", liboai::StreamStatus::incomplete},
            {"fixture-ended", liboai::StreamStatus::ended}})
        require(responses.create_stream({{"input", entry.first}}, callback).status == entry.second);
    auto cancelled = responses.create_stream(streaming, [](const liboai::SseEvent&) { return false; });
    require(cancelled.result.locally_cancelled && cancelled.status == liboai::StreamStatus::locally_cancelled);
    bool exception = false;
    try { responses.create_stream(streaming, [](const liboai::SseEvent&) -> bool { throw std::logic_error("callback"); }); }
    catch (const std::logic_error&) { exception = true; }
    require(exception);

    auto output = Json::parse(R"({"output":[
        {"type":"function_call","arguments":"{}"},
        {"type":"message","role":"assistant","agent":{"agent_name":"/root"},"content":[{"type":"output_text","text":"A","annotations":[{"unknown":1}]},{"type":"refusal","refusal":"no"},{"type":"output_text","text":"B"}]},
        {"type":"message","role":"assistant","agent":{"agent_name":"/root/child"},"content":[{"type":"output_text","text":"C"}]},
        {"type":"message","role":"user","content":[{"type":"output_text","text":"ignored"}]},
        {"type":"message","role":"assistant","content":[null,{"type":"output_text","text":false}]}
    ]})");
    const auto original = output;
    require(liboai::Responses::output_text(output) == "ABC");
    require(liboai::Responses::output_text(output, "/root") == "AB");
    require(liboai::Responses::output_text(Json::object()).empty() && output == original);
    require(liboai::Responses::output_text({{"output", Json::array({{{"type", "function_call"}}})}}).empty());
}
