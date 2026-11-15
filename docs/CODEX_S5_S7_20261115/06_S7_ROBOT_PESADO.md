# Codex L07-A — S7: robot pesado modular con daño real

**Modelo sugerido:** Codex C++/Chaos/IA con razonamiento **High o Extra High**, según capacidad real disponible.
**Worktree:** `../impact-l07a-robot` · **rama:** `prototype/ib-l07a-robot-lab`.
**Escritura exclusiva:** `ImpactBeyond/Source/ImpactBeyond/IB_S7/Robot/**`, `ImpactBeyond/Content/IB_S7/Robot/**`, `Tools/IBL07A_*`, `docs/IB_L07A_*`. No tocar `IB_S7/Kaiju`, `IB_S7/Radial` ni IA global.

## Identificación, seguridad y precedencia

Trabaja SOLO si tienes un chat Codex realmente abierto en el PC Windows del usuario. Esta ficha en GitHub NO inicia Codex ni selecciona modelo. Repositorio: https://github.com/meliodr/juego- . **BASE DE GAMEPLAY EXACTA:** `1d01db54b63250ed0aacb0704a5ac16290324e32` (`prototype/ib-sprint02`, PR #19). **BASE DOCUMENTAL** de estos prompts: PR documental de ocho frentes; léela remotamente, NO hagas merge del branch documental en el juego. En cada ejecución relee `git status`, `git worktree list`, remotes, branches, PRs recientes y diffs de S2.5/S3/S3.5/S4, auditoría PR #21 y refinamiento PR #22. Si nuevas entregas cambiaron el contrato, adapta propuesta SIN rebase destructivo ni duplicar trabajo.

**GATE ESTRICTO:** al corte 15-11-2026 10:28 AST NO hay ramas de implementación S2.5/S3/S3.5/S4 publicadas; PR #20 es una ficha CC0, PR #21 auditoría con validador Git y PR #22 ajustes documentales. No asumas que faltan trabajos locales: inspecciónalos cuando tengas acceso y registra resultados. Mientras los cuatro gates previos no estén aceptados/compatibles, limita este frente a **contrato, prototipo aislado no integrado, tests offline sin Unreal, recursos locales ya legales**, y bloquea cualquier edición del mundo canónico. No escribas `ImpactBeyondCharacter.*`, `ImpactCircuitComponent.*`, `ImpactBeyondPlayerController.*`, `ImpactBeyondGameMode.*`, `ImpactBeyond.Build.cs`, `ImpactBeyond.uproject`, `Config/**`, `IB_C00/**`, `.umap` principal, ni carpetas de otros agentes. Cualquier cambio compartido va como propuesta en docs, sin aplicarlo. Los ocho frentes usan *worktrees diferentes* nacidos del mismo SHA y **una sola sesión Unreal Editor/cook/packaging por PC en todo momento**. No borrar DDC, cachés, datos ni sobrescribir worktrees.

**Licencia/presupuesto:** RD$0; primero reusar Manny, Chaos, Niagara, materiales/meshes existentes y documentación. No descargar City Sample completo, reclamar Fab, aceptar nuevas licencias, habilitar/instalar plugins, comprar ni subir archivos fuente Epic/Fab como packs sueltos al repo público. Registrar `local/propiedad/licencia/tamaño/compatibilidad`; no suministrar bytes NoAI a modelos de IA. Meta de QA **1080p DX12 visible/audio ON y 60 FPS** (p95 frame <=16.7ms orientativo, p99/VRAM/hitches reportados, no declarados sin test); Windows UE5.8.x real por comprobar en máquina. Una prueba headless, screenshot estático o script aislado NO certifica jugabilidad ni FPS.

**Entrega:** rama de laboratorio exclusiva desde SHA S2; PR **draft** dirigido a `prototype/ib-sprint02`, con solo rutas permitidas. Adjunta SHA+diff, lista de clases/activos y licencias, tests PASS/FAIL/SKIPPED con evidencia real, mapa aislado sólo si correspondió acceso/turno a Unreal, dependencias para el integrador y estado explícito `SOLO_DOC / PROTOTIPO / BUILD / DX12_VERIFICADO / BLOQUEADO`. Nunca merge a `main`. La integración será serial y después de la revisión de las entregas antiguas.



## Encargo ejecutable tras los gates previos
- Releer `docs/COMBAT_ROSTER_V03.md`, `docs/REFERENCES_DYNAMIC_EVENTS_V07.md`, `docs/ROADMAP_SPRINTS_02_A_08.md` y resultados de S3.5/S4; escoger **una** amenaza RBT03 «Demoledor» aprox. 5–6m o RBT02 «Carcelero» aprox. 2.5m si rendimiento/recursos desaconsejan la primera; no implementar 10 robots ni otro engine de IA.
- Buscar primero **meshes modulares** de robot que ya estén instalados y legítimamente accesibles; si faltan, prototipo con mallas del motor/materiales de lectura clara e informe de asset pendiente (sin llamar «arte final» a un cubo). No instalar nuevos componentes ni clonar redes de assets.
- Modelo de combate: cuerpo central con 2 piezas separables `PartId`, estados `Intact→Exposed→Detached/Disabled`; perder brazo/arma debe deshabilitar un ataque y actualizar hitbox/AI. Ataque pesado de corto alcance y proyectil telegrafiado opcional, pausa de recuperación legible, 1 robot activo en laboratorio, control de amenaza independiente del karma.
- Consumir el **contrato S4** de DamageKind, Impulse, TargetId y AttackId si existe y está probado; si no, mock local con propuesta de mapeo sin incluir headers de rama ajena. Una parte desprendida puede convertirse en objeto agarrable a través del contrato S3.5; no copiar ni modificar las clases fuente de S3.5.
- Crear microescenario propio `/Game/IB_S7/Robot/Lvl_IB_L07A_Robot` solo si editor disponible y exclusivo. Impactos sobre muro/vehículo preparados; ragdoll/Chaos solo en partes designadas, no destrucción arbitraria total ni gore.

## QA y reglas de aceptación
- 30 golpes H01/H02/H03 simulados o reales (etiquetar cuál), 20 intentos de desarme por pieza, 20 ataques con cobertura/cooldown, 10 reinicios sin duplicar AI/hitbox, 20 pruebas de recoger pieza tras corte. Fallos por doble daño, pieza invisible colisionable o animación sin efecto bloquean la aceptación.
- 1080p DX12/audio: p95/p99 + VRAM, máximos de CPU/Chaos, prueba de 2 ataques simultáneos **solo con presupuesto seguro**; regla un robot activo inicialmente, objetivo 60fps, sin falsificar métricas.
- No afirmar integrado en las 4 ciudades/los 3 héroes sin la build de S6 y S3 realmente integrada.

**Integración:** la rama dueña sólo del robot expone interfaz a miniinvasión S5-B y piezas a H02/H03, con adaptadores textuales pendientes del integrador.