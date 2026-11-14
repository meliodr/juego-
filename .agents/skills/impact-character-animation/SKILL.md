---
name: impact-character-animation
description: Use for importing, retargeting, rigging, assembling, or animating H01/H02/H03, citizens, robots and kaiju in Linux.
---
# Character and animation reuse

Read AGENTS.md, docs/ANIMATION_CHARACTER_PIPELINE_V07.md, docs/FREE_RESOURCE_AUDIT_V05.md and docs/PRODUCTION_CONTRACT_V06.md.
Read docs/ANIMATION_FEEL_OPTIMIZATION_V08.md before designing flight poses, attack transitions, camera impact, IK, physics reactions or animation optimization. This document is currently PLANNING, not implemented.
1. Choose ONE character and ONE needed behavior/pose; don't import entire animation libraries. Record source and precise edition, license and asset names first. Quaternius Standard vs Source is not interchangeable. Rokoko 15 superhero pack contents not assumed until inspected. Epic GASP Fab AI condition requires separate review.
2. Prefer one humanoid rig and shared AnimationBP/IK. If biped lacks rig, consider Mixamo (AdobeID), MPFB/Blender Rigify and retargeting; for kaiju evaluate Mesh2Motion with CC0 source.
3. Keep flight motion controlled by CMC, not root motion. Pose tree Hover/Cruise/Sonic/Brake with additive lean/bank. For carried car use independent IK sockets/collision, not mesh attached blindly.
4. For custom hero visuals, script Blender geometry/plates/materials and inspect meshes, weights, silhouettes and deformation. Do not claim an image-generation result is ready-to-animate 3D.
5. Robots and kaiju detach prepared parts by stable PartId; each affects AI and has valid collision. Avoid arbitrary gory cuts or duplicating parts.
6. Export minimal FBX/GLB, import in a test level, validate scale/skeleton/loop/frame-rate, Linux Vulkan gameplay and memory.
7. QA: 20 state changes, flight + ground, 30 grabs across object sizes and recoveries, screen evidence and user feel score. Fail/blocked instead of claiming a functioning animation from license/README only.
No purchase, unauthorized redistribution, plugin install or unrelated repo edits.

## Extra acceptance: natural feel and performance (v0.8)
- Separate authoritative Flight/Interaction/Combat state from AnimationBP rendering and Niagara/audio. Prefer 1 humanoid skeleton, additive bank/lean, consistent hand IK only when relevant.
- Blend Spaces, short inertial blends, and thread-safe/fast path property access are first-line; use Motion Warping/FullBodyIK only if specific contact alignment fails, and only for meaningful actors.
- Default no global time dilation/hitstop; physics and network keep running in multiplayer. Camera shake, hit reaction and impact dust are player-local/configurable.
- Test Ground/Takeoff/Hover/Cruise/Sonic/Brake/Recovery, 20 transitions, 30 carry-and-throw tests and 20 aerial ragdoll recoveries. Require human score for control/speed/power.
- Animation Budget Allocator and Animation Sharing only after population profiling justifies them. For distant citizens reduce animation rate, not authoritative gameplay physics. Include p95/p99 frame time and VRAM on Vulkan when hardware exists.
- Store sources/licenses; no animation from Roblox/Minecraft/Marvel/GTA nor library of raw Fab assets without approved license.
