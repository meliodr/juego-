"""Compile integrated policy fixtures without Unreal or licensed assets.

Every listed fixture is required. Outputs stay outside the checkout. The report
contains repository-relative names and hashes, never machine-specific paths.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

SOURCE = "ImpactBeyond/Source/ImpactBeyond"
SUITES = (
    ("s3-core", "Tools/IBS3_CoreTests.cpp", None),
    ("s3-review", "Tools/IBS3_ReviewTests.cpp", None),
    ("s35-policy", "Tools/IBS35_TestPolicy.cpp", None),
    ("s35-s4", "Tools/IBS35_CompareS4.cpp", None),
    ("s35-punch", SOURCE + "/IB_S35/IBS35PunchComponentTests.cpp", "IBS35_PUNCH_NATIVE_TEST_MAIN"),
    ("s4-policy", "Tools/IBS4_PolicyTests.cpp", None),
    ("s4-contract", "Tools/IBS4_ContractConsumerTests.cpp", None),
    ("impact-contract", SOURCE + "/IB_Integration/ImpactContract/Tests/ImpactContractNative.cpp", "IB_IMPACT_NATIVE_TEST"),
    ("s5-clock-civics", SOURCE + "/IB_S5/Tests/IBS5NativeTests.cpp", "IBS5_NATIVE_TEST_MAIN"),
    ("s6-heroes", "Tools/IBS6H_Tests.cpp", None),
    ("s7-threats-wheel", "Tools/IBS7T_NativeTests.cpp", None),
    ("s7-persistence", SOURCE + "/IB_S7/Wave8Threats/Tests/ThreatPersistenceTests.cpp", "IBS7T_STANDALONE_TESTS"),
    ("s7-wheel-refresh", SOURCE + "/IB_S7/Wave8Threats/Radial/PowerWheelRefreshTests.cpp", "IBS7_WHEEL_REFRESH_NATIVE_TEST_MAIN"),
)
INCLUDES = (SOURCE, SOURCE + "/IB_S35", SOURCE + "/IB_S4", SOURCE + "/IB_S6/Wave8Heroes",
            SOURCE + "/IB_Integration/ImpactContract")


def run(command, *, cwd, env):
    result = subprocess.run(command, cwd=cwd, env=env, text=True,
                            encoding="utf-8", errors="replace", capture_output=True)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return result.stdout.strip()


def source_hashes(root):
    paths = {root / source for _, source, _ in SUITES}
    paths.update((root / SOURCE).rglob("*.h"))
    paths.update((root / "Tools").glob("IBS*_*.h"))
    return {path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(paths)}


def compiler_environment(requested, output):
    env = dict(os.environ)
    compiler = requested or env.get("CXX") or ("cl" if os.name == "nt" else "c++")
    if shutil.which(compiler):
        return compiler, env
    if os.name != "nt" or compiler.lower() not in ("cl", "cl.exe"):
        raise RuntimeError("C++17 compiler not found; set CXX or --compiler. Nothing is installed.")
    base = Path(env.get("ProgramFiles", "C:/Program Files"))
    candidates = sorted(base.glob("Microsoft Visual Studio/*/*/VC/Auxiliary/Build/vcvars64.bat"))
    if not candidates:
        raise RuntimeError("Existing MSVC toolchain not found. Nothing is installed.")
    vcvars = str(candidates[-1])
    if any(character in vcvars for character in ('%', '!', '"', '\r', '\n')):
        raise RuntimeError("Unsupported shell metacharacter in MSVC installation path")
    batch = output / "native-toolchain.cmd"
    batch.write_text('@echo off\nsetlocal DisableDelayedExpansion\ncall "' + vcvars +
                     '" >nul\nif errorlevel 1 exit /b 1\nset\n', encoding="utf-8")
    result = subprocess.run([env.get("ComSpec", "cmd.exe"), "/d", "/c", str(batch)],
                            capture_output=True, text=True, errors="replace", env=env)
    if result.returncode:
        raise RuntimeError("Could not initialize the existing MSVC toolchain")
    for line in result.stdout.splitlines():
        key, separator, value = line.partition("=")
        if separator and key and not key.startswith("="):
            env[key.upper()] = value
    resolved = shutil.which(compiler, path=env.get("PATH", env.get("Path")))
    if not resolved:
        raise RuntimeError("MSVC initialization did not provide cl.exe")
    return resolved, env


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path)
    parser.add_argument("--compiler", help="Executable name or path, without shell flags")
    args = parser.parse_args()
    root = args.root.resolve()
    missing = [source for _, source, _ in SUITES if not (root / source).is_file()]
    if missing:
        raise RuntimeError("Incomplete integration export; missing required fixtures: " + ", ".join(missing))
    output = (args.output or Path(tempfile.mkdtemp(prefix="IBCI_native_"))).resolve()
    if output == root or root in output.parents:
        raise RuntimeError("Native build output must be outside the source checkout")
    output.mkdir(parents=True, exist_ok=True)
    before = source_hashes(root)
    compiler, env = compiler_environment(args.compiler, output)
    msvc = Path(compiler).name.lower() in ("cl", "cl.exe")
    results = []
    for name, source, define in SUITES:
        program = output / (name + (".exe" if msvc else ""))
        if msvc:
            command = [compiler, "/nologo", "/std:c++17", "/EHsc", "/W4", "/WX", "/O2",
                       *("/I" + str(root / path) for path in INCLUDES),
                       *(["/D" + define] if define else []), str(root / source),
                       "/Fo" + str(output / (name + ".obj")), "/Fe" + str(program)]
        else:
            command = [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-O2",
                       *("-I" + str(root / path) for path in INCLUDES),
                       *(["-D" + define] if define else []), str(root / source), "-o", str(program)]
        run(command, cwd=output, env=env)
        observed = run([str(program)], cwd=output, env=env)
        results.append({"suite": name, "source": source, "status": "PASS", "output": observed})
        print(name + ": " + observed, flush=True)
    if before != source_hashes(root):
        raise RuntimeError("Sources changed during the run; results do not describe a stable snapshot")
    report = {"scope": "NATIVE_CXX17_ONLY", "unreal": "NOT_RUN", "render_audio_input": "NOT_RUN",
              "compiler_family": "MSVC" if msvc else "CXX", "source_sha256": before, "suites": results}
    (output / "native-results.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"PASS {len(results)} required native suites; Unreal/gameplay validation remains separate.")


if __name__ == "__main__":
    main()
