#include "components/decisions.h"
#include "components/responses.h"
#include <limits>
#include <stdexcept>

namespace {
void require_decision(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class Function> void rejects_answer(Function call) {
    try { call(); }
    catch (const liboai::exception::DecisionAnswerError&) { return; }
    throw std::runtime_error("Expected explicit malformed decision answer error");
}
}
void test_decisions(const std::string& root) {
    using namespace liboai;
    using json = nlohmann::json;
    Decisions client(root);
    RequestOptions options;
    options.headers["X-Fixture-Extension"] = "decisions";
    options.project = "decision-project";
    json body = {{"model", "future-compatible-fixture-model"}, {"input", json::array({
        {{"role", "user"}, {"content", json::array({{{"type", "input_text"}, {"text", "Customer reports damage"}},
            {{"type", "input_image"}, {"image_url", "data:image/png;base64,AA=="}}})}}})},
        {"questions", json::array({{{"name", "damaged"}, {"type", "predicate"}, {"instructions", "Is the item damaged?"}},
            {{"name", "route"}, {"type", "choice"}, {"instructions", "Choose a route"}, {"choices", json::array({{{"value", true}}, {{"value", "true"}}})}},
            {{"name", "severity"}, {"type", "score"}, {"instructions", "Rate severity"}, {"levels", json::array({{{"label", "low"}}, {{"label", "high"}}})}}})},
        {"future_request_field", {{"preserve", true}}}, {"prompt_cache_key", "synthetic-cache"}};
    auto result = client.create(body, options);
    const auto& request = result.response.raw_json.at("fixture_request");
    require_decision(request.at("json") == body, "Decisions raw payload retained");
    require_decision(request.at("method") == "POST" && request.at("path") == "/v1/decisions", "Decisions HTTP route");
    require_decision(request.at("headers").at("openai-project") == "decision-project", "Decisions per-call project");
    require_decision(!request.at("headers").contains("openai-beta"), "No undocumented beta header invented");
    require_decision(result.request_id() == "req_fixture", "Decisions request diagnostics");
    auto answers = parse_decision_answers(result.response.raw_json);
    require_decision(answers.size() == 5 && answers.at(0).name == "damaged", "Ordered named answers");
    require_decision(std::get<DecisionPredicate>(answers[0].value).probability == 0.75, "Predicate typed probability");
    const auto& choice = std::get<DecisionChoice>(answers[1].value);
    require_decision(std::get<bool>(choice.choice) && choice.confidence == 0.8, "Boolean choice preserved");
    require_decision(std::get<std::string>(choice.probabilities.at(1).value) == "true", "String distinct from bool");
    const auto& score = std::get<DecisionScore>(answers[2].value);
    require_decision(score.score == 1.25 && score.probabilities.at(2).label == "high" && score.probabilities.at(2).value == 2,
                     "Fractional weighted score and integer levels retained");
    require_decision(std::holds_alternative<DecisionRefusal>(answers[3].value) && !answers[3].name, "Unnamed refusal preserved");
    require_decision(std::holds_alternative<DecisionUnknown>(answers[4].value) && answers[4].raw.at("name").is_object(),
                     "Unknown answer type and future name shape retained");
    require_decision(answers[1].raw.at("probabilities").at(1).at("future") == true &&
                     result.response.raw_json.at("future_top_level").at("preserve") == true, "Unknown fields retained");

    auto string_selection = result.response.raw_json;
    string_selection["answers"][1]["choice"] = "true";
    require_decision(std::get<std::string>(std::get<DecisionChoice>(parse_decision_answers(string_selection)[1].value).choice) == "true",
                     "Selected string choice does not coerce to bool");
    const auto original_body = body;
    options.headers["OpenAI-Beta"] = "caller-explicit-test=v1";
    auto pending = Decisions(root).create_async(body, options); // Original client is destroyed immediately.
    body["input"] = "changed";
    options.project = "changed";
    options.headers["OpenAI-Beta"] = "changed";
    auto snapshot = pending.get().response.raw_json.at("fixture_request");
    require_decision(snapshot.at("json") == original_body && snapshot.at("headers").at("openai-project") == "decision-project",
                     "Async client and request snapshot");
    require_decision(snapshot.at("headers").at("openai-beta") == "caller-explicit-test=v1", "Explicit beta header snapshot");
    auto unrelated = Responses(root).create_with_options({}).response.raw_json;
    require_decision(!unrelated.at("headers").contains("openai-beta"), "Decisions caller headers never leak");
    for (const auto* kind : {"predicate", "choice", "score", "refusal"}) {
        const auto malformed = client.create({{"fixture_malformed", kind}}, options).response.raw_json;
        rejects_answer([&] { parse_decision_answers(malformed); });
    }
    for (const json malformed : std::vector<json>{nullptr, json::object(), {{"answers", "wrong"}}, {{"answers", json::array({nullptr})}},
        {{"answers", json::array({{{"type", "predicate"}, {"name", "x"}, {"probability", -0.1}}})}},
        {{"answers", json::array({{{"type", "predicate"}, {"name", "x"}, {"probability", std::numeric_limits<double>::infinity()}}})}}}) {
        rejects_answer([&] { parse_decision_answers(malformed); });
    }
    auto malformed_score = result.response.raw_json;
    malformed_score["answers"][2]["probabilities"][0]["value"] = std::numeric_limits<std::uint64_t>::max();
    rejects_answer([&] { parse_decision_answers(malformed_score); });
    require_decision(parse_decision_answers({{"answers", json::array()}}).empty(), "Empty answers preserved");
    try {
        client.create({{"fixture_error", true}}, options);
        throw std::runtime_error("Expected Decisions HTTP error");
    } catch (const exception::ApiError& error) {
        require_decision(error.result.response.status_code == 400 && error.error.at("code") == "fixture_decision" &&
                         error.result.request_id() == "req_fixture", "Decisions structured API error");
    }
}
