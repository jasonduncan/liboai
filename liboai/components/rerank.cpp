#include "../include/components/rerank.h"

liboai::Response liboai::Rerank::create(
    const std::string& model_id,
    const std::string& query,
    const std::vector<std::string_view>& documents,
    std::optional<int> top_n
) const & noexcept(false) {
    liboai::JsonConstructor jcon;
    jcon.push_back("model", model_id);
    jcon.push_back("query", query);

    nlohmann::json docs = nlohmann::json::array();
    for (auto sv : documents) docs.emplace_back(std::string(sv));
    jcon.push_back("documents", docs);

    jcon.push_back("top_n", std::move(top_n));

    liboai::Response res;
    res = this->Request(
        Method::HTTP_POST,
        this->openai_root_,
        "/rerank",
        "application/json",
        this->auth_.GetAuthorizationHeaders(),
        netimpl::components::Body{ jcon.dump() },
        this->auth_.GetProxies(),
        this->auth_.GetProxyAuth(),
        this->auth_.GetMaxTimeout()
    );

    return res;
}

liboai::FutureResponse liboai::Rerank::create_async(
    const std::string& model_id,
    const std::string& query,
    const std::vector<std::string_view>& documents,
    std::optional<int> top_n
) const & noexcept(false) {
    // copy documents to owned strings (string_view would dangle in async task)
    std::vector<std::string> owned_documents;
    owned_documents.reserve(documents.size());
    for (auto sv : documents) owned_documents.emplace_back(sv);

    return std::async(
        std::launch::async,
        [this, model_id, query, owned_documents = std::move(owned_documents), top_n]() {
            std::vector<std::string_view> views;
            views.reserve(owned_documents.size());
            for (const auto& d : owned_documents) views.push_back(d);
            return this->create(model_id, query, views, top_n);
        }
    );
}
