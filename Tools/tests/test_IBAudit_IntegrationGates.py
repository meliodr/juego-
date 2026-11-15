"""Independent regression checks for the four S2-derived, isolated lab contracts.

No Unreal, assets, network or real project files are accessed.
"""
import importlib.util
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).parents[1] / 'IBAudit_ValidateLabs.py'
spec = importlib.util.spec_from_file_location('ib_audit_integration', SCRIPT)
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def git(repo, *args):
    result = subprocess.run(['git', '-C', str(repo), *args], check=True,
                            capture_output=True, text=True)
    return result.stdout.strip()


class IntegrationGateRegression(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)
        git(self.repo, 'init', '-q')
        git(self.repo, 'config', 'user.email', 'qa@example.invalid')
        git(self.repo, 'config', 'user.name', 'IMPACT QA')
        (self.repo / 'README.md').write_text('base\n', encoding='utf-8')
        git(self.repo, 'add', '.')
        git(self.repo, 'commit', '-qm', 'baseline')
        self.base = git(self.repo, 'rev-parse', 'HEAD')

    def tearDown(self):
        self.temporary.cleanup()

    def add_lab(self, lab, path):
        git(self.repo, 'checkout', '-qb', audit.LABS[lab]['branch'])
        target = self.repo / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text('test\n', encoding='utf-8')
        git(self.repo, 'add', '.')
        git(self.repo, 'commit', '-qm', f'test {lab}')

    def test_each_laboratory_accepts_only_its_own_subtree(self):
        owned = {
            'S25': 'ImpactBeyond/Content/IB_S25/Facade.uasset',
            'S3': 'ImpactBeyond/Source/ImpactBeyond/IB_S3/Regions.cpp',
            'S35': 'ImpactBeyond/Source/ImpactBeyond/IB_S35/Grab.cpp',
            'S4': 'ImpactBeyond/Source/ImpactBeyond/IB_S4/Alerts.cpp',
        }
        for lab, path in owned.items():
            with self.subTest(lab=lab):
                if lab != 'S25':
                    git(self.repo, 'checkout', '-q', self.base)
                self.add_lab(lab, path)
                result = audit.verify(self.repo, self.base, lab)
                self.assertEqual(result['status'], 'STATIC_PASS', result)
                self.assertTrue(result['lineage_ok'])
                self.assertEqual(result['changed_paths'], [path])

    def test_canonical_character_edit_is_rejected_in_s3(self):
        path = 'ImpactBeyond/Source/ImpactBeyond/ImpactBeyondCharacter.cpp'
        self.add_lab('S3', path)
        result = audit.verify(self.repo, self.base, 'S3')
        self.assertEqual(result['status'], 'STATIC_FAIL')
        self.assertIn(path, result['ownership_violations'])

    def test_shared_map_edit_is_rejected_in_s4(self):
        path = 'ImpactBeyond/Content/IB_C00/Lvl_IB_C00.umap'
        self.add_lab('S4', path)
        result = audit.verify(self.repo, self.base, 'S4')
        self.assertEqual(result['status'], 'STATIC_FAIL')
        self.assertIn(path, result['ownership_violations'])

    def test_require_all_fails_without_lab_branches(self):
        done = subprocess.run([sys.executable, str(SCRIPT), '--repo', str(self.repo),
                               '--base', self.base, '--require-all', '--json'],
                              capture_output=True, text=True)
        self.assertEqual(done.returncode, 1, done.stderr)
        self.assertIn('NO_BRANCH', done.stdout)

    def test_wrong_base_fails_closed_when_required(self):
        done = subprocess.run([sys.executable, str(SCRIPT), '--repo', str(self.repo),
                               '--base', '0' * 40, '--require-all', '--json'],
                              capture_output=True, text=True)
        self.assertEqual(done.returncode, 1, done.stderr)
        self.assertIn('NO_BASE', done.stdout)


if __name__ == '__main__':
    unittest.main()
