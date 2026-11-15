"""Read S4 in place, compile one tiny test in this task folder, save evidence under S3.5 docs.
No Unreal, writes to S4, network, git mutations, or runtime dependency on S4.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def run(args, **kwargs):
    result = subprocess.run(args, text=True, capture_output=True, **kwargs)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return result.stdout.strip()

def enum_names(path, name):
    match = re.search(r'enum class ' + name + r'\s*:\s*\w+\s*\{([^}]+)\}', path.read_text(encoding='utf-8'))
    assert match, name
    return [item.strip() for item in match.group(1).split(',')]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--s4-root', type=Path, default=ROOT.parents[1] / 'task-4' / 'impact-s4-response')
    args = parser.parse_args()
    s4 = args.s4_root.resolve()
    include = s4 / 'ImpactBeyond/Source/ImpactBeyond/IB_S4'
    names = ('IBS4Policy.h', 'IBS4ResponseComponent.h', 'IBS4ResponseComponent.cpp',
             'IBS4PreparedTarget.h', 'IBS4PreparedTarget.cpp', 'IBS4PowerComponent.h',
             'IBS4PowerComponent.cpp', 'IBS4Threat.cpp')
    def hashes():
        return {name: hashlib.sha256((include / name).read_bytes()).hexdigest() for name in names}
    before = hashes()
    assert enum_names(ROOT / 'ImpactBeyond/Source/ImpactBeyond/IB_S35/IBS35Types.h', 'EIBS35DamageKind') == [
        'QuickPunch', 'HeavyPunch', 'HeldTool', 'ThrownObject']
    assert enum_names(include / 'IBS4ResponseComponent.h', 'EIBS4Weapon') == ['Bullet', 'Missile', 'Laser', 'Punch']
    vcvars = Path(r'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat')
    assert vcvars.is_file(), 'Existing MSVC required; nothing will be installed.'
    # Keep artifacts within our assigned task folder. The managed Windows TEMP can be inaccessible.
    # No recursive deletion or permission changes; outputs remain outside the Git checkout.
    output = Path(tempfile.mkdtemp(prefix='IBS35-contract-', dir=ROOT.parent))
    assert output.resolve().is_relative_to(ROOT.parent.resolve())
    program = output / 'IBS35_CompareS4.exe'
    batch = output / 'IBS35_Build.cmd'
    source = ROOT / 'Tools/IBS35_CompareS4.cpp'
    batch.write_text(
        f'@echo off\ncall "{vcvars}" >nul\nif errorlevel 1 exit /b 1\n'
        f'cl /nologo /std:c++17 /EHsc /W4 /WX /I"{include}" "{source}" '
        f'/Fo"{output / "IBS35_CompareS4.obj"}" /Fe"{program}"\n'
        'if errorlevel 1 exit /b 1\n', encoding='utf-8')
    build = run([os.environ.get('ComSpec', 'cmd.exe'), '/d', '/c', str(batch)], cwd=output)
    observed = run([str(program)], cwd=output)
    assert before == hashes(), 'S4 sources changed during review; inspect the new snapshot before claiming a comparison.'
    ref = s4 / '.git/refs/heads/prototype/ib-s4-response-lab'
    report = {
        'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'stage': 'NATIVE_CONTRACT_COMPARISON_ONLY',
        's4_observed_branch_ref': ref.read_text().strip() if ref.is_file() else None,
        's4_sha256': before,
        's4_sources_unchanged_during_test': True,
        'compiler': 'MSVC C++17 /W4 /WX',
        'compiler_output': build,
        'result': observed,
        'unreal_build_and_runtime': 'NOT_RUN',
        'runtime_adapter_installed': False,
    }
    destination = ROOT / 'docs/IB_S35_S4_NATIVE_RESULTS.json'
    destination.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(observed)
    print('PASS S4 read-only snapshot unchanged; report: docs/IB_S35_S4_NATIVE_RESULTS.json')

if __name__ == '__main__':
    main()
