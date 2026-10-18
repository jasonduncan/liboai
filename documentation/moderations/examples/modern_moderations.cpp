#include "liboai.h"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "Usage: modern_moderations MODEL_ID\n"; return 1; }
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n"; return 1;
    }
    try {
    liboai::Moderations moderations("https://api.openai.com/v1");
    auto result = moderations.create_json({{"model", argv[1]}, {"input", nlohmann::json::array({{{"type", "text"}, {"text", "A friendly greeting"}}})}});
    std::cout << result.response << '\n';
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
