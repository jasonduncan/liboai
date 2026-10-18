#pragma once
#include "../core/request.h"
#include <cstdint>
#include <variant>

namespace liboai {
// Choice strings and booleans remain distinct ("true" is not true).
using DecisionChoiceValue = std::variant<std::string, bool>;
struct DecisionPredicate { double probability = 0; };
struct DecisionChoiceProbability { DecisionChoiceValue value; double probability = 0; };
struct DecisionChoice {
    DecisionChoiceValue choice;
    double confidence = 0;
    std::vector<DecisionChoiceProbability> probabilities;
};
struct DecisionScoreProbability {
    std::string label;
    std::int64_t value = 0;
    double probability = 0;
};
struct DecisionScore {
    double score = 0;
    double confidence = 0;
    std::vector<DecisionScoreProbability> probabilities;
};
struct DecisionRefusal {};
struct DecisionUnknown {};
using DecisionAnswerValue = std::variant<DecisionPredicate, DecisionChoice, DecisionScore, DecisionRefusal, DecisionUnknown>;
struct DecisionAnswer {
    std::string type;
    std::optional<std::string> name;
    DecisionAnswerValue value;
    // Complete original answer, including fields absent from the typed view.
    nlohmann::json raw;
};
namespace exception {
class DecisionAnswerError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
}

// Parse a Decision object's ordered answers. Known malformed shapes throw;
// unrecognized answer types are preserved as DecisionUnknown and raw JSON.
LIBOAI_EXPORT std::vector<DecisionAnswer> parse_decision_answers(const nlohmann::json& decision);

class Decisions final {
public:
    explicit Decisions(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    // POST /decisions. Raw JSON is not restricted to a model/field allowlist.
    LIBOAI_EXPORT ApiResult create(nlohmann::json body, RequestOptions options = {}) const;
    // Owns a client and argument snapshot; the original client may be destroyed.
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body, RequestOptions options = {}) const;
private:
    std::string root_;
};
}
