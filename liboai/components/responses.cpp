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

namespace {
std::string response_path(const std::string& id) {
    return "/responses/" + liboai::detail::encode_path_segment(id);
}
}

liboai::ApiResult liboai::Responses::create_with_options(const nlohmann::json& request,
    const RequestOptions& options, std::optional<StreamCallback> stream) const & {
    return detail::request(openai_root_, "POST", "/responses", std::optional<nlohmann::json>{std::in_place, request}, {}, options, stream);
}
liboai::FutureApiResult liboai::Responses::create_with_options_async(const nlohmann::json& request,
    const RequestOptions& options, std::optional<StreamCallback> stream) const & {
    return std::async(std::launch::async, [this, request, options, stream] {
        return create_with_options(request, options, stream);
    });
}
liboai::ApiResult liboai::Responses::retrieve(const std::string& response_id, const Query& query,
    const RequestOptions& options, std::optional<StreamCallback> stream) const & {
    return detail::request(openai_root_, "GET", response_path(response_id), std::nullopt, query, options, stream);
}
liboai::FutureApiResult liboai::Responses::retrieve_async(const std::string& response_id, const Query& query,
    const RequestOptions& options, std::optional<StreamCallback> stream) const & {
    return std::async(std::launch::async, [this, response_id, query, options, stream] {
        return retrieve(response_id, query, options, stream);
    });
}
liboai::ApiResult liboai::Responses::remove(const std::string& response_id, const RequestOptions& options) const & {
    return detail::request(openai_root_, "DELETE", response_path(response_id), std::nullopt, {}, options);
}
liboai::FutureApiResult liboai::Responses::remove_async(const std::string& response_id, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, response_id, options] { return remove(response_id, options); });
}
liboai::ApiResult liboai::Responses::cancel(const std::string& response_id,
    const nlohmann::json& request, const RequestOptions& options) const & {
    return detail::request(openai_root_, "POST", response_path(response_id) + "/cancel", std::optional<nlohmann::json>{std::in_place, request}, {}, options);
}
liboai::FutureApiResult liboai::Responses::cancel_async(const std::string& response_id,
    const nlohmann::json& request, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, response_id, request, options] { return cancel(response_id, request, options); });
}
liboai::ApiResult liboai::Responses::list_input_items(const std::string& response_id,
    const Query& query, const RequestOptions& options) const & {
    return detail::request(openai_root_, "GET", response_path(response_id) + "/input_items", std::nullopt, query, options);
}
liboai::FutureApiResult liboai::Responses::list_input_items_async(const std::string& response_id,
    const Query& query, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, response_id, query, options] { return list_input_items(response_id, query, options); });
}

liboai::ApiResult liboai::Responses::count_input_tokens(const nlohmann::json& request, const RequestOptions& options) const & {
    return detail::request(openai_root_, "POST", "/responses/input_tokens", std::optional<nlohmann::json>{std::in_place, request}, {}, options);
}
liboai::FutureApiResult liboai::Responses::count_input_tokens_async(const nlohmann::json& request, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, request, options] { return count_input_tokens(request, options); });
}

liboai::ApiResult liboai::Responses::compact(const nlohmann::json& request, const RequestOptions& options) const & {
    return detail::request(openai_root_, "POST", "/responses/compact", std::optional<nlohmann::json>{std::in_place, request}, {}, options);
}
liboai::FutureApiResult liboai::Responses::compact_async(const nlohmann::json& request, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, request, options] { return compact(request, options); });
}

liboai::ParsedStreamResult liboai::Responses::create_stream(const nlohmann::json& request,
    SseCallback callback, const RequestOptions& options) const & {
    if (!request.is_object()) throw std::invalid_argument("Responses streaming request must be an object");
    auto body = request;
    body["stream"] = true;
    return detail::parsed_stream_request(openai_root_, "POST", "/responses", body, {}, options, callback);
}
liboai::FutureParsedStreamResult liboai::Responses::create_stream_async(const nlohmann::json& request,
    SseCallback callback, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, request, callback, options] { return create_stream(request, callback, options); });
}
liboai::ParsedStreamResult liboai::Responses::retrieve_stream(const std::string& response_id,
    SseCallback callback, const Query& query, const RequestOptions& options) const & {
    Query streaming_query;
    for (const auto& entry : query) if (entry.first != "stream") streaming_query.push_back(entry);
    streaming_query.emplace_back("stream", "true");
    return detail::parsed_stream_request(openai_root_, "GET", response_path(response_id), std::nullopt, streaming_query, options, callback);
}
liboai::FutureParsedStreamResult liboai::Responses::retrieve_stream_async(const std::string& response_id,
    SseCallback callback, const Query& query, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, response_id, callback, query, options] { return retrieve_stream(response_id, callback, query, options); });
}
std::string liboai::Responses::output_text(const nlohmann::json& response,
    const std::optional<std::string>& agent_name) {
    std::string output;
    if (!response.is_object()) return output;
    auto items = response.find("output");
    if (items == response.end() || !items->is_array()) return output;
    for (const auto& item : *items) {
        if (!item.is_object() || item.value("type", nlohmann::json{}) != "message" ||
            item.value("role", nlohmann::json{}) != "assistant") continue;
        if (agent_name) {
            auto agent = item.find("agent");
            if (agent == item.end() || !agent->is_object() || agent->value("agent_name", nlohmann::json{}) != *agent_name) continue;
        }
        auto content = item.find("content");
        if (content == item.end() || !content->is_array()) continue;
        for (const auto& part : *content) {
            if (!part.is_object() || part.value("type", nlohmann::json{}) != "output_text") continue;
            auto text = part.find("text");
            if (text != part.end() && text->is_string()) output += text->get<std::string>();
        }
    }
    return output;
}
