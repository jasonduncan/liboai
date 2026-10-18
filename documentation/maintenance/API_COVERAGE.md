# API coverage and validation

Implementation date: October 17, 2026. The plan compared commit `69c8a81`; implementation started at `3707248`, which also includes the merged rerank changes. Their regression coverage is retained. [Public baseline](PUBLIC_API_BASELINE.json) records the implementation-baseline header hashes and local class size/alignment measurements.

The [main README grid](../../README.md#openai-api-coverage) compares major API families. The inventory below records the implementation and validation detail. Family availability was checked against the [official API reference](https://developers.openai.com/api/reference/overview). Update both grids when support changes.

States: **implemented** means a named library operation exists; **JSON pass-through** means the documented shape can be sent/retained without a dedicated builder; **partial** identifies an explicit limitation; **absent** means no implementation. None of these states establishes hosted access, model quality, or service availability.

| Surface | State | Source/local validation | Live validation |
| --- | --- | --- | --- |
| Responses creation + async + legacy raw callback | implemented | Existing examples compile; raw behavior retained | Modern creation + async pass with GPT-5.2 and GPT-5.2-pro; legacy callback locally tested |
| Responses retrieve/delete/cancel/input items/token count/compact + async | implemented | Every route, encoded IDs, repeated queries, pagination, background states | Lifecycle, token count and compaction pass; own responses deleted |
| Responses parsed HTTP SSE, resume query, output text/agent selection | implemented | Fragmented UTF-8/CRLF, multiline/unknown events, terminal states, callback stop/throw, ordered output | Create/background SSE and replay pass; agent-selection and cancellation edge cases local only |
| Structured outputs, tools/results, image/file inputs, tiers/cache/context controls | JSON pass-through | JSON preservation and compiling examples; model-dependent fields documented | Strict structured output passes; remaining feature combinations local/source only |
| Project/beta/extra headers, timeout, diagnostics | implemented | Header precedence/isolation/rejection; async snapshots; proxy/default timeout; binary/empty/JSON/non-JSON errors | Authentication/request IDs/error preservation pass; project/proxy/timeout boundaries local only |
| Chat modern requests/messages/tools/structured outputs/reasoning | JSON pass-through | Multiple tools, multimodal and unknown message fields preserved; large integer limits | GPT-5.2 modern JSON generation passes; broader combinations local only |
| Stored Chat CRUD/list/messages + parsed streaming | implemented | Every route and `[DONE]` stream fixtures | Pass after delayed visibility; immediate CRUD returned 404; SSE passes |
| Image generation/edit + parsed streaming | implemented | Explicit model, multiple multipart files, content types, byte preservation, base64/partial events | Low-quality gpt-image-1.5 generation and multipart edit SSE pass |
| Image quality/background/format/compression fields | JSON pass-through | Request field preservation | Low quality/PNG exercised; remaining options local only |
| Audio speech/transcription/translation + streaming | implemented | Async speech route fix, binary/raw audio, SSE, multipart buffer ownership | Legacy async speech, modern binary/SSE speech, transcription SSE and translation pass |
| Audio diarization/timestamps/chunking/instructions | JSON pass-through | Extensible multipart/JSON; model-specific constraints documented | Not specifically qualified live |
| Embedding batches/token arrays/dimensions/encoding | JSON pass-through | All input shapes, float/base64 outputs and unknown fields | Text batch, 16 dimensions and base64 encoding pass; token arrays local only |
| Moderation text batches and image/text inputs | JSON pass-through | Mixed input and category/unknown-field retention | Text batch passes; mixed image inputs local only |
| Files pagination/upload options/content/custom-root download | implemented | Two-page cursor traversal, filter encoding, binary file download | Batch-purpose content/download pass; assistants/user_data downloads rejected; custom root local only |
| Conversations and item lifecycle + async | implemented | Routes, durable two-turn fixture, item pagination | Conversation/item CRUD passes; model continuation local only |
| Batch + async | implemented | Routes, bounded state progression, output/error file workflow | Create/retrieve/list/cancel pass; terminal cancellation confirmed; completion/output workflow local only |
| Uploads + parts + async | implemented | Binary parts, ordering, duplicate IDs, part/total size limits, checked completion | Create/part/checked-complete and cancellation pass |
| Vector Stores, files, file batches/search + async | implemented | Routes; file upload → attach → ingestion polling → Responses file-search fixture | Resource CRUD, ingestion, search and file-batch lifecycle pass; Responses file-search local only |
| Containers + file lifecycle/content + async | implemented | Routes, multipart bytes and binary download | 1 GB container, files, binary content and cleanup pass |
| Webhook endpoint management + async | implemented | Endpoint CRUD/list/rotate/test routes | List passes; mutation/rotation/delivery tests remain local only |
| Webhook signature verification and parsing | partial | Implemented/tested with Apple CommonCrypto and optional OpenSSL; other default builds require a provider | Not run |
| Legacy Conversation, Chat, Azure, keys/proxies/timeouts | implemented | Conversation and rerank regressions, all legacy examples, header isolation; Azure source unchanged | No Azure service test |
| Edits/FineTunes hosted workflows | partial | Symbols retained, historical examples compile; retired endpoint behavior documented | Not applicable |
| Decisions create + async + optional typed answers | implemented | Raw fields, ordered predicate/choice/score/refusal/unknown answers, malformed known shapes, encoded errors and owning async snapshots | gpt-6-luna predicate/choice/score raw and typed answers pass |
| Skills lifecycle, immutable versions, bundle content + async | implemented | All routes, directory/ZIP multipart bytes, cursor pagination, binary download, errors, independent async ownership | All lifecycle/version/content routes pass after a timed visibility delay; see live report |
| Responses hosted-shell skill attachment | JSON pass-through | Stateful bundle/version/default/attachment/cleanup fixture retains pinned version after default changes; example compiles; no local skill runner | Local fixture only; no hosted shell execution |
| Models list/retrieve/delete | implemented | Existing source and examples compile; no new route fixtures | Not run |
| Custom voices and voice consents | absent | Audio speech/transcription/translation support does not cover these management routes | Not run |
| Responses WebSockets and Realtime/GPT-Live | absent | Separate transport/session extensions | Not run |
| Agents | absent | Excluded from the selected extension scope | Not run |
| ChatKit | absent | Deferred by user decision | Not run |
| Current Fine-tuning, Evals and Graders | absent | Legacy FineTunes does not implement current fine-tuning jobs | Not run |
| Administration, Usage/Costs, Safety and Content Provenance | absent | No dedicated clients | Not run |
| ResponseInput/schema/tool convenience builders | absent | Explicitly deferred; complete raw JSON path retained | Not run |

## Evidence and limits

A subsequent authorized [live validation run](LIVE_VALIDATION.md) used synthetic data and bounded generation. The Live validation column above records that run; it does not imply every overload, model, or field combination was exercised.

- Full C++17 build with current and historical examples on macOS arm64, AppleClang 21, nlohmann-json 3.12.0, system cURL 8.7.1.
- CTest: `conversation_history`, `api_modernization`, and `rerank_http` use only synthetic local data. The modernization executable includes transport, SSE, Responses, components, resources, webhook, Decisions, and Skills suites; each failed assertion produces a nonzero exit.
- AddressSanitizer + UndefinedBehaviorSanitizer build and all three CTests pass, including the optional OpenSSL webhook provider. Native CommonCrypto verification is covered by the regular build.
- Install-and-consume build/run passed. Parent-project and vendored-target checks are recorded in [validation commands](VALIDATION.md).
- All 20 measured existing class sizes/alignments match the implementation baseline on this toolchain. Existing public fields/signatures were retained. This is not a cross-toolchain ABI guarantee.
- Linux and Windows runtime/build validation has not been performed in this session. cURL older than 7.56's legacy form path was not exercised. Do not claim cross-platform release qualification from these results.
- The initial implementation checks were offline. Subsequent live smoke results and remaining service/cleanup limitations are recorded in [LIVE_VALIDATION.md](LIVE_VALIDATION.md). No publishing, deployment, or hosted-tool quality evaluation was performed.

A test fixture deliberately returns controlled responses; it proves client behavior and workflow wiring, not server semantics. Global authorization remains process-wide and must be configured before concurrent work. New independent resource clients own their async arguments; existing components retain their documented owner-lifetime requirement. See [request contracts](REQUEST_OPTIONS.md).
