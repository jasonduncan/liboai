#include "liboai.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: create_rerank <API-root> <model>\n";
        return 1;
    }
    liboai::OpenAI client(argv[1]);
    if (std::getenv("RERANK_API_KEY") && !client.auth.SetKeyEnv("RERANK_API_KEY")) {
        std::cerr << "Unable to read RERANK_API_KEY\n";
        return 1;
    }
    try {
        std::vector<std::string_view> documents{
            "A panda is a bear native to China.", "The Moon orbits Earth."
        };
        auto response = client.Rerank->create(argv[2], "What is a panda?", documents, 1);
        std::cout << response << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
