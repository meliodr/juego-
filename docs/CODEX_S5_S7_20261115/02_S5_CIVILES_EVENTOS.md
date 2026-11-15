# Codex L05-B — S5: civiles, incidentes y respuesta ligera

**Modelo sugerido:** Codex de C++/AI y pruebas con razonamiento **High**.
**Worktree:** `../impact-l05b-civics` · **rama:** `prototype/ib-l05b-civics-lab`.
**Escritura exclusiva:** `ImpactBeyond/Source/ImpactBeyond/IB_S5/Civics/**`, `ImpactBeyond/Content/IB_S5/Civics/**`, `Tools/IBL05B_*`, `docs/IB_L05B_*`. No modificar `IB_S5/Time` del L05-A.

## Identificación, seguridad y precedencia

Trabaja SOLO si tienes un chat Codex realmente abierto en el PC Windows del usuario. Esta ficha en GitHub NO inicia Codex ni selecciona modelo. Repositorio: https://github.com/meliodr/juego- . **BASE DE GAMEPLAY EXACTA:** `1d01db54b63250ed0aacb0704a5ac16290324e32` (`prototype/ib-sprint02`, PR #19). **BASE DOCUMENTAL** de estos prompts: PR documental de ocho frentes; léela remotamente, NO hagas merge del branch documental en el juego. En cada ejecución relee `git status`, `git worktree list`, remotes, branches, PRs recientes y diffs de S2.5/S3/S3.5/S4, auditoría PR #21 y refinamiento PR #22. Si nuevas entregas cambiaron el contrato, adapta propuesta SIN rebase destructivo ni duplicar trabajo.

**GATE ESTRICTO:** al corte 15-11-2026 10:28 AST NO hay ramas de implementación S2.5/S3/S3.5/S4 publicadas; PR #20 es una ficha CC0, PR #21 auditoría con validador Git y PR #22 ajustes documentales. No asumas que faltan trabajos locales: inspecciónalos cuando tengas acceso y registra resultados. Mientras los cuatro gates previos no estén aceptados/compatibles, limita este frente a **contrato, prototipo aislado no integrado, tests offline sin Unreal, recursos locales ya legales**, y bloquea cualquier edición del mundo canónico. No escribas `ImpactBeyondCharacter.*`, `ImpactCircuitComponent.*`, `ImpactBeyondPlayerController.*`, `ImpactBeyondGameMode.*`, `ImpactBeyond.Build.cs`, `ImpactBeyond.uproject`, `Config/**`, `IB_C00/**`, `.umap` principal, ni carpetas de otros agentes. Cualquier cambio compartido va como propuesta en docs, sin aplicarlo. Los ocho frentes usan *worktrees diferentes* nacidos del mismo SHA y **una sola sesión Unreal Editor/cook/packaging por PC en todo momento**. No borrar DDC, cachés, datos ni sobrescribir worktrees.

**Licencia/presupuesto:** RD$0; primero reusar Manny, Chaos, Niagara, materiales/meshes existentes y documentación. No descargar City Sample completo, reclamar Fab, aceptar nuevas licencias, habilitar/instalar plugins, comprar ni subir archivos fuente Epic/Fab como packs sueltos al repo público. Registrar `local/propiedad/licencia/tamaño/compatibilidad`; no suministrar bytes NoAI a modelos de IA. Meta de QA **1080p DX12 visible/audio ON y 60 FPS** (p95 frame <=16.7ms orientativo, p99/VRAM/hitches reportados, no declarados sin test); Windows UE5.8.x real por comprobar en máquina. Una prueba headless, screenshot estático o script aislado NO certifica jugabilidad ni FPS.

**Entrega:** rama de laboratorio exclusiva desde SHA S2; PR **draft** dirigido a `prototype/ib-sprint02`, con solo rutas permitidas. Adjunta SHA+diff, lista de clases/activos y licencias, tests PASS/FAIL/SKIPPED con evidencia real, mapa aislado sólo si correspondió acceso/turno a Unreal, dependencias para el integrador y estado explícito `SOLO_DOC / PROTOTIPO / BUILD / DX12_VERIFICADO / BLOQUEADO`. Nunca merge a `main`. La integración será serial y después de la revisión de las entregas antiguas.

## Encargo ejecutable tras los gates previos
- Releer `docs/CIVILIANS_ALERTS_KARMA_V06.md`, `docs/REFERENCES_DYNAMIC_EVENTS_V07.md`, `docs/SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md` y contrato provisional S4 de PR #22. **Una escena de ensayo** con 3–6 civiles, una ruta/volumen de refugio, 1 evento `EVT-ROB` simple o `EVT-DRONE`, sin misiones ni spawns masivos.
- Diseñar `IncidentLedger` con IDs estables y deduplicación de daño, `AlertState` (respuesta policía), `ReputationState` (atribución acciones del jugador) y `ThreatState` (amenaza independiente). No castigar por enfrentarse a un kaiju ni hacer a policía letal para H01. La alerta solo escala ante incidentes rastreables.
- Spawn budget: una amenaza activa, 3–6 civiles cargados y estado dormido fuera de región; pooling limitado, tick por eventos/timers y líneas de visión reducidas, fallback al mannequin ya incluido, navegación simple en micro-lab (no MassCrowd).
- Flujo `Idle→Candidate→Reserve→Telegraph→Active→Resolve→Cooldown`, abortar si colisión/streaming/seguridad región no lista; modo creativo elimina consecuencias sin borrar el registro de QA.
- Cualquier vínculo con Character/World/Alerta S4 debe ser evento/adaptador propuesto en `docs/IB_L05B_INTEGRACION.md`, no parche canónico.

## Pruebas
- 20 incidentes con repetición de `AttackId/TargetId` sin alerta duplicada; 20 huídas/refugios/rescates con limpieza segura; 20 entradas/salidas de región preservando el incidente y sin duplicar actor.
- 5 rondas sin jugador: no spawn espasmódico; 10 casos de prioridad robot/robo/civil con un solo evento activo y cooldown; los civiles no causan bloqueo de carril de vuelo.
- Perfil con 0,3,6 civiles y evento en 1080p DX12 visible y audio; latencia p95/p99, CPU AI, VRAM, hitches. Si no hay editor, tests del FSM + JSON de casos y estado PROTOTIPO, no afirmar mundo vivo.

**Integración futura:** consumir el reloj L05-A, S3 RegionId y S4 DamageKind/Alert únicamente cuando tengan contratos verificados; no añadir nuevos sistemas globales por duplicado.