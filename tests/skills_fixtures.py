"""Synthetic Skills API responses, enabled only by the extension fixture header."""
import json

_workflow = None


def _workflow_fixture(method, path, body, parts):
    """Small stateful fake: records versions/default separately to exercise pinning."""
    global _workflow
    def reply(value, status=200):
        return status, {}, json.dumps(value).encode()
    def missing():
        return reply({"error": {"type": "invalid_request_error", "code": "skill_not_found"}}, 404)
    def skill():
        return {"object": "skill", "id": "skill_workflow", "default_version": _workflow["default"],
                "latest_version": str(len(_workflow["versions"]))}
    def version(number):
        return {"object": "skill.version", "id": "skillver_" + number, "skill_id": "skill_workflow", "version": number}
    if method == "POST" and path == "/v1/skills":
        _workflow = {"default": "1", "versions": {"1": parts}}
        return reply(skill())
    if _workflow is None:
        return missing()
    base = "/v1/skills/skill_workflow"
    if method == "POST" and path == base + "/versions":
        number = str(len(_workflow["versions"]) + 1)
        _workflow["versions"][number] = parts
        if any(part["name"] == "default" and part["data"] == "true" for part in parts):
            _workflow["default"] = number
        return reply(version(number))
    if path == base:
        if method == "POST":
            number = body.get("default_version")
            if number not in _workflow["versions"]:
                return reply({"error": {"code": "invalid_version"}}, 400)
            _workflow["default"] = number
        elif method == "DELETE":
            _workflow = None
            return reply({"object": "skill.deleted", "id": "skill_workflow", "deleted": True})
        return reply(skill())
    if method == "GET" and path.startswith(base + "/versions/") and not path.endswith("/content"):
        number = path.rsplit("/", 1)[-1]
        return reply(version(number)) if number in _workflow["versions"] else missing()
    if method == "GET" and path.endswith("/content"):
        number = _workflow["default"] if path == base + "/content" else path.rsplit("/", 2)[-2]
        if number not in _workflow["versions"]:
            return missing()
        return 200, {"Content-Type": "application/zip", "X-Fixture-Version": number}, b"PK\x03\x04\x00\xffversion-" + number.encode()
    if method == "POST" and path == "/v1/responses":
        try:
            tool = body["tools"][0]
            environment = tool["environment"]
            reference = environment["skills"][0]
            number = reference["version"]
            valid = tool["type"] == "shell" and environment["type"] == "container_auto" and reference["type"] == "skill_reference" and reference["skill_id"] == "skill_workflow" and number in _workflow["versions"]
        except (KeyError, IndexError, TypeError):
            valid = False
        if not valid:
            return reply({"error": {"code": "invalid_skill_reference"}}, 400)
        return reply({"id": "resp_skill_workflow", "object": "response", "status": "completed", "output": [],
                      "captured_request": body, "resolved_version": number, "default_version": _workflow["default"]})
    return missing()


def skill_fixture(method, path, query, headers, body, parts):
    if headers.get("x-fixture-extension") != "skills":
        return None
    if headers.get("x-fixture-skills-workflow") == "true":
        return _workflow_fixture(method, path, body, parts)
    if path.endswith("/skills/error"):
        return 404, {}, json.dumps({"error": {"type": "invalid_request_error", "code": "skill_not_found", "message": "Synthetic missing skill", "future": True}}).encode()
    if path.endswith("/content"):
        return 200, {"Content-Type": "application/zip", "X-Request-ID": "req_skill_content",
                     "X-Fixture-Path": path, "X-Fixture-Method": method}, b"PK\x03\x04\x00\xffsynthetic-zip-content"
    if headers.get("x-fixture-skills-pages") == "true" and method == "GET":
        if query.get("after") == ["page_1"]:
            data = {"object": "list", "data": [{"id": "page_2", "unknown": 2}], "first_id": "page_2", "last_id": "page_2", "has_more": False}
        else:
            data = {"object": "list", "data": [{"id": "page_1", "unknown": 1}], "first_id": "page_1", "last_id": "page_1", "has_more": True}
        return 200, {}, json.dumps(data).encode()
    return None
