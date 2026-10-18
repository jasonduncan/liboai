#pragma once
/*
rerank.h : Rerank component for compatible servers such as llama.cpp.
Endpoint: POST <configured root>/rerank. Supply the server's API root
(for example, http://127.0.0.1:8012/v1) to liboai::OpenAI.
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

    /**
     * @brief Ranks documents by their relevance to a query.
     * @param model_id Model identifier accepted by the configured server.
     * @param query Query against which the documents are ranked.
     * @param documents Document views; their storage must remain valid until return.
     * @param top_n Optional number of results. Omitted from the request when unset;
     *              defaults and accepted values are determined by the server.
     * @return The server's JSON response, without schema conversion.
     * @throws exception::OpenAIException On transport or API failure.
     */
    LIBOAI_EXPORT liboai::Response create(
        const std::string& model_id,
        const std::string& query,
        const std::vector<std::string_view>& documents,
        std::optional<int> top_n = std::nullopt
    ) const & noexcept(false);

    /**
     * @brief Asynchronously ranks documents by their relevance to a query.
     * @param model_id Model identifier; copied before launching the request.
     * @param query Query text; copied before launching the request.
     * @param documents Document views; contents are copied before this method returns.
     * @param top_n Optional result count, with server-defined defaults and validation.
     * @return A future containing the server's JSON response. Request failures are
     *         rethrown by get(); copying or launching the task may throw immediately.
     * @note Keep this component (and its owning OpenAI instance) alive until the
     *       future completes. Do not mutate shared authorization settings in flight.
     */
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
