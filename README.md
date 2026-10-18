<p align="center">
  <img src="/images/_logo.png">
</p>

<hr>
<h1>Introduction</h1>
<p><code>liboai</code> is a simple, <b>unofficial</b> C++17 library for the OpenAI API. It allows developers to access OpenAI endpoints through a simple collection of methods and classes. The library can most effectively be thought of as a <b>spiritual port</b> of OpenAI's Python library, simply called <code>openai</code>, due to its similar structure - with few exceptions.

## OpenAI API coverage

Status of this checkout, reviewed October 17, 2026 against the [OpenAI API reference](https://developers.openai.com/api/reference/overview). This grid covers major API families; it is not a claim of complete endpoint or model coverage.

**Implemented** = named client methods. **JSON pass-through** = supported through raw request/response fields, without dedicated builders. **Partial** = the limitation is stated below. **Absent** = no client implementation.

| OpenAI feature | liboai support | What is available / what is missing |
| --- | --- | --- |
| [Responses](documentation/responses/README.md) | Implemented | Create, retrieve, delete, cancel, input items, token count, compact; async and HTTP SSE |
| Responses WebSocket connection | Absent | HTTP streaming is available; bidirectional WebSockets are not |
| Function calling, structured outputs, multimodal inputs, reasoning controls | JSON pass-through | Send current fields through Responses or modern Chat; retain unknown output fields |
| Hosted tools: web/file search, code interpreter, shell, MCP | JSON pass-through | Responses tool configuration and results; no local tool execution engine |
| [Chat Completions](documentation/chat/README.md) | Implemented | Modern JSON requests, stored completion CRUD/messages, async and HTTP SSE |
| [Images](documentation/images/README.md) | Implemented | Generation, edits, legacy variations; modern options and HTTP SSE |
| [Audio](documentation/audio/README.md) | Partial | Speech, transcription, translation and streaming; no custom-voice or voice-consent management |
| [Embeddings](documentation/embeddings/README.md) | Implemented | Text/token batches, dimensions and encoding through JSON |
| [Moderation](documentation/moderations/README.md) | Implemented | Text and image inputs through JSON |
| [Models](documentation/models/README.md) | Implemented | List, retrieve and delete |
| [Files](documentation/files/README.md) | Implemented | Upload, list/paginate, retrieve, delete and download |
| [Uploads](documentation/resources/resources.md) | Implemented | Multipart upload lifecycle and parts |
| [Conversations](documentation/resources/resources.md) | Implemented | Server-side conversations and item lifecycle |
| [Batch](documentation/resources/resources.md) | Implemented | Create, retrieve, list and cancel; output/error files use Files |
| [Vector Stores](documentation/resources/resources.md) | Implemented | Stores, files, file batches and search |
| [Containers](documentation/resources/resources.md) | Implemented | Container and file lifecycle, binary downloads |
| [Webhooks](documentation/maintenance/WEBHOOK_VERIFICATION.md) | Partial | Endpoint management; signature verification needs Apple CommonCrypto or opt-in OpenSSL |
| [Decisions](documentation/extensions/DECISIONS.md) | Implemented | Raw create/async and optional typed predicate, choice, score and refusal parsing |
| [Skills](documentation/extensions/SKILLS.md) | Implemented | Upload, manage, version and download bundles; hosted-shell attachment through Responses JSON |
| Realtime and GPT-Live | Absent | No session/call management or bidirectional transport |
| Agents | Absent | Excluded from the current scope |
| ChatKit | Absent | Deferred |
| Current Fine-tuning, Evals and Graders | Absent | Legacy `FineTunes` is a historical interface, not current fine-tuning support |
| Administration, Usage/Costs, Safety and Content Provenance | Absent | No dedicated clients |
| [Legacy Completions, Edits and FineTunes](documentation/maintenance/HISTORICAL_APIS.md) | Partial | Compatibility symbols retained; endpoint availability differs, retired interfaces are labeled |
| [Azure](documentation/azure/README.md) | Partial | Existing Azure interfaces retained; Azure Responses is out of scope |

**Validation:** modern APIs have offline HTTP fixtures, compiled examples and local macOS checks. Bounded [live smoke tests](documentation/maintenance/LIVE_VALIDATION.md) also exercised the major implemented families; that report records visibility delays and remaining gaps. Model access and field/tool availability depend on the service and your account. See the [detailed coverage and validation grid](documentation/maintenance/API_COVERAGE.md) for exact evidence and gaps.

Async methods are available across the implemented components. [Rerank](documentation/rerank/README.md) is an additional compatible-server extension, not an OpenAI endpoint. Update both coverage grids whenever public API support changes.

<h1>Usage</h1>

Start with the Responses API. Raw JSON preserves current and future request fields without waiting for a model-specific library update.

```cpp
#include "liboai.h"

int main() {
  liboai::OpenAI api;
  if (!api.auth.SetKeyEnv("OPENAI_API_KEY")) return 1;
  try {
    auto result = api.Responses->create_with_options({
      {"model", "gpt-5.2"},
      {"input", "Explain a C++ reference in one sentence."},
      {"max_output_tokens", 128}
    });
    std::cout << liboai::Responses::output_text(result.response.raw_json) << '\n';
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
```

Running examples makes real API calls and may incur charges; model access is account-specific. The default regression suite uses synthetic loopback fixtures and requires no API key.

See [Responses lifecycle and streaming](documentation/responses/README.md), [request options and diagnostics](documentation/maintenance/REQUEST_OPTIONS.md), [modern component APIs](documentation/maintenance/MODERN_COMPONENTS.md), and [supporting resources](documentation/resources/resources.md). Existing signatures remain available. Retired Edits and FineTunes examples are [historical compatibility examples](documentation/maintenance/HISTORICAL_APIS.md).

<h1>Dependencies</h1>
<p>For the library to work the way it does, it relies on two major dependencies. These dependencies can be found listed below.<p>

- <a href="https://github.com/nlohmann/json">nlohmann-json</a>
- <a href="https://curl.se/">cURL</a>

*If building the library using the provided solution, it is recommended to install these dependencies using <b>vcpkg</b>.*

<h1>Documentation</h1>
<p>For detailed documentation and additional code examples, see the library's documentation <a href="/documentation">here</a>.

<h1>Contributing</h1>
<p>Artificial intelligence is an exciting and quickly-changing field. 

If you'd like to partake in further placing the power of AI in the hands of everyday people, please consider contributing by submitting new code and features via a **Pull Request**. If you have any issues using the library, or just want to suggest new features, feel free to contact me directly using the info on my <a href="https://github.com/D7EAD">profile</a> or open an **Issue**.
