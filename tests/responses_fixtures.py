"""Synthetic Responses fixtures; no account or external network is involved."""
import json


def response_fixture(method, path, query, headers, body):
    if isinstance(body, (bytes, bytearray)):
        try:
            body = json.loads(body)
        except (ValueError, UnicodeDecodeError):
            body = {}
    body = body if isinstance(body, dict) else {}
    mode = body.get("input")
    streaming = method == "POST" and path.endswith("/responses") and isinstance(mode, str) and mode.startswith("fixture-") and body.get("stream")
    retrieving = method == "GET" and path.endswith("/responses/fixture-stream") and query.get("stream") == ["true"]
    if streaming or retrieving:
        mode = mode if streaming else "fixture-stream"
        status = {"fixture-failed": "failed", "fixture-incomplete": "incomplete"}.get(mode, "completed")
        events = [
            {"type": "response.created", "response": {"id": "resp_fixture", "status": "in_progress"}},
            {"type": "response.output_item.added", "item": {"type": "function_call", "name": "synthetic", "call_id": "call_1"}},
            {"type": "response.output_text.delta", "delta": "snowman ☃", "sequence_number": 2},
            {"type": "future.event", "unknown": {"keep": True}},
            {"type": "response." + status, "response": {"id": "resp_fixture", "status": status, "output": [], "error": {"code": "fixture"} if status == "failed" else None}},
        ]
        if mode == "fixture-ended":
            events.pop()
        raw = b": comment\r\n" + b"".join(("data: " + json.dumps(e, ensure_ascii=False) + "\r\n\r\n").encode() for e in events)
        raw += b"data: {\"type\":\"never-dispatched\"}"
        # Tiny chunks deliberately split UTF-8, JSON, and SSE delimiters.
        return 200, {"Content-Type": "text/event-stream", "X-Request-ID": "req_stream"}, [raw[i:i+3] for i in range(0, len(raw), 3)]
    if path.endswith("/responses/fixture-background") and method == "GET":
        return 200, {}, json.dumps({"id": "fixture-background", "status": "in_progress"}).encode()
    if path.endswith("/responses/fixture-background/cancel"):
        return 200, {}, json.dumps({"id": "fixture-background", "status": "cancelled"}).encode()
    if path.endswith("/responses/fixture-pages/input_items"):
        page = {"data": [{"id": "item_1"}], "has_more": True, "last_id": "item_1"}
        if query.get("after") == ["item_1"]:
            page = {"data": [{"id": "item_2"}], "has_more": False, "last_id": "item_2"}
        return 200, {}, json.dumps(page).encode()
    return None
