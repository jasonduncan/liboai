#pragma once
#include "liboai.h"
#include <chrono>
#include <thread>
#include <stdexcept>

inline constexpr const char* resource_root = "https://api.openai.com/v1";
inline void resource_auth() {
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY"))
        throw std::runtime_error("Set OPENAI_API_KEY before explicitly running a live example");
}
inline void polling_pause() { std::this_thread::sleep_for(std::chrono::seconds(2)); }
