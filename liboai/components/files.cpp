#include "../include/components/files.h"

liboai::Response liboai::Files::list() const & noexcept(false) {
	Response res;
	res = this->Request(
		Method::HTTP_GET, this->openai_root_, "/files", "application/json",
		this->auth_.GetAuthorizationHeaders(),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Files::list_async() const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Files::list, this);
}

liboai::Response liboai::Files::create(const std::filesystem::path& file, const std::string& purpose) const & noexcept(false) {
	if (!this->Validate(file)) {
		throw liboai::exception::OpenAIException(
			"File path provided is non-existent, is not a file, or is empty.",
			liboai::exception::EType::E_FILEERROR,
			"liboai::Files::create(...)"
		);
	}

	netimpl::components::Multipart form = {
		{ "purpose", purpose },
		{ "file", netimpl::components::File{file.generic_string()} }
	};
	
	Response res;
	res = this->Request(
		Method::HTTP_POST, this->openai_root_, "/files", "multipart/form-data",
		this->auth_.GetAuthorizationHeaders(),
		std::move(form),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Files::create_async(const std::filesystem::path& file, const std::string& purpose) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Files::create, this, file, purpose);
}

liboai::Response liboai::Files::remove(const std::string& file_id) const & noexcept(false) {
	Response res;
	res = this->Request(
		Method::HTTP_DELETE, this->openai_root_, "/files/" + file_id, "application/json",
		this->auth_.GetAuthorizationHeaders(),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Files::remove_async(const std::string& file_id) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Files::remove, this, file_id);
}

liboai::Response liboai::Files::retrieve(const std::string& file_id) const & {
	Response res;
	res = this->Request(
		Method::HTTP_GET, this->openai_root_, "/files/" + file_id, "application/json",
		this->auth_.GetAuthorizationHeaders(),
		this->auth_.GetProxies(),
		this->auth_.GetProxyAuth(),
		this->auth_.GetMaxTimeout()
	);

	return res;
}

liboai::FutureResponse liboai::Files::retrieve_async(const std::string& file_id) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Files::retrieve, this, file_id);
}

bool liboai::Files::download(const std::string& file_id, const std::string& save_to) const & noexcept(false) {
	return Network::Download(
		save_to,
		(this->openai_root_ + "/files/" + detail::encode_path_segment(file_id) + "/content"),
		this->auth_.GetAuthorizationHeaders()
	);
}

std::future<bool> liboai::Files::download_async(const std::string& file_id, const std::string& save_to) const & noexcept(false) {
	return std::async(std::launch::async, &liboai::Files::download, this, file_id, save_to);
}

liboai::ApiResult liboai::Files::create_multipart(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    return detail::multipart_request(this->openai_root_, "/files", parts, options);
}

std::future<liboai::ApiResult> liboai::Files::create_multipart_async(const netimpl::components::Multipart& parts, const RequestOptions& options) const & {
    auto snapshot = detail::snapshot_multipart(parts);
    return std::async(std::launch::async, [this, snapshot, options]() mutable { return this->create_multipart(*snapshot, options); });
}

liboai::ApiResult liboai::Files::list_with_options(const Query& query, const RequestOptions& options) const & {
    return detail::request(this->openai_root_, "GET", "/files", std::nullopt, query, options);
}

std::future<liboai::ApiResult> liboai::Files::list_with_options_async(const Query& query, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, query, options]() mutable { return this->list_with_options(query, options); });
}

liboai::ApiResult liboai::Files::content(const std::string& id, const RequestOptions& options) const & {
    return detail::request(this->openai_root_, "GET", "/files/" + detail::encode_path_segment(id) + "/content", std::nullopt, {}, options);
}

std::future<liboai::ApiResult> liboai::Files::content_async(const std::string& id, const RequestOptions& options) const & {
    return std::async(std::launch::async, [this, id, options]() mutable { return this->content(id, options); });
}
