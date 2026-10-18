# Rerank

`OpenAI::Rerank` sends `POST <root>/rerank` to a compatible reranking server.
Configure an explicit API root, for example `http://127.0.0.1:8012/v1`, without
a trailing slash. This component is intended for servers implementing this
route; the default OpenAI API root is not a configuration for a llama.cpp server.

[llama.cpp documents `/v1/rerank` as an alias of `/reranking`](https://github.com/ggml-org/llama.cpp/blob/master/tools/server/README.md#post-reranking-rerank-documents-according-to-a-given-query).
Its server requires a reranker model and the `--embedding --pooling rank` options.
Other providers may differ in authentication, supported fields, and response shape.

```cpp
liboai::Response create(
    const std::string& model_id,
    const std::string& query,
    const std::vector<std::string_view>& documents,
    std::optional<int> top_n = std::nullopt
) const &;

liboai::FutureResponse create_async(
    const std::string& model_id,
    const std::string& query,
    const std::vector<std::string_view>& documents,
    std::optional<int> top_n = std::nullopt
) const &;
```

The request contains `model`, `query`, and a JSON array of document strings.
`top_n` is sent only when supplied. The server decides defaults and validates
values, including empty documents and out-of-range result counts.

The returned `Response` exposes the server's JSON unchanged via `raw_json` or
`operator[]`. Jina-style responses commonly contain a `results` array with
`index` and `relevance_score`; check your server's response contract. The library
does not sort, normalize scores, or transform the response schema.

For synchronous calls, document storage must remain valid until the call returns.
For asynchronous calls, document contents, model, and query are copied before
`create_async` returns, so the input storage can then be destroyed or modified.
Keep the `OpenAI` instance and its `Rerank` component alive until the future
completes. Keep shared authorization and proxy settings stable while requests
are running. Network/API errors throw exceptions; async request errors are
re-thrown by `future.get()`.

## Examples

The [synchronous example](examples/create_rerank.cpp) and
[asynchronous example](examples/create_rerank_async.cpp) accept the API root and
model identifier as arguments. If the server requires a bearer token, put it in
`RERANK_API_KEY`; otherwise leave that variable unset.

```sh
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build --target create_rerank create_rerank_async
./build/documentation/rerank/examples/create_rerank http://127.0.0.1:8012/v1 my-reranker
./build/documentation/rerank/examples/create_rerank_async http://127.0.0.1:8012/v1 my-reranker
```

## Local regression test

The standalone test project uses Python 3's standard library to run a loopback
HTTP server. It checks payloads, async input ownership, and error propagation
without an API key, model download, or external API call.

```sh
cmake -S tests/rerank -B build/rerank-tests
cmake --build build/rerank-tests
ctest --test-dir build/rerank-tests --output-on-failure
```
