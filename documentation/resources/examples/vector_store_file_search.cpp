#include "common.h"

// Use a small synthetic document. This example removes the store and source file.
int main(int argc, char** argv) {
    if (argc != 4 || std::string(argv[1]) != "--live") {
        std::cout << "Usage: vector_store_file_search --live MODEL SYNTHETIC_TEXT_FILE\n";
        return 0;
    }
    liboai::Files files(resource_root);
    liboai::VectorStores stores(resource_root);
    std::string file_id, store_id;
    auto cleanup = [&] {
        if (!store_id.empty()) { stores.remove(store_id); store_id.clear(); }
        if (!file_id.empty()) { files.remove(file_id); file_id.clear(); }
    };
    try {
        resource_auth();
        file_id = files.create(argv[3], "assistants").raw_json.at("id").get<std::string>();
        store_id = stores.create({{"name", "liboai synthetic file-search example"}}).response.raw_json.at("id").get<std::string>();
        stores.create_file(store_id, {{"file_id", file_id}});
        bool ready = false;
        for (int attempt = 0; attempt < 30; ++attempt) {
            const auto file = stores.retrieve_file(store_id, file_id).response.raw_json;
            const auto status = file.at("status").get<std::string>();
            if (status == "completed") { ready = true; break; }
            if (status != "in_progress") throw std::runtime_error("Ingestion ended with status: " + status);
            polling_pause();
        }
        if (!ready) throw std::runtime_error("Ingestion did not complete within 30 polls");
        liboai::Responses responses(resource_root);
        const nlohmann::json request{{"model", argv[2]}, {"input", "Summarize the uploaded synthetic document in one sentence."},
            {"max_output_tokens", 128}, {"store", false},
            {"tools", nlohmann::json::array({{{"type", "file_search"}, {"vector_store_ids", {store_id}}}})}};
        std::cout << responses.create(request) << '\n';
        cleanup();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        try { cleanup(); } catch (const std::exception& cleanup_error) {
            std::cerr << "Cleanup failed: " << cleanup_error.what() << "; retained store=" << store_id << " file=" << file_id << '\n';
        }
        return 1;
    }
}
