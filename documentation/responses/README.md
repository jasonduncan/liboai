<h1>Responses</h1>
<p>The <code>Responses</code> class is defined in <code>responses.h</code> at <code>liboai::Responses</code>, and its interface can be accessed through a <code>liboai::OpenAI</code> object.</p>

<p>This class provides access to the <a href="https://platform.openai.com/docs/api-reference/responses/create">Responses API</a>. It offers a typed <code>create</code> overload for common fields and a raw JSON overload for full flexibility.</p>

<h2>Methods</h2>
<p>This document covers the method(s) located in <code>responses.h</code>. You can find their function signature(s) below.</p>

<h3>Build a Request Payload</h3>
<p>Builds a Responses API request payload from typed parameters.</p>

```cpp
static nlohmann::json build_request(
  const std::string& model,
  const nlohmann::json& input,
  std::optional<std::string> instructions = std::nullopt,
  std::optional<nlohmann::json> reasoning = std::nullopt,
  std::optional<nlohmann::json> text = std::nullopt,
  std::optional<uint32_t> max_output_tokens = std::nullopt,
  std::optional<float> temperature = std::nullopt,
  std::optional<float> top_p = std::nullopt,
  std::optional<uint32_t> seed = std::nullopt,
  std::optional<nlohmann::json> tools = std::nullopt,
  std::optional<nlohmann::json> tool_choice = std::nullopt,
  std::optional<bool> parallel_tool_calls = std::nullopt,
  std::optional<bool> store = std::nullopt,
  std::optional<std::string> previous_response_id = std::nullopt,
  std::optional<nlohmann::json> include = std::nullopt,
  std::optional<nlohmann::json> metadata = std::nullopt,
  std::optional<std::string> user = std::nullopt,
  std::optional<std::string> truncation = std::nullopt,
  std::optional<bool> stream = std::nullopt
);
```

<h3>Create a Response</h3>
<p>Creates a response from typed parameters. Returns a <code>liboai::Response</code> containing response data.</p>

```cpp
liboai::Response create(
  const std::string& model,
  const nlohmann::json& input,
  std::optional<std::string> instructions = std::nullopt,
  std::optional<nlohmann::json> reasoning = std::nullopt,
  std::optional<nlohmann::json> text = std::nullopt,
  std::optional<uint32_t> max_output_tokens = std::nullopt,
  std::optional<float> temperature = std::nullopt,
  std::optional<float> top_p = std::nullopt,
  std::optional<uint32_t> seed = std::nullopt,
  std::optional<nlohmann::json> tools = std::nullopt,
  std::optional<nlohmann::json> tool_choice = std::nullopt,
  std::optional<bool> parallel_tool_calls = std::nullopt,
  std::optional<bool> store = std::nullopt,
  std::optional<std::string> previous_response_id = std::nullopt,
  std::optional<nlohmann::json> include = std::nullopt,
  std::optional<nlohmann::json> metadata = std::nullopt,
  std::optional<std::string> user = std::nullopt,
  std::optional<std::string> truncation = std::nullopt,
  std::optional<std::function<bool(std::string, intptr_t)>> stream = std::nullopt
) const & noexcept(false);
```

<h3>Create a Response (raw JSON)</h3>
<p>Creates a response from a raw JSON payload. Returns a <code>liboai::Response</code> containing response data.</p>

```cpp
liboai::Response create(
  const nlohmann::json& request,
  std::optional<std::function<bool(std::string, intptr_t)>> stream = std::nullopt
) const & noexcept(false);
```

<h3>Create a Response (async)</h3>
<p>Asynchronously creates a response from typed parameters. Returns a <code>liboai::FutureResponse</code> containing future response data.</p>

```cpp
liboai::FutureResponse create_async(
  const std::string& model,
  const nlohmann::json& input,
  std::optional<std::string> instructions = std::nullopt,
  std::optional<nlohmann::json> reasoning = std::nullopt,
  std::optional<nlohmann::json> text = std::nullopt,
  std::optional<uint32_t> max_output_tokens = std::nullopt,
  std::optional<float> temperature = std::nullopt,
  std::optional<float> top_p = std::nullopt,
  std::optional<uint32_t> seed = std::nullopt,
  std::optional<nlohmann::json> tools = std::nullopt,
  std::optional<nlohmann::json> tool_choice = std::nullopt,
  std::optional<bool> parallel_tool_calls = std::nullopt,
  std::optional<bool> store = std::nullopt,
  std::optional<std::string> previous_response_id = std::nullopt,
  std::optional<nlohmann::json> include = std::nullopt,
  std::optional<nlohmann::json> metadata = std::nullopt,
  std::optional<std::string> user = std::nullopt,
  std::optional<std::string> truncation = std::nullopt,
  std::optional<std::function<bool(std::string, intptr_t)>> stream = std::nullopt
) const & noexcept(false);
```

<h3>Create a Response (async, raw JSON)</h3>
<p>Asynchronously creates a response from a raw JSON payload. Returns a <code>liboai::FutureResponse</code> containing future response data.</p>

```cpp
liboai::FutureResponse create_async(
  const nlohmann::json& request,
  std::optional<std::function<bool(std::string, intptr_t)>> stream = std::nullopt
) const & noexcept(false);
```

<p>When using streaming, include <code>"stream": true</code> in the request (or pass a stream callback to the typed overload) and provide a stream callback to receive SSE data.</p>

<h2>Example Usage</h2>
<p>For example usage of the above function(s), please refer to the <a href="./examples">examples</a> folder.</p>
<p>Examples include <code>create_response.cpp</code> (raw JSON) and <code>create_response_typed.cpp</code> (typed parameters).</p>

## Modern lifecycle and diagnostics

Existing `create`, `create_async`, and `build_request` signatures and return types remain available. The historical `seed` parameter is retained and forwarded when explicitly provided; it is **not a documented Responses creation field** and should be omitted from new requests. No model allowlist or field filter is applied to JSON.

New methods return `ApiResult`; the ordinary response is `result.response`, headers are `result.headers`, and `result.request_id()` reads the request ID. Each method also has an `_async` counterpart returning a future. Arguments, queries, callbacks, and request options are copied for async work; keep the component alive until the future completes.

| Method | Request |
| --- | --- |
| `create_with_options(json, options, raw_callback)` | `POST /responses` |
| `retrieve(id, query, options, raw_callback)` | `GET /responses/{id}` |
| `remove(id, options)` | `DELETE /responses/{id}` |
| `cancel(id, json, options)` | `POST /responses/{id}/cancel` |
| `list_input_items(id, query, options)` | `GET /responses/{id}/input_items` |
| `count_input_tokens(json, options)` | `POST /responses/input_tokens` |
| `compact(json, options)` | `POST /responses/compact` |

Options default to `{}`, optional raw callbacks to `std::nullopt`, and cancellation's JSON body to an empty object. New POST operations preserve caller JSON. IDs are encoded as individual path segments. `Query` is an ordered vector of string pairs, so repeated parameters are supported without losing values. For retrieval, use `include[]`, `include_obfuscation`, `starting_after`, and `stream` as applicable. Input-item pagination accepts `after`, `before`, `limit`, `order`, and `include[]`; pass the returned cursor explicitly. Integer query values are decimal strings, without a 16-bit limit. [Retrieve reference](https://developers.openai.com/api/reference/resources/responses/methods/retrieve)

```cpp
liboai::RequestOptions options;
options.project = "proj_example";
options.timeout = std::chrono::seconds(60);
auto page = oai.Responses->list_input_items("resp_example",
    {{"limit", "20"}, {"order", "asc"}}, options);
if (page.response.raw_json.value("has_more", false)) {
    auto next = oai.Responses->list_input_items("resp_example",
        {{"after", page.response.raw_json.at("last_id").get<std::string>()},
         {"limit", "20"}, {"order", "asc"}}, options);
}
```

The compiled [modern workflows example](examples/modern_workflows.cpp) demonstrates bounded background polling and explicit server cancellation. Run it only deliberately with a model available to your account; it makes billable requests. No automatic retry or polling is hidden inside library methods. Background response states remain visible in raw JSON, including `queued`, `in_progress`, `completed`, `failed`, `incomplete`, and `cancelled`.

## Parsed HTTP streams

`create_stream(json, callback, options)` and `retrieve_stream(id, callback, query, options)` return `ParsedStreamResult`; their `_async` counterparts return futures. Creation copies the JSON and sets `stream: true`. Retrieval replaces any `stream` query entries with `stream=true`, preserving resume parameters such as `starting_after`. Raw creation methods retain their original behavior: callers supplying raw JSON and a raw callback must set `stream` themselves.

```cpp
auto stream = oai.Responses->create_stream(
    {{"model", "gpt-5.2"}, {"input", "Say hello."}},
    [](const liboai::SseEvent& event) {
        if (auto text = event.text_delta()) std::cout << *text;
        // event.json retains tools, annotations, agent attribution, and unknown fields.
        return true;
    });
if (stream.terminal_response)
    std::cout << liboai::Responses::output_text(*stream.terminal_response);
```

`SseEvent` exposes `type`, exact joined `data`, persistent SSE `id`, and parsed `json` (null for non-JSON). Unknown types and non-JSON data are delivered unchanged. The parser accepts split UTF-8 bytes, LF/CR/CRLF, a leading UTF-8 BOM, comments, multiple frames, and multiline `data`. Only a terminating blank line dispatches an event; `finish()` discards the trailing incomplete frame. `text_delta()` recognizes `response.output_text.delta` only. No reasoning/tool action is executed by this library.

`ParsedStreamResult.status` distinguishes `completed`, `failed`, `incomplete`, `locally_cancelled`, and `ended` (the HTTP stream ended without a recognized Responses terminal event). `terminal_response` retains terminal response JSON. Streaming bodies are delivered to the callback rather than accumulated in `result.response.content`; retain events yourself if you need a transcript. Transport/HTTP failures throw; inspect the structured request exception rather than interpreting them as a model terminal event. A callback exception propagates to the caller (or future). Returning false stops local receipt, sets `result.locally_cancelled`, and **does not cancel server generation**; call `cancel` explicitly for a cancellable background response. An `ended` stream must not be treated as successful model completion. The underlying `SseParser` and parsed transport are shared with Images and Audio; their endpoint-specific completion payloads remain available in events.

`Responses::output_text(json, optional_agent_name)` joins assistant message `output_text` parts in order, returning an empty string for tool-only responses. It does not modify the JSON or conceal refusals, annotations, or other items. The optional selector matches `item.agent.agent_name`; pass `"/root"` for root-agent text in multi-agent responses.

## Complete inputs and continuation

Use `previous_response_id` with new input items for server-managed continuation. For manual replay, append **all** prior output items to the input history, preserving reasoning, tool calls/results, and encrypted compaction items. Do not reconstruct history from `output_text`. Re-send application instructions and tools as required for the next request. The compiled example handles every function call with a deliberately allowlisted synthetic tool; applications must validate arguments, authorize actions, execute tools, and return matching `function_call_output` items themselves. Shell and computer tool calls likewise require an application executor. [Conversation state](https://developers.openai.com/api/docs/guides/conversation-state)

Image and file input use the raw JSON request path:

```cpp
nlohmann::json input = nlohmann::json::array({{
    {"role", "user"}, {"content", nlohmann::json::array({
        {{"type", "input_text"}, {"text", "Summarize these inputs."}},
        {{"type", "input_image"}, {"image_url", "https://example.com/image.png"}},
        {{"type", "input_file"}, {"file_id", "file_example"}}
    })}
}});
nlohmann::json request = {{"model", "gpt-5.2"}, {"input", input}};
```

Replace sample URLs/IDs with inputs authorized for transmission. Structured outputs use `text.format` with `type: json_schema`, `name`, `schema`, and `strict`; see the compiled `structured` workflow. Supported parameters depend on the chosen model; raw JSON forwarding does not establish model compatibility.

## Service tiers, caching, and context

Request controls remain JSON fields, separate from transport `RequestOptions`:

```cpp
request["service_tier"] = "auto";
request["prompt_cache_key"] = "stable-application-key";
request["prompt_cache_retention"] = "24h"; // Only with a supporting model.
request["context_management"] = nlohmann::json::array({
    {{"type", "compaction"}, {"compact_threshold", 200000}}
});
// Optional diagnostics on a supporting model (GPT-5.6 and later supported models):
request["prompt_cache_options"] = {{"comparison_response_id", "resp_baseline"}};
```

Inspect the returned `service_tier`, `usage.input_tokens_details.cached_tokens`, and `prompt_cache_diagnostics` directly, including from the terminal streaming response. A diagnostics comparison ID is not a continuation ID. Cache settings and compaction thresholds must suit the selected model; do not combine every illustrated option blindly. [Cache diagnostics](https://developers.openai.com/api/docs/guides/prompt-caching/diagnostics)

`compact({{"model", model}, {"input", full_history}})` explicitly calls `/responses/compact` and returns a compacted output window. Preserve the complete returned window for subsequent input. This is separate from `context_management` on ordinary creation, which requests server-side context management. [Compaction guide](https://developers.openai.com/api/docs/guides/compaction)

## Beta multi-agent

Set per-request `options.headers["OpenAI-Beta"] = "responses_multi_agent=v1"` and JSON `multi_agent: {enabled: true, max_concurrent_subagents: 2}`. Use a model documented to support this beta; GPT-5.2 support is not implied. The compiled `multi-agent` workflow selects output from `/root` while retaining full response JSON. Hosted collaboration calls are server-managed; client function calls still need authorized application execution. Explicit `/responses/compact` is not supported with multi-agent enabled. [Multi-agent guide](https://developers.openai.com/api/docs/guides/responses-multi-agent)

Validation here is local fixture and compile coverage, not paid live-model or account-access validation. Azure Responses remains outside this interface.
