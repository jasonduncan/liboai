"""Controlled modern component responses, selected only by explicit fixture headers."""
import json


def component_fixture(method, path, query, headers, data, parts):
    kind = headers.get('x-fixture-component')
    if not kind:
        return None
    if kind == 'files-pagination':
        after = query.get('after', [''])[0]
        if after not in ('', 'file /first'):
            return (400, {}, b'{"error":{"message":"unexpected file cursor"}}')
        page = {'object': 'list', 'data': [{'id': 'file /first' if not after else 'file-second'}],
                'has_more': not bool(after), 'last_id': 'file /first' if not after else 'file-second'}
        return (200, {}, json.dumps(page).encode())
    if kind in ('embedding-float', 'embedding-base64'):
        vector = [0.25, -0.5, 1.0] if kind == 'embedding-float' else 'AACAPw=='
        result = {'object': 'list', 'data': [{'index': 0, 'embedding': vector, 'future_field': {'preserve': True}}]}
        return (200, {}, json.dumps(result).encode())
    if kind == 'moderation':
        result = {'results': [{'flagged': False, 'category_scores': {'violence': 0.125, 'future_category': 0.375},
                               'future_field': {'preserve': True}}]}
        return (200, {}, json.dumps(result).encode())
    if kind.endswith('-stream'):
        if isinstance(data, dict):
            valid = data.get('stream_format') == 'sse' if path.endswith('/audio/speech') else data.get('stream') is True
        else:
            flags = [p['data'] for p in parts if p['name'] == 'stream']
            valid = flags == ['true']
        if not valid:
            return (400, {}, b'{"error":{"message":"stream flags were not set exactly once"}}')
        if kind == 'chat-stream':
            events = [{'choices': [{'delta': {'content': 'hello', 'tool_calls': [{'index': 0, 'id': 'call_one'}]}}]}]
            raw = ''.join('data: ' + json.dumps(e) + '\n\n' for e in events) + 'data: [DONE]\n\n'
        elif kind == 'image-stream':
            prefix = 'image_edit' if path.endswith('/edits') else 'image_generation'
            events = [{'type': prefix + '.partial_image', 'b64_json': 'AAEC/w==', 'partial_image_index': 0},
                      {'type': 'future_image_event', 'unknown': {'retain': True}},
                      {'type': prefix + '.completed', 'b64_json': 'AAEC/w=='}]
            raw = ''.join('data: ' + json.dumps(e) + '\n\n' for e in events)
        else:
            # Speech event shapes intentionally stay opaque; no unsupported schema claims.
            events = ([{'type': 'fixture.speech.audio', 'audio': 'AAEC/w=='}] if path.endswith('/audio/speech')
                      else [{'type': 'transcript.text.delta', 'delta': 'héllo'}, {'type': 'transcript.text.done', 'text': 'héllo'}])
            raw = ''.join('data: ' + json.dumps(e, ensure_ascii=False) + '\n\n' for e in events)
        encoded = raw.encode()
        return (200, {'Content-Type': 'text/event-stream'}, [encoded[i:i+1] for i in range(len(encoded))])
    return None
