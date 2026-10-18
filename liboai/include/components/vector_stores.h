#pragma once
#include "../core/request.h"

namespace liboai {
// Independent resource client. JSON and query fields pass through unchanged.
class VectorStores final {
public:
    explicit VectorStores(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    // Async methods own a copy of this client and all arguments.
    // POST /vector_stores
    LIBOAI_EXPORT ApiResult create(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;

    // GET /vector_stores/{vector_store_id}
    LIBOAI_EXPORT ApiResult retrieve(std::string vector_store_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_async(std::string vector_store_id, RequestOptions options = {}) const;

    // POST /vector_stores/{vector_store_id}
    LIBOAI_EXPORT ApiResult update(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult update_async(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;

    // DELETE /vector_stores/{vector_store_id}
    LIBOAI_EXPORT ApiResult remove(std::string vector_store_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_async(std::string vector_store_id, RequestOptions options = {}) const;

    // GET /vector_stores
    LIBOAI_EXPORT ApiResult list(Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_async(Query query = {}, RequestOptions options = {}) const;

    // POST /vector_stores/{vector_store_id}/search
    LIBOAI_EXPORT ApiResult search(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult search_async(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;

    // POST /vector_stores/{vector_store_id}/files
    LIBOAI_EXPORT ApiResult create_file(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_file_async(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;

    // GET /vector_stores/{vector_store_id}/files/{file_id}
    LIBOAI_EXPORT ApiResult retrieve_file(std::string vector_store_id, std::string file_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_file_async(std::string vector_store_id, std::string file_id, RequestOptions options = {}) const;

    // POST /vector_stores/{vector_store_id}/files/{file_id}
    LIBOAI_EXPORT ApiResult update_file(std::string vector_store_id, std::string file_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult update_file_async(std::string vector_store_id, std::string file_id, nlohmann::json body, RequestOptions options = {}) const;

    // DELETE /vector_stores/{vector_store_id}/files/{file_id}
    LIBOAI_EXPORT ApiResult remove_file(std::string vector_store_id, std::string file_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_file_async(std::string vector_store_id, std::string file_id, RequestOptions options = {}) const;

    // GET /vector_stores/{vector_store_id}/files
    LIBOAI_EXPORT ApiResult list_files(std::string vector_store_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_files_async(std::string vector_store_id, Query query = {}, RequestOptions options = {}) const;

    // GET /vector_stores/{vector_store_id}/files/{file_id}/content
    LIBOAI_EXPORT ApiResult file_content(std::string vector_store_id, std::string file_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult file_content_async(std::string vector_store_id, std::string file_id, Query query = {}, RequestOptions options = {}) const;

    // POST /vector_stores/{vector_store_id}/file_batches
    LIBOAI_EXPORT ApiResult create_file_batch(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_file_batch_async(std::string vector_store_id, nlohmann::json body, RequestOptions options = {}) const;

    // GET /vector_stores/{vector_store_id}/file_batches/{batch_id}
    LIBOAI_EXPORT ApiResult retrieve_file_batch(std::string vector_store_id, std::string batch_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_file_batch_async(std::string vector_store_id, std::string batch_id, RequestOptions options = {}) const;

    // POST /vector_stores/{vector_store_id}/file_batches/{batch_id}/cancel
    LIBOAI_EXPORT ApiResult cancel_file_batch(std::string vector_store_id, std::string batch_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult cancel_file_batch_async(std::string vector_store_id, std::string batch_id, RequestOptions options = {}) const;

    // GET /vector_stores/{vector_store_id}/file_batches/{batch_id}/files
    LIBOAI_EXPORT ApiResult list_file_batch_files(std::string vector_store_id, std::string batch_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_file_batch_files_async(std::string vector_store_id, std::string batch_id, Query query = {}, RequestOptions options = {}) const;

private:
    std::string root_;
};
}
