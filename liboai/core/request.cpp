#include "../include/core/request.h"
#include <climits>
#include <stdexcept>

namespace liboai {
namespace {
std::string lower(std::string value) {
    for (char& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return value;
}
void validate_header(const std::string& name, const std::string& value) {
    const std::string symbols = "!#$%&'*+-.^_`|~";
    if (name.empty()) throw std::invalid_argument("Header name must not be empty");
    for (unsigned char c : name) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || symbols.find(c) != std::string::npos))
            throw std::invalid_argument("Invalid HTTP header name");
    }
    for (unsigned char c : value)
        if ((c < 32 && c != '\t') || c == 127)
            throw std::invalid_argument("Invalid HTTP header value");
}
Headers make_headers(const RequestOptions& options, bool multipart) {
    auto headers = Authorization::Authorizer().GetAuthorizationHeaders();
    // Credentials and framing belong to the authorizer/transport, never extra headers.
    for (const auto& item : options.headers) {
        validate_header(item.first, item.second);
        const auto name = lower(item.first);
        if (name == "authorization" || name == "proxy-authorization" || name == "api-key" ||
            name == "content-type" || name == "content-length" || name == "transfer-encoding" ||
            name == "host" || name == "connection" || name == "trailer" || name == "upgrade")
            throw std::invalid_argument("RequestOptions cannot override credentials or HTTP framing");
        headers[item.first] = item.second;
    }
    if (options.project) {
        if (options.project->empty()) throw std::invalid_argument("Project must not be empty");
        validate_header("OpenAI-Project", *options.project);
        headers["OpenAI-Project"] = *options.project;
    }
    if (!multipart) headers["Content-Type"] = "application/json";
    for (const auto& item : headers) validate_header(item.first, item.second);
    return headers;
}
void configure(netimpl::Session& session, const std::string& root, const std::string& path,
               const Query& query, const RequestOptions& options, bool multipart,
               const std::optional<RawStreamCallback>& stream) {
    if ((root.rfind("http://", 0) != 0 && root.rfind("https://", 0) != 0) ||
        root.find_first_of("\r\n?#") != std::string::npos || path.empty() || path[0] != '/')
        throw std::invalid_argument("Expected an HTTP(S) API root and absolute endpoint path");
    auto timeout = Authorization::Authorizer().GetMaxTimeout();
    if (options.timeout) {
        if (options.timeout->count() < 0 || options.timeout->count() > LONG_MAX)
            throw std::invalid_argument("Timeout must fit a nonnegative millisecond count");
        timeout = netimpl::components::Timeout(*options.timeout);
    }
    std::string url = root;
    while (!url.empty() && url.back() == '/') url.pop_back();
    url += path;
    const auto encoded = detail::encode_query(query);
    if (!encoded.empty()) url += "?" + encoded;
    netimpl::set_options(session, netimpl::components::Url{url}, make_headers(options, multipart),
        Authorization::Authorizer().GetProxies(), Authorization::Authorizer().GetProxyAuth(), timeout);
    if (stream) {
        if (!*stream) throw std::invalid_argument("Stream callback must not be empty");
        netimpl::set_options(session, netimpl::components::WriteCallback{*stream});
    }
}
nlohmann::json error_fields(const ApiResult& result) {
    const auto& json = result.response.raw_json;
    return json.is_object() && json.contains("error") ? json["error"] : nlohmann::json();
}
std::string safe_status(const ApiResult& result) {
    return "HTTP " + std::to_string(result.response.status_code);
}
}

std::string ApiResult::request_id() const {
    const auto it = headers.find("x-request-id");
    return it == headers.end() ? "" : it->second;
}
exception::ApiError::ApiError(ApiResult value)
    : OpenAIException(safe_status(value), EType::E_APIERROR, "liboai API request"),
      result(std::move(value)), error(error_fields(result)) {}
exception::ApiRateLimited::ApiRateLimited(ApiResult value)
    : OpenAIRateLimited(safe_status(value), EType::E_RATELIMIT, "liboai API request"),
      result(std::move(value)), error(error_fields(result)) {}
exception::TransportError::TransportError(ApiResult value, int code)
    : OpenAIException(std::string("Transport failure: ") + curl_easy_strerror(static_cast<CURLcode>(code)),
        EType::E_CONNECTIONERROR, "liboai API request"), result(std::move(value)), curl_code(code) {}
exception::ResponseParseError::ResponseParseError(ApiResult value)
    : OpenAIException("Invalid JSON response", EType::E_FAILURETOPARSE, "liboai API request"),
      result(std::move(value)) {}

std::string detail::encode_path_segment(const std::string& value) {
    if (value.empty()) throw std::invalid_argument("Path identifier must not be empty");
    if (value == "." || value == "..") throw std::invalid_argument("Dot path identifier is not allowed");
    static const char hex[] = "0123456789ABCDEF";
    std::string encoded;
    for (unsigned char c : value) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
            encoded += static_cast<char>(c);
        else {
            encoded += '%'; encoded += hex[c >> 4]; encoded += hex[c & 15];
        }
    }
    return encoded;
}
std::string detail::encode_query(const Query& query) {
    std::string result;
    for (const auto& item : query) {
        if (!result.empty()) result += '&';
        // Empty query values are meaningful. Dot segments have no path meaning here.
        auto encode = [](const std::string& s) {
            if (s.empty() || s == "." || s == "..") return s;
            return encode_path_segment(s);
        };
        result += encode(item.first) + "=" + encode(item.second);
    }
    return result;
}
ApiResult detail::request(const std::string& root, const std::string& method, const std::string& path,
                         const std::optional<nlohmann::json>& body, const Query& query,
                         const RequestOptions& options, const std::optional<RawStreamCallback>& stream) {
    netimpl::Session session;
    configure(session, root, path, query, options, false, stream);
    if (body) netimpl::set_options(session, netimpl::components::Body{body->dump()});
    return session.ExecuteDetailed(method);
}
ApiResult detail::multipart_request(const std::string& root, const std::string& path,
                                   const netimpl::components::Multipart& parts, const RequestOptions& options,
                                   const std::optional<RawStreamCallback>& stream) {
    netimpl::Session session;
    configure(session, root, path, {}, options, true, stream);
    netimpl::set_options(session, parts);
    return session.ExecuteDetailed("POST");
}
std::shared_ptr<const netimpl::components::Multipart> detail::snapshot_multipart(
    const netimpl::components::Multipart& parts) {
    struct Snapshot {
        netimpl::components::Multipart parts;
        std::vector<std::vector<unsigned char>> buffers;
    };
    auto snapshot = std::make_shared<Snapshot>();
    snapshot->parts = parts;
    for (auto& part : snapshot->parts.parts) {
        if (!part.is_buffer) continue;
        if (part.datalen < 0 || (!part.data && part.datalen))
            throw std::invalid_argument("Invalid multipart buffer");
        snapshot->buffers.emplace_back();
        auto& buffer = snapshot->buffers.back();
        if (part.datalen) buffer.assign(part.data, part.data + part.datalen);
        part.data = buffer.data();
    }
    return std::shared_ptr<const netimpl::components::Multipart>(snapshot, &snapshot->parts);
}
}
