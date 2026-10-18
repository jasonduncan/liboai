#pragma once

#include "authorization.h"
#include <chrono>
#include <vector>

namespace liboai {
using Headers = netimpl::components::Header;
using Query = std::vector<std::pair<std::string, std::string>>;
using RawStreamCallback = std::function<bool(std::string, intptr_t)>;

// Per-call overrides. Async methods copy this value before launching work.
struct RequestOptions {
    std::optional<std::string> project;
    Headers headers;
    std::optional<std::chrono::milliseconds> timeout;
};

// Additive diagnostics without changing Response or existing exception layouts.
struct ApiResult {
    Response response;
    Headers headers;
    bool locally_cancelled = false;
    LIBOAI_EXPORT std::string request_id() const;
};
using FutureApiResult = std::future<ApiResult>;

namespace exception {
class ApiError : public OpenAIException {
public:
    LIBOAI_EXPORT explicit ApiError(ApiResult result);
    ApiResult result;
    nlohmann::json error;
};
class ApiRateLimited : public OpenAIRateLimited {
public:
    LIBOAI_EXPORT explicit ApiRateLimited(ApiResult result);
    ApiResult result;
    nlohmann::json error;
};
class TransportError : public OpenAIException {
public:
    LIBOAI_EXPORT TransportError(ApiResult result, int curl_code);
    ApiResult result;
    int curl_code;
};
class ResponseParseError : public OpenAIException {
public:
    LIBOAI_EXPORT explicit ResponseParseError(ApiResult result);
    ApiResult result;
};
}

namespace detail {
LIBOAI_EXPORT std::string encode_path_segment(const std::string& value);
LIBOAI_EXPORT std::string encode_query(const Query& query);
// Deep copy buffer parts for async work; referenced files must remain unchanged.
LIBOAI_EXPORT std::shared_ptr<const netimpl::components::Multipart> snapshot_multipart(
    const netimpl::components::Multipart& parts);
LIBOAI_EXPORT ApiResult request(const std::string& root, const std::string& method,
    const std::string& path, const std::optional<nlohmann::json>& body = std::nullopt,
    const Query& query = {}, const RequestOptions& options = {},
    const std::optional<RawStreamCallback>& stream = std::nullopt);
LIBOAI_EXPORT ApiResult multipart_request(const std::string& root, const std::string& path,
    const netimpl::components::Multipart& parts, const RequestOptions& options = {},
    const std::optional<RawStreamCallback>& stream = std::nullopt);
}
}
