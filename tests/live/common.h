#pragma once

#include "liboai.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <set>
#include <thread>

using J = nlohmann::json;
using namespace liboai;

// Test-only support. No retry or polling behavior is added to the SDK.
namespace live {
struct Owned {
    std::string type, id, parent;
    std::function<void()> cleanup;
};
inline std::string suite;
inline std::filesystem::path directory;
inline J records = J::array();
inline std::vector<Owned> owned;
inline unsigned failures = 0;
inline bool finished = false;

inline std::string redact(std::string value) {
    if (const char* key = std::getenv("OPENAI_API_KEY")) {
        const std::string secret(key);
        std::size_t at = 0;
        while (!secret.empty() && (at = value.find(secret, at)) != std::string::npos) {
            value.replace(at, secret.size(), "[REDACTED]");
            at += 10;
        }
    }
    return std::regex_replace(value, std::regex("sk-[A-Za-z0-9_.*-]+"), "[REDACTED]");
}
inline J sanitized(const J& value) {
    if (value.is_string()) return redact(value.get<std::string>());
    if (value.is_array()) {
        J result = J::array();
        for (const auto& item : value) result.push_back(sanitized(item));
        return result;
    }
    if (value.is_object()) {
        J result = J::object();
        for (auto it = value.begin(); it != value.end(); ++it)
            result[redact(it.key())] = sanitized(it.value());
        return result;
    }
    return value;
}
inline void persist() {
    if (directory.empty()) return;
    J pending = J::array();
    for (const auto& item : owned)
        pending.push_back({{"type", item.type}, {"id", item.id}, {"parent", item.parent}});
    const J receipt = sanitized({{"schema", 1}, {"suite", suite}, {"failures", failures}, {"finished", finished},
        {"pending_cleanup", pending}, {"records", records}});
    const auto temporary = directory / "receipt.tmp";
    std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
    out << receipt.dump(2) << '\n';
    out.flush();
    if (!out) throw std::runtime_error("Cannot write live-test receipt");
    out.close();
    // Keep the last complete ledger if writing a new checkpoint is interrupted.
    std::filesystem::rename(temporary, directory / "receipt.json");
}
}

inline RequestOptions opts() {
    RequestOptions result;
    result.timeout = std::chrono::seconds(45);
    return result;
}
inline void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
inline std::string artifact(const std::string& filename) {
    const std::filesystem::path path(filename);
    check(!live::directory.empty() && path == path.filename() && filename != "." && filename != "..",
          "Artifact must be a filename in the live-test output directory");
    return (live::directory / path).string();
}
inline void record(const std::string& name, const std::string& status, J details = J::object()) {
    if (status == "FAIL" || status == "HTTP_ERROR" || status == "RATE_LIMIT" || status == "TRANSPORT_ERROR")
        ++live::failures;
    const J row = live::sanitized({{"test", name}, {"status", status}, {"details", details}});
    live::records.push_back(row);
    live::persist();
    std::cout << row.dump() << std::endl;
}
template<class F> bool run(const std::string& name, F&& operation) {
    try { operation(); record(name, "PASS"); return true; }
    catch (const exception::ApiError& e) {
        record(name, "HTTP_ERROR", {{"http", e.result.response.status_code},
            {"error", e.error}, {"request_id", e.result.request_id()}});
    } catch (const exception::ApiRateLimited& e) {
        record(name, "RATE_LIMIT", {{"http", e.result.response.status_code}, {"error", e.error}});
    } catch (const exception::TransportError& e) {
        record(name, "TRANSPORT_ERROR", {{"curl_code", e.curl_code}, {"http", e.result.response.status_code}});
    } catch (const std::exception& e) {
        record(name, "FAIL", {{"message", e.what()}});
    } catch (...) { record(name, "FAIL", {{"message", "Unknown exception"}}); }
    return false;
}
inline void own(const std::string& type, const std::string& id,
                std::function<void()> cleanup, const std::string& parent = "") {
    check(!id.empty(), "Cannot track an empty resource ID");
    for (const auto& entry : live::owned) check(entry.id != id, "Duplicate resource ownership");
    live::owned.push_back({type, id, parent, std::move(cleanup)});
    live::persist();
}
inline void release(const std::string& id) {
    for (auto it = live::owned.begin(); it != live::owned.end();) {
        if (it->id == id) it = live::owned.erase(it);
        else ++it;
    }
    live::persist();
}

// Retry only reads of the same acknowledged resource. Never retries 429, 5xx,
// transport errors, creation, model generation, or destructive operations.
template<class F> void wait_for_readiness(const std::string& name, F&& read,
                                          int attempts = 13, int delay_seconds = 10) {
    check(attempts > 0 && attempts <= 61 && delay_seconds >= 0 && delay_seconds <= 15,
          "Invalid bounded readiness policy");
    for (int attempt = 0; attempt < attempts; ++attempt) {
        try { if (read()) return; }
        catch (const exception::ApiError& e) { if (e.result.response.status_code != 404) throw; }
        record(name, "WAIT", {{"attempt", attempt + 1}, {"max_attempts", attempts}});
        if (attempt + 1 < attempts) std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));
    }
    throw std::runtime_error(name + ": resource not ready within the bounded read policy");
}
inline void cleanup_owned() {
    std::set<std::string> attempted;
    for (;;) {
        auto it = std::find_if(live::owned.rbegin(), live::owned.rend(),
            [&](const live::Owned& item) { return !attempted.count(item.id); });
        if (it == live::owned.rend()) break;
        const auto item = *it; // Callbacks may register or release other resources.
        attempted.insert(item.id);
        try { run("cleanup." + item.type, [&] { item.cleanup(); release(item.id); }); }
        catch (...) {
            ++live::failures;
            std::cerr << "Cleanup diagnostics could not be persisted; inspect remote resource state.\n";
        }
    }
}
template<class F> int execute(const std::string& suite, F&& body) {
    live::suite = suite;
    live::directory.clear();
    live::records = J::array();
    live::owned.clear();
    live::failures = 0;
    live::finished = false;
    try { run("suite", [&] {
        const char* enabled = std::getenv("LIBOAI_LIVE_TESTS");
        check(enabled && std::string(enabled) == "1", "Live tests require explicit opt-in through tests/live/run.py");
        const char* output = std::getenv("LIBOAI_LIVE_OUTPUT");
        check(output && *output, "Missing live-test output directory");
        const auto directory = std::filesystem::absolute(output);
        std::filesystem::create_directories(directory);
        check(!std::filesystem::exists(directory / "receipt.json"), "Refusing to overwrite an existing receipt");
        live::directory = directory;
        check(Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY"), "Missing OPENAI_API_KEY");
        Authorization::Authorizer().SetMaxTimeout(45000);
        live::persist();
        body();
    }); } catch (...) {
        ++live::failures;
        std::cerr << "Live-test diagnostics could not be persisted; attempting owned-resource cleanup.\n";
    }
    cleanup_owned();
    live::finished = true;
    try { live::persist(); } catch (...) {
        ++live::failures;
        std::cerr << "Final receipt unavailable; inspect remote resource state.\n";
    }
    return live::failures || !live::owned.empty() ? 1 : 0;
}
