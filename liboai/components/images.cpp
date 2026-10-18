#include "../include/components/images.h"

liboai::Response liboai::Images::create(const std::string& prompt, std::optional<uint8_t> n, std::optional<std::string> size, std::optional<std::string> response_format, std::optional<std::string> user) const & noexcept(false) {
	liboai::JsonConstructor jcon;
	jcon.push_back("prompt", prompt);
	jcon.push_back("n", std::move(n));
	jcon.push_back("size", std::move(size));
	jcon.push_back("response_format", std::move(response_format));
	jcon.push_back("user", std::move(user));

	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/images/generations", "application/json",
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

liboai::FutureResponse liboai::Images::create_async(const std::string& prompt, std::optional<uint8_t> n, std::optional<std::string> size, std::optional<std::string> response_format, std::optional<std::string> user) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Images::create, this, prompt, n, size, response_format, user);
}

liboai::Response liboai::Images::create_edit(const std::filesystem::path& image, const std::string& prompt, std::optional<std::filesystem::path> mask, std::optional<uint8_t> n, std::optional<std::string> size, std::optional<std::string> response_format, std::optional<std::string> user) const & noexcept(false) {
	if (!this->Validate(image)) {
		throw liboai::exception::OpenAIException(
			"File path provided is non-existent, is not a file, or is empty.",
			liboai::exception::EType::E_FILEERROR,
			"liboai::Images::create_edit(...)"
		);
	}

	netimpl::components::Multipart form = {
		{ "prompt", prompt },
		{ "image", netimpl::components::File{image.generic_string()} }
	};
	
	if (mask) {
		if (!this->Validate(mask.value())) {
			throw liboai::exception::OpenAIException(
				"File path provided is non-existent, is not a file, or is empty.",
				liboai::exception::EType::E_FILEERROR,
				"liboai::Images::create_edit(...)"
			);
		}
		form.parts.push_back({ "mask", netimpl::components::File{mask.value().generic_string()} });
	}
	if (n) { form.parts.push_back({ "n", n.value() }); }
	if (size) { form.parts.push_back({ "size", size.value() }); }
	if (response_format) { form.parts.push_back({ "response_format", response_format.value() }); }
	if (user) { form.parts.push_back({ "user", user.value() }); }
	
	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/images/edits", "multipart/form-data",
		this->auth_.GetAuthorizationHeaders(),
		std::move(form),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Images::create_edit_async(const std::filesystem::path& image, const std::string& prompt, std::optional<std::filesystem::path> mask, std::optional<uint8_t> n, std::optional<std::string> size, std::optional<std::string> response_format, std::optional<std::string> user) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Images::create_edit, this, image, prompt, mask, n, size, response_format, user);
}

liboai::Response liboai::Images::create_variation(const std::filesystem::path& image, std::optional<uint8_t> n, std::optional<std::string> size, std::optional<std::string> response_format, std::optional<std::string> user) const & noexcept(false) {
	if (!this->Validate(image)) {
		throw liboai::exception::OpenAIException(
			"File path provided is non-existent, is not a file, or is empty.",
			liboai::exception::EType::E_FILEERROR,
			"liboai::Images::create_variation(...)"
		);
	}

	netimpl::components::Multipart form = {
		{ "image", netimpl::components::File{image.generic_string()} }
	};
	
	if (n) { form.parts.push_back({ "n", n.value() }); }
	if (size) { form.parts.push_back({ "size", size.value() }); }
	if (response_format) { form.parts.push_back({ "response_format", response_format.value() }); }
	if (user) { form.parts.push_back({ "user", user.value() }); }
	
	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/images/variations", "multipart/form-data",
		this->auth_.GetAuthorizationHeaders(),
		std::move(form),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Images::create_variation_async(const std::filesystem::path& image, std::optional<uint8_t> n, std::optional<std::string> size, std::optional<std::string> response_format, std::optional<std::string> user) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Images::create_variation, this, image, n, size, response_format, user);
}

liboai::ApiResult liboai::Images::create_json(const nlohmann::json& body, const RequestOptions& options) const & {
    return detail::request(this->openai_root_, "POST", "/images/generations", std::optional<nlohmann::json>{std::in_place, body}, {}, options);
}

std::future<liboai::ApiResult> liboai::Images::create_json_async(const nlohmann::json& body, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, body, options]() mutable { return this->create_json(body, options); });
}

liboai::ApiResult liboai::Images::create_edit_multipart(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    return detail::multipart_request(this->openai_root_, "/images/edits", parts, options);
}

std::future<liboai::ApiResult> liboai::Images::create_edit_multipart_async(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    auto snapshot = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [this, snapshot, options]() mutable { return this->create_edit_multipart(*snapshot, options); });
}

liboai::ParsedStreamResult liboai::Images::create_stream(const nlohmann::json& body, SseCallback callback, const RequestOptions& options) const & {
    auto payload = body;
    payload["stream"] = true;
    return detail::parsed_stream_request(this->openai_root_, "POST", "/images/generations", payload, {}, options, std::move(callback));
}

std::future<liboai::ParsedStreamResult> liboai::Images::create_stream_async(const nlohmann::json& body, SseCallback callback, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, body, callback, options]() mutable { return this->create_stream(body, callback, options); });
}

liboai::ParsedStreamResult liboai::Images::create_edit_stream(const netimpl::components::Multipart& parts, SseCallback callback, const RequestOptions& options) const & {
    auto form = parts;
    form.parts.erase(std::remove_if(form.parts.begin(), form.parts.end(), [](const auto& p) { return p.name == "stream"; }), form.parts.end());
    form.parts.emplace_back("stream", "true");
    return detail::parsed_multipart_stream_request(this->openai_root_, "/images/edits", form, options, std::move(callback));
}

std::future<liboai::ParsedStreamResult> liboai::Images::create_edit_stream_async(const netimpl::components::Multipart& parts, SseCallback callback, const RequestOptions& options) const & {
    auto snapshot = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [this, snapshot, callback, options]() mutable { return this->create_edit_stream(*snapshot, callback, options); });
}
