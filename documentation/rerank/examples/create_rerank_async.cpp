#include "liboai.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: create_rerank_async <API-root> <model>\n";
        return 1;
    }
    // The client must outlive the future.
    liboai::OpenAI client(argv[1]);
    if (std::getenv("RERANK_API_KEY") && !client.auth.SetKeyEnv("RERANK_API_KEY")) {
        std::cerr << "Unable to read RERANK_API_KEY\n";
        return 1;
    }
    try {
        liboai::FutureResponse future;
        {
            std::string document = "A panda is a bear native to China.";
            std::vector<std::string_view> documents{document};
            future = client.Rerank->create_async(argv[2], "What is a panda?", documents);
        } // Input documents are safe to destroy after create_async returns.
        std::cout << future.get() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
