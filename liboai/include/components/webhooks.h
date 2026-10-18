#pragma once
#include "../core/request.h"

namespace liboai {
// Independent resource client. JSON and query fields pass through unchanged.
class Webhooks final {
public:
    explicit Webhooks(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    // Async methods own a copy of this client and all arguments.
    // POST /webhook_endpoints
    LIBOAI_EXPORT ApiResult create(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;

    // GET /webhook_endpoints/{webhook_endpoint_id}
    LIBOAI_EXPORT ApiResult retrieve(std::string webhook_endpoint_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_async(std::string webhook_endpoint_id, RequestOptions options = {}) const;

    // POST /webhook_endpoints/{webhook_endpoint_id}
    LIBOAI_EXPORT ApiResult update(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult update_async(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options = {}) const;

    // DELETE /webhook_endpoints/{webhook_endpoint_id}
    LIBOAI_EXPORT ApiResult remove(std::string webhook_endpoint_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_async(std::string webhook_endpoint_id, RequestOptions options = {}) const;

    // GET /webhook_endpoints
    LIBOAI_EXPORT ApiResult list(Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_async(Query query = {}, RequestOptions options = {}) const;

    // POST /webhook_endpoints/{webhook_endpoint_id}/rotate_secret
    LIBOAI_EXPORT ApiResult rotate_secret(std::string webhook_endpoint_id, nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult rotate_secret_async(std::string webhook_endpoint_id, nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;

    // POST /webhook_endpoints/{webhook_endpoint_id}/test
    LIBOAI_EXPORT ApiResult test(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult test_async(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options = {}) const;

private:
    std::string root_;
};
}
