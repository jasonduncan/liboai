#!/usr/bin/env python3
"""Offline runner contracts. All child execution is mocked; no service access."""
import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('live_runner', Path(__file__).with_name('run.py'))
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class RunnerTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='liboai-live-runner-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.key = 'synthetic-private-key'
        for suite in runner.SUITES:
            (self.root / (f'liboai_live_{suite}' + ('.exe' if os.name == 'nt' else ''))).touch()
        self.args = ['--allow-live', '--suite', 'responses', '--binary-dir', str(self.root), '--output-dir', str(self.root / 'results')]
        self.env = patch.dict(os.environ, {'OPENAI_API_KEY': self.key, 'OTHER_API_KEY': 'must-not-forward'})
        self.env.start()
        self.addCleanup(self.env.stop)

    def invoke(self, args=None):
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return runner.main(self.args if args is None else args)

    def complete(self, command, **kwargs):
        self.assertNotIn(self.key, str(command))
        self.assertEqual(kwargs['env']['OPENAI_API_KEY'], self.key)
        self.assertNotIn('OTHER_API_KEY', kwargs['env'])
        self.assertEqual(kwargs['env']['LIBOAI_LIVE_TESTS'], '1')
        output = Path(kwargs['env']['LIBOAI_LIVE_OUTPUT'])
        (output / 'receipt.json').write_text(json.dumps({'schema': 1, 'suite': output.name,
            'failures': 0, 'finished': True, 'pending_cleanup': [], 'records': [{'status': 'PASS'}]}))
        return subprocess.CompletedProcess(command, 0, f'{self.key} sk-fallback-secret\n'.encode())

    def test_opt_in_precedes_credentials_or_execution(self):
        with patch.object(runner, 'load_key') as load, patch.object(runner.subprocess, 'run') as child:
            for args in ([], ['--suite', 'responses'], ['--allow-live']):
                with self.assertRaises(SystemExit): self.invoke(args)
            self.assertEqual(self.invoke(['--list', '--env-file', '/does/not/exist']), 0)
            load.assert_not_called()
            child.assert_not_called()

    def test_explicit_env_file_is_literal_and_fresh(self):
        env_file = self.root / '.env'
        env_file.write_text("export OPENAI_API_KEY='$(touch should-not-exist)'\nUNRELATED=ignored\n")
        self.assertEqual(runner.load_key(env_file, os.environ), '$(touch should-not-exist)')
        env_file.write_text('OPENAI_API_KEY=changed # comment\n')
        self.assertEqual(runner.load_key(env_file, os.environ), 'changed')
        self.assertFalse((self.root / 'should-not-exist').exists())
        env_file.write_text('OPENAI_API_KEY=\n')
        with self.assertRaises(ValueError): runner.load_key(env_file, os.environ)

    def test_success_redacts_and_deduplicates(self):
        with patch.object(runner.subprocess, 'run', side_effect=self.complete) as child:
            self.assertEqual(self.invoke(self.args + ['--suite', 'responses']), 0)
            self.assertEqual(child.call_count, 1)
        for path in (self.root / 'results').rglob('*.log'):
            value = path.read_text()
            self.assertNotIn(self.key, value)
            self.assertNotIn('sk-fallback', value)
        self.assertEqual(self.summary()['status'], 'passed')

    def summary(self):
        return json.loads(next((self.root / 'results').rglob('summary.json')).read_text())

    def test_pending_cleanup_fails_and_stops_later_suites(self):
        def pending(command, **kwargs):
            result = self.complete(command, **kwargs)
            path = Path(kwargs['env']['LIBOAI_LIVE_OUTPUT']) / 'receipt.json'
            value = json.loads(path.read_text())
            value['pending_cleanup'] = [{'type': 'file', 'id': 'file-synthetic'}]
            path.write_text(json.dumps(value))
            return result
        with patch.object(runner.subprocess, 'run', side_effect=pending) as child:
            self.assertEqual(self.invoke(self.args + ['--suite', 'extensions']), 1)
            self.assertEqual(child.call_count, 1)
        self.assertEqual(self.summary()['not_run'], ['extensions'])
        self.assertEqual(self.summary()['suites'][0]['cleanup'], 'pending')

    def test_timeout_marks_unknown_and_redacts(self):
        with patch.object(runner.subprocess, 'run', side_effect=subprocess.TimeoutExpired('synthetic', 1, output=self.key.encode())):
            self.assertEqual(self.invoke(), 1)
        state = self.summary()['suites'][0]
        self.assertEqual(state['status'], 'timed_out')
        self.assertEqual(state['cleanup'], 'unknown')
        self.assertNotIn(self.key, next((self.root / 'results').rglob('process.log')).read_text())

    def test_zero_exit_without_receipt_is_failure(self):
        with patch.object(runner.subprocess, 'run', return_value=subprocess.CompletedProcess([], 0, b'')):
            self.assertEqual(self.invoke(), 1)
        self.assertEqual(self.summary()['suites'][0]['status'], 'invalid_receipt')

    def test_unfinished_receipt_is_not_success(self):
        def unfinished(command, **kwargs):
            result = self.complete(command, **kwargs)
            path = Path(kwargs['env']['LIBOAI_LIVE_OUTPUT']) / 'receipt.json'
            value = json.loads(path.read_text())
            value['finished'] = False
            path.write_text(json.dumps(value))
            return result
        with patch.object(runner.subprocess, 'run', side_effect=unfinished):
            self.assertEqual(self.invoke(), 1)
        self.assertEqual(self.summary()['suites'][0]['cleanup'], 'unknown')

    def test_nonobject_receipt_is_reported(self):
        def malformed(command, **kwargs):
            result = self.complete(command, **kwargs)
            (Path(kwargs['env']['LIBOAI_LIVE_OUTPUT']) / 'receipt.json').write_text('[]')
            return result
        with patch.object(runner.subprocess, 'run', side_effect=malformed):
            self.assertEqual(self.invoke(), 1)
        self.assertEqual(self.summary()['suites'][0]['status'], 'invalid_receipt')


if __name__ == '__main__':
    unittest.main()
