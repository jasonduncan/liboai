#include "liboai.h"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "Usage: modern_chat MODEL_ID\n"; return 1; }
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n"; return 1;
    }
    try {
    liboai::ChatCompletion chat("https://api.openai.com/v1");
    nlohmann::json messages = nlohmann::json::array({{{"role", "user"}, {"content", "Reply with one short greeting."}}});
    auto result = chat.create_json({{"model", argv[1]}, {"messages", messages}, {"max_completion_tokens", 64}});
    // Retain the entire assistant message when continuing, including tool_calls.
    messages.push_back(result.response.raw_json.at("choices").at(0).at("message"));
    std::cout << result.response << '\n';
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
