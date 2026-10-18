# Continuous integration

[The C++ CI workflow](../../.github/workflows/ci.yml) runs on pushes, pull requests and manual dispatch. Every job builds all current and historical examples, runs the three offline CTests, installs the library, and builds/runs both installed-package and parent-project consumers.

| Job | Configuration | What it adds |
| --- | --- | --- |
| Ubuntu 24.04 / GCC | Release, static, default crypto | Linux build and explicit unavailable-provider behavior |
| Ubuntu 24.04 / GCC | Release, shared, OpenSSL | Shared-library installation and optional Crypto dependency propagation |
| macOS 15 / AppleClang | Release, static, CommonCrypto | Native Apple webhook verification |
| Windows 2022 / MSVC | Release, static liboai, OpenSSL | vcpkg dependency discovery and multi-configuration consumer builds |
| Ubuntu 24.04 / Clang | Debug, static, OpenSSL, ASan + UBSan | Memory/undefined-behavior checks, including both consumer configurations |

Windows uses a pinned vcpkg checkout and its `x64-windows` dependencies; vcpkg copies dependency DLLs beside test/consumer executables. Windows shared **liboai** builds are not covered: their existing export declarations need a separate compatibility review. Linux and macOS packages come from the runner's package managers. Compiler and dependency versions are visible in the run's configuration logs.

## Local reproduction

Install CMake 3.21+, a C++17 compiler, Python 3, cURL development files and nlohmann-json first. Install OpenSSL development files when selecting its provider. Then run:

```sh
python3 tests/ci/run.py --build-dir build/ci-local
python3 tests/ci/run.py --build-dir build/ci-openssl --openssl ON
python3 tests/ci/run.py --build-dir build/ci-sanitized --config Debug --openssl ON --sanitizers
```

Use a separate directory per configuration. Extra configure settings can be repeated, for example `--cmake-arg=-DCMAKE_PREFIX_PATH=/path/to/dependencies`. `CMAKE_TOOLCHAIN_FILE` and `VCPKG_TARGET_TRIPLET` environment variables are forwarded to every configuration; CMake chooses the toolchain before compiler initialization. The legacy `VCPKG_CMAKE_PATH` setting and Windows `VCPKG_ROOT` environment fallback remain supported for top-level builds. Parent projects and explicitly selected toolchains retain control of their dependency setup.

The driver uses CTest for consumer execution, so executable suffixes, multi-configuration output directories and dependency runtime paths are handled by CMake. `--parallel` controls build concurrency (default 2). `--shared ON` tests a shared liboai build on platforms supporting its exports.

## Test and credential boundaries

Only synthetic loopback fixtures and consumers without network requests run in CI. Examples compile but do not execute. No API key or repository secret is needed; the driver never reads `.env` and removes provider API keys from child-process environments. No live API test, model spending, remote resource creation, package publication or deployment is part of this workflow.

Actions are pinned to full commit hashes, the workflow token has read-only repository access, and checkout credentials are not persisted. Each job retains only JUnit results, CTest logs and CMake configuration diagnostics for 14 days. The workflow does not upload `.env`, live-test receipts, source trees or installed packages. Independent jobs continue when another platform fails; a newer run on the same ref cancels the older run.

A workflow definition is not proof that a platform passes. Qualification must cite a successful run for the relevant revision. Historical [local validation](VALIDATION.md) and [live service validation](LIVE_VALIDATION.md) remain separate evidence.
