# Modernization validation record

Initial offline validation on October 17, 2026 used no real API credentials or billable requests. A later user-authorized [live validation run](LIVE_VALIDATION.md) is recorded separately. The source checkout was at `3707248dd6a95a93e559968e6bf7633ef6adbbe9`; implementation remains a working-tree change.

## Core and examples

```sh
cmake -S . -B build/api-modernization \
  -DLIBOAI_BUILD_TESTS=ON -DBUILD_EXAMPLES=ON -DLIBOAI_INSTALL=ON \
  -DCMAKE_PREFIX_PATH=/private/tmp/liboai-json-install
cmake --build build/api-modernization -j 6
ctest --test-dir build/api-modernization --output-on-failure
```

The dependency prefix is a pre-existing local nlohmann-json 3.12.0 installation, not a required hard-coded repository path. Use your installed dependency location elsewhere. Compiler: AppleClang 21.0.0.21000334, C++17, macOS arm64; system cURL 8.7.1. Python is used only by test fixtures. The fixtures bind loopback sockets; the final extension run completed in the current unrestricted local environment.

Result: all example targets compile, and `conversation_history`, `api_modernization`, and `rerank_http` pass. The public legacy signatures are exercised by existing examples and by `tests/consumer/main.cpp`; [baseline metadata](PUBLIC_API_BASELINE.json) preserves header hashes and 20 matching local class size/alignment measurements.

## Sanitizers and optional crypto

```sh
cmake -S . -B build/api-modernization-sanitized \
  -DLIBOAI_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH=/private/tmp/liboai-json-install \
  '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DLIBOAI_WEBHOOK_OPENSSL=ON -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl@3
cmake --build build/api-modernization-sanitized -j 6
ctest --test-dir build/api-modernization-sanitized --output-on-failure
```

Result: all three tests pass under ASan/UBSan, including fragmented streams, callbacks, futures, multipart snapshots, resource workflows, and signature verification. The regular build covers CommonCrypto; the sanitized build covers OpenSSL Crypto. Tests include independent Python HMAC vectors, binary bytes, signed malformed JSON, bad signatures, rotation, missing headers, timestamp boundaries, overflow, and replay-window rejection.

## Consumer integration

```sh
cmake --install build/api-modernization --prefix /private/tmp/liboai-modern-install
cmake -S tests/consumer -B build/api-modern-consumer-installed \
  '-DCMAKE_PREFIX_PATH=/private/tmp/liboai-modern-install;/private/tmp/liboai-json-install'
cmake --build build/api-modern-consumer-installed
build/api-modern-consumer-installed/consumer

cmake -S tests/consumer -B build/api-modern-consumer-parent \
  -DLIBOAI_SOURCE_DIR=/Users/jason/Development/Projects/liboai \
  -DLIBOAI_PARENT_DEPENDENCIES=ON \
  -DCMAKE_PREFIX_PATH=/private/tmp/liboai-json-install
cmake --build build/api-modern-consumer-parent
build/api-modern-consumer-parent/consumer

cmake -S tests/consumer -B build/api-modern-consumer-vendored \
  -DLIBOAI_SOURCE_DIR=/Users/jason/Development/Projects/liboai \
  -DLIBOAI_PARENT_ALIAS_TARGETS=ON \
  -DLIBOAI_JSON_SOURCE_DIR=/private/tmp/liboai-review-json
cmake --build build/api-modern-consumer-vendored
build/api-modern-consumer-vendored/consumer
```

All three consumer configurations build and run. The vendored check builds nlohmann-json from its existing local source and supplies an unnamespaced `libcurl` interface target backed by system cURL, exercising liboai's alias fallback. It does not rebuild vendored cURL from source. These small consumers never send network requests.

`git diff --check` passes. Linux/Windows builds, older cURL form APIs, shared-library ABI compatibility across toolchains, live model/service access, and deployment were not validated. These remain release-qualification boundaries, not evidence implied by the local passes.


## Decisions, Skills and feature comparison

The same normal build (including both new examples), ASan/UBSan build, install-and-consume, parent-dependency and vendored-target consumer configurations were rerun after registering `Decisions` and `Skills`. All three CTests pass in both build configurations; all three consumers build and run with the new public headers and typed Decisions helper. No new fields were added to the existing `OpenAI` aggregate.

Extension fixtures cover every sync/async endpoint, raw/unknown fields, Decisions variants and malformed typed answers, async client/options/buffer snapshots, skill/version pagination, encoded identifiers, binary ZIP downloads, and structured errors. The Skills workflow checks a version-pinned hosted-shell request independently of the mutable default pointer. These requests use synthetic loopback data only.

The README now contains the major-family feature matrix, linked to `API_COVERAGE.md`; local documentation links and `git diff --check` pass. Decisions and Skills have their own usage documents, compiled examples, and implementation plan. Agents remains excluded; ChatKit remains deferred. Live-service and cross-platform qualification limits above still apply.

## Subsequent live validation

The normal and ASan/UBSan three-test suites were rerun and passed before the live run. See [the live validation report](LIVE_VALIDATION.md) for synthetic service checks, observed visibility delays, purpose restrictions, and cleanup status. Live results do not replace the broader offline edge-case coverage.
