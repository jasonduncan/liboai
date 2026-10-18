# Skills API

`liboai::Skills` manages uploaded, versioned skill bundles for Responses hosted shell. Include `components/skills.h` and instantiate the independent client with `Skills(root)`; it is not a member of `OpenAI`. It does not discover local `SKILL.md` files, install skills into an agent, run bundle scripts, or implement the Agents API. ChatKit is deferred.

## Operations

Every operation returns `ApiResult` and has an `_async` counterpart returning `FutureApiResult`. `options` is an optional `RequestOptions`; queries and optional options default to `{}`. All methods are declared in [`skills.h`](../../liboai/include/components/skills.h).

| Method arguments, before optional `options` | HTTP request |
| --- | --- |
| `create(multipart)` | `POST /skills` |
| `list(query)` | `GET /skills` |
| `retrieve(skill_id)` | `GET /skills/{skill_id}` |
| `update(skill_id, json)` | `POST /skills/{skill_id}` |
| `remove(skill_id)` | `DELETE /skills/{skill_id}` |
| `content(skill_id)` | `GET /skills/{skill_id}/content` |
| `create_version(skill_id, multipart)` | `POST /skills/{skill_id}/versions` |
| `list_versions(skill_id, query)` | `GET /skills/{skill_id}/versions` |
| `retrieve_version(skill_id, version)` | `GET /skills/{skill_id}/versions/{version}` |
| `remove_version(skill_id, version)` | `DELETE /skills/{skill_id}/versions/{version}` |
| `version_content(skill_id, version)` | `GET /skills/{skill_id}/versions/{version}/content` |

Versions are immutable: upload a new version rather than updating one. `update` changes the default pointer using JSON such as `{{"default_version", "2"}}`; arbitrary additional fields remain intact. Resource IDs and version strings are encoded as individual path segments. Lists accept `after`, `limit`, and `order`, as well as future query keys through `Query`. Follow `has_more` and `last_id` explicitly; the library does not automatically enumerate every page. [Skills lifecycle reference](https://developers.openai.com/api/reference/python/resources/skills), [default-version update](https://developers.openai.com/api/reference/resources/skills/methods/update), [list query](https://developers.openai.com/api/reference/resources/skills/methods/list).

`result.response.raw_json` preserves returned JSON and unknown fields. Downloads preserve ZIP bytes, including NUL bytes, in `result.response.content`; write them in binary mode if saving them. Headers and request IDs are available through `result.headers` and `result.request_id()`. Downloading does not extract or execute a bundle. HTTP, parse, and transport errors use the existing structured exceptions.

No Skills beta header is automatically inserted: the currently verified official guide examples do not require one. Per-request headers and project/timeout settings remain available in `RequestOptions`; no global authorization state is changed. This client uses ordinary OpenAI endpoints, not Azure endpoints.

## Upload and ownership

Supply the existing `netimpl::components::Multipart` type. Directory uploads use repeated `files[]` fields with relative filenames inside one top-level folder. ZIP uploads use one `files` field. `create_version` also accepts a `default` form field; serialize booleans as `"true"` or `"false"`. Unknown multipart fields, filenames, content types, ordering, and binary bytes are forwarded. The library does not create archives or validate bundle manifests; service validation remains authoritative. [Upload guide](https://developers.openai.com/api/docs/guides/tools-skills), [version creation](https://developers.openai.com/api/reference/resources/skills/subresources/versions/methods/create).

```cpp
using namespace liboai::netimpl::components;
liboai::Skills skills;
Multipart directory{{"files[]", File("./review/SKILL.md", "review/SKILL.md"), "text/markdown"},
                    {"files[]", File("./review/references/policy.txt", "review/references/policy.txt"), "text/plain"}};
auto skill = skills.create(directory);

Multipart archive{{"files", File("./review.zip"), "application/zip"},
                  {"default", "true"}};
auto version = skills.create_version(skill.response.raw_json.at("id"), archive);
```

Synchronous callers keep buffer memory alive until the call returns. Async methods snapshot buffer bytes before launch and capture an independent client, IDs, options, and query/body values. The original `Skills` object may be destroyed while an async call runs. File-backed parts retain paths rather than copying file contents: keep referenced files present and unchanged until the future completes. Use buffer parts when byte-for-byte ownership is required. Multipart snapshots do not authorize upload of arbitrary local files.

## Attach a pinned version

The compiled [example](examples/skills.cpp) requires explicit `--upload MODEL`. It uploads a synthetic manifest, inspects a new version, downloads its bytes, updates the default pointer, and prints this request shape without sending it to Responses:

```json
{
  "model": "YOUR_SUPPORTED_MODEL",
  "input": "Use the reviewed skill on this synthetic note.",
  "tools": [{
    "type": "shell",
    "environment": {
      "type": "container_auto",
      "skills": [{"type": "skill_reference", "skill_id": "skill_example", "version": "2"}]
    }
  }]
}
```

Choose a model supporting hosted shell and review the bundle before attaching it. This JSON uses the hosted-shell attachment contract; local-shell skill paths and Agents sandbox capability directories have different contracts. Uploading a bundle does not run it or establish that a model can use it. A later Responses hosted-shell request can execute tool work and incur model/container usage. [Hosted-shell Skills guide](https://developers.openai.com/api/docs/guides/tools-skills).

Record the returned skill ID and immutable version for reproducibility. Omitting the version uses the default pointer; `latest` intentionally follows subsequent uploads. The example prints `skills --delete SKILL_ID` for explicit cleanup and reports the created ID if later work fails. Deleting a skill removes its versions. The service disallows deleting the default version directly; change the default first when deleting an individual version. [Version management](https://developers.openai.com/api/docs/guides/tools-skills#versioning-and-management).

The example preserves the API's returned version string, such as `"2"`. The Responses `SkillReference.version` schema is a string, and the official guide's JavaScript example uses a string; some guide examples instead use a JSON number. Raw JSON remains available without coercion. [Responses schema](https://developers.openai.com/api/reference/resources/responses/methods/retrieve).

## Validation boundary

Local fixtures cover all sync/async routes, encoded identifiers, pagination, multipart directory/ZIP bytes, unknown fields, independent async client and buffer ownership, binary downloads, structured errors, and a bounded upload/version/default/Responses-attachment/cleanup workflow that keeps an explicit version pinned across default changes. These checks establish request/response behavior only. The initial implementation used offline fixtures. Subsequent authorized [live smoke checks](../maintenance/LIVE_VALIDATION.md) exercised uploads and downloads and confirmed that newly uploaded versions became visible after a delay. Hosted skill execution remains untested. No new runtime dependency or archive packer is introduced.

New resources may not be immediately visible to list or version-specific endpoints. The client and example perform no automatic retries. Retain returned IDs, handle readiness/errors explicitly, and consult the [live validation record](../maintenance/LIVE_VALIDATION.md) before assuming immediate version availability.
