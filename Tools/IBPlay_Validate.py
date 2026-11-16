"""Exclusive local UE gate. Records the exact clean commit and never drives desktop UI."""
import argparse
import ctypes
import json
import os
from pathlib import Path
import subprocess
import sys
from datetime import datetime, timezone

repo = Path(__file__).resolve().parents[1]
task = repo.parent
engine = Path(r"C:\Program Files\Epic Games\UE_5.8")
parser = argparse.ArgumentParser()
parser.add_argument("phase", choices=("build", "tests", "author", "package"))
parser.add_argument("--suite", action="append")
parser.add_argument("--map", default="/Game/IB_C00/Lvl_IB_C00")
args = parser.parse_args()
def git(*values):
    return subprocess.check_output(["git", "-C", str(repo), *values], text=True).strip()
head = git("rev-parse", "HEAD")
if git("status", "--porcelain", "--untracked-files=no"):
    raise RuntimeError("Commit the candidate before validation")
kernel = ctypes.WinDLL("kernel32", use_last_error=True)
kernel.CreateMutexW.argtypes = [ctypes.c_void_p, ctypes.c_bool, ctypes.c_wchar_p]
kernel.CreateMutexW.restype = ctypes.c_void_p
kernel.WaitForSingleObject.argtypes = [ctypes.c_void_p, ctypes.c_uint]
kernel.ReleaseMutex.argtypes = [ctypes.c_void_p]
kernel.CloseHandle.argtypes = [ctypes.c_void_p]
mutex = kernel.CreateMutexW(None, False, "Local\\ImpactBeyond_UnrealExclusive")
if not mutex or kernel.WaitForSingleObject(mutex, 0) not in (0, 128):
    raise RuntimeError("Another UE task owns the exclusive slot")
run = task / ("s7-" + args.phase + "-" + datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ"))
run.mkdir()
state = {"owner": "S7 integration", "head": head, "pid": os.getpid(), "active": True, "run": str(run), "steps": []}
reservation = task / "IB_ALL_editor-turn.json"
def record():
    reservation.write_text(json.dumps(state, indent=2), encoding="utf-8")
def execute(name, command):
    state["status"] = name
    record()
    (run / (name + ".command.json")).write_text(json.dumps(command), encoding="utf-8")
    with (run / (name + ".stdout.log")).open("w", encoding="utf-8") as output:
        child = subprocess.Popen(command, cwd=repo, stdout=output, stderr=subprocess.STDOUT, creationflags=subprocess.CREATE_NO_WINDOW)
        state["child_pid"] = child.pid
        record()
        print("START", name, child.pid, flush=True)
        code = child.wait()
    state["child_pid"] = None
    state["steps"].append({"name": name, "exit_code": code})
    record()
    print("END", name, code, flush=True)
    if code:
        raise RuntimeError(name + " failed " + str(code))
project = str(repo / "ImpactBeyond/ImpactBeyond.uproject")
dotnet = str(engine / "Engine/Binaries/ThirdParty/DotNet/10.0/win-x64/dotnet.exe")
editor = str(engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe")
try:
    if args.phase == "build":
        ubt = str(engine / "Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll")
        for target in ("ImpactBeyondEditor", "ImpactBeyond"):
            execute(target, [dotnet, ubt, target, "Win64", "Development", project, "-DisableUnity", "-MaxParallelActions=4", "-NoUBA", "-NoHotReload", "-NoHotReloadFromIDE"])
    elif args.phase == "tests":
        suites = args.suite or ["Integration.PlayableCombat", "S6.Runtime", "S7.RuntimeContract", "S7.PreparedEncounter", "S7.WheelRefresh", "S5.LifeAndPersistence", "S3.ProductionCities", "S35.PlayablePunches", "Landing.RadialCanonical", "S35.GripAndReset", "S35.ThrowsPhysical", "S4.H01Response", "S4.PreparedCollision"]
        for suite in suites:
            report = run / (suite + "-report")
            execute(suite, [editor, project, args.map, "-game", "-IBPlayQA", "-NullRHI", "-UseFixedTimeStep", "-FPS=60", "-unattended", "-nosplash", "-nosound", "-nowrite", "-NoLiveCoding", "-ExecCmds=Automation RunTests ImpactBeyond." + suite, "-TestExit=Automation Test Queue Empty", "-abslog=" + str(run / (suite + ".unreal.log")), "-ReportExportPath=" + str(report)])
            result = json.loads((report / "index.json").read_text(encoding="utf-8-sig"))
            if result.get("succeeded", 0) < 1 or result.get("failed", 0) or result.get("notRun", 0):
                raise RuntimeError("Missing complete test success: " + suite)
    elif args.phase == "author":
        script = str(repo / "Tools/IBPlay_AuthorProduction.py")
        execute("Author", [editor, project, "-run=pythonscript", "-script=" + script, "-unattended", "-nosplash", "-NullRHI", "-nowrite", "-NoLiveCoding", "-abslog=" + str(run / "author.unreal.log")])
    else:
        uat = str(engine / "Engine/Binaries/DotNET/AutomationTool/AutomationTool.dll")
        execute("CookPackage", [dotnet, uat, "BuildCookRun", "-project=" + project, "-noP4", "-platform=Win64", "-clientconfig=Development", "-skipbuild", "-cook", "-stage", "-pak", "-iostore", "-archive", "-archivedirectory=" + str(run / "package"), "-map=/Game/IB_Playable/Lvl_IB_Playable+/Game/IB_C00/Lvl_IB_C00+/Game/IB_S3/Lvl_IB_S3_PlanetLab", "-unattended", "-utf8output", "-AdditionalCookerOptions=-nowrite"])
    if args.phase != "author" and (git("rev-parse", "HEAD") != head or git("status", "--porcelain", "--untracked-files=no")):
        raise RuntimeError("Candidate changed during validation")
    state["result"] = "PASS"
except Exception as error:
    state["result"] = "FAIL"
    state["error"] = str(error)
    print("FAIL", error, flush=True)
finally:
    state.update(active=False, status="CLOSED")
    record()
    (run / "result.json").write_text(json.dumps(state, indent=2), encoding="utf-8")
    kernel.ReleaseMutex(mutex)
    kernel.CloseHandle(mutex)
print("RESULT", state["result"], str(run), flush=True)
sys.exit(0 if state["result"] == "PASS" else 1)
