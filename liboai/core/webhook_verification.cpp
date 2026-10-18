#include "../include/core/webhook_verification.h"
#include <array>
#include <charconv>
#include <climits>
#include <sstream>

#if defined(LIBOAI_WEBHOOK_OPENSSL)
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#elif defined(__APPLE__)
#include <CommonCrypto/CommonHMAC.h>
#endif

namespace liboai {
namespace {
std::vector<unsigned char> decode_base64(const std::string& input) {
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (input.empty() || input.size() % 4) throw std::invalid_argument("Invalid base64");
    std::vector<unsigned char> result;
    for (size_t i = 0; i < input.size(); i += 4) {
        unsigned values[4]{};
        unsigned padding = 0;
        for (size_t j = 0; j < 4; ++j) {
            const char c = input[i+j];
            if (c == '=') {
                if (i + 4 != input.size() || j < 2) throw std::invalid_argument("Invalid base64 padding");
                ++padding;
            } else {
                auto pos = alphabet.find(c);
                if (padding || pos == std::string::npos) throw std::invalid_argument("Invalid base64");
                values[j] = static_cast<unsigned>(pos);
            }
        }
        if (padding > 2 || (padding == 2 && (values[1] & 15)) || (padding == 1 && (values[2] & 3)))
            throw std::invalid_argument("Invalid base64 padding");
        result.push_back(static_cast<unsigned char>((values[0] << 2) | (values[1] >> 4)));
        if (padding < 2) result.push_back(static_cast<unsigned char>((values[1] << 4) | (values[2] >> 2)));
        if (!padding) result.push_back(static_cast<unsigned char>((values[2] << 6) | values[3]));
    }
    return result;
}
std::array<unsigned char, 32> hmac_sha256(const std::vector<unsigned char>& key, const std::string& data) {
    std::array<unsigned char, 32> digest{};
#if defined(LIBOAI_WEBHOOK_OPENSSL)
    if (key.size() > INT_MAX) throw std::invalid_argument("Webhook secret is too large");
    unsigned int size = 0;
    if (!HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
              reinterpret_cast<const unsigned char*>(data.data()), data.size(), digest.data(), &size) || size != 32)
        throw std::runtime_error("Webhook HMAC operation failed");
#elif defined(__APPLE__)
    CCHmac(kCCHmacAlgSHA256, key.data(), key.size(), data.data(), data.size(), digest.data());
#else
    (void)key; (void)data;
    throw std::logic_error("Webhook verification requires LIBOAI_WEBHOOK_OPENSSL=ON on this platform");
#endif
    return digest;
}
bool matches(const std::array<unsigned char, 32>& expected, const std::vector<unsigned char>& supplied) {
    if (supplied.size() != expected.size()) return false;
#if defined(LIBOAI_WEBHOOK_OPENSSL)
    return CRYPTO_memcmp(expected.data(), supplied.data(), expected.size()) == 0;
#else
    volatile unsigned char difference = 0;
    for (size_t i = 0; i < expected.size(); ++i) difference = difference | (expected[i] ^ supplied[i]);
    return difference == 0;
#endif
}
}

bool webhook_verification_available() noexcept {
#if defined(__APPLE__) || defined(LIBOAI_WEBHOOK_OPENSSL)
    return true;
#else
    return false;
#endif
}
void verify_webhook_signature(const std::string& body, const Headers& headers,
                             const std::string& secret, std::chrono::seconds tolerance,
                             std::chrono::system_clock::time_point now) {
    if (!webhook_verification_available())
        throw std::logic_error("Webhook verification requires LIBOAI_WEBHOOK_OPENSSL=ON on this platform");
    if (tolerance.count() < 0) throw std::invalid_argument("Webhook tolerance must be nonnegative");
    auto required = [&](const char* name) -> const std::string& {
        auto found = headers.find(name);
        if (found == headers.end() || found->second.empty())
            throw exception::WebhookVerificationError("Missing webhook verification header");
        return found->second;
    };
    const auto& id = required("webhook-id");
    const auto& timestamp = required("webhook-timestamp");
    const auto& signatures = required("webhook-signature");
    uint64_t stamp = 0;
    auto parsed = std::from_chars(timestamp.data(), timestamp.data() + timestamp.size(), stamp);
    if (parsed.ec != std::errc() || parsed.ptr != timestamp.data() + timestamp.size())
        throw exception::WebhookVerificationError("Invalid webhook timestamp");
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    if (seconds < 0) throw std::invalid_argument("Webhook clock must be after the epoch");
    const uint64_t current = static_cast<uint64_t>(seconds);
    const uint64_t difference = current >= stamp ? current - stamp : stamp - current;
    if (difference > static_cast<uint64_t>(tolerance.count()))
        throw exception::WebhookVerificationError("Webhook timestamp outside tolerance");
    std::vector<unsigned char> key;
    try { key = decode_base64(secret.rfind("whsec_", 0) == 0 ? secret.substr(6) : secret); }
    catch (const std::invalid_argument&) { throw std::invalid_argument("Invalid webhook signing secret"); }
    const auto expected = hmac_sha256(key, id + "." + timestamp + "." + body);
    std::istringstream candidates(signatures);
    std::string candidate;
    bool valid = false;
    while (candidates >> candidate) {
        if (candidate.rfind("v1,", 0) != 0) continue;
        try { valid = matches(expected, decode_base64(candidate.substr(3))) || valid; }
        catch (const std::invalid_argument&) { /* Another rotated signature may be valid. */ }
    }
    if (!valid) throw exception::WebhookVerificationError("Invalid webhook signature");
}
nlohmann::json unwrap_webhook(const std::string& body, const Headers& headers, const std::string& secret,
                             std::chrono::seconds tolerance, std::chrono::system_clock::time_point now) {
    verify_webhook_signature(body, headers, secret, tolerance, now);
    return nlohmann::json::parse(body);
}
}
