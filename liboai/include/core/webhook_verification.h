#pragma once
#include "request.h"

namespace liboai {
namespace exception {
class WebhookVerificationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
}

// CommonCrypto on Apple; opt-in OpenSSL elsewhere. No cryptography is reimplemented.
LIBOAI_EXPORT bool webhook_verification_available() noexcept;
LIBOAI_EXPORT void verify_webhook_signature(const std::string& raw_body, const Headers& headers,
    const std::string& secret, std::chrono::seconds tolerance = std::chrono::seconds(300),
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now());
LIBOAI_EXPORT nlohmann::json unwrap_webhook(const std::string& raw_body, const Headers& headers,
    const std::string& secret, std::chrono::seconds tolerance = std::chrono::seconds(300),
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now());
}
