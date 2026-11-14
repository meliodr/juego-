---
name: impact-linux-gate
description: Audit, build, package and smoke-test IMPACT BEYOND in native Linux with Unreal/Vulkan. Use at initial L00 and after gameplay systems or engine dependencies change.
---
# Linux gate
Read docs/LINUX_BASELINE.md, docs/PRODUCTION_CONTRACT_V06.md and AGENTS.md. Do not mistake the agent's cloud/container environment for the user's Linux machine.
1. Only on an authorized Linux machine, record uname, distro, CPU, available RAM/SSD, GPU/VRAM, NVIDIA driver, Vulkan version, UE root/version, clang/toolchain. User already stated 64GB RAM and RTX 4070 8GB; verify rather than ask again.
2. Inspect workspace/Git dirty state before changing anything. Do not change system drivers, firewall, distro, Unreal installation or global plugins without explicit authorization.
3. Choose supported stable UE/toolchain based on actual installation. Make minimal Third Person C++ baseline and verify editor launch. Unreal MCP optional/experimental; never essential for game launch.
4. Build, cook and package native Linux, then run **packaged** Vulkan application, verify input/camera/collision/audio and clean exit; record commands and logs.
5. Record screenshot/test map, resolution, graphics settings, avg/p95/p99 frame time, CPU/GPU bottlenecks and VRAM if instrumented. 60 FPS aspiration at 1080p Medium, not assumed.
6. Output PASS/FAIL/BLOCKED and reproducer; conserve previous working build. If editor/GPU inaccessible, report exactly which gates cannot be executed.
Stop on unknown binary-only Windows plugin or need to buy dependency.