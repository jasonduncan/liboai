"""Synthetic Decisions API fixtures; no model or live account access."""
import json


def decision_fixture(method, path, query, headers, body, parts):
    if headers.get('x-fixture-extension') != 'decisions':
        return None
    if method != 'POST' or not path.endswith('/decisions'):
        return (404, {}, b'{"error":{"message":"wrong Decisions route"}}')
    if isinstance(body, dict) and body.get('fixture_error'):
        return (400, {}, b'{"error":{"type":"invalid_request_error","code":"fixture_decision","param":"questions","message":"synthetic"}}')
    answers = [
        {'type': 'predicate', 'name': 'damaged', 'probability': 0.75, 'future_field': {'preserve': True}},
        {'type': 'choice', 'name': 'route', 'choice': True, 'confidence': 0.8,
         'probabilities': [{'value': True, 'probability': 0.8}, {'value': 'true', 'probability': 0.2, 'future': True}]},
        {'type': 'score', 'name': 'severity', 'score': 1.25, 'confidence': 0.75,
         'probabilities': [{'label': 'low', 'value': 0, 'probability': 0.0}, {'label': 'medium', 'value': 1, 'probability': 0.75},
                           {'label': 'high', 'value': 2, 'probability': 0.25}]},
        {'type': 'refusal', 'name': None, 'future_reason': 'synthetic'},
        {'type': 'future.answer', 'name': {'future_shape': True}, 'payload': [1, 2, 3]},
    ]
    malformed = body.get('fixture_malformed') if isinstance(body, dict) else None
    if malformed == 'choice':
        answers[1]['choice'] = 1
    elif malformed == 'predicate':
        answers[0]['probability'] = 'likely'
    elif malformed == 'score':
        answers[2]['probabilities'][0]['value'] = 1.5
    elif malformed == 'refusal':
        del answers[3]['name']
    result = {'answers': answers, 'model': body.get('model', 'fixture'),
              'usage': {'input_tokens': 8, 'output_tokens': 0, 'total_tokens': 8},
              'future_top_level': {'preserve': True},
              'fixture_request': {'method': method, 'path': path, 'json': body, 'headers': headers, 'query': query}}
    return (200, {}, json.dumps(result).encode())
