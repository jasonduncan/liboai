"""Run the rerank executable against a synthetic loopback server only."""
import http.server
import json
import os
import subprocess
import sys
import threading


class Handler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        try:
            assert self.path == "/v1/rerank", self.path
            assert self.headers["Content-Type"] == "application/json"
            assert self.headers["Authorization"] == "Bearer synthetic-test-token"
            data = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert data["model"] == "model"
            assert data["query"] in ("query", "error")
            assert set(data) <= {"model", "query", "documents", "top_n"}
            error = data["query"] == "error"
            response = (
                {"error": {"message": "synthetic API failure"}}
                if error else
                {"request": data, "results": [{"index": 1, "relevance_score": 0.75}]}
            )
            self.send_response(400 if error else 200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            self.server.requests.append(data)
        except Exception as error:
            self.server.failures.append(str(error))
            self.send_error(500)

    def log_message(self, *_args):
        pass


def main():
    with http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler) as server:
        server.requests = []
        server.failures = []
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        env = os.environ.copy()
        env["NO_PROXY"] = env["no_proxy"] = "127.0.0.1,localhost"
        try:
            result = subprocess.run(
                [sys.argv[1], f"http://127.0.0.1:{server.server_port}/v1"],
                env=env, timeout=30, check=False,
            )
        finally:
            server.shutdown()
            thread.join()
    if server.failures or len(server.requests) != 4:
        print(f"Mock failures: {server.failures}; requests: {len(server.requests)}", file=sys.stderr)
        return 1
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
