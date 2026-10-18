#pragma once
#include "../core/request.h"

namespace liboai {
// Independent resource client. JSON and query fields pass through unchanged.
class Batch final {
public:
    explicit Batch(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    // Async methods own a copy of this client and all arguments.
    // POST /batches
    LIBOAI_EXPORT ApiResult create(nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body, RequestOptions options = {}) const;

    // GET /batches/{batch_id}
    LIBOAI_EXPORT ApiResult retrieve(std::string batch_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_async(std::string batch_id, RequestOptions options = {}) const;

    // GET /batches
    LIBOAI_EXPORT ApiResult list(Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_async(Query query = {}, RequestOptions options = {}) const;

    // POST /batches/{batch_id}/cancel
    LIBOAI_EXPORT ApiResult cancel(std::string batch_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult cancel_async(std::string batch_id, RequestOptions options = {}) const;

private:
    std::string root_;
};
}
