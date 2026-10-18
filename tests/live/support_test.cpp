#include "common.h"

namespace {
void env(const char* name, const std::string& value) {
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}
J receipt(const std::filesystem::path& path) {
    std::ifstream input(path / "receipt.json");
    J value; input >> value; return value;
}
}
int main() {
    const auto root = std::filesystem::temp_directory_path() /
        ("liboai-live-contract-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Remove { std::filesystem::path path; ~Remove() { std::error_code e; std::filesystem::remove_all(path, e); } } remove{root};
    try {
        const std::string secret = "synthetic-secret-\"quoted\\value";
        env("OPENAI_API_KEY", secret);
        env("LIBOAI_LIVE_TESTS", "");
        bool invoked = false;
        check(execute("guard", [&] { invoked = true; }) != 0 && !invoked, "Missing opt-in executed body");
        env("LIBOAI_LIVE_TESTS", "1");
        env("LIBOAI_LIVE_OUTPUT", (root / "failure").string());
        std::vector<std::string> order;
        const auto code = execute("failure", [&] {
            own("parent", "parent", [&] { order.push_back("parent"); });
            own("child", "child", [&] {
                order.push_back("child");
                own("generated", "generated", [&] { order.push_back("generated"); });
            });
            record("redact", "INFO", {{secret, J::array({secret, "sk-synthetic-fallback"})}});
            throw std::runtime_error(secret);
        });
        check(code != 0, "Thrown assertion returned success");
        check(order == std::vector<std::string>({"child", "generated", "parent"}), "Cleanup order or dynamic ownership broken");
        const auto failed = receipt(root / "failure");
        check(failed.at("pending_cleanup").empty(), "Successful cleanup remained pending");
        check(failed.at("failures").get<int>() > 0, "Failure not recorded");
        const auto serialized = failed.dump();
        check(serialized.find("synthetic-secret") == std::string::npos && serialized.find("sk-synthetic") == std::string::npos,
              "Receipt leaked credential text");

        env("LIBOAI_LIVE_OUTPUT", (root / "pending").string());
        check(execute("pending", [] {
            own("file", "file-synthetic", [] { throw std::runtime_error("Synthetic cleanup failure"); });
        }) != 0, "Pending cleanup returned success");
        check(receipt(root / "pending").at("pending_cleanup").size() == 1, "Pending ID lost");
        const auto before = receipt(root / "pending");
        check(execute("pending", [] { throw std::runtime_error("Must not run"); }) != 0, "Existing receipt accepted");
        check(receipt(root / "pending") == before, "Existing receipt overwritten");

        env("LIBOAI_LIVE_OUTPUT", (root / "poll").string());
        check(execute("poll", [] {
            int attempts = 0;
            wait_for_readiness("synthetic", [&] { return ++attempts == 3; }, 3, 0);
            check(attempts == 3, "Wrong readiness attempt count");
            int failures = 0;
            try { wait_for_readiness("error", [&]() -> bool { ++failures; throw std::runtime_error("non-retryable"); }, 3, 0); }
            catch (const std::runtime_error&) {}
            check(failures == 1, "Non-readiness error retried");
        }) == 0, "Readiness contract failed");

        env("LIBOAI_LIVE_OUTPUT", (root / "write-error").string());
        bool cleaned = false;
        check(execute("write-error", [&] {
            own("synthetic", "owned-before-disk-error", [&] { cleaned = true; });
            std::filesystem::remove(root / "write-error/receipt.json");
            std::filesystem::create_directory(root / "write-error/receipt.json");
            record("disk-error", "INFO");
        }) != 0 && cleaned, "Receipt failure prevented cleanup");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << live::redact(error.what()) << '\n';
        return 1;
    }
}
