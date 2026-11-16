"""Read-only incoming-commit scope gate. Never fetch, merge, build or approve assets."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess

SCOPES = {
    'S25': ('ImpactBeyond/Content/IB_S25/', 'Tools/IBS25_', 'docs/IB_S25_'),
    'S35': ('ImpactBeyond/Source/ImpactBeyond/IB_S35/', 'ImpactBeyond/Content/IB_S35/',
            'ImpactBeyond/Source/ImpactBeyond/IB_Integration/ImpactContract', 'Tools/IBS35_', 'docs/IB_S35_'),
    'S4': ('ImpactBeyond/Source/ImpactBeyond/IB_S4/', 'ImpactBeyond/Content/IB_S4/', 'Tools/IBS4_', 'docs/IB_S4_'),
    'S6': ('ImpactBeyond/Source/ImpactBeyond/IB_S6/', 'ImpactBeyond/Source/ImpactBeyond/IB_S6H/',
           'ImpactBeyond/Content/IB_S6H/', 'Tools/IBS6H_', 'docs/IB_S6H_'),
    'S7': ('ImpactBeyond/Source/ImpactBeyond/IB_S7/', 'ImpactBeyond/Source/ImpactBeyond/IB_S7T/',
           'ImpactBeyond/Content/IB_S7T/', 'Tools/IBS7T_', 'docs/IB_S7T_'),
}

def git(repo, *args):
    return subprocess.check_output(['git', '-C', str(repo), *args], stderr=subprocess.PIPE,
        env=dict(os.environ, GIT_NO_LAZY_FETCH='1', GIT_OPTIONAL_LOCKS='0')).decode()

def inspect(repo, owner, base, head):
    result = {'owner': owner, 'base': base, 'head': head, 'status': 'BLOCKED',
              'meaning': 'Scope only; no build, license, ABI or gameplay approval'}
    if owner not in SCOPES:
        return dict(result, reason='UNKNOWN_OWNER')
    if not head:
        return dict(result, reason='AWAITING_DELIVERY')
    if not all(isinstance(x, str) and re.fullmatch('[0-9a-f]{40}', x) for x in (base, head)):
        return dict(result, reason='EXACT_COMMIT_REQUIRED')
    try:
        for ref in (base, head):
            if git(repo, 'rev-parse', '--verify', ref+'^{commit}').strip() != ref:
                return dict(result, reason='EXACT_COMMIT_REQUIRED')
        if git(repo, 'merge-base', base, head).strip() != base:
            return dict(result, reason='BASE_NOT_ANCESTOR')
        # Include deletions and both sides of renames: never hide a shared-file removal.
        paths = git(repo, 'diff', '--no-renames', '--name-only', '-z', base, head).split('\0')[:-1]
        violations = [p for p in paths if not p.startswith(SCOPES[owner])]
        return dict(result, status='STATIC_PASS' if paths and not violations else 'BLOCKED',
                    reason='SCOPE_ONLY' if paths and not violations else ('OUTSIDE_OWNER_SCOPE' if violations else 'EMPTY_DELIVERY'),
                    changed_paths=paths, violations=violations)
    except subprocess.CalledProcessError:
        return dict(result, reason='OBJECTS_UNAVAILABLE_OR_GIT_ERROR')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=Path('.'))
    parser.add_argument('--owner', choices=SCOPES, required=True)
    parser.add_argument('--base', required=True)
    parser.add_argument('--head')
    args = parser.parse_args()
    result = inspect(args.repo, args.owner, args.base, args.head)
    print(json.dumps(result, indent=2))
    return 0 if result['status']=='STATIC_PASS' else 1

if __name__ == '__main__':
    raise SystemExit(main())
