#include "../include/components/webhooks.h"

namespace liboai {
ApiResult Webhooks::create(nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/webhook_endpoints", body, {}, options);
}
FutureApiResult Webhooks::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}

ApiResult Webhooks::retrieve(std::string webhook_endpoint_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/webhook_endpoints/" + detail::encode_path_segment(webhook_endpoint_id), std::nullopt, {}, options);
}
FutureApiResult Webhooks::retrieve_async(std::string webhook_endpoint_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, webhook_endpoint_id = std::move(webhook_endpoint_id), options = std::move(options)] {
        return self.retrieve(webhook_endpoint_id, options);
    });
}

ApiResult Webhooks::update(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/webhook_endpoints/" + detail::encode_path_segment(webhook_endpoint_id), body, {}, options);
}
FutureApiResult Webhooks::update_async(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, webhook_endpoint_id = std::move(webhook_endpoint_id), body = std::move(body), options = std::move(options)] {
        return self.update(webhook_endpoint_id, body, options);
    });
}

ApiResult Webhooks::remove(std::string webhook_endpoint_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/webhook_endpoints/" + detail::encode_path_segment(webhook_endpoint_id), std::nullopt, {}, options);
}
FutureApiResult Webhooks::remove_async(std::string webhook_endpoint_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, webhook_endpoint_id = std::move(webhook_endpoint_id), options = std::move(options)] {
        return self.remove(webhook_endpoint_id, options);
    });
}

ApiResult Webhooks::list(Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/webhook_endpoints", std::nullopt, query, options);
}
FutureApiResult Webhooks::list_async(Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, query = std::move(query), options = std::move(options)] {
        return self.list(query, options);
    });
}

ApiResult Webhooks::rotate_secret(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/webhook_endpoints/" + detail::encode_path_segment(webhook_endpoint_id) + "/rotate_secret", body, {}, options);
}
FutureApiResult Webhooks::rotate_secret_async(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, webhook_endpoint_id = std::move(webhook_endpoint_id), body = std::move(body), options = std::move(options)] {
        return self.rotate_secret(webhook_endpoint_id, body, options);
    });
}

ApiResult Webhooks::test(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/webhook_endpoints/" + detail::encode_path_segment(webhook_endpoint_id) + "/test", body, {}, options);
}
FutureApiResult Webhooks::test_async(std::string webhook_endpoint_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, webhook_endpoint_id = std::move(webhook_endpoint_id), body = std::move(body), options = std::move(options)] {
        return self.test(webhook_endpoint_id, body, options);
    });
}

}
