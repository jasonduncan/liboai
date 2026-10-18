#include "liboai.h"
#include <stdexcept>

namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class Exception, class Function> void rejects(Function call) {
    try { call(); } catch (const Exception&) { return; }
    throw std::runtime_error("Expected webhook verification failure");
}
}
void test_webhooks() {
    const std::string body = R"({"id":"evt_fixture","type":"future.event","data":{"id":"resp_fixture"}})";
    const std::string secret = "whsec_c3ludGhldGljLXdlYmhvb2stc2VjcmV0";
    liboai::Headers headers{{"Webhook-ID", "evt_fixture"}, {"webhook-timestamp", "1700000000"},
        {"webhook-signature", "v1,tpMK2AQsiOg2VDMBtpnTDg33c5ysWLZZ27GEaVVXyEA="}};
    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(1700000000));
    const auto tolerance = std::chrono::seconds(300);
    using Invalid = liboai::exception::WebhookVerificationError;
    if (!liboai::webhook_verification_available()) {
        rejects<std::logic_error>([&] { liboai::unwrap_webhook(body, headers, secret, tolerance, now); });
        return;
    }
    const auto event = liboai::unwrap_webhook(body, headers, secret, tolerance, now);
    require(event["type"] == "future.event", "Unknown webhook event preserved");
    rejects<Invalid>([&] { liboai::unwrap_webhook(body + " ", headers, secret, tolerance, now); });
    rejects<Invalid>([&] { liboai::unwrap_webhook(body, headers, secret, tolerance, now + std::chrono::seconds(301)); });
    rejects<Invalid>([&] { liboai::unwrap_webhook(body, headers, secret, tolerance, now - std::chrono::seconds(301)); });
    liboai::verify_webhook_signature(body, headers, secret, tolerance, now + tolerance);
    liboai::verify_webhook_signature(body, headers, secret, tolerance, now - tolerance);
    // Independent Python hmac/hashlib vectors for non-text bytes and malformed JSON.
    const std::string binary("\0snowman \xe2\x98\x83\xff", 13);
    auto binary_headers = headers;
    binary_headers["webhook-signature"] = "v1,+ZzJ53t/5JYv6Fas8e/Kg36U2jG2VS9Jv1tkQjkJaqw=";
    liboai::verify_webhook_signature(binary, binary_headers, secret, tolerance, now);
    rejects<Invalid>([&] { liboai::verify_webhook_signature(binary.substr(1), binary_headers, secret, tolerance, now); });
    auto malformed_headers = headers;
    malformed_headers["webhook-signature"] = "v1,3eQMHZnBZjmoHBZJPuZl4vrZprBMdP/ssdmB4+XecE4=";
    liboai::verify_webhook_signature("{not json", malformed_headers, secret, tolerance, now);
    rejects<nlohmann::json::parse_error>([&] { liboai::unwrap_webhook("{not json", malformed_headers, secret, tolerance, now); });
    auto changed = headers;
    changed["webhook-signature"] = "v2,unknown v1,malformed " + headers["webhook-signature"];
    liboai::verify_webhook_signature(body, changed, secret, tolerance, now);
    changed["webhook-id"] = "different-id";
    rejects<Invalid>([&] { liboai::unwrap_webhook(body, changed, secret, tolerance, now); });
    changed = headers; changed["webhook-signature"] = "v1,AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
    rejects<Invalid>([&] { liboai::unwrap_webhook(body, changed, secret, tolerance, now); });
    changed = headers; changed.erase("webhook-timestamp");
    rejects<Invalid>([&] { liboai::unwrap_webhook(body, changed, secret, tolerance, now); });
    for (const auto stamp : {"bad", "1700000000x", "-1", "18446744073709551616", "18446744073709551615", "1700000000,1700000000"}) {
        changed = headers; changed["webhook-timestamp"] = stamp;
        rejects<Invalid>([&] { liboai::unwrap_webhook(body, changed, secret, tolerance, now); });
    }
    rejects<std::invalid_argument>([&] { liboai::unwrap_webhook(body, headers, "whsec_!bad", tolerance, now); });
    rejects<std::invalid_argument>([&] { liboai::unwrap_webhook(body, headers, secret, std::chrono::seconds(-1), now); });
}
