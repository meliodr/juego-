"""Compile/run the production S5 policy without starting Unreal."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
setups = sorted(Path(os.environ.get("ProgramFiles", "C:/Program Files")).glob(
    "Microsoft Visual Studio/*/*/VC/Auxiliary/Build/vcvars64.bat"))
if not setups:
    raise RuntimeError("MSVC toolchain not found")
output = Path(tempfile.mkdtemp(prefix="IBS5_native_", dir=root.parent))
source = root / "ImpactBeyond/Source/ImpactBeyond/IB_S5/Tests/IBS5NativeTests.cpp"
executable = output / "IBS5_tests.exe"
batch = output / "build.cmd"
batch.write_text(
    f'@echo off\ncall "{setups[-1]}" >nul\nif errorlevel 1 exit /b 1\n'
    f'cl /nologo /std:c++17 /EHsc /W4 /WX /DIBS5_NATIVE_TEST_MAIN "{source}" '
    f'/Fe:"{executable}" /Fo:"{output / "IBS5_tests.obj"}"\n'
    'if errorlevel 1 exit /b 1\n', encoding="utf-8")
subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=output, check=True)
subprocess.run([str(executable)], cwd=output, check=True)
print(f"Native build artifacts: {output}")
