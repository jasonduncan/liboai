# Changelog

## 4.1.0 — 2026-10-18

First release of the maintained [jasonduncan/liboai fork](https://github.com/jasonduncan/liboai), continuing the historical 4.0.1 version line. This is a source release with additive APIs; existing public signatures remain available. Rebuild your application against the new version; binary compatibility across toolchains is not promised.

### Added

- Responses API lifecycle, async calls, HTTP SSE streaming, and raw JSON requests for structured outputs, tools, and multimodal inputs.
- Request options, per-request timeouts and headers, structured API errors, and response/request diagnostics.
- Modern JSON methods for Chat Completions, stored completions, Images, Audio, Embeddings, Moderation, and Files.
- Conversations, multipart Uploads, Batch, Vector Stores, Containers, and webhook management and signature verification.
- Decisions typed parsing helpers and Skills bundle/version management.
- Rerank support for compatible servers (not an OpenAI endpoint).
- API coverage grids, usage examples, offline HTTP regression fixtures, and an explicitly opt-in live regression suite.
- Cross-platform CI covering Linux static/shared builds, macOS static builds, Windows static builds, sanitizers, installed packages, and parent-project integration.

### Fixed

- Async speech dispatch, multipart request handling, custom-root downloads, and conversation-history bounds.
- MSVC JSON/optional conversions, Windows macro collisions, and CMake toolchain initialization and dependency target handling.

### Scope and limitations

- Requires C++17, CMake 3.21+, cURL, and nlohmann-json. Release archives do not bundle dependencies or prebuilt binaries.
- Windows shared liboai exports remain unqualified; use a static liboai build on Windows. Linux shared builds are covered by CI.
- Webhook signature verification uses CommonCrypto on Apple platforms or opt-in OpenSSL elsewhere.
- JSON pass-through does not establish live coverage of every model, field, or tool. See [coverage](documentation/maintenance/API_COVERAGE.md) and the separate [live validation report](documentation/maintenance/LIVE_VALIDATION.md).
- Legacy interfaces remain for source compatibility; some service endpoints are retired. Azure Responses and Realtime/WebSocket support are absent; Agents are excluded and ChatKit is deferred.
- The legacy `Files::download` path does not propagate the configured request timeout. Use the bounded `Files::content` path when timeout control is required.
- vcpkg packaging is deferred.

See [building from a release](README.md#releases) and the [release procedure](documentation/maintenance/RELEASING.md).
