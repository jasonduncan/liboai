# Modern component APIs

These additive interfaces retain the legacy methods and `Conversation` behavior. Modern JSON requests use distinct names to keep existing overloads and function pointers unambiguous. The payload is forwarded intact: model identifiers, message items, tool calls/results, multimodal inputs, unknown fields, and integer output limits are not narrowed to legacy types.

Every operation below also has an `_async` variant. Results use `ApiResult` (`response`, `headers`, `request_id()`, `locally_cancelled`), or `ParsedStreamResult` for parsed streams. Supply `RequestOptions` as the last argument for per-call headers, project, and timeout. Async methods copy JSON, query, options, and callback values before starting. The component must remain alive until the future completes; reference captures inside a caller's callback remain the caller's responsibility. Multipart async calls copy buffer bytes and part metadata, but referenced files must remain present and unchanged until completion.

| Component | Operations |
| --- | --- |
| ChatCompletion | `create_json(body, options)`, `create_stream(body, callback, options)`, `retrieve(id, options)`, `update(id, body, options)`, `remove(id, options)`, `list(query, options)`, `list_messages(id, query, options)` |
| Images | `create_json(body, options)`, `create_edit_multipart(parts, options)`, `create_stream(body, callback, options)`, `create_edit_stream(parts, callback, options)` |
| Embeddings, Moderations | `create_json(body, options)` |
| Audio | `speech_json(body, options)`, `speech_bytes(body, callback, options)`, `speech_stream(body, callback, options)`, `transcribe_multipart(parts, options)`, `translate_multipart(parts, options)`, `transcribe_stream(parts, callback, options)` |
| Files | `list_with_options(query, options)`, `create_multipart(parts, options)`, `content(id, options)` |

## Chat messages and stored completions

Use JSON `messages` for modern conversations. Preserve the complete assistant message, including its array of `tool_calls`, then append a `role: "tool"` message with the matching `tool_call_id` for each result. Preserve image/audio content parts and unknown fields when replaying. Calling the model does not execute tools. Legacy `Conversation` remains available for its existing text/function-call workflows; it is not a general modern-message container.

`create_json` accepts fields such as `tools`, `response_format`, `reasoning_effort`, `max_completion_tokens`, and `store` directly. Stored-completion retrieval, metadata updates, deletion, listing, and message listing are server operations; support depends on whether the completion was stored. `Query` is a vector of string pairs and supports filters/cursors without an SDK allowlist. [Chat API reference](https://developers.openai.com/api/reference/resources/chat/subresources/completions/methods/create)

## Images and multipart data

Generation JSON can include `model`, `quality`, `background`, `output_format`, and `output_compression` where the selected model supports them. Image output is retained in `response.raw_json`, including `data[*].b64_json`; the library does not decode or write it automatically.

Multipart methods take `netimpl::components::Multipart`. Use repeated `image[]` file parts for multiple edit inputs, `mask` for an optional mask, and text parts for options. This representation preserves field names and repeated fields. For nested multipart options use the API's bracket notation (for example `expires_after[seconds]`); it does not automatically flatten JSON. Files are uploaded as bytes, not encoded into JSON. [Image edit reference](https://developers.openai.com/api/reference/resources/images/methods/edit)

## Embeddings and moderation

Embedding `input` accepts a string, string batch, token array, or batch of token arrays. Include `dimensions` and `encoding_format` directly. Float vectors and base64 outputs remain untouched in the result. Moderation JSON accepts text, text batches, or supported typed image/text item arrays; category scores and added fields remain accessible. These wrappers validate transport structure, not model-specific parameter combinations. [Embeddings](https://developers.openai.com/api/reference/resources/embeddings/methods/create), [Moderations](https://developers.openai.com/api/reference/resources/moderations/methods/create)

## Audio and streaming

Transcription multipart fields can express `language`, `prompt`, repeated `timestamp_granularities[]`, `chunking_strategy`, and `known_speaker_names[]` / `known_speaker_references[]`. Select a model that supports the requested controls. For example, `whisper-1` supports verbose timestamps, while `gpt-4o-transcribe-diarize` supports diarization and chunking controls; those features are not universal. Translation uses its separate multipart route. [Transcription reference](https://developers.openai.com/api/reference/resources/audio/subresources/transcriptions/methods/create)

`speech_json` preserves binary audio in `response.content`. `speech_bytes` delivers raw byte chunks and forces `stream_format: "audio"`; callback boundaries are transport boundaries. `speech_stream` sets `stream_format: "sse"` and delivers complete parsed events; inspect each event's JSON for audio data. Use an SSE-capable model such as `gpt-4o-mini-tts`; `tts-1` and `tts-1-hd` do not support SSE. Instructions and output formats are JSON fields. [Speech reference](https://developers.openai.com/api/reference/resources/audio/subresources/speech/methods/create)

Chat/image JSON parsed-stream methods force `stream: true`; multipart parsed-stream methods replace any existing `stream` parts with `stream=true`. Event JSON and original data are preserved, including partial-image base64 data and unknown events. A callback returning false stops local receipt; it does not cancel server work. Transport failures throw. `ended` means the stream ended without a recognized terminal event; consult event data and status rather than treating all end-of-stream results as successful generation. See [Responses streaming contracts](../responses/README.md).

## Files and explicit pagination

`list_with_options` accepts `after`, `limit`, `order`, and `purpose` through `Query`. Traverse pages explicitly, inspecting `has_more` and `last_id`, with an application limit to avoid unbounded work. `create_multipart` supports `purpose`, file bytes, and upload options such as `expires_after[anchor]` and `expires_after[seconds]`. `content` returns original bytes with response diagnostics; legacy `download` now honors the component's configured API root. [Files reference](https://developers.openai.com/api/reference/resources/files/methods/list)

The examples under each component's `examples/modern_*.cpp` compile as ordinary examples and require an explicit key/model or filename to run. Default tests use a loopback fixture and make no paid calls. Offline tests establish serialization, routes, data preservation, and stream parsing; they do not establish account access or model behavior.
