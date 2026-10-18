#include "../include/components/batch.h"

namespace liboai {
ApiResult Batch::create(nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/batches", body, {}, options);
}
FutureApiResult Batch::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}

ApiResult Batch::retrieve(std::string batch_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/batches/" + detail::encode_path_segment(batch_id), std::nullopt, {}, options);
}
FutureApiResult Batch::retrieve_async(std::string batch_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, batch_id = std::move(batch_id), options = std::move(options)] {
        return self.retrieve(batch_id, options);
    });
}

ApiResult Batch::list(Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/batches", std::nullopt, query, options);
}
FutureApiResult Batch::list_async(Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, query = std::move(query), options = std::move(options)] {
        return self.list(query, options);
    });
}

ApiResult Batch::cancel(std::string batch_id, RequestOptions options) const {
    return detail::request(root_, "POST", "/batches/" + detail::encode_path_segment(batch_id) + "/cancel", std::nullopt, {}, options);
}
FutureApiResult Batch::cancel_async(std::string batch_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, batch_id = std::move(batch_id), options = std::move(options)] {
        return self.cancel(batch_id, options);
    });
}

}
