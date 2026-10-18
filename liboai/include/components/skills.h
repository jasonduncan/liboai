#pragma once
#include "../core/request.h"

namespace liboai {
// Hosted, versioned Skills API bundles. This client does not discover or run local skills.
// Async requests own a client copy and snapshot multipart buffers before launch.
class Skills final {
public:
    explicit Skills(std::string root = "https://api.openai.com/v1") : root_(std::move(root)) {}
    LIBOAI_EXPORT ApiResult create(const netimpl::components::Multipart& parts, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_async(const netimpl::components::Multipart& parts, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult list(Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_async(Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult retrieve(std::string skill_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_async(std::string skill_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult update(std::string skill_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult update_async(std::string skill_id, nlohmann::json body, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult remove(std::string skill_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_async(std::string skill_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult content(std::string skill_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult content_async(std::string skill_id, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult create_version(std::string skill_id, const netimpl::components::Multipart& parts, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult create_version_async(std::string skill_id, const netimpl::components::Multipart& parts, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult list_versions(std::string skill_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult list_versions_async(std::string skill_id, Query query = {}, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult retrieve_version(std::string skill_id, std::string version, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult retrieve_version_async(std::string skill_id, std::string version, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult remove_version(std::string skill_id, std::string version, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult remove_version_async(std::string skill_id, std::string version, RequestOptions options = {}) const;
    LIBOAI_EXPORT ApiResult version_content(std::string skill_id, std::string version, RequestOptions options = {}) const;
    LIBOAI_EXPORT FutureApiResult version_content_async(std::string skill_id, std::string version, RequestOptions options = {}) const;
private:
    std::string root_;
};
}
