#include "../include/components/decisions.h"
#include <cmath>
#include <limits>

namespace liboai {
namespace {
using json = nlohmann::json;
[[noreturn]] void malformed(const std::string& location, const char* expected) {
    throw exception::DecisionAnswerError(location + " must be " + expected);
}
const json& field(const json& object, const char* name, const std::string& location) {
    auto found = object.find(name);
    if (found == object.end()) throw exception::DecisionAnswerError(location + "." + name + " is missing");
    return *found;
}
std::string string_value(const json& value, const std::string& location) {
    if (!value.is_string()) malformed(location, "a string");
    return value.get<std::string>();
}
double number_value(const json& value, const std::string& location) {
    if (!value.is_number()) malformed(location, "a finite number");
    const auto number = value.get<double>();
    if (!std::isfinite(number)) malformed(location, "a finite number");
    return number;
}
double probability(const json& value, const std::string& location) {
    const auto number = number_value(value, location);
    if (number < 0 || number > 1) malformed(location, "a probability between zero and one");
    return number;
}
DecisionChoiceValue choice_value(const json& value, const std::string& location) {
    if (value.is_string()) return value.get<std::string>();
    if (value.is_boolean()) return value.get<bool>();
    malformed(location, "a string or boolean");
}
std::int64_t level_value(const json& value, const std::string& location) {
    if (!value.is_number_integer() ||
        (value.is_number_unsigned() && value.get<std::uint64_t>() > static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())))
        malformed(location, "an int64 integer");
    return value.get<std::int64_t>();
}
void require_array(const json& value, const std::string& location) {
    if (!value.is_array()) malformed(location, "an array");
}
}

std::vector<DecisionAnswer> parse_decision_answers(const nlohmann::json& decision) {
    if (!decision.is_object()) malformed("decision", "an object");
    const auto& answers = field(decision, "answers", "decision");
    require_array(answers, "decision.answers");
    std::vector<DecisionAnswer> result;
    result.reserve(answers.size());
    for (std::size_t index = 0; index < answers.size(); ++index) {
        const auto& raw = answers[index];
        const auto location = "decision.answers[" + std::to_string(index) + "]";
        if (!raw.is_object()) malformed(location, "an object");
        DecisionAnswer answer;
        answer.raw = raw;
        answer.type = string_value(field(raw, "type", location), location + ".type");
        const bool known = answer.type == "predicate" || answer.type == "choice" || answer.type == "score" || answer.type == "refusal";
        if (known) {
            const auto& name = field(raw, "name", location);
            if (!name.is_null()) answer.name = string_value(name, location + ".name");
        } else if (raw.contains("name") && raw["name"].is_string()) answer.name = raw["name"].get<std::string>();
        if (answer.type == "predicate") {
            answer.value = DecisionPredicate{probability(field(raw, "probability", location), location + ".probability")};
        } else if (answer.type == "choice") {
            DecisionChoice typed;
            typed.choice = choice_value(field(raw, "choice", location), location + ".choice");
            typed.confidence = number_value(field(raw, "confidence", location), location + ".confidence");
            const auto& values = field(raw, "probabilities", location);
            require_array(values, location + ".probabilities");
            for (std::size_t i = 0; i < values.size(); ++i) {
                const auto& item = values[i];
                const auto item_path = location + ".probabilities[" + std::to_string(i) + "]";
                if (!item.is_object()) malformed(item_path, "an object");
                typed.probabilities.push_back({choice_value(field(item, "value", item_path), item_path + ".value"),
                    probability(field(item, "probability", item_path), item_path + ".probability")});
            }
            answer.value = std::move(typed);
        } else if (answer.type == "score") {
            DecisionScore typed;
            typed.score = number_value(field(raw, "score", location), location + ".score");
            typed.confidence = number_value(field(raw, "confidence", location), location + ".confidence");
            const auto& values = field(raw, "probabilities", location);
            require_array(values, location + ".probabilities");
            for (std::size_t i = 0; i < values.size(); ++i) {
                const auto& item = values[i];
                const auto item_path = location + ".probabilities[" + std::to_string(i) + "]";
                if (!item.is_object()) malformed(item_path, "an object");
                typed.probabilities.push_back({string_value(field(item, "label", item_path), item_path + ".label"),
                    level_value(field(item, "value", item_path), item_path + ".value"),
                    probability(field(item, "probability", item_path), item_path + ".probability")});
            }
            answer.value = std::move(typed);
        } else if (answer.type == "refusal") answer.value = DecisionRefusal{};
        else answer.value = DecisionUnknown{};
        result.push_back(std::move(answer));
    }
    return result;
}

ApiResult Decisions::create(nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/decisions", body, {}, options);
}
FutureApiResult Decisions::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}
}
