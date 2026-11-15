# Codex L06-C — S6: selector H01/H02/H03 y contratos de integración

**Modelo sugerido:** Codex arquitectura UE C++/UMG/QA, razonamiento **Extra High** para estudiar compatibilidad entre laboratorios.
**Worktree:** `../impact-l06c-selector` · **rama:** `prototype/ib-l06c-selector-lab`.
**Escritura exclusiva:** `ImpactBeyond/Source/ImpactBeyond/IB_S6/Selector/**`, `ImpactBeyond/Content/IB_S6/Selector/**`, `Tools/IBL06C_*`, `docs/IB_L06C_*`. No tocar H02, H03, Character, playercontroller ni GameMode.

## Identificación, seguridad y precedencia

Trabaja SOLO si tienes un chat Codex realmente abierto en el PC Windows del usuario. Esta ficha en GitHub NO inicia Codex ni selecciona modelo. Repositorio: https://github.com/meliodr/juego- . **BASE DE GAMEPLAY EXACTA:** `1d01db54b63250ed0aacb0704a5ac16290324e32` (`prototype/ib-sprint02`, PR #19). **BASE DOCUMENTAL** de estos prompts: PR documental de ocho frentes; léela remotamente, NO hagas merge del branch documental en el juego. En cada ejecución relee `git status`, `git worktree list`, remotes, branches, PRs recientes y diffs de S2.5/S3/S3.5/S4, auditoría PR #21 y refinamiento PR #22. Si nuevas entregas cambiaron el contrato, adapta propuesta SIN rebase destructivo ni duplicar trabajo.

**GATE ESTRICTO:** al corte 15-11-2026 10:28 AST NO hay ramas de implementación S2.5/S3/S3.5/S4 publicadas; PR #20 es una ficha CC0, PR #21 auditoría con validador Git y PR #22 ajustes documentales. No asumas que faltan trabajos locales: inspecciónalos cuando tengas acceso y registra resultados. Mientras los cuatro gates previos no estén aceptados/compatibles, limita este frente a **contrato, prototipo aislado no integrado, tests offline sin Unreal, recursos locales ya legales**, y bloquea cualquier edición del mundo canónico. No escribas `ImpactBeyondCharacter.*`, `ImpactCircuitComponent.*`, `ImpactBeyondPlayerController.*`, `ImpactBeyondGameMode.*`, `ImpactBeyond.Build.cs`, `ImpactBeyond.uproject`, `Config/**`, `IB_C00/**`, `.umap` principal, ni carpetas de otros agentes. Cualquier cambio compartido va como propuesta en docs, sin aplicarlo. Los ocho frentes usan *worktrees diferentes* nacidos del mismo SHA y **una sola sesión Unreal Editor/cook/packaging por PC en todo momento**. No borrar DDC, cachés, datos ni sobrescribir worktrees.

**Licencia/presupuesto:** RD$0; primero reusar Manny, Chaos, Niagara, materiales/meshes existentes y documentación. No descargar City Sample completo, reclamar Fab, aceptar nuevas licencias, habilitar/instalar plugins, comprar ni subir archivos fuente Epic/Fab como packs sueltos al repo público. Registrar `local/propiedad/licencia/tamaño/compatibilidad`; no suministrar bytes NoAI a modelos de IA. Meta de QA **1080p DX12 visible/audio ON y 60 FPS** (p95 frame <=16.7ms orientativo, p99/VRAM/hitches reportados, no declarados sin test); Windows UE5.8.x real por comprobar en máquina. Una prueba headless, screenshot estático o script aislado NO certifica jugabilidad ni FPS.

**Entrega:** rama de laboratorio exclusiva desde SHA S2; PR **draft** dirigido a `prototype/ib-sprint02`, con solo rutas permitidas. Adjunta SHA+diff, lista de clases/activos y licencias, tests PASS/FAIL/SKIPPED con evidencia real, mapa aislado sólo si correspondió acceso/turno a Unreal, dependencias para el integrador y estado explícito `SOLO_DOC / PROTOTIPO / BUILD / DX12_VERIFICADO / BLOQUEADO`. Nunca merge a `main`. La integración será serial y después de la revisión de las entregas antiguas.



## Encargo ejecutable tras los gates previos
- Releer `docs/SPRINT06_H02_H03_REUTILIZACION.md`, `docs/ROADMAP_SPRINTS_02_A_08.md`, S3 Map/RegionId y contratos S3.5/S4, además de ramas L06-A H02 y L06-B H03 si existen. No inferir funcionalidad por tener nombre de clase; comprobar API real, ABI, includes, `UCLASS`/UHT y mecanismos de input.
- Diseñar **una matriz de puertos** y un adaptador `HeroDescriptor` (ID, mesh autorizado/placeholder, locomoción, región actual, input/cámara compartidos, 3-4 habilidades de prueba, estado de agarre, cooldown, persistencia) para H01/H02/H03. La estructura de selector de personaje se guarda en TU subcarpeta y usa mock si las otras ramas no están listas.
- En mapa UI de laboratorio propio solo si Unreal disponible, construir un selector simple con 3 fichas original H01/H02/H03, estado visual elegido/ocupado, texto de controles, y retorno limpio al héroe anterior. No modificar ni anticipar el menú pausa completo S8.
- **Adaptador pasivo**, nunca duplicar `ImpactBeyondCharacter`, `GameMode`, `PlayerController` ni su InputMappingContext. Una propuesta de cambios en host se documenta con diff NO APLICADO (archivo textual `docs/IB_L06C_HOOKS.md`). Si no hay firma real de H02/H03, usar `MockHeroPort` y dejar selector como prototipo, no afirmar build tres héroes.
- Reutilización de VFX/animaciones: validar alcance de tres héroes, pero NO importar paquetes enteros, instalar GAS/GameplayAbilities/CommonUI ni subir fuentes Fab. Los agentes dueños de H02/H03 manejan su propia estética.

## Pruebas
- 30 selecciones H01→H02→H03 en flujo de prueba sin instancia duplicada ni audio/cámara colgados; 20 cambios durante agarre/efecto deben rechazar o posponer con feedback explícito; 10 resets de región preservan solo el estado correcto.
- Matriz teclado/ratón y DualSense **solo con periférico real disponible**: acciones al seleccionar no se ejecutan automáticamente, cooldown independiente, mapa/minimapa y estado RegionId consistente.
- Pruebas offline de contratos y estructura de DataAsset, compilación Unreal cuando sea posible, luego 1080p DX12/audio con meta 60fps y vídeo de cambio real. Sin 3 implementaciones de héroes verificadas, reportar `selector aislado, NO integrado`.

**Orden de integración:** el integrador compara contrato de H02/H03 contra el host; si no es compatible, no se cambia el host automáticamente. La rueda de poderes S7-D es un consumidor de HeroDescriptor, no parte de este encargo.