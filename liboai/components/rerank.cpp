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
    //docs.reserve(documents.size());
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
    return std::async(
        std::launch::async,
        &liboai::Rerank::create,
        this,
        model_id,
        query,
        documents,
        top_n
    );
}
