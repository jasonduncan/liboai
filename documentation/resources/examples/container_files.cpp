#include "common.h"
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 3 || std::string(argv[1]) != "--live") {
        std::cout << "Usage: container_files --live SYNTHETIC_FILE\n";
        return 0;
    }
    liboai::Containers containers(resource_root);
    std::string container_id;
    try {
        resource_auth();
        container_id = containers.create({{"name", "liboai synthetic container"}}).response.raw_json.at("id").get<std::string>();
        const auto file = containers.upload_file(container_id, argv[2]).response.raw_json;
        const auto file_id = file.at("id").get<std::string>();
        auto content = containers.file_content(container_id, file_id);
        std::cout << "Retrieved " << content.response.content.size() << " bytes\n";
        // content is binary-safe; do not parse it as JSON or use C string length.
        containers.remove_file(container_id, file_id);
        containers.remove(container_id);
        container_id.clear();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        if (!container_id.empty()) {
            try { containers.remove(container_id); }
            catch (const std::exception& cleanup_error) { std::cerr << "Remove " << container_id << " manually: " << cleanup_error.what() << '\n'; }
        }
        return 1;
    }
}
