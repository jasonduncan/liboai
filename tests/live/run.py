#!/usr/bin/env python3
"""Explicit opt-in runner for synthetic, potentially billable OpenAI smoke tests."""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import uuid

SUITES = {
    'responses': '4 bounded generations (including pro), compaction, SSE and lifecycle; stored responses deleted',
    'components': '2 Chat generations, 2 low-quality images, 3 tiny speech requests, transcription/translation, embeddings/moderation',
    'resources': 'small synthetic resource CRUD, vector ingestion, one 1 GB container, one 16-token batch submitted/cancelled',
    'extensions': 'one Decisions request with 3 questions; one synthetic Skill and 2 versions; no hosted execution',
}


def load_key(env_file, environ):
    if env_file is None:
        key = environ.get('OPENAI_API_KEY', '')
    else:
        key = ''
        for line in Path(env_file).read_text(encoding='utf-8').splitlines():
            line = line.strip()
            if line.startswith('export '):
                line = line[7:].lstrip()
            if '=' not in line or line.startswith('#'):
                continue
            name, raw = line.split('=', 1)
            if name.strip() != 'OPENAI_API_KEY':
                continue
            try:
                values = shlex.split(raw, comments=True, posix=True)
            except ValueError:
                raise ValueError('Invalid literal OPENAI_API_KEY assignment') from None
            if len(values) != 1:
                raise ValueError('OPENAI_API_KEY must have one literal value')
            key = values[0]
    if not key or '\n' in key or '\r' in key:
        raise ValueError('A nonempty, single-line OPENAI_API_KEY is required')
    return key


def redact(text, key):
    return re.sub(r'sk-[A-Za-z0-9_.*-]+', '[REDACTED]', text.replace(key, '[REDACTED]'))


def executable(binary_dir, suite, config):
    name = f'liboai_live_{suite}' + ('.exe' if os.name == 'nt' else '')
    for path in (binary_dir / config / name, binary_dir / name):
        if path.is_file():
            return path.resolve()
    raise ValueError(f'Missing compiled {suite} suite; build LIBOAI_BUILD_LIVE_TESTS first')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--list', action='store_true', help='Show suite scope without reading credentials or executing anything')
    parser.add_argument('--suite', action='append', choices=SUITES, help='Explicit selection; repeat to run multiple suites once each')
    parser.add_argument('--allow-live', action='store_true', help='Authorize live requests and the selected resource/model usage')
    parser.add_argument('--binary-dir', type=Path, default=Path('build/live-tests/tests/live'))
    parser.add_argument('--config', choices=('Release', 'Debug'), default='Release')
    parser.add_argument('--env-file', type=Path, help='Read only OPENAI_API_KEY as a literal; never source shell code')
    parser.add_argument('--output-dir', type=Path, default=Path('build/live-results'))
    parser.add_argument('--timeout', type=int, default=1200, help='Maximum seconds per suite (1..1800); interrupted runs may leave cleanup pending')
    args = parser.parse_args(argv)
    if args.list:
        for name, scope in SUITES.items():
            print(f'{name}: {scope}')
        return 0
    if not args.allow_live or not args.suite:
        parser.error('Both --allow-live and an explicit --suite are required; use --list to inspect scope')
    if not 1 <= args.timeout <= 1800:
        parser.error('--timeout must be between 1 and 1800 seconds')
    suites = list(dict.fromkeys(args.suite))
    try:
        binaries = {suite: executable(args.binary_dir, suite, args.config) for suite in suites}
        key = load_key(args.env_file, os.environ)
    except (OSError, ValueError) as error:
        # Input exception text can contain credential material: keep this generic.
        print('Cannot prepare live run: check compiled binaries, credential source and literal key assignment.', file=sys.stderr)
        return 2
    run_id = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + uuid.uuid4().hex[:12]
    output = args.output_dir.resolve() / run_id
    output.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    # Do not forward other provider credentials or ambient organization/project selection.
    for name in list(env):
        if name.endswith('_API_KEY') or name in ('OPENAI_ORG_ID', 'OPENAI_PROJECT_ID'):
            env.pop(name)
    env['OPENAI_API_KEY'] = key
    env['LIBOAI_LIVE_TESTS'] = '1'
    summary = {'schema': 1, 'run_id': run_id, 'suites': [], 'status': 'running'}
    summary_path = output / 'summary.json'

    def save():
        summary_path.write_text(json.dumps(summary, indent=2) + '\n', encoding='utf-8')

    save()
    print(f'Live receipts: {output}', flush=True)
    failed = False
    for suite in suites:
        directory = output / suite
        directory.mkdir()
        env['LIBOAI_LIVE_OUTPUT'] = str(directory)
        print(f'Running {suite}: {SUITES[suite]}', flush=True)
        state = {'suite': suite, 'status': 'running', 'cleanup': 'unknown'}
        summary['suites'].append(state)
        save()
        raw = ''
        try:
            result = subprocess.run([str(binaries[suite])], env=env, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, timeout=args.timeout)
            raw = result.stdout.decode('utf-8', errors='replace')
            state['exit_code'] = result.returncode
            state['status'] = 'passed' if result.returncode == 0 else 'failed'
        except subprocess.TimeoutExpired as error:
            raw = (error.stdout or b'').decode('utf-8', errors='replace')
            state['status'] = 'timed_out'
        except KeyboardInterrupt:
            state['status'] = 'interrupted'
        except OSError:
            state['status'] = 'launch_failed'
        safe = redact(raw, key)
        (directory / 'process.log').write_text(safe, encoding='utf-8')
        print(safe, end='', flush=True)
        try:
            receipt = json.loads((directory / 'receipt.json').read_text(encoding='utf-8'))
            valid = (isinstance(receipt, dict) and receipt.get('schema') == 1 and receipt.get('suite') == suite
                     and type(receipt.get('failures')) is int and receipt['failures'] >= 0
                     and type(receipt.get('finished')) is bool
                     and isinstance(receipt.get('pending_cleanup'), list)
                     and isinstance(receipt.get('records'), list))
            if not valid:
                raise ValueError('Invalid receipt')
            pending = receipt['pending_cleanup']
            state['cleanup'] = 'pending' if pending else ('known_ids_clear' if receipt['finished'] else 'unknown')
            state['pending_count'] = len(pending)
            if receipt['failures'] or pending or not receipt['finished']:
                state['status'] = 'failed' if state['status'] == 'passed' else state['status']
        except (OSError, ValueError, TypeError):
            if state['status'] == 'passed':
                state['status'] = 'invalid_receipt'
        # A timeout may have interrupted a create before its ID was returned.
        if state['status'] in ('timed_out', 'interrupted', 'launch_failed'):
            state['cleanup'] = 'unknown'
        failed |= state['status'] != 'passed'
        save()
        if failed:
            break  # Do not spend on later selected suites after a failure.
    summary['status'] = 'failed' if failed else 'passed'
    summary['not_run'] = suites[len(summary['suites']):]
    save()
    print(f'Run {summary["status"]}; inspect receipt.json for owned IDs and pending cleanup.', flush=True)
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
