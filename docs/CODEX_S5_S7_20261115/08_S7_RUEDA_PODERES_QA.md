# Codex L07-C — S7: rueda radial de poderes 6–8 espacios y QA de interfaz

**Modelo sugerido:** Codex UMG/Enhanced Input/UX+QA, razonamiento **High**; análisis avanzado cuando haya conflictos de prioridades.
**Worktree:** `../impact-l07c-powerwheel` · **rama:** `prototype/ib-l07c-powerwheel-lab`.
**Escritura exclusiva:** `ImpactBeyond/Source/ImpactBeyond/IB_S7/Radial/**`, `ImpactBeyond/Content/IB_S7/Radial/**`, `Tools/IBL07C_*`, `docs/IB_L07C_*`. No editar Character, GameMode, HUD global, selector S6 ni HUD S3.

## Identificación, seguridad y precedencia

Trabaja SOLO si tienes un chat Codex realmente abierto en el PC Windows del usuario. Esta ficha en GitHub NO inicia Codex ni selecciona modelo. Repositorio: https://github.com/meliodr/juego- . **BASE DE GAMEPLAY EXACTA:** `1d01db54b63250ed0aacb0704a5ac16290324e32` (`prototype/ib-sprint02`, PR #19). **BASE DOCUMENTAL** de estos prompts: PR documental de ocho frentes; léela remotamente, NO hagas merge del branch documental en el juego. En cada ejecución relee `git status`, `git worktree list`, remotes, branches, PRs recientes y diffs de S2.5/S3/S3.5/S4, auditoría PR #21 y refinamiento PR #22. Si nuevas entregas cambiaron el contrato, adapta propuesta SIN rebase destructivo ni duplicar trabajo.

**GATE ESTRICTO:** al corte 15-11-2026 10:28 AST NO hay ramas de implementación S2.5/S3/S3.5/S4 publicadas; PR #20 es una ficha CC0, PR #21 auditoría con validador Git y PR #22 ajustes documentales. No asumas que faltan trabajos locales: inspecciónalos cuando tengas acceso y registra resultados. Mientras los cuatro gates previos no estén aceptados/compatibles, limita este frente a **contrato, prototipo aislado no integrado, tests offline sin Unreal, recursos locales ya legales**, y bloquea cualquier edición del mundo canónico. No escribas `ImpactBeyondCharacter.*`, `ImpactCircuitComponent.*`, `ImpactBeyondPlayerController.*`, `ImpactBeyondGameMode.*`, `ImpactBeyond.Build.cs`, `ImpactBeyond.uproject`, `Config/**`, `IB_C00/**`, `.umap` principal, ni carpetas de otros agentes. Cualquier cambio compartido va como propuesta en docs, sin aplicarlo. Los ocho frentes usan *worktrees diferentes* nacidos del mismo SHA y **una sola sesión Unreal Editor/cook/packaging por PC en todo momento**. No borrar DDC, cachés, datos ni sobrescribir worktrees.

**Licencia/presupuesto:** RD$0; primero reusar Manny, Chaos, Niagara, materiales/meshes existentes y documentación. No descargar City Sample completo, reclamar Fab, aceptar nuevas licencias, habilitar/instalar plugins, comprar ni subir archivos fuente Epic/Fab como packs sueltos al repo público. Registrar `local/propiedad/licencia/tamaño/compatibilidad`; no suministrar bytes NoAI a modelos de IA. Meta de QA **1080p DX12 visible/audio ON y 60 FPS** (p95 frame <=16.7ms orientativo, p99/VRAM/hitches reportados, no declarados sin test); Windows UE5.8.x real por comprobar en máquina. Una prueba headless, screenshot estático o script aislado NO certifica jugabilidad ni FPS.

**Entrega:** rama de laboratorio exclusiva desde SHA S2; PR **draft** dirigido a `prototype/ib-sprint02`, con solo rutas permitidas. Adjunta SHA+diff, lista de clases/activos y licencias, tests PASS/FAIL/SKIPPED con evidencia real, mapa aislado sólo si correspondió acceso/turno a Unreal, dependencias para el integrador y estado explícito `SOLO_DOC / PROTOTIPO / BUILD / DX12_VERIFICADO / BLOQUEADO`. Nunca merge a `main`. La integración será serial y después de la revisión de las entregas antiguas.



## Encargo ejecutable tras los gates previos
- Revisar `docs/ROADMAP_SPRINTS_02_A_08.md`, `docs/DIRECCION_SANDBOX_S08_S10_S15.md`, `docs/SPRINT06_H02_H03_REUTILIZACION.md`, `docs/GAMEPLAY_SPEC.md`; S7 admite **rueda radial inicial**, **no** menú pausa completo (S8). Inspiración es patrón ergonómico, NO copiar imágenes/códigos de GTA V ni iconos ajenos.
- Crear widget de laboratorio UMG radial con **6–8 slots** provenientes de descriptor `AbilityId`, nombre breve, icono propio geométrico/placeholder, estado disponible, cooldown, coste, habilidad activa, atajo `volver a poder previo`. No codificar lógica de habilidades ni duplicar H02/H03; usar un proveedor mock de descriptors del selector L06-C hasta que se valide.
- Modo ratón: abrir/hover/confirmar al soltar tecla-botón, cancelar/Escape sin disparar poder; modo mando: stick con dead-zone/histeresis, centro neutro sin selección, confirmar o cancelar sin acciones no deseadas. Seleccionar y **activar** son eventos distintos. Opcional cámara/tiempo lento en laboratorio con restauración garantizada tras cerrar; sin pausar mundo/IA global por defecto.
- UI legible con movimiento rápido y alto contraste, feedback de 100ms orientativo sin depender de ninguna latencia real medida, ajuste de sensibilidad y accesibilidad visual. No crear generador de iconos por IA utilizando assets Fab/NoAI.
- Exponer interfaz `PowerWheelPort` que consume `HeroDescriptor` de L06-C y `AbilityId` de H01/H02/H03, sin escribir ni mapear input en los archivos canónicos; hooks de integración como pseudodiff en `docs/IB_L07C_HOOKS.md`.

## Pruebas
- 50 aperturas/cierres con mando simulado, 50 con ratón; 30 selecciones canceladas sin activar; 20 cambios de héroe H01→H02→H03 usando mock y reset de slots/cooldown; 20 cierres por caída/pausa/colisión sin quedarse en slow-motion.
- Comprobar conflicto abrir rueda mientras se agarra/carga vehículo, blink H03, escudo H02, golpe H01 y vuelo a 320m/s; prioridad menú > incapacidad > transporte > acción, preservar botón de lanzamiento. Documentar la matriz antes de conectar.
- Medir animación/UI en 1080p DX12 si se obtiene turno de Editor, FPS p95/p99/VRAM con rueda abierta/cerrada y audio; jamás declarar 60fps alcanzados por test unitario.
- Integración y aceptación solo tras tres héroes verificables, contrato de entrada único y DX12/humano; si no, entregar widget mock más tests, no rueda conectada a gameplay.

**Integración:** el integrador conserva menú pausa S8 pendiente y vincula selector S6; ninguna acción a código compartido desde este lab.