"""Summarize actual UE CSV profiler captures; never fabricate missing GPU/VRAM data.

Requires explicit FrameTime milliseconds. Input can include UE metadata rows,
which are reported as skipped. Quantiles use nearest rank. A/B stay separate.
"""
import argparse
import csv
import json
import math
from pathlib import Path


def summarize(path, warmup_seconds=0):
    groups = {}
    elapsed, rejected, trimmed = 0.0, 0, 0
    with Path(path).open(encoding="utf-8-sig",newline="") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or "FrameTime" not in reader.fieldnames:
            raise ValueError("Uncompressed UE CSV with FrameTime in ms required")
        names = [name for name in reader.fieldnames if name in ("FrameTime","GameThreadTime","RenderThreadTime","RHIThreadTime","GPUTime")]
        for row in reader:
            try:
                frame = float(row["FrameTime"])
            except (ValueError,TypeError):
                rejected += 1
                continue
            if not math.isfinite(frame) or frame <= 0:
                rejected += 1
                continue
            elapsed += frame
            if elapsed <= warmup_seconds*1000:
                trimmed += 1
                continue
            for name in names:
                try:
                    value = float(row[name])
                except (ValueError,TypeError):
                    continue
                if math.isfinite(value) and value > 0:
                    groups.setdefault(name,[]).append(value)
    if not groups.get("FrameTime"):
        raise ValueError("No valid measured FrameTime samples after warmup")
    result = {}
    for name,values in groups.items():
        ordered = sorted(values)
        mean = sum(values)/len(values)
        result[name] = {"samples":len(values),"mean_ms":mean,
                        "p95_ms":ordered[math.ceil(0.95*len(values))-1],
                        "p99_ms":ordered[math.ceil(0.99*len(values))-1],
                        "max_ms":ordered[-1],"over_16_667ms":sum(v>1000/60 for v in values)}
        if name == "FrameTime":
            result[name]["mean_fps"] = 1000/mean
    return {"capture":Path(path).name,"timing":result,"warmup_seconds":warmup_seconds,
            "trimmed_frames":trimmed,"rejected_rows":rejected,
            "vram":"NOT_PRESENT: attach timestamped GPU memory sampling separately",
            "visual_audio_dx12":"NOT_INFERABLE_FROM_CSV",
            "acceptance":"PENDING: inspect render, audio, streaming and session metadata"}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("captures",nargs="+",type=Path)
    parser.add_argument("--warmup-seconds",type=float,default=0)
    args = parser.parse_args()
    if not math.isfinite(args.warmup_seconds) or args.warmup_seconds < 0:
        parser.error("Warmup must be finite and nonnegative")
    print(json.dumps([summarize(p,args.warmup_seconds) for p in args.captures],indent=2))
