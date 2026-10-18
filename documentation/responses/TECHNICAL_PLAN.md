# Responses API Migration Plan (GPT-5.2)

> Historical design: retained as the original migration proposal. Responses creation, async creation, and common parameter serialization are now implemented. The current implementation backlog and acceptance criteria are in the [OpenAI API modernization plan](../maintenance/API_MODERNIZATION_PLAN.md). The original state descriptions and checklists below are not current status; recheck model-specific constraints against the official API before implementation.

## Goal
Add first-class support for the OpenAI Responses API so liboai can use GPT-5.2 and gpt-5.2-pro, while keeping existing Chat Completions support for backward compatibility.

## Current State (Repo Reality)
- `liboai::ChatCompletion` calls `/v1/chat/completions` and uses `Conversation` to manage messages and function calls.
- No Responses API component exists.
- Streaming parsers and conversation updates are tightly coupled to Chat Completions response shape.
- JSON request building uses `JsonConstructor` with explicit parameter lists.

## Requirements from GPT-5.2 / Responses API
- New endpoint: `POST /v1/responses`.
- New request shape: `input` can be a string or an array of items; `instructions` can be separate.
- New parameters: `reasoning.effort`, `text.verbosity`, `text.format` (structured outputs), `max_output_tokens`.
- Model-specific constraint: `temperature`, `top_p`, and `logprobs` are only allowed when `reasoning.effort == "none"`.
- Tool definitions use `tools` with internally-tagged objects, and `tool_choice` supports `allowed_tools`.
- Multi-turn can chain with `previous_response_id`.

## Proposed API Design

### 1) New Component: `liboai::Responses`
- Files:
  - `liboai/components/responses.cpp`
  - `liboai/include/components/responses.h`
- Wiring:
  - Add to `liboai/include/liboai.h` and `liboai/CMakeLists.txt`.
- Methods:
  - `Response create(const nlohmann::json& request, ...) const`.
  - `FutureResponse create_async(const nlohmann::json& request, ...) const`.
  - Raw JSON payloads only for the initial implementation.

### 2) Request Builder Types (Deferred)
We will start with raw JSON requests to avoid introducing new abstractions.
If needed later, add a `ResponseRequest` or similar builder to simplify common cases.

### 3) Input Helpers (Deferred)
- `ResponseInput` helper to build `input` items:
  - `AddSystem(string)`
  - `AddUser(string)`
  - `AddAssistant(string)`
  - `AddToolCall(...)`
  - `AddToolOutput(...)`
- Provide a static adapter: `ResponseInput::FromConversation(const Conversation&)`.

### 4) Tool Definitions
- New `Tools` helper to build `tools` arrays:
  - Function tools: `{ "type": "function", "name", "description", "parameters" }`
  - Custom tools: `{ "type": "custom", "name", "description" }`
  - Built-ins: `{ "type": "web_search" }` and others (pass-through JSON)
- New `ToolChoice` helper to build `tool_choice` objects, including `allowed_tools`.

### 5) Response Parsing Helpers
- Add optional helpers to extract `output_text` from the response JSON.
- Keep `liboai::Response` unchanged for compatibility; add a small utility function in `Responses` or a separate helper header.

### 6) Streaming
- New streaming parser for Responses SSE events.
- Provide `ResponsesStreamCallback` that surfaces:
  - `delta_text` (if any)
  - `event_type` (for tool calls, output items, completion)
  - a partial `Response` or updated `ResponseInput` (optional)
- Confirm event schema from official Responses streaming docs before implementation.

### 7) Backward Compatibility
- Keep `ChatCompletion` intact.
- Consider adding optional `verbosity` and `reasoning_effort` parameters to `ChatCompletion` for GPT-5.2 users who stay on Chat Completions.
- Do not remove `Conversation` or existing behavior.

## Implementation Plan (Milestones)

### Milestone 1: Scaffolding
- [x] Add Responses component files and wire into build system.
- [x] Add minimal `create` that accepts a raw JSON payload.
- [x] Add a basic usage example (string input).

### Milestone 2: Core Params and Validation
- [ ] Implement `instructions`, `reasoning`, `text`, `max_output_tokens`.
- [ ] Add validation for `temperature` / `top_p` / `logprobs` when `reasoning.effort != "none"`.
- [ ] Add `store`, `previous_response_id`, `include`, `metadata`.

### Milestone 3: Tools and Structured Outputs
- [ ] Add `Tools` and `ToolChoice` helpers.
- [ ] Support `text.format` for JSON schema structured outputs.
- [ ] Provide minimal tool-call example usage.

### Milestone 4: Streaming and Output Helpers
- [ ] Implement SSE parsing for Responses.
- [ ] Add `output_text` helper extraction.
- [ ] Add streaming example and docs.

### Milestone 5: Docs and Migration Guidance
- [x] Add `documentation/responses/README.md` with API usage.
- [x] Update `documentation/README.md` to link Responses docs.
- [x] Update root `README.md` feature list to include Responses API.

## Testing Plan
- Compile examples in `documentation/responses/examples`.
- Add at least one integration test snippet (manual run) for:
  - simple string input
  - tool definition + tool call response parsing
  - structured output
- If a unit test framework is added later, add a JSON shape test for `ResponseInput` and `ToolChoice` builders.

## Risks and Mitigations
- Streaming event schema is different from Chat Completions: verify with official docs before implementing.
- Tool calling item shapes in Responses are not identical to Chat Completions: parse by `type` and `call_id`.
- Parameter incompatibilities for GPT-5.2: add validation and helpful error messages.

## Open Questions for Jason
- Should `ResponseInput` be added after the basic Responses component ships?
- Should we add `output_text()` convenience on `liboai::Response`, or keep helpers in `Responses` only?
