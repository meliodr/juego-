"""Inventory real local S3 logs/captures. Never infers PASS from exit code or a screenshot alone."""
import hashlib
import csv
import json
from pathlib import Path
import re
import struct

repo = Path(__file__).resolve().parent.parent
evidence = repo / "ImpactBeyond/Saved/IB_S3_QA"
report = {"logs": [], "captures": [], "source_sha256": {}, "scope": "Local Unreal execution evidence; not packaged acceptance or FPS certification"}
flight_rows = []
for path in sorted(evidence.glob("*.log")):
    raw = path.read_bytes()
    text = raw.decode("utf-8", errors="replace")
    completed = re.findall(r"Test Completed\. Result=\{([^}]+)\} Name=\{[^}]+\} Path=\{([^}]+)\}", text)
    for values in re.findall(r"IB_S3 flight QA: elapsed=([\d.]+)s angle=([\d.]+)deg returns=(\d+) speed=([\d.]+)m/s altitude_error=([\d.]+)cm", text):
        flight_rows.append([path.name, *values])
    findings = sorted(set(line.split("]", 2)[-1].strip() for line in text.splitlines()
                          if any(token in line for token in ("Ensure condition failed", "Assertion failed", "LogAutomationController: Error:", "CDO Constructor"))))
    report["logs"].append({"file": str(path.relative_to(evidence)), "bytes": len(raw), "sha256": hashlib.sha256(raw).hexdigest(),
                           "tests": [{"path": name, "result": result} for result, name in completed],
                           "build_succeeded": "Result: Succeeded" in text, "findings": findings})
for path in sorted((evidence / "Captures").rglob("*.png")):
    raw = path.read_bytes()
    if raw[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Invalid PNG signature: {path}")
    width, height = struct.unpack(">II", raw[16:24])
    report["captures"].append({"file": str(path.relative_to(evidence)), "width": width, "height": height,
                               "bytes": len(raw), "sha256": hashlib.sha256(raw).hexdigest()})
paths = list((repo / "ImpactBeyond/Source/ImpactBeyond/IB_S3").glob("*")) + list((repo / "Tools").glob("IBS3_*"))
for path in sorted(p for p in paths if p.is_file()):
    report["source_sha256"][str(path.relative_to(repo)).replace("\\", "/")] = hashlib.sha256(path.read_bytes()).hexdigest()
output = evidence / "evidence-summary.json"
output.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")
with (evidence / "flight-progress.csv").open("w", newline="", encoding="utf-8") as f:
    writer = csv.writer(f)
    writer.writerow(["log", "elapsed_game_seconds", "angle_degrees", "reentries", "speed_m_s", "altitude_error_cm"])
    writer.writerows(flight_rows)
print(json.dumps({"report": str(output), "logs": len(report["logs"]), "captures": len(report["captures"])}, indent=2))
