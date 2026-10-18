#include "../include/components/vector_stores.h"

namespace liboai {
ApiResult VectorStores::create(nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores", body, {}, options);
}
FutureApiResult VectorStores::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}

ApiResult VectorStores::retrieve(std::string vector_store_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores/" + detail::encode_path_segment(vector_store_id), std::nullopt, {}, options);
}
FutureApiResult VectorStores::retrieve_async(std::string vector_store_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), options = std::move(options)] {
        return self.retrieve(vector_store_id, options);
    });
}

ApiResult VectorStores::update(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores/" + detail::encode_path_segment(vector_store_id), body, {}, options);
}
FutureApiResult VectorStores::update_async(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), body = std::move(body), options = std::move(options)] {
        return self.update(vector_store_id, body, options);
    });
}

ApiResult VectorStores::remove(std::string vector_store_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/vector_stores/" + detail::encode_path_segment(vector_store_id), std::nullopt, {}, options);
}
FutureApiResult VectorStores::remove_async(std::string vector_store_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), options = std::move(options)] {
        return self.remove(vector_store_id, options);
    });
}

ApiResult VectorStores::list(Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores", std::nullopt, query, options);
}
FutureApiResult VectorStores::list_async(Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, query = std::move(query), options = std::move(options)] {
        return self.list(query, options);
    });
}

ApiResult VectorStores::search(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/search", body, {}, options);
}
FutureApiResult VectorStores::search_async(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), body = std::move(body), options = std::move(options)] {
        return self.search(vector_store_id, body, options);
    });
}

ApiResult VectorStores::create_file(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/files", body, {}, options);
}
FutureApiResult VectorStores::create_file_async(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), body = std::move(body), options = std::move(options)] {
        return self.create_file(vector_store_id, body, options);
    });
}

ApiResult VectorStores::retrieve_file(std::string vector_store_id, std::string file_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/files/" + detail::encode_path_segment(file_id), std::nullopt, {}, options);
}
FutureApiResult VectorStores::retrieve_file_async(std::string vector_store_id, std::string file_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), file_id = std::move(file_id), options = std::move(options)] {
        return self.retrieve_file(vector_store_id, file_id, options);
    });
}

ApiResult VectorStores::update_file(std::string vector_store_id, std::string file_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/files/" + detail::encode_path_segment(file_id), body, {}, options);
}
FutureApiResult VectorStores::update_file_async(std::string vector_store_id, std::string file_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), file_id = std::move(file_id), body = std::move(body), options = std::move(options)] {
        return self.update_file(vector_store_id, file_id, body, options);
    });
}

ApiResult VectorStores::remove_file(std::string vector_store_id, std::string file_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/files/" + detail::encode_path_segment(file_id), std::nullopt, {}, options);
}
FutureApiResult VectorStores::remove_file_async(std::string vector_store_id, std::string file_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), file_id = std::move(file_id), options = std::move(options)] {
        return self.remove_file(vector_store_id, file_id, options);
    });
}

ApiResult VectorStores::list_files(std::string vector_store_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/files", std::nullopt, query, options);
}
FutureApiResult VectorStores::list_files_async(std::string vector_store_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), query = std::move(query), options = std::move(options)] {
        return self.list_files(vector_store_id, query, options);
    });
}

ApiResult VectorStores::file_content(std::string vector_store_id, std::string file_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/files/" + detail::encode_path_segment(file_id) + "/content", std::nullopt, query, options);
}
FutureApiResult VectorStores::file_content_async(std::string vector_store_id, std::string file_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), file_id = std::move(file_id), query = std::move(query), options = std::move(options)] {
        return self.file_content(vector_store_id, file_id, query, options);
    });
}

ApiResult VectorStores::create_file_batch(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/file_batches", body, {}, options);
}
FutureApiResult VectorStores::create_file_batch_async(std::string vector_store_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), body = std::move(body), options = std::move(options)] {
        return self.create_file_batch(vector_store_id, body, options);
    });
}

ApiResult VectorStores::retrieve_file_batch(std::string vector_store_id, std::string batch_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/file_batches/" + detail::encode_path_segment(batch_id), std::nullopt, {}, options);
}
FutureApiResult VectorStores::retrieve_file_batch_async(std::string vector_store_id, std::string batch_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), batch_id = std::move(batch_id), options = std::move(options)] {
        return self.retrieve_file_batch(vector_store_id, batch_id, options);
    });
}

ApiResult VectorStores::cancel_file_batch(std::string vector_store_id, std::string batch_id, RequestOptions options) const {
    return detail::request(root_, "POST", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/file_batches/" + detail::encode_path_segment(batch_id) + "/cancel", std::nullopt, {}, options);
}
FutureApiResult VectorStores::cancel_file_batch_async(std::string vector_store_id, std::string batch_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), batch_id = std::move(batch_id), options = std::move(options)] {
        return self.cancel_file_batch(vector_store_id, batch_id, options);
    });
}

ApiResult VectorStores::list_file_batch_files(std::string vector_store_id, std::string batch_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/vector_stores/" + detail::encode_path_segment(vector_store_id) + "/file_batches/" + detail::encode_path_segment(batch_id) + "/files", std::nullopt, query, options);
}
FutureApiResult VectorStores::list_file_batch_files_async(std::string vector_store_id, std::string batch_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, vector_store_id = std::move(vector_store_id), batch_id = std::move(batch_id), query = std::move(query), options = std::move(options)] {
        return self.list_file_batch_files(vector_store_id, batch_id, query, options);
    });
}

}
