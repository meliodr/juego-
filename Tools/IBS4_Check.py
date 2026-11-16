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
    parser.add_argument("--contract-root", type=Path,
                        help="Optional read-only S3.5 ImpactContract directory; never copied into S4")
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
    contract_root = args.contract_root.resolve() if args.contract_root else None
    contract_header = contract_root / "ImpactContract.h" if contract_root else None
    contract_before = hashlib.sha256(contract_header.read_bytes()).hexdigest() if contract_header else None
    if os.name == "nt":
        candidates = list(Path(os.environ.get("ProgramFiles", "C:/Program Files")).glob(
            "Microsoft Visual Studio/*/*/VC/Auxiliary/Build/vcvars64.bat"))
        if not candidates:
            raise RuntimeError("MSVC environment not found; no installation attempted")
        vcvars = sorted(candidates)[-1]
        compiler = str(vcvars)
    else:
        compiler = shutil.which("c++")
        if not compiler:
            raise RuntimeError("C++ compiler missing")

    def compile_and_run(test_source, stem, includes):
        exe = output / (stem + (".exe" if os.name == "nt" else ""))
        if os.name == "nt":
            # Paths are local filesystem inputs, never raw shell fragments. Percent
            # and quote expansion must not turn an optional include into cmd code.
            for path in (vcvars, test_source, output, *includes):
                if any(c in str(path) for c in ('%', '"', '\r', '\n')):
                    raise RuntimeError("Unsupported cmd metacharacter in native test path")
            batch = output / (stem + "_build.cmd")
            include_flags = " ".join(f'/I"{p}"' for p in includes)
            batch.write_text(
                f'@echo off\nsetlocal DisableDelayedExpansion\ncall "{vcvars}" >nul\nif errorlevel 1 exit /b 1\n'
                f'cl /nologo /std:c++17 /EHsc /W4 /WX {include_flags} "{test_source}" '
                f'/Fe:"{exe}" /Fo:"{output / (stem + ".obj")}"\n'
                'if errorlevel 1 exit /b 1\n', encoding="utf-8")
            log = run(["cmd.exe", "/d", "/c", str(batch)])
        else:
            log = run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                       *(f"-I{p}" for p in includes), str(test_source), "-o", str(exe)])
        return log, run([str(exe)])

    compiler_log, result = compile_and_run(source, "IBS4_policy", (include,))
    contract_result = {"status": "NOT_RUN", "unreal_bridge": "NOT_RUN"}
    if contract_root:
        contract_source = ROOT / "Tools/IBS4_ContractConsumerTests.cpp"
        log, observed = compile_and_run(contract_source, "IBS4_contract_consumer", (include, contract_root))
        if hashlib.sha256(contract_header.read_bytes()).hexdigest() != contract_before:
            raise RuntimeError("External ImpactContract changed during tests; no stable snapshot claimed")
        contract_result = {"status": "PASS", "result": observed.strip(),
                           "external_read_only_header": str(contract_header),
                           "external_header_sha256": contract_before,
                           "fixture_sha256": hashlib.sha256(contract_source.read_bytes()).hexdigest(),
                           "unreal_bridge": "NOT_RUN"}
        compiler_log += log
        print(observed.strip())
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
        "canonical_consumer": contract_result,
        "source_sha256": {
            str(p.relative_to(ROOT)).replace("\\", "/"): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(include.glob("*")) if p.is_file()
        } | {str(p.relative_to(ROOT)).replace("\\", "/"): hashlib.sha256(p.read_bytes()).hexdigest()
             for p in (source, ROOT / "Tools/IBS4_ConsumerFixtures.h", ROOT / "Tools/IBS4_ContractConsumerTests.cpp")},
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
