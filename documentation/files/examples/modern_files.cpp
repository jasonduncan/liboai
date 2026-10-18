#include "liboai.h"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n"; return 1;
    }
    try {
    liboai::Files files("https://api.openai.com/v1");
    liboai::Query query = {{"purpose", "user_data"}, {"limit", "10"}, {"order", "desc"}};
    // Bounded, explicit pagination. This example does not upload or delete files.
    for (int page = 0; page != 3; ++page) {
        const auto result = files.list_with_options(query);
        std::cout << result.response << '\n';
        const auto& data = result.response.raw_json;
        if (!data.value("has_more", false)) break;
        if (query.size() == 4) query.pop_back();
        query.emplace_back("after", data.at("last_id").get<std::string>());
    }
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
