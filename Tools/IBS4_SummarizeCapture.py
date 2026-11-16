"""Summarize a normalized REAL capture CSV; never manufactures performance results.

Required column: frame_ms. Optional: gpu_ms,vram_mb. Rows must represent frames
from one labelled run. Export/normalize an Unreal Insights or CSV Profiler capture
before using this tool; do not combine warmup and repeated traversal.
"""
import argparse
import csv
import json
import math
from pathlib import Path


def percentile(values, quantile):
    ordered = sorted(values)
    index = (len(ordered) - 1) * quantile
    lo = math.floor(index)
    hi = math.ceil(index)
    return ordered[lo] + (ordered[hi] - ordered[lo]) * (index - lo)


def summarize(rows):
    if len(rows) < 120:
        raise ValueError("Need at least 120 measured frames; short captures cannot satisfy the QA run")
    frames = [float(row["frame_ms"]) for row in rows]
    if any(not math.isfinite(t) or t <= 0 for t in frames):
        raise ValueError("frame_ms must be finite and positive for every frame")
    p95, p99 = percentile(frames, .95), percentile(frames, .99)
    memory = [float(row["vram_mb"]) for row in rows if row.get("vram_mb", "").strip()]
    gpu = [float(row["gpu_ms"]) for row in rows if row.get("gpu_ms", "").strip()]
    if any(not math.isfinite(v) or v < 0 for v in memory + gpu):
        raise ValueError("Optional metrics must be finite and nonnegative")
    return {
        "frames": len(frames), "duration_seconds": sum(frames) / 1000,
        "fps_mean": len(frames) * 1000 / sum(frames),
        "frame_ms_p95": p95, "frame_ms_p99": p99, "worst_frame_ms": max(frames),
        "fps_at_p95_frame_time": 1000 / p95, "fps_at_p99_frame_time": 1000 / p99,
        "hitches_over_50ms": sum(t > 50 for t in frames),
        "gpu_ms_p95": percentile(gpu, .95) if gpu else None,
        "vram_peak_mb": max(memory) if memory else None,
        "vram_samples": len(memory), "gpu_samples": len(gpu),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("capture", type=Path)
    parser.add_argument("--run-label", required=True)
    parser.add_argument("--resolution", required=True)
    parser.add_argument("--rhi", required=True)
    parser.add_argument("--audio", choices=["on", "off"], required=True)
    args = parser.parse_args()
    with args.capture.open(newline="", encoding="utf-8-sig") as stream:
        result = summarize(list(csv.DictReader(stream)))
    result.update(run=args.run_label, resolution=args.resolution, rhi=args.rhi, audio=args.audio)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
