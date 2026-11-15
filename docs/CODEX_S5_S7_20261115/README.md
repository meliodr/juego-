# IMPACT: BEYOND — ocho encargos Codex aislados hacia S7 (corte 15-nov-2026, 10:28 AST)

**ESTADO DE ESTA ENTREGA:** ocho **PROMPTS PUBLICADOS**, no ocho chats iniciados, ni pruebas Unreal ni sprints implementados. Esta rama añade documentación solamente. Su base de comparación GitHub es `docs/roadmap-sprints-02-08-20261115`, SHA `bb97a37df78cf29324e8bdfe7b6e17c50cc2cffe`. La base **para cada futuro worktree de gameplay** es *otra*, fijada en PR #19: `1d01db54b63250ed0aacb0704a5ac16290324e32` de `prototype/ib-sprint02`. No copiar documentos a la rama gameplay por merge indiscriminado.

## Evidencia examinada antes de escribir

- Repositorio `meliodr/juego-`: [branches](https://github.com/meliodr/juego-/branches), [pull requests](https://github.com/meliodr/juego-/pulls), [GitHub Actions](https://github.com/meliodr/juego-/actions). Cortes consultados el 15-11-2026: **23 ramas listadas, 20 PRs existentes** (después de aparecer PR #22; recuentos previos #22: 22 ramas/19 PRs), **0 ejecuciones Actions** en la API. No estaban publicadas ramas finales `prototype/ib-s25-realism-lab`, `prototype/ib-s3-planet-lab`, `prototype/ib-s35-interactions-lab`, `prototype/ib-s4-response-lab`.
- [PR #19 Sprint 2](https://github.com/meliodr/juego-/pull/19): base de gameplay `1d01db54...` con vuelo/travesía/entorno costero. Sus FPS publicados de 143,94 son de **render offscreen automatizado**; NO certifican visual/humano/60FPS con audio.
- [PR #20](https://github.com/meliodr/juego-/pull/20): ficha documental candidata CC0 de vegetación; no objeto importado.
- [PR #21 auditoría 08:45](https://github.com/meliodr/juego-/pull/21): verificación de branches/rutas y 3/3 tests Git-only aislados; no UE/DX12.
- [PR #22 refinamiento 09:56](https://github.com/meliodr/juego-/pull/22): seis documentos de refinamiento, sin gameplay: P0-01 calidad visual de fachada/costa, P0-02 objeto cargado a 320/368 m/s, P0-03 policía/tanque no letales y ensayo S3 de 2 regiones. Ajusta clasificación de licencia del paquete City Sample Vehicles (AI permitida según ficha Fab; licencia exacta/no redistribución aún a verificar). No confundir recomendaciones con instalación ni test real.
- [PR #17 roadmap y prompts S2.5/S3/S3.5/S4](https://github.com/meliodr/juego-/pull/17) y fichas [S5 ciclo/civiles](../SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md), [S6 H02/H03 reutilización](../SPRINT06_H02_H03_REUTILIZACION.md), [S7 amenazas/rueda](../ROADMAP_SPRINTS_02_A_08.md). La base `main` vista es `aaad4491ca3610ba22d12f764d1ed0e35ca0356f`, **no contiene** la cadena de PRs de gameplay.
- **Aciertos** existentes: vuelo/travesía, tests numéricos, fachada PBR parcial, trabajo de contrato de daño/gravedad radial. **Fallos o evidencia insuficiente**: costa/primitivas low-poly y autos de kit, barrido de carga sujetada no probado, ninguna implementación S2.5–S4 publicada y ninguna prueba DX12/audio/VRAM de esas fases. Falta de push NO demuestra falta de trabajo local.

## Ocho chats posibles, uno por laboratorio

| Frente y prompt | Sprint | Carpeta/propiedad de C++ y Contenido | Trabajo aislado; condición |
|---|---|---|---|
| [01 — Reloj solar + luz](01_S5_SOLAR.md) | S5 | `IB_S5/Time/**` | Tiempo global 36min; enlazar solo después de S3 |
| [02 — Civiles + incidentes](02_S5_CIVILES_EVENTOS.md) | S5 | `IB_S5/Civics/**` | 3–6 civiles y un evento, sin Massive Crowds |
| [03 — H02 genio cósmico](03_S6_H02_GENIO.md) | S6 | `IB_S6/H02/**` | escudo, mano y mazo usando daños/agarres compartidos |
| [04 — H03 blink](04_S6_H03_BLINK.md) | S6 | `IB_S6/H03/**` | solver seguro 25–40m y combate post-blink |
| [05 — Selector 3 héroes](05_S6_SELECTOR_Y_CONTRATOS.md) | S6 | `IB_S6/Selector/**` | interfaz y mocks; host compartido se conecta DESPUÉS |
| [06 — Robot pesado](06_S7_ROBOT_PESADO.md) | S7 | `IB_S7/Robot/**` | una amenaza con dos piezas funcionales |
| [07 — Kaiju mediano](07_S7_KAIJU_MEDIANO.md) | S7 | `IB_S7/Kaiju/**` | un KAI01, tres partes, 2 ataques |
| [08 — Rueda radial](08_S7_RUEDA_PODERES_QA.md) | S7 | `IB_S7/Radial/**` | 6–8 slots, input mando/ratón, no menú S8 |

Cada prompt además posee su prefijo exclusivo `Tools/IBLxx_*` y `docs/IB_Lxx_*`, worktree y rama individual desde la misma base S2. **No hay dos dueños del mismo `.uasset`, `.umap`, Character ni mapa global.** Las rutas de 8 labs se reservaron para futuras implementaciones, no se crearon ni modificaron archivos gameplay aquí.

## Cómo priorizar si hay ocho chats disponibles

1. **Primero P0**: terminar/validar los laboratorios S2.5, S3, S3.5, S4 de PR #17/#22; exigir Git gate PR #21 + build real + 5 vistas A/B + fisicas + DX12. No adelantar la integración del futuro. Si no están listos, los ocho frentes **solo** producen contratos, prototipos/mocks aislados, tests de lógica y decisiones de licencias, sin tocar mapa/Character canónicos.
2. **Oleada de diseño segura**: L05-A/B, L06-A/B/C, L07-A/B/C trabajan sobre módulos/rutas propios; solo un desarrollador escribe `Character/GameMode/Build.cs`, **el integrador** después y en rama separada. No programar ocho cooks/editor simultáneos.
3. **Orden de integración serial**: S2.5 visual + S3.5 física → S3 planeta/regiones → S4 combate/alerta → L05-A reloj → L05-B civismo → L06-A H02 + L06-B H03 → L06-C selector → L07-A robot → L07-B kaiju → L07-C rueda. Debe existir un hito jugable por ola, con rollback y no regresión del vuelo.
4. **Gate antes de publicar avance como jugable**: (a) diff/commit SHA y licencia, (b) C++ compila para UE5.8 Windows, (c) mapa de laboratorio abre, (d) pruebas con audio/imagen DX12 a 1080p y GPU VRAM/frame p95/p99/60fps objetivo, (e) humano prueba 10–15min, (f) PR revisión con dependencias. Sin esos gates, estado `SOLO_DOC` o `PROTOTIPO`, jamás `ACEPTADO`.
5. **Rendimiento**: la meta 60 FPS con RTX 4070 Laptop 8GB es un objetivo, no una medición alcanzada. Mantener poco contenido activado simultáneamente, ISM/HLOD solo después de medir; City Sample-lite, NO City Sample completo ni descarga innecesaria de packs. H02/H03 deben reutilizar modelos ya reclamados/legalmente descargables, de lo contrario Manny placeholder. S7 robot+un kaiju, no diez enemigos.
6. **Publicación y seguridad**: PRs individuales en draft hacia `prototype/ib-sprint02` para cada lab. No merges a `main`, force-push, clean destructivo, compra, reclamación de Fab, instalación plugin ni publicar meshes/texturas UE-Only como packs. No suministrar bytes de assets NoAI al modelo.

## Límites de acceso verificados

La herramienta de GitHub permite crear rama/documentación/PR; esta sesión **no tiene acción Codex de crear chats o mensajes ni ejecución Windows/Unreal Editor**. Como resultado, no puede seleccionar modelos en Codex ni confirmar que alguno esté ejecutando. Estos 8 archivos son briefs listos para esa herramienta cuando esté disponible. La revisión de integración debe contrastar las nuevas ramas/publicaciones con los SHA listados, actualizar gates antes de cualquier acción, y **no asumir que los ocho labs se han ejecutado solo porque existen los prompts**.

**Nota de fecha:** 15-nov-2026. El objetivo del usuario de llegar hasta S7 antes de las 12:30 AST no es verificable sin las sesiones Codex/Unreal; preferir calidad honesta y respaldo del juego a anunciar falsamente sprints completados.
