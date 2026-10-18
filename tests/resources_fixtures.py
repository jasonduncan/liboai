"""Stateful synthetic resource workflows. No external requests or model execution."""
import base64
import json
import threading

_lock = threading.Lock()
_state = {}


def resource_fixture(method, path, query, headers, body, parts):
    mode = headers.get('x-fixture-resource')
    if not mode:
        return None
    with _lock:
        return _resource_fixture(mode, method, path, query, body or {}, parts)


def _resource_fixture(mode, method, path, query, body, parts):
    def result(data, status=200):
        return status, {}, json.dumps(data).encode()

    def error(message):
        return result({'error': {'message': message, 'type': 'fixture_state_error'}}, 400)

    if mode == 'conversation':
        if path.endswith('/conversations') and method == 'POST':
            _state[mode] = []
            return result({'id': 'conv_fixture', 'object': 'conversation'})
        if path.endswith('/responses') and method == 'POST':
            if body.get('conversation') != 'conv_fixture' or mode not in _state:
                return error('Missing retained conversation ID')
            _state[mode].append({'id': 'item_' + str(len(_state[mode]) + 1), 'type': 'message', 'role': 'user', 'content': body['input']})
            return result({'id': 'resp_conv', 'status': 'completed', 'turn': len(_state[mode]), 'first_input': _state[mode][0]['content'], 'output': []})
        if path.endswith('/conversations/conv_fixture/items') and method == 'GET':
            index = 1 if query.get('after') == ['item_1'] else 0
            items = _state[mode]
            return result({'data': items[index:index+1], 'has_more': index + 1 < len(items), 'last_id': items[index]['id']})
        if path.endswith('/conversations/conv_fixture') and method == 'DELETE':
            _state.pop(mode, None)
            return result({'id': 'conv_fixture', 'deleted': True})
    if mode in ('vector', 'batch'):
        if path.endswith('/files') and '/vector_stores/' not in path and method == 'POST':
            if not any(p['name'] == 'file' and p['data_base64'] for p in parts):
                return error('Expected uploaded file bytes')
            purpose = next((p['data'] for p in parts if p['name'] == 'purpose'), None)
            if purpose != ('batch' if mode == 'batch' else 'assistants'):
                return error('Incorrect file purpose')
            _state[mode] = {'file': 'file_' + mode, 'polls': 0}
            return result({'id': 'file_' + mode, 'object': 'file'})
        if path.endswith('/files/file_' + mode) and method == 'DELETE':
            return result({'id': 'file_' + mode, 'deleted': True})
    if mode == 'vector':
        state = _state.get(mode, {})
        if path.endswith('/vector_stores') and method == 'POST':
            state['store'] = 'vs_fixture'
            return result({'id': 'vs_fixture', 'status': 'in_progress'})
        if path.endswith('/vector_stores/vs_fixture/files') and method == 'POST':
            if body.get('file_id') != state.get('file'):
                return error('Attach the uploaded file ID')
            state['attached'] = True
            return result({'id': 'file_vector', 'status': 'in_progress'})
        if path.endswith('/vector_stores/vs_fixture/files/file_vector') and method == 'GET':
            if not state.get('attached'):
                return error('File is not attached')
            state['polls'] += 1
            state['ready'] = state['polls'] >= 2
            return result({'id': 'file_vector', 'status': 'completed' if state['ready'] else 'in_progress'})
        if path.endswith('/responses') and method == 'POST':
            if not state.get('ready') or not any(t.get('type') == 'file_search' and t.get('vector_store_ids') == ['vs_fixture'] for t in body.get('tools', [])):
                return error('Search must follow ingestion and reference its store')
            return result({'id': 'resp_search', 'status': 'completed', 'output': [{'type': 'message', 'role': 'assistant', 'content': [{'type': 'output_text', 'text': 'Synthetic source content.', 'annotations': [{'type': 'file_citation', 'file_id': 'file_vector'}]}]}]})
        if path.endswith('/vector_stores/vs_fixture') and method == 'DELETE':
            return result({'id': 'vs_fixture', 'deleted': True})
    if mode == 'batch':
        state = _state.get(mode, {})
        if path.endswith('/batches') and method == 'POST':
            if body.get('input_file_id') != state.get('file') or body.get('endpoint') != '/v1/responses':
                return error('Batch must reference the uploaded input')
            state['created'] = True
            return result({'id': 'batch_fixture', 'status': 'validating'})
        if path.endswith('/batches/batch_fixture') and method == 'GET':
            if not state.get('created'):
                return error('Batch not created')
            state['polls'] += 1
            status = ['validating', 'in_progress', 'completed'][min(state['polls']-1, 2)]
            return result({'id': 'batch_fixture', 'status': status, 'output_file_id': 'file_output' if status == 'completed' else None, 'error_file_id': 'file_error' if status == 'completed' else None})
        if path.endswith('/files/file_output/content'):
            return 200, {'Content-Type': 'application/jsonl'}, b'{"custom_id":"success","response":{"status_code":200}}\n'
        if path.endswith('/files/file_error/content'):
            return 200, {'Content-Type': 'application/jsonl'}, b'{"custom_id":"failure","error":{"code":"synthetic"}}\n'
        if path.endswith('/batches/batch_fixture/cancel'):
            return result({'id': 'batch_fixture', 'status': 'cancelling'})
    if mode == 'uploads':
        if path.endswith('/uploads') and method == 'POST':
            _state[mode] = {'bytes': body['bytes'], 'parts': {}}
            return result({'id': 'upload_fixture', 'status': 'pending'})
        state = _state.get(mode, {})
        if path.endswith('/uploads/upload_fixture/parts') and method == 'POST':
            raw = next((base64.b64decode(p['data_base64']) for p in parts if p['name'] == 'data'), None)
            if raw is None or 'parts' not in state:
                return error('Missing upload or data')
            part_id = 'part_' + str(len(state['parts']) + 1)
            state['parts'][part_id] = raw
            return result({'id': part_id, 'object': 'upload.part'})
        if path.endswith('/uploads/upload_fixture/complete') and method == 'POST':
            assembled = b''.join(state['parts'][part_id] for part_id in body['part_ids'])
            if len(assembled) != state['bytes']:
                return error('Uploaded total differs from declared bytes')
            return result({'id': 'upload_fixture', 'status': 'completed', 'file': {'id': 'file_uploaded', 'bytes': len(assembled)}, 'assembled_base64': base64.b64encode(assembled).decode()})
    if mode == 'container-content' and path.endswith('/content'):
        return 200, {'Content-Type': 'application/octet-stream'}, b'container\x00\xffbytes'
    return error('Unexpected resource workflow route')
