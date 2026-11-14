---
name: impact-civic-systems
description: Plan or implement IMPACT BEYOND citizens, witness reports, local alert levels, karma, evacuation or rescue. Use on U00-U05 city-system tickets.
---
# Civilian / Alert / Karma gate
Read docs/CIVILIANS_ALERTS_KARMA_V06.md and docs/PRODUCTION_CONTRACT_V06.md.
1. Determine target ticket: U00 event attribution, U01 citizen reaction, U02 alert 0-5, U03 karma -100..100, U04 LOD, or U05 replication. Do not ship all systems simultaneously.
2. Route impact and rescue events through authoritative incident ledger with IncidentId/CausationId/district, dedup and instigator; FX cannot emit reputation points. Enemy-caused destruction never lowers player karma by itself.
3. Alert is short-lived zone-specific response, karma is long-lived personal/region reputation, threat severity is independent. Support Creative mode toggles with no power lockout.
4. For early NPCs, StateTree + SmartObjects + simple NavMesh before experimental MassCrowd/MassAI. Add tiers A/B/C only if profiling calls for them. Civilians seek safe routes; no spawn pop-in in view.
5. Protect against reward farming: repeating self-caused incidents or 20 fragments from one throw gives one logical incident; rescue only with validated pre-existing danger.
6. Test no player blame for kaiju attack, one incident per causal group, witness delays, 20 evacuations, alert escalation/decay, mode toggles and two-player ownership when available.
7. Report actual counts, FPS/VRAM and state across save/reload where tested. Never claim crowd scale from a UE marketing demo.
Stop if plugin experimental/Linux incompatible; fall back to fewer ACharacter NPCs.