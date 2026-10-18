#pragma once

/*
	images.h : Images component class for OpenAI.
		This class contains all the methods for the Images component
		of the OpenAI API. This class provides access to 'Images' 
		endpoints on the OpenAI API and should be accessed via the
		liboai.h header file through an instantiated liboai::OpenAI
		object after setting necessary authentication information
		through the liboai::Authorization::Authorizer() singleton
		object.
*/

#include "../core/authorization.h"
#include "../core/response.h"
#include "../core/request.h"
#include "../core/sse.h"

namespace liboai {
	class Images final : private Network {
		public:
			Images(const std::string &root): Network(root) {}
			NON_COPYABLE(Images)
			NON_MOVABLE(Images)
			~Images() = default;
			
			/*
				@brief Images component method to create an image from
					provided text.
				
				@param *prompt         The text to create an image from.
				@param n               The number of images to create.
				@param size            The size of the image to create.
				@param response_format The format of the response.
				@param user            A unique identifier representing an end-user.

				@return A liboai::Response object containing the image(s)
					data in JSON format.
			*/
			LIBOAI_EXPORT liboai::Response create(
				const std::string& prompt,
				std::optional<uint8_t> n = std::nullopt,
				std::optional<std::string> size = std::nullopt,
				std::optional<std::string> response_format = std::nullopt,
				std::optional<std::string> user = std::nullopt
			) const & noexcept(false);

			/*
				@brief Images component method to asynchronously create an
					image from provided text.

				@param *prompt         The text to create an image from.
				@param n               The number of images to create.
				@param size            The size of the image to create.
				@param response_format The format of the response.
				@param user            A unique identifier representing an end-user.

				@return A liboai::Response future containing the image(s)
					data in JSON format.
			*/
			LIBOAI_EXPORT liboai::FutureResponse create_async(
				const std::string& prompt,
				std::optional<uint8_t> n = std::nullopt,
				std::optional<std::string> size = std::nullopt,
				std::optional<std::string> response_format = std::nullopt,
				std::optional<std::string> user = std::nullopt
			) const & noexcept(false);
			
			/*
				@brief Images component method to produce an edited
					image from a provided base image and mask image
					according to given text.

				@param *image          The image to edit (path).
				@param *prompt         The text description of the desired image.
				@param mask            The mask to edit the image with (path).
				@param n		       The number of images to create.
				@param size            The size of the image to create.
				@param response_format The format of the response.
				@param user            A unique identifier representing an end-user.

				@return A liboai::Response object containing the image(s)
					data in JSON format.
			*/		
			LIBOAI_EXPORT liboai::Response create_edit(
				const std::filesystem::path& image,
				const std::string& prompt,
				std::optional<std::filesystem::path> mask = std::nullopt,
				std::optional<uint8_t> n = std::nullopt,
				std::optional<std::string> size = std::nullopt,
				std::optional<std::string> response_format = std::nullopt,
				std::optional<std::string> user = std::nullopt
			) const & noexcept(false);

			/*
				@brief Images component method to asynchronously
					produce an edited image from a provided base
					image and mask image according to given text.

				@param *image          The image to edit (path).
				@param *prompt         The text description of the desired image.
				@param mask            The mask to edit the image with (path).
				@param n		       The number of images to create.
				@param size            The size of the image to create.
				@param response_format The format of the response.
				@param user            A unique identifier representing an end-user.

				@return A liboai::Response future containing the image(s)
					data in JSON format.
			*/		
			LIBOAI_EXPORT liboai::FutureResponse create_edit_async(
				const std::filesystem::path& image,
				const std::string& prompt,
				std::optional<std::filesystem::path> mask = std::nullopt,
				std::optional<uint8_t> n = std::nullopt,
				std::optional<std::string> size = std::nullopt,
				std::optional<std::string> response_format = std::nullopt,
				std::optional<std::string> user = std::nullopt
			) const & noexcept(false);

			/*
				@brief Images component method to produce a variation
					of a supplied image.
				
				@param *image          The image to produce a variation of (path).
				@param n               The number of images to create.
				@param size            The size of the image to create.
				@param response_format The format of the response.
				@param user            A unique identifier representing an end-user.

				@return A liboai::Response object containing the image(s)
					data in JSON format.
			*/
			LIBOAI_EXPORT liboai::Response create_variation(
				const std::filesystem::path& image,
				std::optional<uint8_t> n = std::nullopt,
				std::optional<std::string> size = std::nullopt,
				std::optional<std::string> response_format = std::nullopt,
				std::optional<std::string> user = std::nullopt
			) const & noexcept(false);

			/*
				@brief Images component method to asynchronously produce
					a variation of a supplied image.

				@param *image          The image to produce a variation of (path).
				@param n               The number of images to create.
				@param size            The size of the image to create.
				@param response_format The format of the response.
				@param user            A unique identifier representing an end-user.

				@return A liboai::Response future containing the image(s)
					data in JSON format.
			*/
			LIBOAI_EXPORT liboai::FutureResponse create_variation_async(
				const std::filesystem::path& image,
				std::optional<uint8_t> n = std::nullopt,
				std::optional<std::string> size = std::nullopt,
				std::optional<std::string> response_format = std::nullopt,
				std::optional<std::string> user = std::nullopt
			) const & noexcept(false);

			// Modern raw APIs preserve all JSON fields. See documentation/maintenance/MODERN_COMPONENTS.md.
			LIBOAI_EXPORT ApiResult create_json(const nlohmann::json& body, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT std::future<ApiResult> create_json_async(const nlohmann::json& body, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT ApiResult create_edit_multipart(const netimpl::components::Multipart& parts, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT std::future<ApiResult> create_edit_multipart_async(const netimpl::components::Multipart& parts, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT ParsedStreamResult create_stream(const nlohmann::json& body, SseCallback callback, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT std::future<ParsedStreamResult> create_stream_async(const nlohmann::json& body, SseCallback callback, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT ParsedStreamResult create_edit_stream(const netimpl::components::Multipart& parts, SseCallback callback, const RequestOptions& options = {}) const &;
			LIBOAI_EXPORT std::future<ParsedStreamResult> create_edit_stream_async(const netimpl::components::Multipart& parts, SseCallback callback, const RequestOptions& options = {}) const &;

		private:
			Authorization& auth_ = Authorization::Authorizer();
	};
}