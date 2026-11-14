---
name: impact-character-animation
description: Use for importing, retargeting, rigging, assembling, or animating H01/H02/H03, citizens, robots and kaiju in Linux.
---
# Character and animation reuse

Read AGENTS.md, docs/ANIMATION_CHARACTER_PIPELINE_V07.md, docs/FREE_RESOURCE_AUDIT_V05.md and docs/PRODUCTION_CONTRACT_V06.md.
1. Choose ONE character and ONE needed behavior/pose; don't import entire animation libraries. Record source and precise edition, license and asset names first. Quaternius Standard vs Source is not interchangeable. Rokoko 15 superhero pack contents not assumed until inspected. Epic GASP Fab AI condition requires separate review.
2. Prefer one humanoid rig and shared AnimationBP/IK. If biped lacks rig, consider Mixamo (AdobeID), MPFB/Blender Rigify and retargeting; for kaiju evaluate Mesh2Motion with CC0 source.
3. Keep flight motion controlled by CMC, not root motion. Pose tree Hover/Cruise/Sonic/Brake with additive lean/bank. For carried car use independent IK sockets/collision, not mesh attached blindly.
4. For custom hero visuals, script Blender geometry/plates/materials and inspect meshes, weights, silhouettes and deformation. Do not claim an image-generation result is ready-to-animate 3D.
5. Robots and kaiju detach prepared parts by stable PartId; each affects AI and has valid collision. Avoid arbitrary gory cuts or duplicating parts.
6. Export minimal FBX/GLB, import in a test level, validate scale/skeleton/loop/frame-rate, Linux Vulkan gameplay and memory.
7. QA: 20 state changes, flight + ground, 30 grabs across object sizes and recoveries, screen evidence and user feel score. Fail/blocked instead of claiming a functioning animation from license/README only.
No purchase, unauthorized redistribution, plugin install or unrelated repo edits.
