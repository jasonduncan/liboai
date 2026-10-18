#include "../include/components/embeddings.h"

liboai::Response liboai::Embeddings::create(const std::string& model_id, std::optional<std::string> input, std::optional<std::string> user) const & noexcept(false) {
	liboai::JsonConstructor jcon;
	jcon.push_back("model", model_id);
	jcon.push_back("input", std::move(input));
	jcon.push_back("user", std::move(user));

	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/embeddings", "application/json",
		this->auth_.GetAuthorizationHeaders(),
		netimpl::components::Body {
			jcon.dump()
		},
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Embeddings::create_async(const std::string& model_id, std::optional<std::string> input, std::optional<std::string> user) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Embeddings::create, this, model_id, input, user);
}

liboai::ApiResult liboai::Embeddings::create_json(const nlohmann::json& body, const RequestOptions& options) const & {
    return detail::request(this->openai_root_, "POST", "/embeddings", body, {}, options);
}

std::future<liboai::ApiResult> liboai::Embeddings::create_json_async(const nlohmann::json& body, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, body, options]() mutable { return this->create_json(body, options); });
}
