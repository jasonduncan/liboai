#pragma once
#include "../core/request.h"

namespace liboai {
// Independent resource client. JSON and query fields pass through unchanged.
class Containers final {
public:
    explicit Containers(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    // Async methods own a copy of this client and all arguments.
    // POST /containers
    LIBOAI_EXPORT ApiResult create(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;

    // GET /containers/{container_id}
    LIBOAI_EXPORT ApiResult retrieve(std::string container_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_async(std::string container_id, RequestOptions options = {}) const;

    // DELETE /containers/{container_id}
    LIBOAI_EXPORT ApiResult remove(std::string container_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_async(std::string container_id, RequestOptions options = {}) const;

    // GET /containers
    LIBOAI_EXPORT ApiResult list(Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_async(Query query = {}, RequestOptions options = {}) const;

    // POST /containers/{container_id}/files
    LIBOAI_EXPORT ApiResult create_file(std::string container_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_file_async(std::string container_id, nlohmann::json body, RequestOptions options = {}) const;

    // GET /containers/{container_id}/files/{file_id}
    LIBOAI_EXPORT ApiResult retrieve_file(std::string container_id, std::string file_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_file_async(std::string container_id, std::string file_id, RequestOptions options = {}) const;

    // DELETE /containers/{container_id}/files/{file_id}
    LIBOAI_EXPORT ApiResult remove_file(std::string container_id, std::string file_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_file_async(std::string container_id, std::string file_id, RequestOptions options = {}) const;

    // GET /containers/{container_id}/files
    LIBOAI_EXPORT ApiResult list_files(std::string container_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_files_async(std::string container_id, Query query = {}, RequestOptions options = {}) const;

    // GET /containers/{container_id}/files/{file_id}/content
    LIBOAI_EXPORT ApiResult file_content(std::string container_id, std::string file_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult file_content_async(std::string container_id, std::string file_id, RequestOptions options = {}) const;

    // Multipart local-file upload. The file must remain unchanged until completion.
    LIBOAI_EXPORT ApiResult upload_file(std::string container_id, std::filesystem::path file, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult upload_file_async(std::string container_id, std::filesystem::path file, RequestOptions options = {}) const;

private:
    std::string root_;
};
}
