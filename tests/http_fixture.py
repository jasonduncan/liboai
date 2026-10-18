#!/usr/bin/env python3
"""Loopback-only HTTP capture fixture; Python standard library, no credentials."""
import base64
import email.parser
import email.policy
import http.server
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import time
import urllib.parse
from responses_fixtures import response_fixture
from components_fixtures import component_fixture
from resources_fixtures import resource_fixture
from decisions_fixtures import decision_fixture
from skills_fixtures import skill_fixture


class Handler(http.server.BaseHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'

    def log_message(self, *args):
        pass

    def handle(self):
        try:
            super().handle()
        except (BrokenPipeError, ConnectionResetError):
            pass

    def handle_request(self):
        parsed = urllib.parse.urlsplit(self.path)
        path = parsed.path
        query = urllib.parse.parse_qs(parsed.query, keep_blank_values=True)
        headers = {k.lower(): v for k, v in self.headers.items()}
        body = self.rfile.read(int(headers.get('content-length', 0)))
        data = None
        parts = []
        if 'application/json' in headers.get('content-type', '') and body:
            data = json.loads(body)
        if headers.get('content-type', '').startswith('multipart/form-data'):
            message = email.parser.BytesParser(policy=email.policy.default).parsebytes(
                ('Content-Type: ' + headers['content-type'] + '\r\n\r\n').encode() + body)
            for part in message.iter_parts():
                raw = part.get_payload(decode=True)
                parts.append({'name': part.get_param('name', header='content-disposition'),
                              'filename': part.get_filename(), 'content_type': part.get_content_type(),
                              'data': raw.decode('utf-8', errors='replace'),
                              'data_base64': base64.b64encode(raw).decode()})
        echo = {'method': self.command, 'path': path, 'query': query,
                'headers': headers, 'json': data, 'multipart': parts}
        response = response_fixture(self.command, path, query, headers, data)
        if response is None:
            response = component_fixture(self.command, path, query, headers, data, parts)
        if response is None:
            response = resource_fixture(self.command, path, query, headers, data, parts)
        if response is None:
            response = decision_fixture(self.command, path, query, headers, data, parts)
        if response is None:
            response = skill_fixture(self.command, path, query, headers, data, parts)
        if response is None and ('/fixture/error-json' in path):
            response = (400, {}, json.dumps({'error': {'message': 'synthetic bad request', 'type': 'invalid_request_error',
                'code': 'fixture_error', 'param': 'input', 'unknown': True}}).encode())
        if response is None and ('/fixture/rate-limit' in path):
            response = (429, {'Retry-After': '2'}, b'{"error":{"type":"rate_limit_error","code":"rate_limit_exceeded"}}')
        if response is None and ('/fixture/error-text' in path):
            response = (502, {'Content-Type': 'text/plain'}, b'synthetic gateway failure')
        if response is None and ('/fixture/json-array' in path):
            response = (200, {}, b' \n [1,{"future":true}]')
        if response is None and ('/fixture/empty' in path):
            response = (204, {}, b'')
        if response is None and ('/fixture/binary' in path):
            response = (200, {'Content-Type': 'application/octet-stream'}, b'{\x00\xff\x01not-json')
        if response is None and ('/fixture/invalid-json' in path):
            response = (200, {}, b'{invalid')
        if response is None and ('/fixture/slow' in path):
            time.sleep(0.25)
        if response is None and path.endswith('/files/file-download/content'):
            response = (200, {'Content-Type': 'application/octet-stream'}, b'file\x00content\xff')
        if response is None and path.endswith('/audio/speech') and isinstance(data, dict) and data.get('input') == 'fixture-binary':
            response = (200, {'Content-Type': 'audio/mpeg'}, b'ID3\x00\xffaudio')
        if response is None and headers.get('x-fixture-stream'):
            kind = headers['x-fixture-stream']
            events = {
                'images': [{'type': 'image_generation.partial_image', 'b64_json': 'aGVsbG8=', 'partial_image_index': 0},
                           {'type': 'image_generation.completed', 'b64_json': 'aGVsbG8='}],
                'audio': [{'type': 'transcript.text.delta', 'delta': 'hello'}, {'type': 'transcript.text.done', 'text': 'hello'}],
            }.get(kind, [{'type': 'unknown.future', 'delta': 'hello'}])
            raw = ''.join('event: ' + e['type'] + '\ndata: ' + json.dumps(e) + '\n\n' for e in events).encode()
            response = (200, {'Content-Type': 'text/event-stream'}, [raw[i:i+3] for i in range(0, len(raw), 3)])
        if response is None:
            response = (200, {}, json.dumps(echo, ensure_ascii=False).encode())
        status, extra_headers, payload = response
        chunks = payload if isinstance(payload, list) else [payload]
        self.send_response(status)
        outgoing = {'Content-Type': 'application/json', 'X-Request-ID': 'req_fixture', **extra_headers}
        for name, value in outgoing.items():
            self.send_header(name, value)
        self.send_header('Connection', 'close')
        self.close_connection = True
        self.send_header('Content-Length', str(sum(map(len, chunks))))
        self.end_headers()
        try:
            for chunk in chunks:
                self.wfile.write(chunk)
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            pass

    do_GET = do_POST = do_DELETE = do_PATCH = handle_request


def main():
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    server.daemon_threads = True
    worker = threading.Thread(target=server.serve_forever, daemon=True)
    worker.start()
    env = dict(os.environ)
    # No ambient proxy may redirect loopback fixtures to an external service.
    env['NO_PROXY'] = env['no_proxy'] = '127.0.0.1,localhost'
    try:
        with tempfile.TemporaryDirectory(prefix='liboai-fixture-') as directory:
            result = subprocess.run([sys.argv[1], f'http://127.0.0.1:{server.server_port}/v1', directory],
                                    env=env, timeout=90)
        return result.returncode
    finally:
        server.shutdown()
        server.server_close()
        worker.join()

if __name__ == '__main__':
    sys.exit(main())
