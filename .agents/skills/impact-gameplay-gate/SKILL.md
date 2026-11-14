---
name: impact-gameplay-gate
description: Implement one small IMPACT BEYOND gameplay vertical slice, such as flight, grab and throw, destructible walls, enemy limb, energy construct, or safe blink, and prove it works.
---
# Gameplay implementation
Read AGENTS.md, docs/MASTER_PLAN_V05.md, docs/CITY_SOLAR_TELEPORT_V04.md, docs/COMBAT_ROSTER_V03.md and docs/PRODUCTION_CONTRACT_V06.md.
1. Start with one ready ticket: exact observable outcome, allowed paths, test scenario, rollback, upstream license. Do not begin the planet or a second framework while testing a wall or grab.
2. Prefer native Unreal components and audited open components. One implementation of Flight/Grab/Destruction as source of truth; separate gameplay state from animation and FX.
3. Implement smallest change in isolated branch. Reuse Data Assets, typed events and Gameplay Tags for tuning. Avoid a whole ability framework unless required and approved.
4. Respect future co-op: sender action validated by authority, stable ActorId/PartId, single holder of object, idempotent incident/damage, no hardcoded player zero.
5. A fast car held in hands needs its own swept collision test; a broken wall needs a genuinely passable collision change. Safe teleport checks capsule/loaded destination. Robot arm loss must remove the corresponding attack.
6. Run actual build/test in Linux, document log, 30 cycles grab or 50 impacts or 50 blink destinations as relevant. Check regression of previous working mechanics.
7. Ask for human playtest when camera/feel is affected; do not claim fun based on code. Report PASS/FAIL/BLOCKED, commit, risk, next task. Roll back when a donor requires rewriting half the project.
Stop if no Linux runtime/editor test is available: code inspection only, do not state that gameplay was executed.