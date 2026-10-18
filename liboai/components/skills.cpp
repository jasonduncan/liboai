#include "../include/components/skills.h"

namespace liboai {
namespace {
std::string skill_path(const std::string& id) { return "/skills/" + detail::encode_path_segment(id); }
std::string version_path(const std::string& id, const std::string& version) {
    return skill_path(id) + "/versions/" + detail::encode_path_segment(version);
}
}

ApiResult Skills::create(const netimpl::components::Multipart& parts, RequestOptions options) const {
    return detail::multipart_request(root_, "/skills", parts, options);
}
FutureApiResult Skills::create_async(const netimpl::components::Multipart& parts, RequestOptions options) const {
    auto owned = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [self = *this, owned = std::move(owned), options = std::move(options)] {
        return self.create(*owned, options);
    });
}

ApiResult Skills::list(Query query, RequestOptions options) const {
    return detail::request(root_, "GET", "/skills", std::nullopt, query, options);
}
FutureApiResult Skills::list_async(Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, query = std::move(query), options = std::move(options)] {
        return self.list(query, options);
    });
}

ApiResult Skills::retrieve(std::string skill_id, RequestOptions options) const {
    return detail::request(root_, "GET", skill_path(skill_id), std::nullopt, {}, options);
}
FutureApiResult Skills::retrieve_async(std::string skill_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), options = std::move(options)] {
        return self.retrieve(skill_id, options);
    });
}

ApiResult Skills::update(std::string skill_id, nlohmann::json body, RequestOptions options) const {
    return detail::request(root_, "POST", skill_path(skill_id), body, {}, options);
}
FutureApiResult Skills::update_async(std::string skill_id, nlohmann::json body, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), body = std::move(body), options = std::move(options)] {
        return self.update(skill_id, body, options);
    });
}

ApiResult Skills::remove(std::string skill_id, RequestOptions options) const {
    return detail::request(root_, "DELETE", skill_path(skill_id), std::nullopt, {}, options);
}
FutureApiResult Skills::remove_async(std::string skill_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), options = std::move(options)] {
        return self.remove(skill_id, options);
    });
}

ApiResult Skills::content(std::string skill_id, RequestOptions options) const {
    return detail::request(root_, "GET", skill_path(skill_id) + "/content", std::nullopt, {}, options);
}
FutureApiResult Skills::content_async(std::string skill_id, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), options = std::move(options)] {
        return self.content(skill_id, options);
    });
}

ApiResult Skills::create_version(std::string skill_id, const netimpl::components::Multipart& parts, RequestOptions options) const {
    return detail::multipart_request(root_, skill_path(skill_id) + "/versions", parts, options);
}
FutureApiResult Skills::create_version_async(std::string skill_id, const netimpl::components::Multipart& parts, RequestOptions options) const {
    auto owned = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [self = *this, owned = std::move(owned), skill_id = std::move(skill_id), options = std::move(options)] {
        return self.create_version(skill_id, *owned, options);
    });
}

ApiResult Skills::list_versions(std::string skill_id, Query query, RequestOptions options) const {
    return detail::request(root_, "GET", skill_path(skill_id) + "/versions", std::nullopt, query, options);
}
FutureApiResult Skills::list_versions_async(std::string skill_id, Query query, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), query = std::move(query), options = std::move(options)] {
        return self.list_versions(skill_id, query, options);
    });
}

ApiResult Skills::retrieve_version(std::string skill_id, std::string version, RequestOptions options) const {
    return detail::request(root_, "GET", version_path(skill_id, version), std::nullopt, {}, options);
}
FutureApiResult Skills::retrieve_version_async(std::string skill_id, std::string version, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), version = std::move(version), options = std::move(options)] {
        return self.retrieve_version(skill_id, version, options);
    });
}

ApiResult Skills::remove_version(std::string skill_id, std::string version, RequestOptions options) const {
    return detail::request(root_, "DELETE", version_path(skill_id, version), std::nullopt, {}, options);
}
FutureApiResult Skills::remove_version_async(std::string skill_id, std::string version, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), version = std::move(version), options = std::move(options)] {
        return self.remove_version(skill_id, version, options);
    });
}

ApiResult Skills::version_content(std::string skill_id, std::string version, RequestOptions options) const {
    return detail::request(root_, "GET", version_path(skill_id, version) + "/content", std::nullopt, {}, options);
}
FutureApiResult Skills::version_content_async(std::string skill_id, std::string version, RequestOptions options) const {
    return std::async(std::launch::async, [self = *this, skill_id = std::move(skill_id), version = std::move(version), options = std::move(options)] {
        return self.version_content(skill_id, version, options);
    });
}
}
