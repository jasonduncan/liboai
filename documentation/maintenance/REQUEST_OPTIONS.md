# Request options and diagnostics

New APIs accept a `RequestOptions` value separately from JSON and query data. Existing overloads, return types, global authorizer configuration, and Azure calls remain available. New methods use names such as `create_with_options`, `create_json`, and `list_with_options` to avoid changing legacy overload resolution.

```cpp
liboai::OpenAI api;
if (!api.auth.SetKeyEnv("OPENAI_API_KEY")) return 1;
liboai::RequestOptions options;
options.project = "proj_your_project";
options.timeout = std::chrono::seconds(90);
options.headers["OpenAI-Beta"] = "responses_multi_agent=v1";
auto result = api.Responses->create_with_options(request_json, options);
std::cout << result.request_id() << '\n';
std::cout << result.response.raw_json << '\n';
```

This is opt-in transport configuration, not JSON sent to the model. For beta-specific payloads see the [Responses examples](../responses/README.md).

## Headers and timeouts

Header names are case-insensitive. Precedence is authorizer defaults, then extra request headers, then the dedicated `project` field. Extra headers can override organization selection. Header names must be HTTP tokens; control characters other than horizontal tab are rejected in values. Empty project IDs are rejected.

Credential headers (`Authorization`, `api-key`, `Proxy-Authorization`), `Content-Type`, `Host`, and HTTP framing/connection headers cannot be overridden through extra headers. Configure credentials through the authorizer. JSON operations use `application/json`; cURL supplies multipart boundaries. Explicit beta headers apply only to that call. Nothing mutates the authorizer or adds headers to Azure requests.

An omitted timeout inherits `Authorization::SetMaxTimeout`; explicit zero disables the total timeout, and a positive duration bounds the entire request in milliseconds, including streaming. Negative or unrepresentable values throw `std::invalid_argument`. Existing proxy and proxy-auth settings are reused. There are no automatic retries, resource polling, or tool execution.

`Query` is an ordered vector of string pairs. Keys and values are percent-encoded, spaces become `%20`, and repeated names are preserved (for example `include[]`). Values must already be their intended text form (`"true"`, `"20"`). Identifiers are encoded as single path segments; empty IDs and dot segments are rejected.

## Async ownership

New async methods copy JSON, options, query parameters, and callbacks before launching. Existing component methods (including new methods on existing components) require the component and `OpenAI` owner to outlive the future. The new independent resource clients copy their root into async work and may be destroyed after launching. Buffer-based multipart parts are deep-copied before launch. File paths are copied, but files must remain present and unchanged until completion. References captured inside user callbacks remain the caller's responsibility.

Configure the global authorizer before starting concurrent work; do not mutate its keys, proxies, or defaults while requests run. Per-request project/header/timeout overrides avoid that need.

## Results and failures

`ApiResult.response` retains the familiar `Response` fields; `ApiResult.headers` offers case-insensitive response-header lookup, and `request_id()` reads `x-request-id`. Repeated response-header values are comma-joined. Empty 2xx and binary responses remain successful. Binary bytes belong in `response.content`; `raw_json` is populated for JSON media types. Whitespace-prefixed JSON and JSON arrays are supported.

New error types preserve existing catch compatibility:

| Exception | Existing base | Additional data |
| --- | --- | --- |
| `exception::ApiError` | `OpenAIException` | `result`, `error` (original structured API error, if present) |
| `exception::ApiRateLimited` | `OpenAIRateLimited` | `result`, `error`, including response retry headers |
| `exception::TransportError` | `OpenAIException` | `result` with any received metadata, `curl_code` |
| `exception::ResponseParseError` | `OpenAIException` | `result` retaining invalid JSON and response metadata |

HTTP error `what()` strings include status but omit request bodies, credentials, and untrusted server text. Inspect structured fields explicitly when appropriate for your logging policy. Non-JSON failures retain status, headers, and body; malformed JSON on a successful JSON response raises `ResponseParseError`. These diagnostics do not change exception behavior on legacy overloads.

Raw callbacks receive successful response bytes. HTTP error bodies go to error diagnostics instead. Returning false stops the local transfer and sets `locally_cancelled`; it does not cancel a server job. Callback exceptions are rethrown after cURL returns. Stream bytes are delivered to callbacks, not accumulated in `response.content`. Parsed SSE helpers expose individual events and, for terminal Responses events, `terminal_response`. A stream ending without a recognized terminal event has status `ended`; transport failure throws separately.

## Compatibility boundary

No fields were added to existing public classes, including `OpenAI`, `Response`, `Session`, or legacy exceptions. New resources are independently constructible and included by `liboai.h`. All legacy examples remain compile checks. This is source compatibility evidence, not a promise of ABI compatibility across compilers, standard libraries, or build flags.

References checked October 17, 2026: [authentication and request headers](https://developers.openai.com/api/reference/overview), [Responses multi-agent requirements](https://developers.openai.com/api/docs/guides/responses-multi-agent).
