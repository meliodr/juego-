"""Reject private packs, binary additions and personal paths in new Git history.

The existing public baseline is retained. Inspect every introduced commit, so
adding a protected asset and deleting it later cannot hide it from this gate.
No asset bytes are read, rendered, uploaded, or interpreted.
"""
from __future__ import annotations

import argparse
import ast
from pathlib import Path
import re
import subprocess

PREFIXES = ("ImpactBeyond/Source/", "ImpactBeyond/Config/", "Tools/", "docs/", ".github/workflows/")
ROOT_FILES = {".gitignore", "AGENTS.md", "README.md", "ImpactBeyond/ImpactBeyond.uproject"}
EXTENSIONS = {".h", ".cpp", ".cs", ".py", ".ps1", ".cmd", ".md", ".json", ".yml", ".yaml", ".ini", ".csv", ".txt", ".toml", ".uproject"}
PRIVATE_PATH = re.compile(r"[A-Za-z]:[/\\]+Users[/\\]+|/(?:home|Users)/[^/\s<>]+/")


def git(root, *args):
    return subprocess.check_output(["git", "-C", str(root), *args]).decode("utf-8", "strict")


def check_path(path):
    if path not in ROOT_FILES and not path.startswith(PREFIXES):
        return "outside public source allowlist"
    if Path(path).name != ".gitignore" and Path(path).suffix.lower() not in EXTENSIONS:
        return "not an allowed source/metadata extension"
    return None


def audit(root, base, head="HEAD"):
    git(root, "rev-parse", "--verify", base + "^{commit}")
    git(root, "rev-parse", "--verify", head + "^{commit}")
    commits = git(root, "rev-list", "--reverse", base + ".." + head).splitlines()
    violations = []
    inspected = 0
    for commit in commits:
        parents = git(root, "rev-list", "--parents", "-n", "1", commit).split()[1:]
        if not parents:
            violations.append((commit, "<history>", "unexpected root commit"))
            continue
        # --no-renames ensures both ends are visible; deletions do not remove the
        # need to inspect the earlier commit that introduced their contents.
        fields = git(root, "diff", "--name-status", "--no-renames", "-z", parents[0], commit).split("\0")
        for index in range(0, len(fields) - 1, 2):
            status, path = fields[index:index + 2]
            if status == "D":
                continue
            inspected += 1
            reason = check_path(path)
            if reason:
                violations.append((commit, path, reason))
                continue  # Never read rejected packs/assets.
            text = git(root, "show", commit + ":" + path)
            for line_number, line in enumerate(text.splitlines(), 1):
                if PRIVATE_PATH.search(line):
                    violations.append((commit, path + ":" + str(line_number), "personal absolute path"))
    return {"commits": len(commits), "inspected_text_files": inspected, "violations": violations}


def check_syntax(root):
    files = sorted((root / "Tools").rglob("*.py"))
    # Parse only versioned source, excluding local caches or downloaded packs.
    tracked = set(git(root, "ls-files", "-z").split("\0"))
    count = 0
    for path in files:
        if path.relative_to(root).as_posix() in tracked:
            ast.parse(path.read_text(encoding="utf-8-sig"), filename=path.relative_to(root).as_posix())
            count += 1
    return count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--base")
    parser.add_argument("--syntax", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    if args.syntax:
        print(f"PASS parsed {check_syntax(root)} tracked Python sources; no Unreal import executed")
        return
    base = args.base if args.base and set(args.base) != {"0"} else "origin/main"
    result = audit(root, base)
    if result["violations"]:
        for commit, path, reason in result["violations"]:
            print(f"FAIL {commit[:12]} {path}: {reason}")
        raise SystemExit(1)
    print(f"PASS public source policy: {result['commits']} introduced commits, {result['inspected_text_files']} file changes")


if __name__ == "__main__":
    main()
