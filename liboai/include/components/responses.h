#pragma once

/*
	responses.h : Responses component class for OpenAI.
		This class provides access to the Responses API endpoints.
		It is available via liboai.h through an instantiated liboai::OpenAI
		object after setting necessary authentication information.
*/

#include "../core/authorization.h"
#include "../core/response.h"
#include "../core/sse.h"

namespace liboai {
	class Responses final : private Network {
		public:
			using StreamCallback = std::function<bool(std::string, intptr_t)>;

			Responses(const std::string &root): Network(root) {}
			NON_COPYABLE(Responses)
			NON_MOVABLE(Responses)
			~Responses() = default;

			/*
				@brief Builds a Responses API request payload.

				@param *model              The model ID to use.
				@param *input              Input for the response (string or array of items).
				@param instructions        Optional system-level instructions.
				@param reasoning           Optional reasoning configuration.
				@param text                Optional text output configuration.
				@param max_output_tokens   Optional max output tokens to generate.
				@param temperature         Optional sampling temperature.
				@param top_p               Optional nucleus sampling value.
				@param seed                Legacy pass-through; not a documented Responses field.
				@param tools               Optional tools array.
				@param tool_choice         Optional tool choice configuration.
				@param parallel_tool_calls Optional parallel tool calls toggle.
				@param store               Optional storage toggle.
				@param previous_response_id Optional prior response ID for continuity.
				@param include             Optional include array.
				@param metadata            Optional metadata object.
				@param user                Optional user ID.
				@param truncation          Optional truncation setting.
				@param stream              Optional stream flag.

				@return A JSON object representing the request payload.
			*/
			LIBOAI_EXPORT static nlohmann::json build_request(
				const std::string& model,
				const nlohmann::json& input,
				std::optional<std::string> instructions = std::nullopt,
				std::optional<nlohmann::json> reasoning = std::nullopt,
				std::optional<nlohmann::json> text = std::nullopt,
				std::optional<uint32_t> max_output_tokens = std::nullopt,
				std::optional<float> temperature = std::nullopt,
				std::optional<float> top_p = std::nullopt,
				std::optional<uint32_t> seed = std::nullopt,
				std::optional<nlohmann::json> tools = std::nullopt,
				std::optional<nlohmann::json> tool_choice = std::nullopt,
				std::optional<bool> parallel_tool_calls = std::nullopt,
				std::optional<bool> store = std::nullopt,
				std::optional<std::string> previous_response_id = std::nullopt,
				std::optional<nlohmann::json> include = std::nullopt,
				std::optional<nlohmann::json> metadata = std::nullopt,
				std::optional<std::string> user = std::nullopt,
				std::optional<std::string> truncation = std::nullopt,
				std::optional<bool> stream = std::nullopt
			);

			/*
				@brief Creates a response using the Responses API.

				@param *model             The model ID to use.
				@param *input             Input for the response (string or array of items).
				@param instructions       Optional system-level instructions.
				@param reasoning          Optional reasoning configuration.
				@param text               Optional text output configuration.
				@param max_output_tokens  Optional max output tokens to generate.
				@param temperature        Optional sampling temperature.
				@param top_p              Optional nucleus sampling value.
				@param seed               Legacy pass-through; not a documented Responses field.
				@param tools              Optional tools array.
				@param tool_choice        Optional tool choice configuration.
				@param parallel_tool_calls Optional parallel tool calls toggle.
				@param store              Optional storage toggle.
				@param previous_response_id Optional prior response ID for continuity.
				@param include            Optional include array.
				@param metadata           Optional metadata object.
				@param user               Optional user ID.
				@param truncation         Optional truncation setting.
				@param stream             Optional stream callback for SSE responses.

				@return A liboai::Response object containing response data.
			*/
			LIBOAI_EXPORT liboai::Response create(
				const std::string& model,
				const nlohmann::json& input,
				std::optional<std::string> instructions = std::nullopt,
				std::optional<nlohmann::json> reasoning = std::nullopt,
				std::optional<nlohmann::json> text = std::nullopt,
				std::optional<uint32_t> max_output_tokens = std::nullopt,
				std::optional<float> temperature = std::nullopt,
				std::optional<float> top_p = std::nullopt,
				std::optional<uint32_t> seed = std::nullopt,
				std::optional<nlohmann::json> tools = std::nullopt,
				std::optional<nlohmann::json> tool_choice = std::nullopt,
				std::optional<bool> parallel_tool_calls = std::nullopt,
				std::optional<bool> store = std::nullopt,
				std::optional<std::string> previous_response_id = std::nullopt,
				std::optional<nlohmann::json> include = std::nullopt,
				std::optional<nlohmann::json> metadata = std::nullopt,
				std::optional<std::string> user = std::nullopt,
				std::optional<std::string> truncation = std::nullopt,
				std::optional<StreamCallback> stream = std::nullopt
			) const & noexcept(false);

			/*
				@brief Creates a response using the Responses API from a raw JSON payload.

				@param *request  The raw JSON payload for the request.
				@param stream    Optional stream callback for SSE responses.

				@return A liboai::Response object containing response data.
			*/
			LIBOAI_EXPORT liboai::Response create(
				const nlohmann::json& request,
				std::optional<StreamCallback> stream = std::nullopt
			) const & noexcept(false);

			/*
				@brief Asynchronously creates a response using the Responses API.

				@param *model             The model ID to use.
				@param *input             Input for the response (string or array of items).
				@param instructions       Optional system-level instructions.
				@param reasoning          Optional reasoning configuration.
				@param text               Optional text output configuration.
				@param max_output_tokens  Optional max output tokens to generate.
				@param temperature        Optional sampling temperature.
				@param top_p              Optional nucleus sampling value.
				@param seed               Legacy pass-through; not a documented Responses field.
				@param tools              Optional tools array.
				@param tool_choice        Optional tool choice configuration.
				@param parallel_tool_calls Optional parallel tool calls toggle.
				@param store              Optional storage toggle.
				@param previous_response_id Optional prior response ID for continuity.
				@param include            Optional include array.
				@param metadata           Optional metadata object.
				@param user               Optional user ID.
				@param truncation         Optional truncation setting.
				@param stream             Optional stream callback for SSE responses.

				@return A liboai::FutureResponse containing future response data.
			*/
			LIBOAI_EXPORT liboai::FutureResponse create_async(
				const std::string& model,
				const nlohmann::json& input,
				std::optional<std::string> instructions = std::nullopt,
				std::optional<nlohmann::json> reasoning = std::nullopt,
				std::optional<nlohmann::json> text = std::nullopt,
				std::optional<uint32_t> max_output_tokens = std::nullopt,
				std::optional<float> temperature = std::nullopt,
				std::optional<float> top_p = std::nullopt,
				std::optional<uint32_t> seed = std::nullopt,
				std::optional<nlohmann::json> tools = std::nullopt,
				std::optional<nlohmann::json> tool_choice = std::nullopt,
				std::optional<bool> parallel_tool_calls = std::nullopt,
				std::optional<bool> store = std::nullopt,
				std::optional<std::string> previous_response_id = std::nullopt,
				std::optional<nlohmann::json> include = std::nullopt,
				std::optional<nlohmann::json> metadata = std::nullopt,
				std::optional<std::string> user = std::nullopt,
				std::optional<std::string> truncation = std::nullopt,
				std::optional<StreamCallback> stream = std::nullopt
			) const & noexcept(false);

			/*
				@brief Asynchronously creates a response using the Responses API from a raw JSON payload.

				@param *request  The raw JSON payload for the request.
				@param stream    Optional stream callback for SSE responses.

				@return A liboai::FutureResponse containing future response data.
			*/
			LIBOAI_EXPORT liboai::FutureResponse create_async(
				const nlohmann::json& request,
				std::optional<StreamCallback> stream = std::nullopt
			) const & noexcept(false);


            // Modern operations preserve raw JSON and return additive diagnostics.
            LIBOAI_EXPORT ApiResult create_with_options(const nlohmann::json& request,
                const RequestOptions& options = {}, std::optional<StreamCallback> stream = std::nullopt) const &;
            LIBOAI_EXPORT FutureApiResult create_with_options_async(const nlohmann::json& request,
                const RequestOptions& options = {}, std::optional<StreamCallback> stream = std::nullopt) const &;
            LIBOAI_EXPORT ApiResult retrieve(const std::string& response_id, const Query& query = {},
                const RequestOptions& options = {}, std::optional<StreamCallback> stream = std::nullopt) const &;
            LIBOAI_EXPORT FutureApiResult retrieve_async(const std::string& response_id, const Query& query = {},
                const RequestOptions& options = {}, std::optional<StreamCallback> stream = std::nullopt) const &;
            LIBOAI_EXPORT ApiResult remove(const std::string& response_id, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureApiResult remove_async(const std::string& response_id, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT ApiResult cancel(const std::string& response_id,
                const nlohmann::json& request = nlohmann::json::object(), const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureApiResult cancel_async(const std::string& response_id,
                const nlohmann::json& request = nlohmann::json::object(), const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT ApiResult list_input_items(const std::string& response_id, const Query& query = {},
                const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureApiResult list_input_items_async(const std::string& response_id, const Query& query = {},
                const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT ApiResult count_input_tokens(const nlohmann::json& request, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureApiResult count_input_tokens_async(const nlohmann::json& request, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT ApiResult compact(const nlohmann::json& request, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureApiResult compact_async(const nlohmann::json& request, const RequestOptions& options = {}) const &;

            // Parsed methods force streaming; returning false stops local receipt only.
            LIBOAI_EXPORT ParsedStreamResult create_stream(const nlohmann::json& request,
                SseCallback callback, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureParsedStreamResult create_stream_async(const nlohmann::json& request,
                SseCallback callback, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT ParsedStreamResult retrieve_stream(const std::string& response_id,
                SseCallback callback, const Query& query = {}, const RequestOptions& options = {}) const &;
            LIBOAI_EXPORT FutureParsedStreamResult retrieve_stream_async(const std::string& response_id,
                SseCallback callback, const Query& query = {}, const RequestOptions& options = {}) const &;
            // Optional agent_name selects item.agent.agent_name (for example /root).
            LIBOAI_EXPORT static std::string output_text(const nlohmann::json& response,
                const std::optional<std::string>& agent_name = std::nullopt);

		private:
			Authorization& auth_ = Authorization::Authorizer();
	};
}
