#include "liboai.h"
#include <iostream>
#include <stdexcept>

void test_responses(const std::string& root);
void test_components(const std::string& root);
void test_resources(const std::string& root);
void test_decisions(const std::string& root);
void test_skills(const std::string& root);
void test_sse();
void test_transport(const std::string& root);
void test_webhooks();

int main(int argc, char** argv) {
    try {
        if (argc < 2) throw std::runtime_error("Pass a loopback fixture root");
        auto& auth = liboai::Authorization::Authorizer();
        if (!auth.SetKey("synthetic-fixture-key")) throw std::runtime_error("Cannot set fixture key");
        test_transport(argv[1]);
        test_sse();
        test_responses(argv[1]);
        test_components(argv[1]);
        test_resources(argv[1]);
        test_webhooks();
        test_decisions(argv[1]);
        test_skills(argv[1]);
        std::cout << "All API modernization fixtures passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
