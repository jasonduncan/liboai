#include "../include/components/conversations.h"

namespace liboai {
ApiResult Conversations::create(nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/conversations", body, {}, options);
}
FutureApiResult Conversations::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}

ApiResult Conversations::retrieve(std::string conversation_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/conversations/" + detail::encode_path_segment(conversation_id), std::nullopt, {}, options);
}
FutureApiResult Conversations::retrieve_async(std::string conversation_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), options = std::move(options)] {
        return self.retrieve(conversation_id, options);
    });
}

ApiResult Conversations::update(std::string conversation_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/conversations/" + detail::encode_path_segment(conversation_id), body, {}, options);
}
FutureApiResult Conversations::update_async(std::string conversation_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), body = std::move(body), options = std::move(options)] {
        return self.update(conversation_id, body, options);
    });
}

ApiResult Conversations::remove(std::string conversation_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/conversations/" + detail::encode_path_segment(conversation_id), std::nullopt, {}, options);
}
FutureApiResult Conversations::remove_async(std::string conversation_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), options = std::move(options)] {
        return self.remove(conversation_id, options);
    });
}

ApiResult Conversations::create_items(std::string conversation_id, nlohmann::json body, Query query, RequestOptions options) const {
    return detail::request(root_, "POST", "/conversations/" + detail::encode_path_segment(conversation_id) + "/items", body, query, options);
}
FutureApiResult Conversations::create_items_async(std::string conversation_id, nlohmann::json body, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), body = std::move(body), query = std::move(query), options = std::move(options)] {
        return self.create_items(conversation_id, body, query, options);
    });
}

ApiResult Conversations::retrieve_item(std::string conversation_id, std::string item_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/conversations/" + detail::encode_path_segment(conversation_id) + "/items/" + detail::encode_path_segment(item_id), std::nullopt, query, options);
}
FutureApiResult Conversations::retrieve_item_async(std::string conversation_id, std::string item_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), item_id = std::move(item_id), query = std::move(query), options = std::move(options)] {
        return self.retrieve_item(conversation_id, item_id, query, options);
    });
}

ApiResult Conversations::remove_item(std::string conversation_id, std::string item_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/conversations/" + detail::encode_path_segment(conversation_id) + "/items/" + detail::encode_path_segment(item_id), std::nullopt, {}, options);
}
FutureApiResult Conversations::remove_item_async(std::string conversation_id, std::string item_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), item_id = std::move(item_id), options = std::move(options)] {
        return self.remove_item(conversation_id, item_id, options);
    });
}

ApiResult Conversations::list_items(std::string conversation_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/conversations/" + detail::encode_path_segment(conversation_id) + "/items", std::nullopt, query, options);
}
FutureApiResult Conversations::list_items_async(std::string conversation_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, conversation_id = std::move(conversation_id), query = std::move(query), options = std::move(options)] {
        return self.list_items(conversation_id, query, options);
    });
}

}
