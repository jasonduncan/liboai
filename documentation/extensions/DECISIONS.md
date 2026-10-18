# Decisions API

`liboai::Decisions` provides `create(body, options)` and `create_async(body, options)` for `POST /decisions`. It returns the existing `ApiResult`, so raw JSON, response headers, request IDs, and structured transport/API errors remain available. Async calls own their client and argument snapshots; the originating client can be destroyed while the future runs.

The official guide describes Decisions as public beta and currently lists `gpt-6-luna`. The SDK does not impose a model allowlist. The fetched guide and create reference do not specify a Decisions beta header, and their HTTP examples omit one. The client therefore adds no undocumented beta header. Explicit caller headers can still be supplied through `RequestOptions` and remain local to that request. [Guide](https://developers.openai.com/api/docs/guides/decisions), [create reference](https://developers.openai.com/api/reference/resources/decisions/methods/create).

## Requests

Supply raw JSON containing `model`, `input`, and `questions`. Input can be text or user messages containing text/image parts. The client preserves fields such as cache controls and future options without translating or restricting the payload. Question forms include:

| Question type | Configuration | Answer |
| --- | --- | --- |
| `predicate` | Instructions describing a condition | Estimated `probability` |
| `choice` | Instructions and `choices` containing string or boolean `value`, with optional descriptions | Selected `choice`, `confidence`, and option probabilities |
| `score` | Instructions and ordered `levels` containing `label`, with optional descriptions | Fractional `score`, `confidence`, and level probabilities |

Use unique question names when matching results. Answers retain question order, and individual questions may return a refusal. A score summarizes the ordered distribution rather than selecting a category. Application thresholds require validation on representative examples. [Decisions guide](https://developers.openai.com/api/docs/guides/decisions).

## Optional typed answers

`parse_decision_answers(result.response.raw_json)` produces an ordered `std::vector<DecisionAnswer>`. Each item includes `type`, nullable `name`, the complete original `raw` answer, and a `value` variant:

- `DecisionPredicate`: probability.
- `DecisionChoice`: choice, confidence, and per-value probabilities. `DecisionChoiceValue` is `std::variant<std::string, bool>`; the string `"true"` remains distinct from boolean `true`.
- `DecisionScore`: score, confidence, and probabilities with string labels and signed 64-bit level indices.
- `DecisionRefusal`: an explicit refusal, separate from missing or malformed output.
- `DecisionUnknown`: a future answer type, retained intact in `raw`.

The helper checks required known fields and rejects malformed shapes with `exception::DecisionAnswerError`. It requires finite numbers, probabilities in `[0,1]`, and representable integer score indices. It does not reinterpret confidence as the largest option probability, normalize distributions, invent defaults for missing fields, or select an application threshold. Unknown fields remain in `raw`; unknown answer types can contain future field shapes. The original `ApiResult` is unchanged if typed parsing fails. Top-level model, usage, and future fields remain in `response.raw_json`.

```cpp
liboai::Decisions decisions;
auto result = decisions.create({
    {"model", "gpt-6-luna"},
    {"input", "The parcel arrived with a cracked screen."},
    {"questions", nlohmann::json::array({{
        {"type", "predicate"}, {"name", "damaged"},
        {"instructions", "Does the customer report physical damage?"}
    }})}
});
for (const auto& answer : liboai::parse_decision_answers(result.response.raw_json)) {
    if (const auto* predicate = std::get_if<liboai::DecisionPredicate>(&answer.value)) {
        std::cout << predicate->probability << '\n';
    } else if (std::holds_alternative<liboai::DecisionRefusal>(answer.value)) {
        // Send this item to the application's refusal/review path.
    }
}
```

The [standalone example](examples/decisions.cpp) requires `OPENAI_API_KEY` and an explicit model argument. Running it makes one API request; compilation and default tests do not. The offline suite covers routes, raw multimodal/typed-choice payloads, request option isolation, async lifetime, ordered answers, refusal/unknown types, malformed known output, and structured errors. It establishes local serialization and parsing behavior, not live account access or model quality. Source inspection date: October 17, 2026.

A subsequent authorized [live smoke test](../maintenance/LIVE_VALIDATION.md) successfully exercised all three answer families with `gpt-6-luna`. This validates client interoperability for that synthetic request, not classification quality.
