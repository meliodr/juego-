---
name: impact-world-events
description: Use for designing or implementing scripted/procedural sandbox random events such as robberies, meteors, drone raids, invasions and kaiju alerts.
---
# Dynamic world-event implementation

Read AGENTS.md, docs/REFERENCES_DYNAMIC_EVENTS_V07.md, docs/CIVILIANS_ALERTS_KARMA_V06.md and docs/EXECUTION_BACKLOG_V06.md.
1. Start with a single EventID and acceptance test. Do not activate every event; require the underlying gameplay actor(s) to already exist.
2. Use EventDataAsset with prerequisites, District/Body tags, mode filter, weight, threat tier, cooldown, spawn budget, telegraph/active/resolve/cleanup phases, participant roles.
3. Validate loaded collision/nav/streaming and spawn points; never spawn meteor inside closed building or enemies directly in front of player. Allow player to ignore event; finish/cleanup deterministically.
4. Route damage/rescue through IncidentId/CausationId and authoritative ledger. Alert district != per-player karma != threat. An invading robot or natural meteor does NOT automatically blame hero.
5. Reuse existing robot, citizen, projectile, Chaos prefab and interaction. Meteor crater prefab with passable/valid collision and 2–5 reusable fragments rather than arbitrary terrain booleans. Max one major event in prototype.
6. Creative mode toggles, cooldown, spawner and safe reset mandatory. Two-player eventual server owns event and damage, clients visual FX.
7. QA deterministic seed, 30 incidents, 20 civilian escape runs, meteor zone tests, invasion actor counts, no overdraw/VRAM spike, performance measured on Linux 4070 8GB and user surprise/fun feedback.
No global AI simulation, paid weather service, real-time external AI, arbitrary actor duplication or claims of tested behavior without game execution.
