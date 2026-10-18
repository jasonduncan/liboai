# Opt-in live regression suite

The maintained harness under [`tests/live`](../../tests/live) preserves the synthetic scenarios from [the October 17 live validation](LIVE_VALIDATION.md). It is separate from ordinary offline tests. Building it does not make requests. Running it can incur API charges and create remote resources.

## Build and inspect

Use installed cURL and nlohmann-json dependencies as for the library. A serial build is suitable for machines with limited free memory:

```sh
cmake -S . -B build/live-tests \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBOAI_BUILD_TESTS=ON -DLIBOAI_BUILD_LIVE_TESTS=ON
cmake --build build/live-tests --config Release --parallel 1
ctest --test-dir build/live-tests -C Release --output-on-failure
python3 tests/live/run.py --list
```

Supply your usual `CMAKE_PREFIX_PATH` or vcpkg toolchain if needed. Windows uses the same vcpkg setup described in [CI.md](CI.md). The runner locates both single-configuration and `Release`/`Debug` executable directories. No additional Python packages are needed.

`LIBOAI_BUILD_LIVE_TESTS` defaults to `OFF`. CI explicitly compiles the four executables but never runs them. CTest runs only the existing offline fixtures plus synthetic checks of opt-in enforcement, redaction, receipts, readiness bounds, and cleanup after failure. It does not load `.env` or use a real key.

## Explicit execution

Choose a suite and authorize its scope explicitly. This example reads only the literal `OPENAI_API_KEY` assignment from the named file; it does not source shell commands:

```sh
python3 tests/live/run.py --allow-live --suite responses --env-file .env
```

Without `--env-file`, the runner uses `OPENAI_API_KEY` from the environment. It never automatically reads `.env`. A supplied file takes precedence over the environment and is read afresh on each invocation. Keys are passed through the child environment, never command arguments, and redacted from process logs and structured receipts. Ambient organization/project selection and other `*_API_KEY` variables are not forwarded.

Repeat `--suite` to select more than one family. Each named suite runs once, sequentially; the runner stops before later suites if one fails. There is no default selection and no implicit “all” mode. Models are explicit in the source; the runner does not substitute models or expand request bounds after a failure.

| Suite | Synthetic scope and upper request bounds |
| --- | --- |
| `responses` | Four generation calls: three GPT-5.2 requests capped at 64 output tokens and one GPT-5.2-pro request capped at 128. Structured output, async, stored lifecycle, background SSE/replay, token count, cancellation, deletion. One tiny compaction request; compaction exposes no output-token cap. |
| `components` | Two GPT-5.2 Chat generations capped at 64 tokens each; stored Chat lifecycle and SSE. Two low-quality 1024×1024 `gpt-image-1.5` outputs (generation and edit). Three tiny speech requests, one transcription and one translation of generated synthetic audio. One two-input embedding request and one two-input moderation request. |
| `resources` | Small synthetic Conversations, Uploads, Files, Vector Stores, and Containers workflows; one 1 GB container. One single-request `gpt-4.1-nano` batch capped at 16 output tokens, then cancellation/terminal settlement. File downloads use the supported `batch` purpose. Webhooks are list-only. |
| `extensions` | One `gpt-6-luna` Decisions request with three fixed questions. One synthetic Skill with two distinct versions, version/content reads, default update/restore, and deletion. No hosted skill execution. |

These are scenario bounds, not a dollar budget or a promise of account/model access. Resource storage, containers, embedding/ingestion, compaction and generation can be billed. No real document, external URL, hosted tool execution or webhook receiver is supplied by the harness.

Every request uses a 45-second client timeout, including legacy calls through global authorization. Readiness polling is test-only: stored Chat and Skills use at most 13 reads with 10-second spacing; vector ingestion uses three reads with three-second spacing. Only a pending result or HTTP 404 causes another readiness read; transport errors, rate limits and other HTTP errors surface immediately. No create or generation request is retried. Cleanup may make a separate best-effort delete/cancel after a failed suite operation.

The maintained suite checks file bytes through `Files::content`. The legacy `Files::download` path does not propagate the authorization timeout, so its existing historical live and offline checks are retained without rerunning it in this bounded harness. SDK behavior is unchanged by this promotion.

Batch cancellation is attempted at most once. Cleanup checks for terminal status with at most 61 polling reads at 10-second intervals, with a 900-second cutoff measured from suite start. That cutoff reserves time for other cleanup; it is not a service readiness guarantee. The runner additionally enforces a 1200-second process timeout by default (`--timeout`, maximum 1800). Request time contributes to elapsed runtime.

## Receipts and cleanup

Each invocation creates a unique directory under ignored `build/live-results/`. `--output-dir` can select another location; keep any chosen output directory out of source control. Each selected suite writes:

- `receipt.json`: sanitized case results, failure count, completion marker, and pending owned resource IDs/parents. Checkpoints replace the prior receipt only after a complete temporary file is written.
- `process.log`: sanitized stdout/stderr.
- Small generated synthetic audio/image/download artifacts when relevant.

The run directory also contains `summary.json`, including suites not run after an earlier failure. Reusing a receipt directory is rejected. The original diagnostic sources and receipts under ignored `build/live-smoke/` are historical evidence and are not copied into Git.

Owned IDs are recorded immediately after creation returns an ID. Cleanup executes in reverse registration order even after failed assertions, exceptions, or receipt-write errors; cleanup failures retain the affected entries and produce a nonzero exit. Children discovered during cleanup, such as batch output/error files, are tracked before their cleanup. The batch input is retained until terminal batch state is confirmed. No resource selected from a general list is mutated or deleted.

A process crash, kill, timeout, disk failure, or transport interruption during creation can leave remote state unresolved, including a creation whose ID was never received. An empty known-ID ledger alone does not prove no such resource exists: the summary calls that state `known_ids_clear`. Interrupted runs are reported as unknown; a successful exit requires a finished valid receipt with no failures or pending IDs. Inspect retained IDs and reconcile them before starting another run. The harness does not automatically resume old receipts or repeat generation to diagnose failures. Terminal batch records remain service history because there is no delete operation.

## Validation boundary

The promoted suite is compiled and its control/cleanup behavior tested offline. The earlier [live report](LIVE_VALIDATION.md) records actual service calls from the original harnesses; it is not evidence of a new live run of this maintained version. No new live requests were made while implementing this promotion. Live runs remain deliberate, separately authorized invocations.
