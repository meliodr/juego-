# S3 — descripción del PR de laboratorio

Repositorio: `meliodr/juego-`
Head: `prototype/ib-s3-planet-lab`
Base: `prototype/ib-sprint02`
Draft: **true**
Título del PR publicado: **S3 lab: planeta radial, cuatro hubs, UMG y persistencia - QA Unreal pendiente**

Esta es la actualización local propuesta para el [PR draft #27](https://github.com/meliodr/juego-/pull/27). El remoto permanece en `2f08add`; no se publica esta validación hasta coordinar alcance.

## Descripción propuesta

La base S2 solo contiene un ensayo matemático radial. Este laboratorio añade un manifiesto de planeta configurable, una variante aislada de CMC para vuelo curvo con sweeps, cuatro hubs con superficies/interacciones técnicas, mapa global/minimapa UMG, residencia predictiva y deltas USaveGame con IDs de origen estables. Los hubs son **placeholder técnico**; la integración artística S2.5 y el Pawn/mapa principal quedan fuera del cambio.

Base fija `1d01db54b63250ed0aacb0704a5ac16290324e32`. Solo rutas IB_S3, Tools/IBS3_* y docs/IB_S3_*; no cambios a Character/Circuit globales, IB_C00, configuración, .uproject o Build.cs. Sin dependencias/bytes Fab o plugin externo añadido. RD$0. No merge automático.

Validación ejecutada:

- C++17 nativo MSVC /W4 /WX: **336.251 aserciones PASS**, 30.000 coordenadas aleatorias y seis vueltas numéricas con radios 25/60/90 km, incluyendo polos/dateline/transporte de orientación y presupuesto de frenada.
- Ensayo numérico R=25 km, altura 500 m, 270 m/s: **593,411946 s**; error máximo de retorno **0,00006556 cm**. No es una vuelta jugada ni benchmark.
- Generación OBJ propia: topología/manifold/orientación/radio PASS. Esfera 65.024 tris, cap cercano 32.768 tris; error máximo de arista del cap **2,1973 cm** a 25 km.
- Sintaxis Python/PowerShell, guard de turno exclusivo y alcance de archivos PASS.
- Revisión estática posterior: cinco fallos nativos reproducidos y corregidos; **100.172 aserciones adicionales PASS** sobre orientación UE, integrador de vuelo real, subpasos acelerados, límites de precarga, barrera de carga y poses persistidas. Total actual **436.423 aserciones nativas**. Manifiesto revisión 2 por corrección de coordenadas; ticks, cuerpos físicos y carga transaccional de saves reforzados. Detalle: `docs/IB_S3_STATIC_REVIEW.md`.

**Validación Unreal local:** UHT/UBT, autoría del mapa, Manifest/RuntimeResidency/SaveValidation, seis caídas/contactos/marchas (cuatro hubs y polos), reinicio de proceso con save, diez reentradas mediante vuelo a 320 m/s y siete capturas DX12 1920x1080 completados. Se corrigieron fallos reales de compilación, input digital, fixtures y legibilidad. Detalle y límites en `docs/IB_S3_UNREAL_VALIDATION.md`. Una circunnavegación CMC real sin render pasó en 594,932818 s: 15 cruces, 270 m/s, retorno a 7,621711 m (tolerancia de muestreo 10 m), sin escrituras de posición durante el recorrido. **No listo para integración global:** faltan paquete Win64, perfil FPS/VRAM/audio, mando físico/Steam Input y aceptación humana. Hubs pequeños síncronos; no streaming asíncrono de ciudades artísticas grandes.

Reproducción y arquitectura: `docs/IB_S3_ARCHITECTURE.md`. Evidencia numérica: `docs/IB_S3_QA.json`. Herramientas: `Tools/IBS3_TestCore.ps1`, `IBS3_TestAuthoring.py`, `IBS3_AuthorLab.py`, `IBS3_RunUnreal.ps1`.

## Autorización de publicación

El primer intento de publicación fue denegado antes de ejecutarse por falta de autorización explícita para un push externo; no se eludió esa decisión. El 2026-11-15 se recibió y verificó una nueva autorización expresa para subir las ramas de laboratorio al repositorio público `meliodr/juego-` y abrir sus PR en borrador, sin fusionarlos. Esta autorización permite publicar `prototype/ib-s3-planet-lab` en `https://github.com/meliodr/juego-.git` y abrir un único draft contra `prototype/ib-sprint02`.

La publicación no cambia los estados de QA anteriores ni constituye integración jugable. Este documento conserva la descripción y alcance del PR; la URL y el hash remoto se verifican por separado al finalizar la operación.
