#include "common.h"
#include <fstream>

// Submit a small synthetic JSONL file, or resume a retained batch ID. A batch can
// take up to its completion window: polling is bounded to 30 requests per run.
int main(int argc, char** argv) {
    if (argc != 4 || std::string(argv[1]) != "--live" ||
        (std::string(argv[2]) != "--input" && std::string(argv[2]) != "--resume")) {
        std::cout << "Usage: batch_completion --live (--input SYNTHETIC_JSONL | --resume BATCH_ID)\n";
        return 0;
    }
    try {
        resource_auth();
        liboai::Files files(resource_root);
        liboai::Batch batches(resource_root);
        std::string id;
        if (std::string(argv[2]) == "--input") {
            const auto input = files.create(argv[3], "batch").raw_json.at("id").get<std::string>();
            std::cout << "Input file retained for processing: " << input << '\n';
            id = batches.create({{"input_file_id", input}, {"endpoint", "/v1/responses"}, {"completion_window", "24h"}}).response.raw_json.at("id").get<std::string>();
        } else { id = argv[3]; }
        std::cout << "Retain/resume this batch ID: " << id << '\n';
        for (int attempt = 0; attempt < 30; ++attempt) {
            const auto batch = batches.retrieve(id).response.raw_json;
            const auto status = batch.at("status").get<std::string>();
            if (status == "completed" || status == "failed" || status == "cancelled" || status == "expired") {
                std::cout << "Terminal status: " << status << '\n';
                for (const auto* field : {"output_file_id", "error_file_id"}) {
                    if (batch.contains(field) && batch[field].is_string()) {
                        const auto content = files.content(batch[field].get<std::string>());
                        const std::string destination = std::string(field) + ".jsonl";
                        std::ofstream out(destination, std::ios::binary);
                        out.write(content.response.content.data(), static_cast<std::streamsize>(content.response.content.size()));
                        if (!out) throw std::runtime_error("Cannot write " + destination);
                        std::cout << "Saved " << destination << '\n';
                    }
                }
                // Input/output/error files are deliberately retained for inspection.
                // Use files.remove(file_id) after your application's retention ends.
                return status == "completed" ? 0 : 1;
            }
            polling_pause();
        }
        std::cout << "Polling limit reached; resume with --resume " << id << " or explicitly call batches.cancel(id).\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
