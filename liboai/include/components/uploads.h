#pragma once
#include "../core/request.h"
#include <cstdint>

namespace liboai {
// Caller-retained receipt used by complete_checked; order is assembly order.
struct UploadedPart {
    std::string id;
    std::uint64_t bytes;
};

class Uploads final {
public:
    explicit Uploads(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    static constexpr std::uint64_t max_upload_bytes = 8ULL * 1024 * 1024 * 1024;
    static constexpr std::uint64_t max_part_bytes = 64ULL * 1024 * 1024;

    // JSON extensibility is preserved. bytes must be an integer in [1, 8 GiB].
    LIBOAI_EXPORT ApiResult create(nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body, RequestOptions options = {}) const;
    // Bytes are owned, including in async work. Empty/oversized parts are rejected.
    LIBOAI_EXPORT ApiResult add_part(std::string upload_id, std::vector<unsigned char> data, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult add_part_async(std::string upload_id, std::vector<unsigned char> data, RequestOptions options = {}) const;
    // Reads one file (at most 64 MiB) as one part. No implicit chunking or retries.
    LIBOAI_EXPORT ApiResult add_part_file(std::string upload_id, std::filesystem::path file, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult add_part_file_async(std::string upload_id, std::filesystem::path file, RequestOptions options = {}) const;
    // Raw JSON completion preserves part_ids order; the service validates total bytes.
    LIBOAI_EXPORT ApiResult complete(std::string upload_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult complete_async(std::string upload_id, nlohmann::json body, RequestOptions options = {}) const;
    // Checks unique IDs, per-part limits, and exact total before submitting completion.
    // extra may contain md5 or future fields, but cannot override part_ids.
    LIBOAI_EXPORT ApiResult complete_checked(std::string upload_id, std::vector<UploadedPart> ordered_parts, std::uint64_t expected_bytes, nlohmann::json extra = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult complete_checked_async(std::string upload_id, std::vector<UploadedPart> ordered_parts, std::uint64_t expected_bytes, nlohmann::json extra = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult cancel(std::string upload_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult cancel_async(std::string upload_id, RequestOptions options = {}) const;
private:
    std::string root_;
};
}
