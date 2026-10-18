#include "common.h"

// Explicitly running this example creates a durable conversation and two responses.
// Pass a retained conversation ID to continue it in a later process.
int main(int argc, char** argv) {
    if (argc < 3 || std::string(argv[1]) != "--live") {
        std::cout << "Usage: conversation_continuation --live MODEL [CONVERSATION_ID]\n";
        return 0;
    }
    try {
        resource_auth();
        liboai::Conversations conversations(resource_root);
        liboai::Responses responses(resource_root);
        const auto id = argc > 3 ? std::string(argv[3]) : conversations.create().response.raw_json.at("id").get<std::string>();
        std::cout << "Retain this conversation ID: " << id << '\n';
        nlohmann::json request{{"model", argv[2]}, {"conversation", id},
            {"input", "Remember the synthetic project codename: Maple."}, {"max_output_tokens", 64}};
        std::cout << responses.create(request) << '\n';
        request["input"] = "What is the project codename?";
        std::cout << responses.create(request) << '\n';
        // Conversations automatically retain input/output items from Responses.
        std::cout << conversations.list_items(id, {{"limit", "10"}, {"order", "asc"}}).response << '\n';
        // The ID is intentionally retained for continuation. Delete it explicitly
        // with conversations.remove(id) when your application's retention ends.
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
