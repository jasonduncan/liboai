#pragma once
/*
rerank.h : Rerank component class for OpenAI.
Endpoint: POST /v1/rerank
*/

#include "../core/authorization.h"
#include "../core/response.h"

#include <optional>
#include <string_view>
#include <vector>

namespace liboai {

class Rerank final : private Network {
public:
    Rerank(const std::string& root) : Network(root) {}
    NON_COPYABLE(Rerank)
    NON_MOVABLE(Rerank)
    ~Rerank() = default;

    LIBOAI_EXPORT liboai::Response create(
        const std::string& model_id,
        const std::string& query,
        const std::vector<std::string_view>& documents,
        std::optional<int> top_n = std::nullopt
    ) const & noexcept(false);

    LIBOAI_EXPORT liboai::FutureResponse create_async(
        const std::string& model_id,
        const std::string& query,
        const std::vector<std::string_view>& documents,
        std::optional<int> top_n = std::nullopt
    ) const & noexcept(false);

private:
    Authorization& auth_ = Authorization::Authorizer();
};

} // namespace liboai
