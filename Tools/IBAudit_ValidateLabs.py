#!/usr/bin/env python3
"""Git-only gate for IMPACT: BEYOND's four isolated lab branches.

This script never starts Unreal, fetches assets, changes branches, or writes files.
Its PASS means only: branch ancestry and changed paths respect ownership.
It does not certify compilation, gameplay, licenses, fps or human review.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

BASE_DEFAULT = '1d01db54b63250ed0aacb0704a5ac16290324e32'
LABS = {
    'S25': {
        'branch': 'prototype/ib-s25-realism-lab',
        'paths': ('ImpactBeyond/Content/IB_S25/', 'Tools/IBS25_', 'docs/IB_S25_'),
    },
    'S3': {
        'branch': 'prototype/ib-s3-planet-lab',
        'paths': ('ImpactBeyond/Source/ImpactBeyond/IB_S3/', 'ImpactBeyond/Content/IB_S3/', 'Tools/IBS3_', 'docs/IB_S3_'),
    },
    'S35': {
        'branch': 'prototype/ib-s35-interactions-lab',
        'paths': ('ImpactBeyond/Source/ImpactBeyond/IB_S35/', 'ImpactBeyond/Content/IB_S35/', 'Tools/IBS35_', 'docs/IB_S35_'),
    },
    'S4': {
        'branch': 'prototype/ib-s4-response-lab',
        'paths': ('ImpactBeyond/Source/ImpactBeyond/IB_S4/', 'ImpactBeyond/Content/IB_S4/', 'Tools/IBS4_', 'docs/IB_S4_'),
    },
}


def git(repo: Path, *args: str, allow_failure: bool = False) -> subprocess.CompletedProcess[bytes]:
    p = subprocess.run(['git', '-C', str(repo), *args], capture_output=True, check=False)
    if p.returncode and not allow_failure:
        raise RuntimeError('git ' + ' '.join(args) + ': ' + p.stderr.decode('utf8', 'replace').strip())
    return p


def verify(repo: Path, base: str, code: str) -> dict:
    spec = LABS[code]
    branch = spec['branch']
    base_ref = git(repo, 'rev-parse', '--verify', f'{base}^{{commit}}', allow_failure=True)
    if base_ref.returncode:
        return {'lab': code, 'branch': branch, 'status': 'NO_BASE', 'errors': ['Pinned base commit missing locally. No automatic fetch performed.']}
    base_sha = base_ref.stdout.decode().strip()
    head_ref = git(repo, 'rev-parse', '--verify', f'refs/heads/{branch}^{{commit}}', allow_failure=True)
    if head_ref.returncode:
        head_ref = git(repo, 'rev-parse', '--verify', f'refs/remotes/origin/{branch}^{{commit}}', allow_failure=True)
    if head_ref.returncode:
        return {'lab': code, 'branch': branch, 'base_sha': base_sha, 'status': 'NO_BRANCH', 'errors': ['No local or origin-tracking lab branch; not evidence that the remote lacks it.']}
    head_sha = head_ref.stdout.decode().strip()
    merge_base = git(repo, 'merge-base', base_sha, head_sha, allow_failure=True)
    common_sha = merge_base.stdout.decode().strip() if merge_base.returncode == 0 else None
    lineage_ok = common_sha == base_sha
    diff = git(repo, 'diff', '--name-only', '--no-renames', '-z', f'{base_sha}...{head_sha}', allow_failure=True)
    files = [p.decode('utf8', 'replace') for p in diff.stdout.split(b'\0') if p] if diff.returncode == 0 else []
    violations = [name for name in files if not name.startswith(spec['paths'])]
    errors = []
    if not lineage_ok:
        errors.append(f'Pinned base is not a commit ancestor (merge-base={common_sha}).')
    if diff.returncode:
        errors.append('git diff failed: '+diff.stderr.decode('utf8', 'replace').strip())
    if not files and not errors:
        errors.append('No tracked changes against pinned base.')
    if violations:
        errors.append('Modified outside exclusive owner prefixes: ' + ', '.join(violations))
    result = {
        'lab': code, 'branch': branch, 'base_sha': base_sha, 'head_sha': head_sha,
        'status': 'STATIC_PASS' if not errors else 'STATIC_FAIL',
        'files_changed': len(files), 'changed_paths': files,
        'lineage_ok': lineage_ok, 'ownership_violations': violations,
        'errors': errors,
    }
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=Path('.'))
    parser.add_argument('--base', default=BASE_DEFAULT)
    parser.add_argument('--lab', action='append', choices=LABS.keys(), help='One or more labs; default all')
    parser.add_argument('--json', action='store_true', help='Print machine-readable report')
    parser.add_argument('--require-all', action='store_true', help='Also fail when a branch is absent')
    args = parser.parse_args()
    repo = args.repo.resolve()
    try:
        git(repo, 'rev-parse', '--show-toplevel')
        results = [verify(repo, args.base, code) for code in (args.lab or list(LABS))]
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    report = {
        'schema': 'ib-lab-path-gate-v1', 'base_expected': args.base,
        'meaning_of_pass': 'git ancestry and exclusive changed paths ONLY; not a build or gameplay approval',
        'labs': results,
    }
    if args.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        for result in results:
            print(f"{result['lab']}: {result['status']} · {result['branch']} · {result.get('files_changed', 0)} changed files")
            for error in result['errors']:
                print('  - ' + error)
        print('STATIC_PASS does NOT mean compiled, DX12 rendered, gameplay-ready or licensed.')
    failing = any(r['status'] == 'STATIC_FAIL' for r in results)
    missing = args.require_all and any(r['status'] in ('NO_BASE', 'NO_BRANCH') for r in results)
    return 1 if failing or missing else 0


if __name__ == '__main__':
    sys.exit(main())
