"""Native-only C++17 tests. No Unreal, UHT/UBT, renderer, network or installation."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BASE = "1d01db54b63250ed0aacb0704a5ac16290324e32"
PREFIXES = ("ImpactBeyond/Source/ImpactBeyond/IB_S6/Wave8Heroes/",
            "ImpactBeyond/Source/ImpactBeyond/IB_S6H/",
            "ImpactBeyond/Content/IB_S6/Wave8Heroes/", "Tools/IBS6H_", "docs/IB_S6H_")


def run(args, cwd=ROOT):
    p = subprocess.run(args, cwd=cwd, text=True, capture_output=True)
    if p.returncode:
        raise RuntimeError(f"{args}\nexit={p.returncode}\n{p.stdout}\n{p.stderr}")
    return p.stdout + p.stderr


def main():
    changed = set(run(["git", "diff", "--name-only", BASE]).splitlines())
    changed.update(run(["git", "ls-files", "--others", "--exclude-standard"]).splitlines())
    unexpected = sorted(p for p in changed if not p.startswith(PREFIXES))
    if unexpected:
        raise RuntimeError(f"Ownership violation: {unexpected}")
    run(["git", "diff", "--check"])
    run(["git", "merge-base", "--is-ancestor", BASE, "HEAD"])
    candidates = sorted(Path(os.environ.get("ProgramFiles", "C:/Program Files")).glob(
        "Microsoft Visual Studio/*/*/VC/Auxiliary/Build/vcvars64.bat"))
    if not candidates:
        raise RuntimeError("MSVC not found; installation is not authorized")
    out = Path(tempfile.mkdtemp(prefix="IBS6H_", dir=ROOT.parent))
    source = ROOT / "Tools/IBS6H_Tests.cpp"
    header = ROOT / "ImpactBeyond/Source/ImpactBeyond/IB_S6/Wave8Heroes/IBS6Heroes.h"
    exe = out / "IBS6H_tests.exe"
    batch = out / "IBS6H_build.cmd"
    batch.write_text(
        f'@echo off\ncall "{candidates[-1]}" >nul\nif errorlevel 1 exit /b 1\n'
        f'cl /nologo /std:c++17 /EHsc /W4 /WX /I"{header.parent}" "{source}" '
        f'/Fe:"{exe}" /Fo:"{out / "IBS6H_tests.obj"}"\n'
        'if errorlevel 1 exit /b 1\n', encoding="utf-8")
    build = run(["cmd.exe", "/d", "/c", str(batch)], cwd=out)
    tests = run([str(exe)], cwd=out)
    result = {
        "utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "base": BASE, "head_at_run": run(["git", "rev-parse", "HEAD"]).strip(),
        "branch": run(["git", "branch", "--show-current"]).strip(),
        "evidence": ["CODIGO", "COMPILADO_NATIVE_MSVC", "TEST_OFFLINE_MOCK"],
        "compiler_setup": str(candidates[-1]), "build_output": build, "test_output": tests,
        "source_sha256": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                          for p in (header, source, Path(__file__).resolve())},
        "ownership": "PASS", "changed_paths": sorted(changed),
        "skipped": ["Unreal/UHT/UBT", "10 S3 real cross-cell jumps", "DX12/audio",
                    "physical gamepad", "VFX GPU cost", "Manny/rig/animation"],
    }
    report = ROOT / "docs/IB_S6H_NATIVE_RESULTS.json"
    report.parent.mkdir(exist_ok=True)
    report.write_text(json.dumps(result, indent=2, ensure_ascii=False)+"\n", encoding="utf-8")
    print(build + tests)
    print(f"Report: {report}\nBuild files: {out}")


if __name__ == "__main__":
    main()
