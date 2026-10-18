#include "liboai.h"
#include <stdexcept>

namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class Exception, class Function> void rejects(Function call) {
    try { call(); }
    catch (const Exception&) { return; }
    throw std::runtime_error("Expected exception was not raised");
}
}
void test_transport(const std::string& root) {
    liboai::Responses responses(root);
    liboai::RequestOptions options;
    options.project = "project-specific";
    options.headers = {{"OpenAI-Project", "lower-priority"}, {"OpenAI-Beta", "responses_multi_agent=v1"},
                       {"openai-organization", "request-org"}};
    options.timeout = std::chrono::seconds(2);
    require(liboai::Authorization::Authorizer().SetOrganization("default-org"), "set organization");
    auto reply = responses.create_with_options({{"input", "synthetic"}}, options);
    const auto& echo = reply.response.raw_json;
    require(echo["headers"]["openai-project"] == "project-specific", "Project precedence");
    require(echo["headers"]["openai-beta"] == "responses_multi_agent=v1", "Beta header");
    require(echo["headers"]["openai-organization"] == "request-org", "Case-insensitive override");
    require(reply.request_id() == "req_fixture", "Request ID metadata");
    require(reply.headers.at("CONTENT-TYPE") == "application/json", "Response header case");
    auto async = responses.create_with_options_async({{"input", "snapshot"}}, options);
    options.project = "mutated";
    options.headers["OpenAI-Beta"] = "mutated";
    require(async.get().response.raw_json["headers"]["openai-project"] == "project-specific", "Async option snapshot");
    const auto plain = responses.create_with_options({{"input", "plain"}}).response.raw_json;
    require(!plain["headers"].contains("openai-project") && !plain["headers"].contains("openai-beta"), "No request option leak");
    require(plain["headers"]["openai-organization"] == "default-org", "Global authorization unchanged");
    auto legacy = responses.create(nlohmann::json{{"input", "legacy"}});
    require(!legacy.raw_json["headers"].contains("openai-beta"), "Legacy header isolation");
    require(liboai::Authorization::Authorizer().SetAzureKey("synthetic-azure-key"), "Set Azure key");
    const auto azure = liboai::Authorization::Authorizer().GetAzureAuthorizationHeaders();
    require(azure.find("OpenAI-Beta") == azure.end(), "Azure header isolation");
    for (const auto& name : {"AUTHORIZATION", "api-key", "Proxy-Authorization", "Content-Type", "Host", "Content-Length", "Transfer-Encoding"}) {
        liboai::RequestOptions invalid;
        invalid.headers[name] = "override";
        rejects<std::invalid_argument>([&] { responses.create_with_options({}, invalid); });
    }
    for (auto header : liboai::Headers{{"bad:name", "value"}, {"bad name", "value"}, {"good", "value\r\nInjected: true"}}) {
        liboai::RequestOptions invalid;
        invalid.headers.insert(header);
        rejects<std::invalid_argument>([&] { responses.create_with_options({}, invalid); });
    }
    liboai::RequestOptions invalid;
    invalid.project = "bad\nproject";
    rejects<std::invalid_argument>([&] { responses.create_with_options({}, invalid); });
    invalid = {}; invalid.timeout = std::chrono::milliseconds(-1);
    rejects<std::invalid_argument>([&] { responses.create_with_options({}, invalid); });
    liboai::Responses binary(root + "/fixture/binary");
    const auto bytes = binary.create_with_options({});
    require(bytes.response.content == std::string("{\0\xff\1not-json", 12), "Binary content retained even with opening brace");
    require(bytes.response.raw_json.is_null(), "Binary not parsed");
    liboai::Responses json_array(root + "/fixture/json-array");
    require(json_array.create_with_options({}).response.raw_json.at(1).at("future") == true,
        "Whitespace-prefixed JSON arrays parsed");
    liboai::Responses empty(root + "/fixture/empty");
    require(empty.create_with_options({}).response.status_code == 204, "Empty success");
    liboai::Responses bad(root + "/fixture/error-json");
    try {
        bad.create_with_options({{"private", "must-not-be-in-error"}});
        throw std::runtime_error("Expected API error");
    } catch (const liboai::exception::ApiError& e) {
        require(e.result.response.status_code == 400 && e.result.request_id() == "req_fixture", "Error diagnostics");
        require(e.error["code"] == "fixture_error" && e.error["unknown"] == true, "Structured error preservation");
        require(std::string(e.what()).find("400") != std::string::npos, "Status in safe diagnostic");
        require(std::string(e.what()).find("must-not") == std::string::npos, "No request body disclosure");
    }
    liboai::Responses limited(root + "/fixture/rate-limit");
    try { limited.create_with_options({}); throw std::runtime_error("Expected rate limit"); }
    catch (const liboai::exception::ApiRateLimited& e) {
        require(e.result.headers.at("retry-after") == "2" && e.error["type"] == "rate_limit_error", "Rate limit diagnostics");
    }
    rejects<liboai::exception::OpenAIRateLimited>([&] { limited.create_with_options({}); });
    liboai::Responses gateway(root + "/fixture/error-text");
    try { gateway.create_with_options({}); throw std::runtime_error("Expected non-JSON API error"); }
    catch (const liboai::exception::ApiError& e) {
        require(e.result.response.status_code == 502 && e.error.is_null(), "Non-JSON error retained");
    }
    liboai::Responses malformed(root + "/fixture/invalid-json");
    rejects<liboai::exception::OpenAIException>([&] { malformed.create_with_options({}); });
    liboai::Responses slow(root + "/fixture/slow");
    liboai::RequestOptions short_timeout; short_timeout.timeout = std::chrono::milliseconds(20);
    rejects<liboai::exception::TransportError>([&] { slow.create_with_options({}, short_timeout); });
    liboai::Authorization::Authorizer().SetMaxTimeout(20);
    rejects<liboai::exception::TransportError>([&] { slow.create_with_options({}); });
    liboai::Authorization::Authorizer().SetMaxTimeout(30000);
    // Route the synthetic non-resolving host through the loopback HTTP proxy.
    liboai::Authorization::Authorizer().SetProxies(std::map<std::string, std::string>{{"http", root.substr(0, root.size() - 3)}});
    liboai::Responses proxied("http://fixture.invalid/v1");
    auto via_proxy = proxied.create_with_options({{"input", "proxy-test"}});
    liboai::Authorization::Authorizer().SetProxies(std::map<std::string, std::string>{});
    require(via_proxy.response.raw_json["headers"]["host"] == "fixture.invalid", "Configured proxy preserved");
    auto cancelled = responses.create_with_options({{"input", "fixture-stream"}, {"stream", true}}, {},
        [](std::string, intptr_t) { return false; });
    require(cancelled.locally_cancelled, "Local callback cancellation distinguished");
    rejects<std::logic_error>([&] {
        responses.create_with_options({{"input", "fixture-stream"}, {"stream", true}}, {},
            [](std::string, intptr_t) -> bool { throw std::logic_error("callback sentinel"); });
    });
    // Reusing the low-level session must not retain earlier request diagnostics.
    liboai::netimpl::Session session;
    liboai::netimpl::set_options(session, liboai::netimpl::components::Url{root + "/responses"});
    require(session.ExecuteDetailed("GET").request_id() == "req_fixture", "Reusable session first result");
    liboai::netimpl::set_options(session, liboai::netimpl::components::Url{root + "/fixture/slow"},
        liboai::netimpl::components::Timeout{20});
    try { session.ExecuteDetailed("GET"); throw std::runtime_error("Expected reused session timeout"); }
    catch (const liboai::exception::TransportError& e) {
        require(e.result.headers.empty() && e.result.response.status_line.empty() &&
                e.result.response.status_code == 0, "Stale metadata in failed reused session");
    }
    // HTTP errors go to diagnostics, never through a stream parser.
    bool invoked = false;
    rejects<liboai::exception::ApiError>([&] { bad.create_with_options({}, {}, [&](std::string, intptr_t) { invoked = true; return true; }); });
    require(!invoked, "HTTP error body must not be dispatched to stream callback");
}
