# OpenAI API modernization plan

Status: core implementation completed locally for Phases 0–4 on October 17, 2026; cross-platform and live-service qualification remain open. Original comparison baseline: commit `69c8a81f2904c345c198134d7ee081013df02233`. Implementation baseline: `3707248dd6a95a93e559968e6bf7633ef6adbbe9`, preserving the later rerank merges. See [coverage and evidence](API_COVERAGE.md) and [reproduction commands](VALIDATION.md).

Modernize liboai in small releases while preserving existing C++ APIs. Complete Responses support first, repair outdated interfaces, and then add the resource APIs needed for practical applications. Treat Realtime, GPT-Live, Decisions, and Agents as separate extensions with their own completion criteria.

This plan follows a source comparison against the official OpenAI API. Request serialization support is not evidence of a successful live request or account access. The starting-point table below records the pre-implementation comparison; current state and explicit limitations are in the coverage inventory.

## Original starting point

| Area | Verified local state | Planned change |
| --- | --- | --- |
| Responses | Raw JSON and parameter-based creation, async variants, raw streaming callback | Lifecycle methods, parsed streaming, output helpers, request options |
| Chat Completions | Legacy function calling and fixed request fields | Modern request overload and tool-call handling |
| Images | Generation/edit/variation methods without model selection | Explicit model and current generation/edit controls |
| Audio | Basic transcription, translation, and speech | Fix async speech dispatch; extend options and streaming |
| Embeddings and moderation | Single text input | Batch/multimodal input as appropriate and current options |
| Files | Basic operations; list takes no arguments | Pagination/filtering; separate Uploads component |
| Other resources | No dedicated components | Conversations, Batch, Vector Stores, Containers, Webhooks |
| Legacy endpoints | FineTunes uses `/fine-tunes`; Edits uses `/edits` | Preserve symbols and clearly document retired service behavior |

Source entry points: [component inventory](../../liboai/include/liboai.h), [Responses](../../liboai/components/responses.cpp), [Chat](../../liboai/components/chat.cpp), [Images](../../liboai/components/images.cpp), [Audio](../../liboai/components/audio.cpp), and [test registration](../../tests/CMakeLists.txt).

Responses already forwards arbitrary JSON. New model IDs and supported JSON options generally need examples and validation, not a new model-specific C++ class. Structured outputs and built-in tools can already be represented this way. Resource management and new transports still require implementation. [Responses API](https://developers.openai.com/api/reference/resources/responses/methods/create)

## Compatibility and scope rules

- Retain existing signatures, default behavior, return types, and exported symbols. Keep Chat Completions and Azure behavior covered by regression checks; Azure Responses remains out of scope.
- Add named operations or overloads with options objects instead of lengthening positional argument lists. Check overload resolution for existing calls, brace initialization, callbacks, and function pointers; use distinct method names where overloads would introduce ambiguity.
- Preserve arbitrary JSON fields and unknown response/event types. Avoid a hard-coded model allowlist or universal parameter rules that become stale as models change.
- Keep JSON request data separate from transport options. Define header precedence, query serialization, timeout behavior, and async ownership before adding public APIs.
- Maintain C++17 and existing dependencies for the core work. Evaluate a transport dependency only when the existing cURL configuration cannot meet a required capability.
- Source compatibility is the immediate requirement. Assess binary compatibility before changing public class layout; do not advertise ABI compatibility without checking it.
- Do not automatically retry billable creation requests or replay tool actions. A future retry policy must define operation safety explicitly.
- Keep `ResponseInput` and tool/schema builders optional and deferred. Preserve raw JSON as a complete request path.

## Phase 0 — Correctness and documentation baseline

1. Fix `Audio::speech_async` to dispatch to `speech`, with a local HTTP regression test proving it sends the speech payload to `/audio/speech`.
2. Record the existing public surface and representative consumer builds. Extend the current opt-in CTest setup with a loopback HTTP fixture that captures requests and returns controlled JSON, binary data, errors, and fragmented streams. Keep test-only infrastructure out of runtime dependencies.
3. Update current examples and getting-started documentation. Move retired endpoint examples into clearly labeled historical usage; do not silently redirect legacy methods to different APIs.
4. Correct the Responses `seed` documentation: it is not a documented Responses creation field. Retain the signature for compatibility and avoid silently dropping explicit caller data; new examples should omit it.
5. Refresh the coverage inventory with four states: implemented, JSON pass-through, partial, absent. Record source validation and live validation separately.

Acceptance: the async speech regression passes; existing examples still compile; documentation distinguishes supported workflows from historical interfaces. The initial Responses implementation is accurately recorded as complete.

Deprecation policy: keep legacy C++ symbols, but do not promise that retired server endpoints work. Recheck the official retirement schedule before changing example models. Assistants and Videos are retired; Evals and self-serve fine-tuning have published wind-down schedules, so they are not expansion priorities. [OpenAI deprecations](https://developers.openai.com/api/docs/deprecations)

## Phase 1 — Shared request options and diagnostics

Add an opt-in request-options API, initially used by new Responses operations, then reused by other components.

- Support per-request project selection, extra headers, and timeouts without mutating the global authorizer during a request. Preserve the existing authorization configuration path.
- Define case-insensitive header merging. Reject malformed header names/values and make credential/content-type override behavior explicit. Request-specific beta headers must not leak into unrelated calls or Azure requests.
- Snapshot options used by asynchronous work. Document that existing async operations require the owning component to remain alive; do not introduce an unrelated ownership rewrite.
- Provide additive access to request ID, response headers, and structured API error fields. Choose an additive result/error interface after reviewing exported layouts and exception compatibility.
- Preserve binary and empty successful responses. Include HTTP status and safe API context in failures without exposing authorization headers or request bodies.

Acceptance: loopback tests verify project/beta headers, precedence, async option isolation, JSON and non-JSON errors, empty responses, and binary speech. Existing key/proxy/timeout and Azure paths remain unchanged when no new options are supplied.

This enables explicit project routing and beta features. Responses multi-agent requires `OpenAI-Beta: responses_multi_agent=v1`; sending only its JSON body is insufficient. [Authentication](https://developers.openai.com/api/reference/overview), [multi-agent requirements](https://developers.openai.com/api/docs/guides/responses-multi-agent)

## Phase 2 — Complete Responses support

### Resource lifecycle

Add synchronous and asynchronous methods following existing naming conventions:

| Proposed operation | HTTP operation |
| --- | --- |
| `retrieve` | `GET /responses/{response_id}` |
| `remove` | `DELETE /responses/{response_id}` |
| `cancel` | `POST /responses/{response_id}/cancel` |
| `list_input_items` | `GET /responses/{response_id}/input_items` |
| `count_input_tokens` | `POST /responses/input_tokens` |
| `compact` | `POST /responses/compact` |

Support documented query options, pagination, and retrieval streaming/resume parameters. Encode path identifiers and query values correctly. Accept raw JSON for new POST payloads. Include an example that creates a background response, polls its terminal state, and handles cancellation; keep polling explicit and bounded.

### Streaming and output

- Add an incremental SSE parser that handles split lines, CRLF, multiple events per chunk, comments, multiline data, and a final incomplete frame according to the SSE contract.
- Expose event type and original JSON, with optional convenience access to text deltas. Preserve unknown events instead of rejecting them.
- Distinguish completion, failure, incomplete output, transport failure, and local callback cancellation. Stopping a callback is not server-side cancellation.
- Keep the existing raw callback path. A new parsed-stream method should set `stream: true` consistently without changing legacy raw-overload behavior.
- Add an output-text helper that reads assistant message `output_text` parts in order, tolerates tool-only responses, and leaves refusals, annotations, and other items accessible through raw JSON.
- Document full-item replay and `previous_response_id` so users do not discard reasoning or tool items when continuing work.

### Current options and examples

Demonstrate structured outputs, function-call/result exchange, image/file inputs, service tiers, cache controls/diagnostics, background mode, and server-side context management through JSON. Add a beta multi-agent example after header support exists, with agent-aware output selection. Do not imply that emitting a shell/computer/function tool call executes it locally.

Acceptance: fixtures cover every new route, pagination, background states, interleaved tool/text events, fragmented UTF-8, terminal errors, and unknown fields. Examples compile. Explicit compaction is documented separately from `context_management` on creation. [Responses lifecycle](https://developers.openai.com/api/reference/python/resources/responses), [token counting](https://developers.openai.com/api/reference/python/resources/responses/subresources/input_tokens/methods/count), [cache diagnostics](https://developers.openai.com/api/docs/guides/prompt-caching/diagnostics)

## Phase 3 — Modernize existing components

Ship these as independently reviewable changes once shared request options are available.

| Component | Work | Acceptance evidence |
| --- | --- | --- |
| Chat Completions | Raw JSON request path; modern tool calls/results; structured outputs, reasoning and output limits; current optional fields; stored-completion CRUD/list/messages | Preserve legacy `Conversation` behavior; round-trip multiple tool calls and multimodal messages; retain unknown message fields |
| Images | JSON generation requests and extensible multipart edits; explicit model, quality, background, output format/compression; multiple edit inputs; parsed streaming | Model is serialized; multipart files remain intact; base64 output and partial-image events handled; legacy signatures still compile |
| Embeddings | Strings, string batches, token arrays; dimensions and encoding format | Input shapes preserved; float/base64 responses retained; old single-string calls unchanged |
| Moderation | Text batches and supported image/text item arrays | Mixed input serialized correctly; category scores and unknown fields preserved |
| Audio | Extensible transcription options, timestamps, speaker/chunking controls, multilingual hints; speech instructions and streaming | Binary responses preserved; stream parser tested; model-specific examples match current reference |
| Files | Cursor pagination, purpose/order/limit filters, supported upload options; honor configured root for content download | Multi-page traversal, encoded queries, and custom-root downloads verified locally |

Do not represent all modern chat messages through the old text-only helper. Raw messages must remain usable even before richer convenience methods exist. For large output limits, use suitable integer widths in new APIs rather than inheriting the legacy Chat `uint16_t` limit.

References: [Chat](https://developers.openai.com/api/reference/resources/chat/subresources/completions/methods/create), [Images](https://developers.openai.com/api/reference/resources/images/methods/generate), [Embeddings](https://developers.openai.com/api/reference/resources/embeddings/methods/create), [Moderation](https://developers.openai.com/api/reference/python/resources/moderations/methods/create), [Transcription](https://developers.openai.com/api/reference/python/resources/audio/subresources/transcriptions/methods/create), [Speech](https://developers.openai.com/api/reference/python/resources/audio/subresources/speech/methods/create), [Files](https://developers.openai.com/api/reference/resources/files/methods/list).

## Phase 4 — Supporting resource APIs

| Order | Component | Scope |
| --- | --- | --- |
| 1 | Conversations | Create/retrieve/update/delete; item create/retrieve/delete/list; examples with Responses |
| 2 | Batch | Create/retrieve/list/cancel; input/output/error files; explicit bounded polling |
| 3 | Uploads | Create, upload parts, complete, cancel; part ordering and file-size validation |
| 4 | Vector Stores | Store CRUD/search; files and file batches; ingestion status and pagination |
| 5 | Containers | Container and file lifecycle/content for hosted tools |
| 6 | Webhooks | Endpoint management and signature verification against exact raw request bytes; event parsing and duplicate-delivery guidance |

Reuse transport and pagination helpers, but keep each component independently usable. Add sync/async methods, documentation, one practical example, and fixture coverage with each component. Webhook verification must evaluate existing cryptographic facilities before proposing a dependency; avoid inventing cryptographic primitives.

Acceptance: demonstrate file upload → vector-store ingestion → Responses file search, a batch completion workflow, and durable conversation continuation. Offline fixtures establish request/response behavior; optional live smoke checks establish account-specific service access. [API resource inventory](https://developers.openai.com/api/reference/overview)

## Phase 5 — Optional platform extensions

These are separate projects after the core modernization, selected by application demand. They are not prerequisites for the first release.

| Extension | Required design work |
| --- | --- |
| Responses WebSockets | Connection ownership, reconnect/resume limits, backpressure, cancellation, and mid-turn steering; avoid duplicate submissions |
| Realtime and GPT-Live | Sessions, credentials, audio formats, bidirectional events, tools/delegation; define whether the C++ scope is WebSocket-only before considering WebRTC/SIP |
| Decisions | Implemented in the [selected extension](DECISIONS_SKILLS_PLAN.md); no undocumented beta header inserted |
| Agents | Excluded from this implementation |
| Skills and ChatKit | Skills implemented in the [selected extension](DECISIONS_SKILLS_PLAN.md); ChatKit deferred |
| Administration and safety resources | Separate admin credentials and client configuration from ordinary inference; prioritize usage/cost visibility and required safety event retrieval |

Recheck maturity, access requirements, and deprecations before starting each extension. Assess cURL WebSocket availability across supported builds before selecting a transport. Keep optional dependencies out of the default installation. [API changelog](https://developers.openai.com/api/docs/changelog)

## Delivery and validation

Recommended first implementation slice: Phase 0, then request options, then Responses retrieve/cancel/remove. Follow with Responses SSE/output helpers and the remaining lifecycle methods. Phase 3 components can ship individually after Phase 1; Phase 4 workflows depend on the applicable earlier components.

For every public API change, update its documentation and examples in the same change. Validate behavior through the public library against the local HTTP fixture, rather than testing only JSON-builder internals.

```sh
cmake -S . -B build/api-modernization -DLIBOAI_BUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build/api-modernization
ctest --test-dir build/api-modernization --output-on-failure
```

Use focused sanitizer runs for parser, async, and ownership changes. Verify install-and-consume and `add_subdirectory` builds for public header/CMake changes, including available imported and vendored dependency configurations. Run relevant builds on macOS, Linux, and Windows before claiming platform coverage.

Live smoke tests must be opt-in, use synthetic inputs, specify a model and small budget, and clean up created resources. Never require a key or incur model charges in the default test suite. Publishing, pushing, or spending on live validation is not part of writing this plan.

A release is ready when the selected phase's acceptance criteria pass, existing consumer examples compile, public docs describe its limitations, and the coverage table records exactly what was validated. A local fixture pass does not establish hosted-tool quality or model behavior.

## Completion tracking

- [x] Phase 0: correctness and documentation baseline — implemented and locally validated; see coverage limits
- [x] Phase 1: shared request options and diagnostics — implemented and locally validated; see coverage limits
- [x] Phase 2: complete Responses lifecycle and HTTP streaming — implemented and locally validated; see coverage limits
- [x] Phase 3: modernize existing components — implemented and locally validated; see coverage limits
- [x] Phase 4: supporting resource APIs — implemented and locally validated; see coverage limits
- [x] Phase 5 selection: Decisions and Skills implemented; see the [extension plan](DECISIONS_SKILLS_PLAN.md). Agents excluded and ChatKit deferred. Other extensions remain unimplemented.

Convenience builders such as `ResponseInput`, schema helpers, and tool builders remain a later usability increment. Add them after the underlying lifecycle and data-preservation contracts are stable.

### Implementation decisions

New operations return additive `ApiResult`/`ParsedStreamResult` values without changing `Response`, legacy exception, `Session`, or `OpenAI` layouts. New resource clients are independently constructible through `liboai.h`; existing components use distinctly named JSON/options methods to preserve overload resolution. No runtime JSON, HTTP, or streaming dependency was added. Webhook verification uses Apple CommonCrypto or opt-in OpenSSL Crypto; without a crypto provider it reports unavailable rather than accepting unchecked events.

The HTTP fixtures also exposed and now cover a pre-existing multipart content-type bug (an extra unnamed part), alongside the planned async speech/custom-root download fixes. Verification and release limitations, including untested Linux/Windows and live service behavior, are recorded separately from implementation completion.
