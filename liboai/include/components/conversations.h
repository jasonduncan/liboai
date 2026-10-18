#pragma once
#include "../core/request.h"

namespace liboai {
// Independent resource client. JSON and query fields pass through unchanged.
class Conversations final {
public:
    explicit Conversations(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    // Async methods own a copy of this client and all arguments.
    // POST /conversations
    LIBOAI_EXPORT ApiResult create(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(nlohmann::json body = nlohmann::json::object(), RequestOptions options = {}) const;

    // GET /conversations/{conversation_id}
    LIBOAI_EXPORT ApiResult retrieve(std::string conversation_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_async(std::string conversation_id, RequestOptions options = {}) const;

    // POST /conversations/{conversation_id}
    LIBOAI_EXPORT ApiResult update(std::string conversation_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult update_async(std::string conversation_id, nlohmann::json body, RequestOptions options = {}) const;

    // DELETE /conversations/{conversation_id}
    LIBOAI_EXPORT ApiResult remove(std::string conversation_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_async(std::string conversation_id, RequestOptions options = {}) const;

    // POST /conversations/{conversation_id}/items
    LIBOAI_EXPORT ApiResult create_items(std::string conversation_id, nlohmann::json body, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_items_async(std::string conversation_id, nlohmann::json body, Query query = {}, RequestOptions options = {}) const;

    // GET /conversations/{conversation_id}/items/{item_id}
    LIBOAI_EXPORT ApiResult retrieve_item(std::string conversation_id, std::string item_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_item_async(std::string conversation_id, std::string item_id, Query query = {}, RequestOptions options = {}) const;

    // DELETE /conversations/{conversation_id}/items/{item_id}
    LIBOAI_EXPORT ApiResult remove_item(std::string conversation_id, std::string item_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_item_async(std::string conversation_id, std::string item_id, RequestOptions options = {}) const;

    // GET /conversations/{conversation_id}/items
    LIBOAI_EXPORT ApiResult list_items(std::string conversation_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_items_async(std::string conversation_id, Query query = {}, RequestOptions options = {}) const;

private:
    std::string root_;
};
}
