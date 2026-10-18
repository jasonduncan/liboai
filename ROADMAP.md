# liboai Roadmap

This is a living backlog of improvements and ideas as we deepen our use of the library. The [OpenAI API modernization plan](documentation/maintenance/API_MODERNIZATION_PLAN.md) defines the implementation phases, compatibility rules, and acceptance criteria from the October 17, 2026 API comparison.

## Implemented

- Responses creation, lifecycle, async operations, parsed HTTP SSE, and output helpers
- Per-request project/beta headers, timeouts, and additive structured diagnostics
- Modern JSON/multipart APIs for Chat, Images, Audio, Embeddings, Moderation, and Files
- Conversations, Batch, Uploads, Vector Stores, Containers, and Webhook endpoint management
- Decisions creation and optional typed answers; Skills bundle/version management and downloads
- README feature comparison with detailed implementation/validation inventory
- Webhook verification through native Apple or optional OpenSSL cryptography
- Async speech dispatch, multipart content types, and custom-root file download fixes
- Offline HTTP workflow fixtures, compatibility examples, sanitizer and consumer-build checks

## Now

- Cross-platform build/runtime qualification on Linux and Windows
- Resolve remaining [live qualification gaps](documentation/maintenance/LIVE_VALIDATION.md), including resource visibility and externally hosted workflows
- Review the [coverage inventory](documentation/maintenance/API_COVERAGE.md) before release claims

## Next

- Select optional extensions only for concrete application demand
- Keep all existing APIs stable and update current examples alongside public changes

## Later

- Optional Responses WebSockets and Realtime/GPT-Live components
- ChatKit deferred; Agents excluded from the current scope
- ResponseInput, structured-output, and tool-definition convenience builders
- Performance pass on JSON construction and streaming

Tests, documentation, and examples accompany each public API change; they are not deferred to a later phase.

## Observations

- Raw JSON already accommodates many current Responses fields and tools; endpoint and transport gaps require implementation.
- Local Conversation history and server-side Conversations are distinct features.
- Retired endpoints stay source-compatible but must be labeled accurately; new work should follow current service availability.
- Maintaining compatibility is critical for existing users.
