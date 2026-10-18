# Supporting resources

`Conversations`, `Batch`, `Uploads`, `VectorStores`, `Containers`, and `Webhooks` are independently constructible clients included by `liboai.h`. They reuse the configured `Authorization::Authorizer()` and accept an optional API root, defaulting to `https://api.openai.com/v1`. They do not add members to the existing `OpenAI` class.

```cpp
liboai::Conversations conversations("https://api.openai.com/v1");
liboai::RequestOptions options;
options.project = "proj_example";
auto created = conversations.create({{"metadata", {{"purpose", "example"}}}}, options);
auto id = created.response.raw_json.at("id").get<std::string>();
auto items = conversations.list_items(id, {{"limit", "20"}, {"order", "asc"}}, options);
```

Every operation has an `_async` counterpart returning `FutureApiResult`. Results expose `.response` (including complete `.raw_json` and binary-safe `.content`), `.headers`, and `.request_id()`. New resource clients copy their root and arguments into asynchronous work, so the client need not outlive its future. File-path uploads still require the file to remain available and unchanged until completion. Configure the shared authorizer before launching concurrent work and do not mutate it during requests.

JSON bodies preserve future/unknown fields; response status and unknown fields remain accessible without a schema conversion. `Query` is an ordered vector of string pairs, including repeated keys such as `include[]`. IDs and query values are encoded by the shared transport. Pass cursors exactly as received; do not pre-encode them. Pagination and polling are explicit. Cursor-list endpoints expose `has_more`/`last_id` and accept `after`; other endpoints can use different tokens, so follow the resource-specific reference. A creation call is never retried automatically.

## Conversations

Operations: `create(body, options)`, `retrieve(id, options)`, `update(id, body, options)`, `remove(id, options)`, `create_items(id, body, query, options)`, `retrieve_item(id, item_id, query, options)`, `remove_item(id, item_id, options)`, and `list_items(id, query, options)`.

Pass the retained conversation ID as `conversation` in a Responses JSON request to continue across sessions. Preserve full conversation items, including tool and reasoning items. Deleting the conversation does not delete its items; delete individual items according to your application's retention policy. The [continuation example](examples/conversation_continuation.cpp) demonstrates two turns and accepts an existing ID. [Conversation reference](https://developers.openai.com/api/reference/resources/conversations/methods/create), [conversation deletion](https://developers.openai.com/api/reference/resources/conversations/methods/delete).

## Batch

Operations: `create(body, options)`, `retrieve(id, options)`, `list(query, options)`, and `cancel(id, options)`. Upload a JSONL input using `Files` with purpose `batch`; creation includes `input_file_id`, `endpoint`, and `completion_window`. Inspect terminal states and both `output_file_id` and `error_file_id`, including on partially successful/cancelled jobs. Cancellation is asynchronous and does not undo already processed requests.

The [batch example](examples/batch_completion.cpp) uses `/v1/responses`, polls at most 30 times, saves output/error bytes through `Files::content`, and prints a retained ID for later resumption. It does not automatically delete diagnostic files. Each JSONL record should set a unique `custom_id`, `method: "POST"`, `url: "/v1/responses"`, and a small synthetic `body` with an explicitly selected model and output limit. [Batch reference](https://developers.openai.com/api/reference/resources/batches/methods/create).

## Uploads

Operations: `create(body, options)`, `add_part(id, bytes, options)`, `add_part_file(id, path, options)`, `complete(id, body, options)`, `complete_checked(id, ordered_parts, expected_bytes, extra, options)`, and `cancel(id, options)`.

`create` checks a positive integer `bytes` no larger than 8 GiB. Each part must be nonempty and no larger than 64 MiB. `add_part` owns a byte vector, including during asynchronous transfer. `add_part_file` reads a regular file as one part and detects short reads/size changes; it does not automatically split a file. Use fixed chunks for larger inputs.

Retain each returned part ID alongside its byte count. Assemble `UploadedPart` receipts in original file order, even if parallel transfers finish out of order. `complete_checked` rejects duplicate/empty IDs, oversized parts, and a sum differing from `expected_bytes`; the `extra` JSON permits `md5` and future fields but cannot replace `part_ids`. Raw `complete` checks only ID validity/order; the server verifies actual uploaded byte counts. Local receipts are caller-supplied evidence, not a server attestation. The final upload contains a usable `file` object. The [upload example](examples/upload_parts.cpp) implements bounded chunks, exact total checks, cancellation on failure, and completed-file cleanup. [Create upload](https://developers.openai.com/api/reference/resources/uploads/methods/create), [parts](https://developers.openai.com/api/reference/resources/uploads/subresources/parts/methods/create), [completion](https://developers.openai.com/api/reference/resources/uploads/methods/complete).

## Vector stores

Store operations: `create`, `retrieve`, `update`, `remove`, `list`, `search`. File operations: `create_file`, `retrieve_file`, `update_file`, `remove_file`, `list_files`, `file_content`. Batch operations: `create_file_batch`, `retrieve_file_batch`, `cancel_file_batch`, `list_file_batch_files`. All accept the relevant parent IDs first, then JSON bodies or query options, then `RequestOptions`; see [the public header](../../liboai/include/components/vector_stores.h) for exact signatures.

Store/file creation does not establish completed ingestion. Poll the file or batch `status`, handle failure/cancellation, and inspect per-file failures before using file search. Removing a store attachment does not delete its source file from the Files API. Search accepts raw JSON including `query`, filters, ranking options, and future fields. The [file-search example](examples/vector_store_file_search.cpp) uploads a synthetic document, attaches it, waits with a finite poll limit, sends a Responses `file_search` request, and cleans up the store and source file. [Vector store reference](https://developers.openai.com/api/reference/resources/vector_stores), [file search guide](https://developers.openai.com/api/docs/guides/tools-file-search).

## Containers

Operations: `create`, `retrieve`, `remove`, `list`, `create_file(id, body, options)`, `upload_file(id, path, options)`, `retrieve_file`, `remove_file`, `list_files`, and `file_content`. `create_file` sends JSON with an existing `file_id`; `upload_file` sends multipart file bytes. Content is returned without text conversion in `result.response.content`. Use binary writes and string length, never C-string termination, when saving it. Container lifetime and hosted-tool execution remain server managed. The [container example](examples/container_files.cpp) uploads, retrieves bytes, and removes the resources. [Container reference](https://developers.openai.com/api/reference/resources/containers), [file creation formats](https://developers.openai.com/api/reference/resources/containers/subresources/files/methods/create).

## Webhook endpoints

Operations: `create`, `retrieve`, `update`, `remove`, `list`, `rotate_secret(id, body, options)`, and `test(id, body, options)`. These manage `/webhook_endpoints` for the authenticated project. `create` and `rotate_secret` return a signing secret: store it securely and do not print the raw result. `test` actively sends a sample delivery; its returned `success` does not imply a successful receiver HTTP status, so inspect `status_code`. The [endpoint example](examples/webhook_endpoints.cpp) reads only safe metadata. [Webhook creation](https://developers.openai.com/api/reference/resources/webhooks/methods/create), [secret rotation](https://developers.openai.com/api/reference/resources/webhooks/methods/rotate_secret), [delivery test](https://developers.openai.com/api/reference/resources/webhooks/methods/test).

Verify signatures against the exact bytes read from the HTTP request before parsing JSON; reserialization changes the signed data. Enforce timestamp tolerance and persist verified event IDs with your action transaction so duplicate deliveries do not repeat work. Acknowledge promptly after durable acceptance, then process asynchronously. Unknown event types should be retained/ignored safely rather than treated as failed authentication. `verify_webhook_signature(raw_body, headers, signing_secret)` checks authenticity without parsing; `unwrap_webhook(raw_body, headers, signing_secret)` verifies first and returns the parsed JSON. Both default to a 300-second tolerance and throw `exception::WebhookVerificationError` on verification failure. Check `webhook_verification_available()` during startup; unavailable cryptographic backends fail closed. See [the public verification header](../../liboai/include/core/webhook_verification.h) for timestamp controls and the shared transport documentation for build configuration.

## Validation boundaries

`tests/resources_modern.cpp` exercises every resource route through the public library against the loopback HTTP fixture, checks JSON/query preservation, path encoding, binary multipart bytes, async ownership, upload order, and invalid upload sizes. Stateful synthetic fixtures also exercise ingestion-to-search, conversation continuation, batch terminal polling with output/error files, multipart assembly, and binary downloads. These tests establish local request behavior, not account access, hosted ingestion quality, model behavior, or live service acceptance. Examples compile by default with `BUILD_EXAMPLES=ON`; they only send requests after explicit `--live` arguments. Do not use real documents for smoke tests. Live examples can incur charges, and the batch/conversation examples intentionally retain IDs for resumption.
