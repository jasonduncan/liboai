#include "common.h"

// Read endpoint metadata without printing signing secrets. Endpoint creation and
// rotation return secrets; store them in your secret manager, never application logs.
int main(int argc, char** argv) {
    if (argc != 3 || std::string(argv[1]) != "--live") {
        std::cout << "Usage: webhook_endpoints --live ENDPOINT_ID\n";
        return 0;
    }
    try {
        resource_auth();
        liboai::Webhooks hooks(resource_root);
        const auto endpoint = hooks.retrieve(argv[2]).response.raw_json;
        std::cout << endpoint.at("id") << ' ' << endpoint.at("event_types") << '\n';
        // hooks.test(id, {{"event_type", "response.completed"}}) sends a delivery.
        // Verify exact request bytes before parsing; see ../resources.md for
        // signature verification, acknowledgement and duplicate handling.
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
