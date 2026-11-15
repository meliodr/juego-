"""Unit tests for the static git branch/ownership audit. No Unreal required."""
import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).parents[1] / 'IBAudit_ValidateLabs.py'
spec = importlib.util.spec_from_file_location('ib_audit', SCRIPT)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def cmd(cwd, *args):
    return subprocess.run(['git', '-C', str(cwd), *args], capture_output=True, text=True, check=True).stdout.strip()


class AuditSmoke(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.repo = Path(self.tmp.name)
        cmd(self.repo, 'init', '-q')
        cmd(self.repo, 'config', 'user.name', 'IB QA')
        cmd(self.repo, 'config', 'user.email', 'ib@example.invalid')
        (self.repo / 'README.md').write_text('base\n')
        cmd(self.repo, 'add', '.')
        cmd(self.repo, 'commit', '-qm', 'base')
        self.base = cmd(self.repo, 'rev-parse', 'HEAD')

    def tearDown(self):
        self.tmp.cleanup()

    def make_branch(self, file):
        cmd(self.repo, 'checkout', '-qb', 'prototype/ib-s25-realism-lab')
        f = self.repo / file
        f.parent.mkdir(parents=True, exist_ok=True)
        f.write_text('content\n')
        cmd(self.repo, 'add', '.')
        cmd(self.repo, 'commit', '-qm', 'test')

    def test_detects_missing_branch(self):
        result = mod.verify(self.repo, self.base, 'S25')
        self.assertEqual(result['status'], 'NO_BRANCH')

    def test_safe_owned_file(self):
        self.make_branch('docs/IB_S25_REVIEW.md')
        result = mod.verify(self.repo, self.base, 'S25')
        self.assertEqual(result['status'], 'STATIC_PASS')
        self.assertEqual(result['files_changed'], 1)

    def test_shared_code_is_rejected(self):
        self.make_branch('ImpactBeyond/Source/ImpactBeyond/ImpactBeyondCharacter.cpp')
        result = mod.verify(self.repo, self.base, 'S25')
        self.assertEqual(result['status'], 'STATIC_FAIL')
        self.assertIn('ImpactBeyond/Source/ImpactBeyond/ImpactBeyondCharacter.cpp', result['ownership_violations'])


if __name__ == '__main__':
    unittest.main()
