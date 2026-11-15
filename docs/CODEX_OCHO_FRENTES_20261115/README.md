# IMPACT: BEYOND — oleada coordinada de 8 frentes S2.5→S7
**Corte 2026-11-15 ~12:05 AST. ESTADO: OCHO PROMPTS VERIFICABLES PUBLICADOS, CERO CHATS CODEX INICIADOS DESDE ESTA EJECUCIÓN, CERO IMPLEMENTACIONES DE ESTOS PROMPTS COMPROBADAS.** Documento de trabajo, no build ni sprint terminado.

## Por qué este segundo set no duplica intencionalmente el anterior
Ya existe [PR #23](https://github.com/meliodr/juego-/pull/23) con ocho tareas granuladas **únicamente S5–S7** bajo `docs/CODEX_S5_S7_20261115`; son útiles cuando S2.5–S4 alcancen un gate. El usuario pidió **hasta ocho streams totales distribuidos desde S2.5, S3, S3.5, S4 dividido, S5, S6 y S7**. Este directorio es la **oleada coordinada vigente** y reutiliza los criterios anteriores, sin eliminar nada. Las ocho tareas de `CODEX_S5_S7_20261115` pasan a ser **backlog granular OPCIONAL**, no lanzar a la vez con este set. Los primeros cuatro prompts de [PR #17](https://github.com/meliodr/juego-/pull/17) siguen siendo referencia primaria y [PR #22](https://github.com/meliodr/juego-/pull/22) refinamientos P0 obligatorios. Para S4 ahora hay dos propietarios no superpuestos bajo `IB_S4/Response` y `IB_S4/Destruction`: el agente del S4 antiguo NO se lanza al mismo tiempo que ambos porque su propiedad amplia `IB_S4/**` entraría en conflicto.

## Estado y evidencia obligatoriamente leída
- Repositorio https://github.com/meliodr/juego- ; rama canónica de juego **`prototype/ib-sprint02` @ `1d01db54b63250ed0aacb0704a5ac16290324e32`**, PR #19 abierto. La rama `main` @ `aaad4491ca3610ba22d12f764d1ed0e35ca0356f` es principalmente documentación; no basar código nuevo en `main` ni fusionar gameplay automáticamente.
- [Auditoría PR #21](https://github.com/meliodr/juego-/pull/21) se ejecutó 08:45: no se publicaron los cuatro laboratorios S2.5/S3/S3.5/S4; contiene gate de ownership y **3/3 pruebas Python unitarias** de validador Git, NO Unreal.
- [Refinamiento PR #22](https://github.com/meliodr/juego-/pull/22) se publicó después: calidad visual, sweep de carga a 320/368 m/s y policía/tanque no letales; también corregida ficha de City Sample Vehicles (Free, AI allowed; NO standalone redistribution). No hay gameplay en ese PR.
- Revisión API de GitHub alrededor de **12:05 AST**: PR #23 ya publicado, PRs #17/#19/#20/#21/#22 en draft y sin merges, las ramas de implementaciones originales `prototype/ib-s25-realism-lab`, `prototype/ib-s3-planet-lab`, `prototype/ib-s35-interactions-lab`, `prototype/ib-s4-response-lab` NO aparecen; **0 GitHub Actions workflow runs**. No descarta cambios locales sin push.
- S2 **contiene implementación verificable en Git** (Character/FlightFeedback, pruebas y mapa). FPS 143.94 de PR #19 medidos offscreen con input simulado y audio WASAPI en su medición; no usar para dar por validadas 5 vistas realistas, experiencia humana, VRAM/H01 con coche levantado.

## Tres fallos que obligan a frenar promesas de avance
1. **P0 visual:** periferia S2 usa Cube/Sphere/Cylinder/Cone/Plane y coches Kit Cidade Low Poly; la silueta visual no cumple lo solicitado. Una manzana PBR antes de escala planetaria.
2. **P0 física:** barridos de cápsula a 320–368m/s NO confirman que un vehículo agarrado barre/colisiona; riesgo de penetración de paredes durante vuelo supersónico.
3. **P0 evidencia/integración:** cuatro labs S2.5–S4 sin commits/PR públicos/compilaciones DX12 visibles; ocho sesiones futuras sin dependencias no producen mágicamente S7 jugable. No usar benchmarks offscreen como aceptación visual.

## Ocho contratos listos para abrir en Codex de forma CONTROLADA

| # | Enlace | Rama exclusiva desde S2 SHA | Propietario principal | Gate para integrar |
|---|---|---|---|---|
| 01 | [S2.5 realismo](01_S25_REALISMO.md) | `prototype/ib-wave8-s25-visual` | `IB_S25/**` | 5 vistas A/B y PBR real |
| 02 | [S3 planeta](02_S3_PLANETA.md) | `prototype/ib-wave8-s3-planet` | `IB_S3/**` | 2 regiones persistentes → 4 ciudades |
| 03 | [S3.5 interacción](03_S35_INTERACCIONES.md) | `prototype/ib-wave8-s35-physics` | `IB_S35/**` | sweep de carga y 2 golpes |
| 04 | [S4 policía/tanque](04_S4_RESPUESTA.md) | `prototype/ib-wave8-s4-response` | `IB_S4/Response/**` | 20 balas + 20 misiles H01 sin HP |
| 05 | [S4 Chaos/VFX](05_S4_DESTRUCCION.md) | `prototype/ib-wave8-s4-destruction` | `IB_S4/Destruction/**` | muro transitable + cráter de colisión |
| 06 | [S5 vida y hora](06_S5_CICLO_CIVILES.md) | `prototype/ib-wave8-s5-life` | `IB_S5/Wave8Life/**` | S3 + S4; 36min y evento opcional |
| 07 | [S6 H02/H03](07_S6_H02_H03.md) | `prototype/ib-wave8-s6-heroes` | `IB_S6/Wave8Heroes/**` | 2 avatares/habilidades + selector |
| 08 | [S7 amenazas/rueda](08_S7_AMENAZAS_RUEDA.md) | `prototype/ib-wave8-s7-threats` | `IB_S7/Wave8Threats/**` | 1 robot/1 kaiju/rueda propuesta |

**Ocho worktrees Git exclusivos en el PC** pueden compartir objetos Git del repo, pero los caches y assets del proyecto NO se deben duplicar masivamente. Antes de crear más de dos worktrees, medir espacio libre SSD y tamaño real de Content. Todos nacen del mismo SHA S2, sin rebase forzado ni cambios globales. **Un solo Unreal Editor/cook/packaging a la vez**; trabajo de C++/estudio en paralelo, sesiones de Editor solo seriales. No se escriben `Character`, `Circuit`, `GameMode`, `Build.cs`, `uproject`, `Config`, `IB_C00.umap` desde ningún laboratorio. Si un integrador necesita cambiarlo, que lo haga DESPUÉS en branch separado y con tests. Ninguna sesión toca paths de otra.

## Secuencia de producción y definición de avance
- **Ola P0:** realismo (01) y colisiones (03) primero; planeta (02), policía (04) y destrucción (05) pueden producir prototipos aislados. Antes de comunicar S4 jugable hay que reconciliar DamageKind/AttackId/Collision y compilar juntos.
- **Ola P1 condicional:** S5 reloj/civiles 06 en mock o tests offline hasta S3 y alertas S4; S6 07 con H02/H03 y selector en laboratorio, no en mapa principal.
- **Ola P2 condicional:** S7 08 robot+kaiju+rueda bajo presupuesto GPU; solo un robot y un kaiju en ensayo, con la arena y entradas desacopladas. S8 **espacio + pausa** queda después y está fuera de esta oleada.
- **Etiqueta de entrega:** `SIN_EJECUCION`, `SOLO_DOC`, `CODIGO`, `COMPILADO`, `HEADLESS`, `DX12_VISIBLE`, `HUMANO_ACEPTADO`. Publicar SHA, diff, licencias y pruebas; un prompt no es un commit de gameplay. PR individual draft hacia `prototype/ib-sprint02`, mantener última build jugable, no merge/force push/main.
- **Presupuesto:** RD$0, sin City Sample completo, sin claim Fab/licencias nuevas/plugins/Windows cambios no autorizados. El paquete Fab puede requerir descargar ENTERO aunque se utilicen solo 2–3 módulos: nunca garantizar descarga selectiva. UE-Only no se publica standalone en GitHub; respetar NoAI.

## Resultado real de esta tarea
La API de GitHub **sí** permite publicar estas instrucciones y su PR #23. La conexión actual NO incluye control de Windows, Unreal Editor ni una acción para crear/enviar chats Codex: **se crearon 8 prompts, NO 8 chats, NO worktrees ni ramas gameplay**. Otro integrador/revisor debe leer este manifiesto y las nuevas entregas PR reales antes de intentar merge; si persiste falta de delivery, dejar S2 intacto y no prometer un sprint 7 terminado.

**Nota:** no lanzar simultáneamente los ocho prompts granulares de `docs/CODEX_S5_S7_20261115` y los ocho de esta ola, ya que duplica trabajo de S5/6/7 aunque las carpetas varíen. Elegir un único plan de coordinación.
