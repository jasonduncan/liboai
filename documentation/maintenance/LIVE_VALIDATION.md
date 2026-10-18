# Live validation — October 17, 2026

The user supplied `OPENAI_API_KEY` in the local `.env` and authorized testing the modernization and extensions. The key was loaded afresh into child-process environments, never placed in command arguments or receipts. `.env` and local variants are now ignored by Git. Authentication through the compiled C++ library returned HTTP 200.

The normal and ASan/UBSan suites each passed all three CTests before the live checks. Existing example, installed-consumer and parent-consumer builds had already passed for this source state. No library implementation changed during these smoke tests; they exercised the current compiled library. The original live harnesses and sanitized receipts are retained under the ignored `build/live-smoke/` directory. A maintained [opt-in suite](LIVE_TESTS.md) now preserves these scenarios; this report remains evidence of the original run only.

## Results by API family

| Family | Live result | Evidence and limits |
| --- | --- | --- |
| Responses | Pass | GPT-5.2 strict structured output; GPT-5.2-pro async generation; retrieve, input items, input-token count, compaction; background HTTP SSE, output-text assembly and replay; cancellation and deletion |
| Chat Completions | Pass after visibility delay | Creation and parsed SSE passed immediately. Stored retrieve/update/messages/list/delete initially failed or returned no match, then all five passed on the same completion after a delay; deletion confirmed |
| Images | Pass | One low-quality `gpt-image-1.5` generation and one multipart edit with SSE; output bytes received |
| Audio | Pass | Legacy `speech_async` dispatch produced WAV; modern speech binary and SSE, transcription SSE, and translation multipart all succeeded on synthetic audio |
| Embeddings | Pass | Two short inputs, 16 dimensions, base64 encoding |
| Moderation | Pass | Two benign synthetic text inputs |
| Files | Pass with purpose restrictions | Upload/list/retrieve/deletion passed. Content and legacy download preserved an uploaded `batch` JSONL file exactly. OpenAI rejected content/download for both `assistants` and `user_data` purposes with HTTP 400 |
| Uploads | Pass | Cancellation and create → binary part → checked completion; resulting file deleted |
| Conversations | Pass | Conversation and item create/retrieve/update/list/delete operations; no live model continuation test |
| Vector Stores | Pass | Store/file CRUD, ingestion, content, search, file-batch create/retrieve/list/cancel; owned store deleted. Responses file-search workflow remains locally tested only |
| Containers | Pass | One 1 GB container, upload/reference/list/retrieve/content/delete file operations, container deletion |
| Batch | Pass for submission/cancellation | Create/retrieve/list/cancel succeeded; terminal cancellation confirmed after 606 seconds. Zero completed requests and no generated output/error files; completion/output workflow remains offline-only |
| Webhooks | List only | Live list succeeded. Endpoint mutations, secret rotation, signatures and delivery remain locally tested; no receiver was created or contacted |
| Decisions | Pass | One `gpt-6-luna` request with predicate, choice and score questions; all raw answers parsed into the expected typed variants |
| Skills | Pass after visibility delay | Create/retrieve/list, default and version ZIP content, version create/retrieve/list/delete, default update/restore and whole-skill deletion pass. Immediate version operations returned 404; a timed delayed workflow succeeded |

“Pass” covers the named synthetic scenario, not every model, input shape or overload. The wider sync/async route matrix, unknown fields, malformed output, timeout/proxy/header isolation, callback failures, UTF-8 fragmentation and cryptographic edge cases remain covered by offline fixtures.

## Visibility and service restrictions

Stored Chat creation returned a valid completion ID with `store: true`, but immediate retrieve/update/messages/delete calls returned 404 and a filtered list was empty. A later read of the **same ID** succeeded, followed by update, messages, filtered listing and deletion. This supports delayed resource visibility; it was not a wrapper route defect. No generation was repeated for the diagnostic.

Skills initially returned default version `"1"` and a successful new version `"2"`, but updating the default to `"2"` returned 404. A separate diagnostic with distinct version content confirmed direct skill retrieval/default ZIP download while lists were empty and version-specific retrieve/content/delete calls returned 404. The current documented ordinal-string contract matches the client implementation. The earlier created skills were deleted successfully. A final diagnostic waited 120.015 seconds after a new version was acknowledged. Retrieval of that same ordinal version then passed, as did skill/version listing, version ZIP download, default update/restore, version deletion and whole-skill deletion. The workflow completed in 122.71 seconds from version acknowledgement. This supports delayed visibility; it does not establish a universal readiness interval.

Files content availability depends on service purpose restrictions. The rejection messages explicitly prohibited downloading `assistants` and `user_data` files. A separate upload with purpose `batch` passed both `Files::content` and legacy `Files::download` with exact byte comparison, then was deleted. The custom-root fix and encoded-ID behavior remain validated by local fixtures, since the hosted service uses the standard API root.

The SDK does not add automatic retries or hide these errors. Applications should handle resource readiness explicitly; these tests do not establish a guaranteed visibility interval.

## Cleanup and final visibility checks

Responses and the stored Chat completion were deleted successfully. Created conversations/items, uploaded files, vector stores and containers were deleted, and disposable uploads were cancelled or completed. All three synthetic Skills bundles were deleted, including the version used by the delayed diagnostic. The sole batch reached terminal `cancelled` state; the final owned-resource receipt is empty. No pre-existing remote resource was changed.

Batch records have no delete operation; the intended terminal action is cancellation, followed by deletion of any generated files. The initial cancellation was accepted but remained `cancelling` during the first follow-up reads. The service timestamps then confirmed cancellation 606 seconds after it began. `request_counts` reported zero completed/failed/total requests, no output/error files were present, and `resources-owned.json` is now `[]`. The retained cancelled batch record is service history, not an active job.

## Request bounds

- Responses generation: GPT-5.2 requests capped at 64 output tokens; GPT-5.2-pro capped at 128. Compaction used a tiny two-message input; its endpoint has no output-token limit parameter.
- Chat: two generation calls, each capped at 64 output tokens. Subsequent diagnostics used only the original stored ID.
- Decisions: one synthetic input with three fixed questions; the response reported 392 input tokens and zero output tokens.
- Images: exactly two low-quality outputs, one generation and one edit.
- Audio: three tiny speech requests; transcription/translation used only generated synthetic audio.
- Batch: one `gpt-4.1-nano` request capped at 16 output tokens. No batch resubmission.
- Resources: small synthetic text/bundle files and one 1 GB container; only resources created by the tests were mutated or deleted. No hosted skill execution, external MCP/tool call, webhook delivery, real document upload, model training, deployment or publishing occurred.

Actual billing was not queried. Usage fields and model IDs were recorded where available; an exact dollar total is not claimed.

## Reproducible evidence

Tested static archive: `build/api-modernization/liboai/liboai.a`, SHA-256 `3e77bd824a51fbae447aff40a5660f551da900714449cc2f64cdd4b8cbe5fac8` (macOS arm64). The live runners linked this archive directly with system cURL and the existing nlohmann-json installation.

The ignored local harness directory contains `common.h`, a safe `.env` loader (`run.py`), source files, compiled executables and JSON receipts. It can be inspected without invoking it. Running a harness again makes new live requests and may create billable resources; none is part of default CTest.

| Receipt | Scope |
| --- | --- |
| `build/live-smoke/auth.json` | Authentication through the modern transport |
| `build/live-smoke/responses.json` | Responses lifecycle/generation/streaming and cleanup |
| `build/live-smoke/components.json` | Initial Chat, Images, Audio, Embeddings, Moderation results |
| `build/live-smoke/chat_diagnostic.json` | Delayed stored Chat success and confirmed deletion |
| `build/live-smoke/resources.json` | Resource routes and initial cleanup |
| `build/live-smoke/diagnostic.json` | Rejected user-data download and an intermediate batch status |
| `build/live-smoke/file_batch_diagnostic.json` | Successful batch-file content/download and deletion |
| `build/live-smoke/batch_settlement.json` | Later batch state and generated-file cleanup |
| `build/live-smoke/extensions.json` | Decisions and initial Skills results |
| `build/live-smoke/skills_diagnostic.json` | Skills list/version visibility investigation |
| `build/live-smoke/skills_delayed.json` | Timed Skills visibility check and cleanup |

Initial subagent runs were blocked by their network sandbox before HTTP contact (cURL error 6, HTTP 0). They created no remote resources and are not counted as API failures. The actual requests were executed through the authorized local environment. No retries were added to production code.

Linux/Windows builds were outside this live run and were subsequently qualified in [CI](CI.md). Older cURL backends, Azure credentials/service access, live webhook delivery, every model/parameter combination and semantic model quality remain unqualified. Agents and ChatKit were not implemented or exercised.
