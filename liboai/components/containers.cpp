#include "../include/components/containers.h"

namespace liboai {
ApiResult Containers::create(nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/containers", body, {}, options);
}
FutureApiResult Containers::create_async(nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, body = std::move(body), options = std::move(options)] {
        return self.create(body, options);
    });
}

ApiResult Containers::retrieve(std::string container_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/containers/" + detail::encode_path_segment(container_id), std::nullopt, {}, options);
}
FutureApiResult Containers::retrieve_async(std::string container_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), options = std::move(options)] {
        return self.retrieve(container_id, options);
    });
}

ApiResult Containers::remove(std::string container_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/containers/" + detail::encode_path_segment(container_id), std::nullopt, {}, options);
}
FutureApiResult Containers::remove_async(std::string container_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), options = std::move(options)] {
        return self.remove(container_id, options);
    });
}

ApiResult Containers::list(Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/containers", std::nullopt, query, options);
}
FutureApiResult Containers::list_async(Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, query = std::move(query), options = std::move(options)] {
        return self.list(query, options);
    });
}

ApiResult Containers::create_file(std::string container_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", "/containers/" + detail::encode_path_segment(container_id) + "/files", body, {}, options);
}
FutureApiResult Containers::create_file_async(std::string container_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), body = std::move(body), options = std::move(options)] {
        return self.create_file(container_id, body, options);
    });
}

ApiResult Containers::retrieve_file(std::string container_id, std::string file_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/containers/" + detail::encode_path_segment(container_id) + "/files/" + detail::encode_path_segment(file_id), std::nullopt, {}, options);
}
FutureApiResult Containers::retrieve_file_async(std::string container_id, std::string file_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), file_id = std::move(file_id), options = std::move(options)] {
        return self.retrieve_file(container_id, file_id, options);
    });
}

ApiResult Containers::remove_file(std::string container_id, std::string file_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", "/containers/" + detail::encode_path_segment(container_id) + "/files/" + detail::encode_path_segment(file_id), std::nullopt, {}, options);
}
FutureApiResult Containers::remove_file_async(std::string container_id, std::string file_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), file_id = std::move(file_id), options = std::move(options)] {
        return self.remove_file(container_id, file_id, options);
    });
}

ApiResult Containers::list_files(std::string container_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/containers/" + detail::encode_path_segment(container_id) + "/files", std::nullopt, query, options);
}
FutureApiResult Containers::list_files_async(std::string container_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), query = std::move(query), options = std::move(options)] {
        return self.list_files(container_id, query, options);
    });
}

ApiResult Containers::file_content(std::string container_id, std::string file_id, RequestOptions options) const {
    return detail::request(root_, "GET", "/containers/" + detail::encode_path_segment(container_id) + "/files/" + detail::encode_path_segment(file_id) + "/content", std::nullopt, {}, options);
}
FutureApiResult Containers::file_content_async(std::string container_id, std::string file_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), file_id = std::move(file_id), options = std::move(options)] {
        return self.file_content(container_id, file_id, options);
    });
}

ApiResult Containers::upload_file(std::string container_id, std::filesystem::path file, RequestOptions options) const {
    if (!std::filesystem::is_regular_file(file)) throw std::invalid_argument("Container file must be a regular file");
    netimpl::components::Multipart parts{{"file", netimpl::components::File{file.string()}}};
    return detail::multipart_request(root_, "/containers/" + detail::encode_path_segment(container_id) + "/files", parts, options);
}
FutureApiResult Containers::upload_file_async(std::string container_id, std::filesystem::path file, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, container_id = std::move(container_id), file = std::move(file), options = std::move(options)] {
        return self.upload_file(container_id, file, options);
    });
}

}
