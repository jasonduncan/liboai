#include "liboai.h"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "Usage: modern_images MODEL_ID\n"; return 1; }
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n"; return 1;
    }
    try {
    liboai::Images images("https://api.openai.com/v1");
    auto result = images.create_json({{"model", argv[1]}, {"prompt", "A small blue ceramic cup"}, {"n", 1}, {"quality", "low"}, {"output_format", "png"}});
    std::cout << result.response << '\n';
    // For edits: Multipart parts = {{"model", argv[1]}, {"prompt", "Combine"},
    //   {"image[]", File{"first.png"}}, {"image[]", File{"second.png"}}};
    // images.create_edit_multipart(parts);
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
