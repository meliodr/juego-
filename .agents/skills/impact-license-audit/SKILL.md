---
name: impact-license-audit
description: Audit a free third-party Unreal or 3D asset for IMPACT BEYOND before importing it. Use for Quaternius, Kenney, Fab, GitHub code, animations and plugins.
---
# Resource audit — IMPACT: BEYOND
Read AGENTS.md, docs/FREE_RESOURCE_AUDIT_V05.md and docs/PRODUCTION_CONTRACT_V06.md. This skill does not authorize downloads, installations, purchases, releases or redistribution.
1. Record exact upstream URL, package name/edition, download contents, license text/URL and date, version/commit, and whether attribution or source distribution is permitted. Public repository does NOT mean reusable.
2. Distinguish Quaternius Standard from paid Source and CC0 from QAL. Do not push Fab/QAL packages as standalone assets. Do not incorporate GTA/Marvel assets or uncertain third-party logos. Preserve MIT notices.
3. Inspect .uplugin/.uproject/Build.cs/C++ for Win64-only binary libraries and required UE version. Inspect scripts before running. A Windows demo is not Linux proof.
4. Evaluate the **smallest needed subset** against a built-in alternative. Say exactly which files we actually inspected and which capabilities are only author claims.
5. Output ACCEPT_FOR_ISOLATED_TEST / HOLD_FOR_LICENSE / REJECT with evidence links, risk, fallback and Linux build test needed. Mark COMPATIBILITY_UNVERIFIED until packaged on Linux.
Stop: when license missing, package inaccessible or paid dependency required. Do not invent permission or compile status.