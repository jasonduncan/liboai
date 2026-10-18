#include "../include/components/audio.h"

liboai::Response liboai::Audio::transcribe(const std::filesystem::path& file, const std::string& model, std::optional<std::string> prompt, std::optional<std::string> response_format, std::optional<float> temperature, std::optional<std::string> language) const& noexcept(false) {
	if (!this->Validate(file)) {
		throw liboai::exception::OpenAIException(
			"File path provided is non-existent, is not a file, or is empty.",
			liboai::exception::EType::E_FILEERROR,
			"liboai::Audio::transcribe(...)"
		);
	}

	netimpl::components::Multipart form = {
		{ "file", netimpl::components::File{file.generic_string()} },
		{ "model", model }
	};
	
	if (prompt) { form.parts.push_back({ "prompt", prompt.value() }); }
	if (response_format) { form.parts.push_back({ "response_format", response_format.value() }); }
	if (temperature) { form.parts.push_back({ "temperature", std::to_string(temperature.value()) }); }
	if (language) { form.parts.push_back({ "language", language.value() }); }

	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/audio/transcriptions", "multipart/form-data",
		this->auth_.GetAuthorizationHeaders(),
		std::move(form),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Audio::transcribe_async(const std::filesystem::path& file, const std::string& model, std::optional<std::string> prompt, std::optional<std::string> response_format, std::optional<float> temperature, std::optional<std::string> language) const& noexcept(false) {
	return std::async(std::launch::async, &liboai::Audio::transcribe, this, file, model, prompt, response_format, temperature, language);
}

liboai::Response liboai::Audio::translate(const std::filesystem::path& file, const std::string& model, std::optional<std::string> prompt, std::optional<std::string> response_format, std::optional<float> temperature) const& noexcept(false) {
	if (!this->Validate(file)) {
		throw liboai::exception::OpenAIException(
			"File path provided is non-existent, is not a file, or is empty.",
			liboai::exception::EType::E_FILEERROR,
			"liboai::Audio::translate(...)"
		);
	}

	netimpl::components::Multipart form = {
		{ "file", netimpl::components::File{file.generic_string()} },
		{ "model", model }
	};
	
	if (prompt) { form.parts.push_back({ "prompt", std::move(prompt.value()) }); }
	if (response_format) { form.parts.push_back({ "response_format", std::move(response_format.value()) }); }
	if (temperature) { form.parts.push_back({ "temperature", std::to_string(temperature.value()) }); }

	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/audio/translations", "multipart/form-data",
		this->auth_.GetAuthorizationHeaders(),
		std::move(form),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Audio::translate_async(const std::filesystem::path& file, const std::string& model, std::optional<std::string> prompt, std::optional<std::string> response_format, std::optional<float> temperature) const& noexcept(false) {
	return std::async(std::launch::async, &liboai::Audio::translate, this, file, model, prompt, response_format, temperature);
}

liboai::Response liboai::Audio::speech(const std::string& model, const std::string& voice, const std::string& input, std::optional<std::string> response_format, std::optional<float> speed) const& noexcept(false) {
	liboai::JsonConstructor jcon;
	jcon.push_back("model", model);
	jcon.push_back("voice", voice);
	jcon.push_back("input", input);
	
	if (response_format) { jcon.push_back("response_format", std::move(response_format.value())); }
	if (speed) { jcon.push_back("speed", speed.value()); }

	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/audio/speech", "application/json",
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

liboai::FutureResponse liboai::Audio::speech_async(const std::string& model, const std::string& voice, const std::string& input, std::optional<std::string> response_format, std::optional<float> speed) const& noexcept(false) {
	return std::async(std::launch::async, &liboai::Audio::speech, this, model, voice, input, response_format, speed);
}

liboai::ApiResult liboai::Audio::speech_json(const nlohmann::json& body, const RequestOptions& options) const & {
    return detail::request(this->openai_root_, "POST", "/audio/speech", std::optional<nlohmann::json>{std::in_place, body}, {}, options);
}

std::future<liboai::ApiResult> liboai::Audio::speech_json_async(const nlohmann::json& body, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, body, options]() mutable { return this->speech_json(body, options); });
}

liboai::ApiResult liboai::Audio::transcribe_multipart(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    return detail::multipart_request(this->openai_root_, "/audio/transcriptions", parts, options);
}

std::future<liboai::ApiResult> liboai::Audio::transcribe_multipart_async(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    auto snapshot = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [this, snapshot, options]() mutable { return this->transcribe_multipart(*snapshot, options); });
}

liboai::ApiResult liboai::Audio::translate_multipart(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    return detail::multipart_request(this->openai_root_, "/audio/translations", parts, options);
}

std::future<liboai::ApiResult> liboai::Audio::translate_multipart_async(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    auto snapshot = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [this, snapshot, options]() mutable { return this->translate_multipart(*snapshot, options); });
}

liboai::ParsedStreamResult liboai::Audio::speech_stream(const nlohmann::json& body, SseCallback callback, const RequestOptions& options) const & {
    auto payload = body;
    payload["stream_format"] = "sse";
    return detail::parsed_stream_request(this->openai_root_, "POST", "/audio/speech", payload, {}, options, std::move(callback));
}

std::future<liboai::ParsedStreamResult> liboai::Audio::speech_stream_async(const nlohmann::json& body, SseCallback callback, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, body, callback, options]() mutable { return this->speech_stream(body, callback, options); });
}

liboai::ApiResult liboai::Audio::speech_bytes(const nlohmann::json& body, RawStreamCallback callback, const RequestOptions& options) const & {
    auto payload = body;
    payload["stream_format"] = "audio";
    return detail::request(this->openai_root_, "POST", "/audio/speech", payload, {}, options, std::move(callback));
}

std::future<liboai::ApiResult> liboai::Audio::speech_bytes_async(const nlohmann::json& body, RawStreamCallback callback, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, body, callback, options]() mutable { return this->speech_bytes(body, callback, options); });
}

liboai::ParsedStreamResult liboai::Audio::transcribe_stream(const netimpl::components::Multipart& parts, SseCallback callback, const RequestOptions& options) const & {
    auto form = parts;
    form.parts.erase(std::remove_if(form.parts.begin(), form.parts.end(), [](const auto& p) { return p.name == "stream"; }), form.parts.end());
    form.parts.emplace_back("stream", "true");
    return detail::parsed_multipart_stream_request(this->openai_root_, "/audio/transcriptions", form, options, std::move(callback));
}

std::future<liboai::ParsedStreamResult> liboai::Audio::transcribe_stream_async(const netimpl::components::Multipart& parts, SseCallback callback, const RequestOptions& options) const & {
    auto snapshot = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [this, snapshot, callback, options]() mutable { return this->transcribe_stream(*snapshot, callback, options); });
}
