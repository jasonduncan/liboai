#include "../include/components/uploads.h"
#include <fstream>
#include <set>
#include <stdexcept>

namespace liboai {
namespace {
void validate_part_ids(const nlohmann::json& body) {
    if (!body.is_object() || !body.contains("part_ids") || !body["part_ids"].is_array() || body["part_ids"].empty())
        throw std::invalid_argument("Upload completion requires a nonempty part_ids array");
    std::set<std::string> seen;
    for (const auto& id : body["part_ids"]) {
        if (!id.is_string() || id.get_ref<const std::string&>().empty() || !seen.insert(id.get<std::string>()).second)
            throw std::invalid_argument("Upload part IDs must be nonempty unique strings");
    }
}
void validate_part_size(std::uint64_t bytes) {
    if (bytes == 0 || bytes > Uploads::max_part_bytes)
        throw std::invalid_argument("Upload part must contain 1 to 64 MiB of data");
}
}

ApiResult Uploads::create(nlohmann::json body, RequestOptions options) const {
    if (!body.is_object() || !body.contains("bytes") || !body["bytes"].is_number_integer() ||
        (body["bytes"].is_number_integer() && !body["bytes"].is_number_unsigned() && body["bytes"].get<std::int64_t>() <= 0))
        throw std::invalid_argument("Upload bytes must be a positive integer");
    const auto bytes = body["bytes"].get<std::uint64_t>();
    if (bytes == 0 || bytes > max_upload_bytes) throw std::invalid_argument("Upload exceeds the 8 GiB size limit");
    return detail::request(root_, "POST", "/uploads", body, {}, options);
}
FutureApiResult Uploads::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}
ApiResult Uploads::add_part(std::string upload_id, std::vector<unsigned char> data, RequestOptions options) const {
    validate_part_size(data.size());
    netimpl::components::Buffer buffer(data.begin(), data.end(), std::filesystem::path("part.bin"));
    netimpl::components::Multipart form{{"data", buffer, "application/octet-stream"}};
    return detail::multipart_request(root_, "/uploads/" + detail::encode_path_segment(upload_id) + "/parts", form, options);
}
FutureApiResult Uploads::add_part_async(std::string upload_id, std::vector<unsigned char> data, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, upload_id = std::move(upload_id), data = std::move(data), options = std::move(options)] {
        return self.add_part(upload_id, data, options);
    });
}
ApiResult Uploads::add_part_file(std::string upload_id, std::filesystem::path file, RequestOptions options) const {
    if (!std::filesystem::is_regular_file(file)) throw std::invalid_argument("Upload part must be a regular file");
    const auto bytes = std::filesystem::file_size(file);
    validate_part_size(bytes);
    std::ifstream input(file, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open upload part file");
    std::vector<unsigned char> data(static_cast<std::size_t>(bytes));
    if (!input.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(bytes)) || input.peek() != std::char_traits<char>::eof())
        throw std::runtime_error("Upload part file changed size or could not be read completely");
    return add_part(std::move(upload_id), std::move(data), std::move(options));
}
FutureApiResult Uploads::add_part_file_async(std::string upload_id, std::filesystem::path file, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, upload_id = std::move(upload_id), file = std::move(file), options = std::move(options)] {
        return self.add_part_file(upload_id, file, options);
    });
}
ApiResult Uploads::complete(std::string upload_id, nlohmann::json body, RequestOptions options) const {
    validate_part_ids(body);
    return detail::request(root_, "POST", "/uploads/" + detail::encode_path_segment(upload_id) + "/complete", body, {}, options);
}
FutureApiResult Uploads::complete_async(std::string upload_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, upload_id = std::move(upload_id), body = std::move(body), options = std::move(options)] {
        return self.complete(upload_id, body, options);
    });
}
ApiResult Uploads::complete_checked(std::string upload_id, std::vector<UploadedPart> ordered_parts, std::uint64_t expected_bytes, nlohmann::json extra, RequestOptions options) const {
    if (!extra.is_object() || extra.contains("part_ids")) throw std::invalid_argument("Extra upload fields must be an object without part_ids");
    if (expected_bytes == 0 || expected_bytes > max_upload_bytes) throw std::invalid_argument("Invalid upload total size");
    auto ids = nlohmann::json::array();
    std::uint64_t total = 0;
    for (const auto& part : ordered_parts) {
        validate_part_size(part.bytes);
        if (part.bytes > expected_bytes - total) throw std::invalid_argument("Upload parts exceed expected total size");
        total += part.bytes;
        ids.push_back(part.id);
    }
    if (total != expected_bytes) throw std::invalid_argument("Upload parts do not match expected total size");
    extra["part_ids"] = std::move(ids);
    return complete(std::move(upload_id), std::move(extra), std::move(options));
}
FutureApiResult Uploads::complete_checked_async(std::string upload_id, std::vector<UploadedPart> ordered_parts, std::uint64_t expected_bytes, nlohmann::json extra, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, upload_id = std::move(upload_id), ordered_parts = std::move(ordered_parts), expected_bytes, extra = std::move(extra), options = std::move(options)] {
        return self.complete_checked(upload_id, ordered_parts, expected_bytes, extra, options);
    });
}
ApiResult Uploads::cancel(std::string upload_id, RequestOptions options) const {
    return detail::request(root_, "POST", "/uploads/" + detail::encode_path_segment(upload_id) + "/cancel", std::nullopt, {}, options);
}
FutureApiResult Uploads::cancel_async(std::string upload_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, upload_id = std::move(upload_id), options = std::move(options)] {
        return self.cancel(upload_id, options);
    });
}
}
