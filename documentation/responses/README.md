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
