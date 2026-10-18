#include "common.h"
#include <fstream>

// Demonstrates sequential bounded chunks and exact total validation. For parallel
// uploads, keep receipts in source order, independent of request completion order.
int main(int argc, char** argv) {
    if (argc != 3 || std::string(argv[1]) != "--live") {
        std::cout << "Usage: upload_parts --live SYNTHETIC_JSONL_FILE\n";
        return 0;
    }
    liboai::Uploads uploads(resource_root);
    std::string upload_id;
    try {
        resource_auth();
        const std::filesystem::path path(argv[2]);
        const auto total = std::filesystem::file_size(path);
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Cannot open input");
        upload_id = uploads.create({{"bytes", total}, {"filename", path.filename().string()},
            {"mime_type", "application/jsonl"}, {"purpose", "batch"}}).response.raw_json.at("id").get<std::string>();
        std::vector<liboai::UploadedPart> receipts;
        std::uint64_t consumed = 0;
        while (consumed < total) {
            const auto count = std::min<std::uint64_t>(1024 * 1024, total - consumed);
            std::vector<unsigned char> bytes(static_cast<std::size_t>(count));
            if (!input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(count)))
                throw std::runtime_error("File became shorter during upload");
            const auto part = uploads.add_part(upload_id, std::move(bytes));
            receipts.push_back({part.response.raw_json.at("id").get<std::string>(), count});
            consumed += count;
        }
        if (input.peek() != std::char_traits<char>::eof()) throw std::runtime_error("File grew during upload");
        const auto complete = uploads.complete_checked(upload_id, receipts, total);
        upload_id.clear();
        const auto file_id = complete.response.raw_json.at("file").at("id").get<std::string>();
        std::cout << "Completed file: " << file_id << '\n';
        liboai::Files files(resource_root);
        files.remove(file_id);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        if (!upload_id.empty()) {
            try { uploads.cancel(upload_id); }
            catch (const std::exception& cleanup_error) { std::cerr << "Cancel " << upload_id << " manually: " << cleanup_error.what() << '\n'; }
        }
        return 1;
    }
}
