# Decisions and Skills extension plan

Date: October 17, 2026. Scope authorized after the core modernization: **Decisions and Skills**. Agents was explicitly excluded and ChatKit deferred. This extends the existing working-tree implementation; it does not replace or undo Phases 0–4.

## Outcomes

1. A C++ application can submit a Decisions request, inspect every answer and diagnostic, and optionally read known answer shapes through typed helpers.
2. A C++ application can upload, inspect, version, select, and delete Skills bundles, then reference an explicit version in an ordinary Responses request.
3. Existing APIs, object layouts, authorizer behavior, and consumers continue to work.
4. The README contains a discoverable feature matrix covering OpenAI's major API families, with a detailed inventory separating source/fixture coverage from live service validation.

These are HTTP clients. They do not train models, execute skill scripts inside the application, implement the Agents SDK, or add a ChatKit UI/server. Model suitability and account access remain service-specific.

## Verified API basis

Use official sources, checked October 17, 2026:

- [Decisions guide](https://developers.openai.com/api/docs/guides/decisions)
- [Create a decision](https://developers.openai.com/api/reference/resources/decisions/methods/create)
- [Skills guide](https://developers.openai.com/api/docs/guides/tools-skills)
- [Skills reference](https://developers.openai.com/api/reference/resources/skills)
- [Skill versions](https://developers.openai.com/api/reference/resources/skills/subresources/versions)
- [API inventory](https://developers.openai.com/api/reference/overview)

Decisions is described as a public beta. Current official examples for Decisions and Skills do not require an `OpenAI-Beta` header. The clients must not invent or globally inject one. `RequestOptions.headers` remains available for an explicitly documented feature or compatible server requirement.

The Skills guide specifies multipart directory/ZIP uploads. Generated reference pages may render file parameters as JSON strings; implement actual multipart file transport following the guide, with fixture checks for field names, filenames, MIME types, and exact bytes.

## Architecture and compatibility

Create independent `liboai::Decisions` and `liboai::Skills` classes, each with an API root defaulting to `https://api.openai.com/v1`. Include them from `liboai.h`, and register headers/sources with CMake install/export rules. Do not add instance fields to the existing `OpenAI` aggregate.

Reuse:

- `RequestOptions` for project, headers, and timeout overrides.
- `ApiResult` / `FutureApiResult` for original response data and diagnostics.
- `detail::request` for JSON and query-based operations.
- `detail::multipart_request` for bundle uploads.
- `encode_path_segment` and `Query` for safe identifiers and cursor pagination.
- `snapshot_multipart` to own async buffer bytes.

Every operation has synchronous and asynchronous forms. Async work copies the independent client/root and arguments before launch. File paths are copied, but referenced files must remain unchanged until work completes. Unknown JSON fields, response types, and query fields remain available. No model allowlists, implicit retries, implicit polling, or new runtime dependencies.

## Decisions surface

| Operation | HTTP | Input / output |
| --- | --- | --- |
| `create` / `create_async` | `POST /decisions` | Raw JSON; `ApiResult` |
| `parse_decision_answers` | Local helper | Known typed answers plus original JSON; unknown variants retained |

The complete raw request path accepts the documented input, model, questions, and optional configuration. Do not filter future fields or narrow integers to legacy Chat widths.

Document and demonstrate these question/answer families:

- Predicate: probability that the requested condition holds.
- Choice: selected string or boolean value, confidence, and the returned distribution.
- Score: score, confidence, and rubric-level probabilities.
- Refusal: an explicit answer variant, never silently interpreted as false/zero.
- Unknown future type: preserve as an opaque variant with the original JSON.

A typed helper is optional. Requests and responses must remain usable without it. Typed extraction validates the known structural fields it reads, reports a clear error for malformed known answers, and does not discard additional fields. Preserve answer ordering and nullable names; require documented fields for known answer types. Do not turn probabilities into application decisions automatically; thresholds and consequences are application policy.

The current guide names `gpt-6-luna`; examples may describe that source fact but must accept an explicit model and avoid claiming account access or measured latency. No performance claim is established by our fixture tests.

### Decisions acceptance

- Sync/async POST method, route, JSON payload, large integer options, and unknown fields are captured through the public library.
- Per-request project/extra headers are isolated and copied for async work.
- Predicate, choice, score, refusal, and unknown answers are retained and distinguishable.
- Malformed known answers produce explicit helper errors; valid raw JSON is still available to the caller.
- Structured/non-JSON HTTP errors remain available through existing diagnostics.
- The example compiles and requires deliberate execution; default tests make no service request.

## Skills surface

| Operation | HTTP |
| --- | --- |
| `create` / async | `POST /skills` (multipart) |
| `list` / async | `GET /skills` |
| `retrieve` / async | `GET /skills/{skill_id}` |
| `update` / async | `POST /skills/{skill_id}` (JSON metadata/default pointer) |
| `remove` / async | `DELETE /skills/{skill_id}` |
| `content` / async | `GET /skills/{skill_id}/content` |
| `create_version` / async | `POST /skills/{skill_id}/versions` (multipart) |
| `list_versions` / async | `GET /skills/{skill_id}/versions` |
| `retrieve_version` / async | `GET /skills/{skill_id}/versions/{version}` |
| `remove_version` / async | `DELETE /skills/{skill_id}/versions/{version}` |
| `version_content` / async | `GET /skills/{skill_id}/versions/{version}/content` |

Versions are immutable uploads; no update-version endpoint will be invented. Default-version selection is a skill update. Preserve version identifiers as strings at the resource boundary, and use the documented version representation when constructing a Responses attachment.

### Bundle transport

A bundle contains a `SKILL.md` manifest and may contain references, scripts, and templates. The client uploads caller-supplied files; it does not inspect or execute instructions during upload.

- Directory upload: repeated `files[]` fields with filenames retaining the single top-level folder and relative paths.
- ZIP upload: `files` field with `application/zip` content type.
- Version creation: preserve extra multipart fields such as `default`.
- Content retrieval: retain exact binary archive bytes in `ApiResult.response.content`.
- Do not add a ZIP library, archive extraction, recursive directory traversal, or automatic publishing in this slice.

The existing multipart interface provides the complete transport path. A future convenience packer can be added separately after defining symlink, traversal, archive-size, manifest, and reproducibility requirements.

### Use with Responses

The documentation and compiled example show the hosted shell attachment:

```json
{
  "tools": [{
    "type": "shell",
    "environment": {
      "type": "container_auto",
      "skills": [{"type": "skill_reference", "skill_id": "skill_example", "version": "2"}]
    }
  }]
}
```

The model sees skill metadata and can read the instructions when relevant. Explicit prompts may request a particular skill. Pin versions for reproducible configuration; raw response events remain the evidence of what happened. Uploading or attaching a skill does not prove it was selected, followed correctly, or suitable for a task.

Local shell skill files are managed by the application's execution environment and do not use hosted `skill_reference` attachments. This feature does not require implementation of the separate managed Agents API.

### Skills acceptance

- Every route and both sync/async forms compile and are exercised by local public-library tests.
- Identifiers and pagination values containing spaces, slashes, and reserved characters are encoded correctly.
- Directory filenames and repeated fields survive multipart transport, including MIME types and binary/NUL data.
- Async buffer data is snapshotted; the caller can release/mutate its original buffers safely.
- Binary skill/version content is returned without JSON parsing.
- Fixtures cover create → version → default update → list/retrieve/content → Responses attachment → cleanup.
- Pagination is explicit and bounded in examples/tests.
- No skill content is executed and no remote resource is created during default tests.

## Feature matrix

Add a prominent table in the main README covering Responses, Chat, models, images, audio, embeddings, moderation, files/uploads, conversations, Batch, Vector Stores, Containers, webhooks, Decisions, Skills, Realtime/live, Agents, ChatKit, Evals/fine-tuning, administration/safety, and retired interfaces.

Use four defined states:

- **Implemented:** named endpoint/client support exists.
- **JSON pass-through:** supported through raw payloads without a dedicated builder.
- **Partial:** a concrete transport, endpoint, or platform limitation is identified.
- **Absent:** no implementation; note whether deferred or intentionally excluded.

A row is not a claim that every model/parameter or every subresource is supported. Link to the detailed coverage inventory for granularity. Explicitly label local fixture verification versus live API testing and preserve deprecation context for historical interfaces.

## Validation and delivery

1. Record the plan and source references before integration.
2. Implement Decisions and Skills independently with public-header documentation and examples.
3. Extend the existing loopback fixture using distinct, synthetic extension scenarios.
4. Build every example and run the full CTest suite, including earlier modernization regressions.
5. Run ASan/UBSan for typed parsing and asynchronous/multipart ownership.
6. Verify install-and-consume and parent-provided dependency targets after adding public headers.
7. Update README coverage, detailed inventory, roadmap, and validation record with exact results.
8. Check the diff for accidental Agents/ChatKit changes and whitespace errors.

All work is additive to the existing uncommitted modernization. Live calls, account qualification, release publication, and deployment are separate actions. Linux/Windows remain unqualified until those builds actually run.

## Tracking

- [x] Scope narrowed to Decisions and Skills; Agents excluded, ChatKit deferred
- [x] Current official routes and header requirements inspected
- [x] Decisions client, typed extraction, docs, fixtures
- [x] Skills/version client, multipart/content support, docs, fixtures
- [x] README feature matrix and detailed coverage refresh
- [x] Full build, tests, sanitizers, consumer integration
- [x] Final review and validation record
