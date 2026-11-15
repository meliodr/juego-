"""Lightweight native policy tests and ownership audit. Never launches Unreal.

Output/build files live in a unique sibling work directory. No packages are installed.
Run: python Tools/IBS4_Check.py [--report docs/IB_S4_NATIVE_RESULTS.json]
"""
import argparse
import ast
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BASE = "1d01db54b63250ed0aacb0704a5ac16290324e32"
PREFIXES = ("ImpactBeyond/Source/ImpactBeyond/IB_S4/", "ImpactBeyond/Content/IB_S4/", "Tools/IBS4_", "docs/IB_S4_")


def run(args, **kwargs):
    result = subprocess.run(args, cwd=ROOT, text=True, capture_output=True, **kwargs)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {args}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    changed = set(run(["git", "diff", "--name-only", BASE]).splitlines())
    changed.update(run(["git", "ls-files", "--others", "--exclude-standard"]).splitlines())
    unexpected = sorted(p for p in changed if not p.startswith(PREFIXES))
    if unexpected:
        raise RuntimeError(f"Files outside S4 ownership: {unexpected}")
    run(["git", "diff", "--check"])
    scripts = list((ROOT / "Tools").glob("IBS4_*.py"))
    for path in scripts:
        ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    source = ROOT / "Tools/IBS4_PolicyTests.cpp"
    include = ROOT / "ImpactBeyond/Source/ImpactBeyond/IB_S4"
    temp_root = ROOT.parent / "IBS4_native_runs"
    temp_root.mkdir(exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix="IBS4_native_", dir=temp_root))
    exe = output / ("IBS4_policy.exe" if os.name == "nt" else "IBS4_policy")
    if os.name == "nt":
        candidates = list(Path(os.environ.get("ProgramFiles", "C:/Program Files")).glob(
            "Microsoft Visual Studio/*/*/VC/Auxiliary/Build/vcvars64.bat"))
        if not candidates:
            raise RuntimeError("MSVC environment not found; no installation attempted")
        vcvars = sorted(candidates)[-1]
        batch = output / "IBS4_build.cmd"
        # All arguments are local paths constructed here; no user-supplied shell text.
        batch.write_text(
            f'@echo off\ncall "{vcvars}" >nul\nif errorlevel 1 exit /b 1\n'
            f'cl /nologo /std:c++17 /EHsc /W4 /WX /I"{include}" "{source}" '
            f'/Fe:"{exe}" /Fo:"{output / "IBS4_policy.obj"}"\n'
            'if errorlevel 1 exit /b 1\n', encoding="utf-8")
        compiler_log = run(["cmd.exe", "/d", "/c", str(batch)])
        compiler = str(vcvars)
    else:
        compiler = shutil.which("c++")
        if not compiler:
            raise RuntimeError("C++ compiler missing")
        compiler_log = run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{include}", str(source), "-o", str(exe)])
    result = run([str(exe)])
    report = {
        "utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "base": BASE,
        "head_at_run": run(["git", "rev-parse", "HEAD"]).strip(),
        "stage": "NATIVE_POLICY_ONLY",
        "ownership": "PASS",
        "changed_files": sorted(changed),
        "python_parse": [p.name for p in scripts],
        "compiler": compiler,
        "compiler_log": compiler_log.replace(str(ROOT), "<checkout>").replace(str(output), "<native-test-temp>"),
        "native_result": result.strip(),
        "source_sha256": {
            str(p.relative_to(ROOT)).replace("\\", "/"): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(include.glob("*")) if p.is_file()
        } | {"Tools/IBS4_PolicyTests.cpp": hashlib.sha256(source.read_bytes()).hexdigest()},
        "unreal_compile": "NOT_RUN",
        "unreal_automation": "NOT_RUN",
        "dx12_audio_gamepad": "NOT_RUN",
    }
    if args.report:
        destination = (ROOT / args.report).resolve()
        if not destination.is_relative_to(ROOT / "docs") or not destination.name.startswith("IB_S4_"):
            raise RuntimeError("Report must be docs/IB_S4_*")
        destination.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(result.strip())
    print(f"PASS ownership: {len(changed)} files; Python parse: {len(scripts)} scripts")
    print(f"Build artifacts: {output}")


if __name__ == "__main__":
    main()
