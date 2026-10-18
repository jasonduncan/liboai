#include "components/decisions.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: decisions MODEL_ID (for example, gpt-6-luna)\n";
        return 1;
    }
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n";
        return 1;
    }
    try {
        liboai::Decisions decisions;
        const auto result = decisions.create({{"model", argv[1]}, {"input", "The parcel arrived with a cracked screen."},
            {"questions", nlohmann::json::array({{{"type", "predicate"}, {"name", "damaged"},
                {"instructions", "Does the customer report physical damage?"}}})}});
        for (const auto& answer : liboai::parse_decision_answers(result.response.raw_json)) {
            if (const auto* predicate = std::get_if<liboai::DecisionPredicate>(&answer.value))
                std::cout << answer.name.value_or("unnamed") << ": " << predicate->probability << '\n';
            else if (std::holds_alternative<liboai::DecisionRefusal>(answer.value))
                std::cout << answer.name.value_or("unnamed") << ": refused\n";
            else std::cout << answer.name.value_or("unnamed") << ": " << answer.raw.dump() << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
