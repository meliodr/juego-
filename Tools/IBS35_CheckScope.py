"""Lightweight ownership/syntax gate; does not claim UE compilation or gameplay."""
import ast
import pathlib
import subprocess

root = pathlib.Path(__file__).resolve().parents[1]
base = '1d01db54b63250ed0aacb0704a5ac16290324e32'

def git(*args):
    return subprocess.check_output(['git', '-C', str(root), *args], text=True).splitlines()

paths = set(git('diff', '--name-only', base)) | set(git('ls-files', '--others', '--exclude-standard'))
prefixes = ('ImpactBeyond/Source/ImpactBeyond/IB_S35/', 'ImpactBeyond/Content/IB_S35/', 'Tools/IBS35_', 'docs/IB_S35_')
bad = sorted(p for p in paths if not p.startswith(prefixes))
assert not bad, 'Outside S3.5 ownership: ' + repr(bad)
for path in sorted(root.joinpath('Tools').glob('IBS35_*.py')):
    ast.parse(path.read_text(encoding='utf-8'), filename=str(path))
for path in paths:
    assert pathlib.PurePosixPath(path).suffix not in ('.exe', '.dll', '.pdb', '.obj'), 'Build output in change: ' + path
subprocess.run(['git', '-C', str(root), 'diff', '--check', base], check=True)
print(f'PASS ownership: {len(paths)} changed/new files, all under S3.5 allowlist; Python syntax and diff whitespace clean.')
print('Unreal compilation, generated assets, collisions, animation and performance are NOT tested by this gate.')
