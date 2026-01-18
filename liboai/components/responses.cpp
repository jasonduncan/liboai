#include "../include/components/responses.h"

nlohmann::json liboai::Responses::build_request(
	const std::string& model,
	const nlohmann::json& input,
	std::optional<std::string> instructions,
	std::optional<nlohmann::json> reasoning,
	std::optional<nlohmann::json> text,
	std::optional<uint32_t> max_output_tokens,
	std::optional<float> temperature,
	std::optional<float> top_p,
	std::optional<uint32_t> seed,
	std::optional<nlohmann::json> tools,
	std::optional<nlohmann::json> tool_choice,
	std::optional<bool> parallel_tool_calls,
	std::optional<bool> store,
	std::optional<std::string> previous_response_id,
	std::optional<nlohmann::json> include,
	std::optional<nlohmann::json> metadata,
	std::optional<std::string> user,
	std::optional<std::string> truncation,
	std::optional<bool> stream
) {
	nlohmann::json request;
	request["model"] = model;
	request["input"] = input;

	if (instructions) {
		request["instructions"] = std::move(*instructions);
	}
	if (reasoning) {
		request["reasoning"] = std::move(*reasoning);
	}
	if (text) {
		request["text"] = std::move(*text);
	}
	if (max_output_tokens) {
		request["max_output_tokens"] = *max_output_tokens;
	}
	if (temperature) {
		request["temperature"] = *temperature;
	}
	if (top_p) {
		request["top_p"] = *top_p;
	}
	if (seed) {
		request["seed"] = *seed;
	}
	if (tools) {
		request["tools"] = std::move(*tools);
	}
	if (tool_choice) {
		request["tool_choice"] = std::move(*tool_choice);
	}
	if (parallel_tool_calls) {
		request["parallel_tool_calls"] = *parallel_tool_calls;
	}
	if (store) {
		request["store"] = *store;
	}
	if (previous_response_id) {
		request["previous_response_id"] = std::move(*previous_response_id);
	}
	if (include) {
		request["include"] = std::move(*include);
	}
	if (metadata) {
		request["metadata"] = std::move(*metadata);
	}
	if (user) {
		request["user"] = std::move(*user);
	}
	if (truncation) {
		request["truncation"] = std::move(*truncation);
	}
	if (stream) {
		request["stream"] = *stream;
	}

	return request;
}

liboai::Response liboai::Responses::create(
	const std::string& model,
	const nlohmann::json& input,
	std::optional<std::string> instructions,
	std::optional<nlohmann::json> reasoning,
	std::optional<nlohmann::json> text,
	std::optional<uint32_t> max_output_tokens,
	std::optional<float> temperature,
	std::optional<float> top_p,
	std::optional<uint32_t> seed,
	std::optional<nlohmann::json> tools,
	std::optional<nlohmann::json> tool_choice,
	std::optional<bool> parallel_tool_calls,
	std::optional<bool> store,
	std::optional<std::string> previous_response_id,
	std::optional<nlohmann::json> include,
	std::optional<nlohmann::json> metadata,
	std::optional<std::string> user,
	std::optional<std::string> truncation,
	std::optional<StreamCallback> stream
) const & noexcept(false) {
	const auto request = liboai::Responses::build_request(
		model,
		input,
		std::move(instructions),
		std::move(reasoning),
		std::move(text),
		std::move(max_output_tokens),
		std::move(temperature),
		std::move(top_p),
		std::move(seed),
		std::move(tools),
		std::move(tool_choice),
		std::move(parallel_tool_calls),
		std::move(store),
		std::move(previous_response_id),
		std::move(include),
		std::move(metadata),
		std::move(user),
		std::move(truncation),
		stream ? std::optional<bool>(true) : std::nullopt
	);

	return this->create(request, std::move(stream));
}

liboai::Response liboai::Responses::create(const nlohmann::json& request, std::optional<StreamCallback> stream) const & noexcept(false) {
	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/responses", "application/json",
		this->auth_.GetAuthorizationHeaders(),
		netimpl::components::Body {
			request.dump(4)
		},
		stream ? netimpl::components::WriteCallback{std::move(stream.value())} : netimpl::components::WriteCallback{},
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Responses::create_async(
	const std::string& model,
	const nlohmann::json& input,
	std::optional<std::string> instructions,
	std::optional<nlohmann::json> reasoning,
	std::optional<nlohmann::json> text,
	std::optional<uint32_t> max_output_tokens,
	std::optional<float> temperature,
	std::optional<float> top_p,
	std::optional<uint32_t> seed,
	std::optional<nlohmann::json> tools,
	std::optional<nlohmann::json> tool_choice,
	std::optional<bool> parallel_tool_calls,
	std::optional<bool> store,
	std::optional<std::string> previous_response_id,
	std::optional<nlohmann::json> include,
	std::optional<nlohmann::json> metadata,
	std::optional<std::string> user,
	std::optional<std::string> truncation,
	std::optional<StreamCallback> stream
) const & noexcept(false) {
	return std::async(
		std::launch::async,
		[this,
		 model,
		 input,
		 instructions,
		 reasoning,
		 text,
		 max_output_tokens,
		 temperature,
		 top_p,
		 seed,
		 tools,
		 tool_choice,
		 parallel_tool_calls,
		 store,
		 previous_response_id,
		 include,
		 metadata,
		 user,
		 truncation,
		 stream]() mutable {
			return this->create(
				model,
				input,
				std::move(instructions),
				std::move(reasoning),
				std::move(text),
				std::move(max_output_tokens),
				std::move(temperature),
				std::move(top_p),
				std::move(seed),
				std::move(tools),
				std::move(tool_choice),
				std::move(parallel_tool_calls),
				std::move(store),
				std::move(previous_response_id),
				std::move(include),
				std::move(metadata),
				std::move(user),
				std::move(truncation),
				std::move(stream)
			);
		}
	);
}

liboai::FutureResponse liboai::Responses::create_async(const nlohmann::json& request, std::optional<StreamCallback> stream) const & noexcept(false) {
	return std::async(
		std::launch::async,
		[this, request, stream]() mutable {
			return this->create(request, std::move(stream));
		}
	);
}
