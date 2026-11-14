---
name: impact-qa-review
description: Review implementation tickets and Linux gameplay evidence for IMPACT BEYOND; produce PASS/FAIL/BLOCKED and approve only independently reproduced gates.
---
# QA and release review
Read AGENTS.md, docs/PRODUCTION_CONTRACT_V06.md, docs/MASTER_PLAN_V05.md and docs/CODEX_QA_PLAYBOOK_V05.md.
1. Identify ticket/Gate/DoD, commit, source changes, license provenance and expected vs actual behavior. Do not take another agent's "done" as evidence.
2. Run native Linux build/package and tests when environment available, with graphics/driver/system specs. Visual camera/VFX/feel need GPU render and human session.
3. For physics: 30 consecutive grabs, 50 panel impacts, reset/cleanup; validate actual collision changes, no duplicate hit/limb and no held item crossing solid objects.
4. For city: witnesses/alert decay/reputation attribution, no rescue farming, no NPC spawner in direct view; compare CPU/GPU/VRAM with NPC tiers and Chaos running.
5. For future network: two clients/host and latency/emulation, one owner per object, same wall state and same alert, per-player karma, stable safe teleport.
6. Report one of PASS/FAIL/BLOCKED with reproducible steps, screenshots/logs and metrics. Separate tool-generated evidence from human subjective score. Require control/flight/fun playtest before feature expansion.
7. Merge recommendation only after passing gate and verifying regressions. Never claim '60 FPS' unless measured on target GPU/resolution/profile.
Stop and mark BLOCKED if Linux hardware, asset licensing or engine build unavailable. Do not silently change system drivers or discard work.