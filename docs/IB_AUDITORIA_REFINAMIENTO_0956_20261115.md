# IMPACT: BEYOND — auditoría de corte y refinamiento P0 · 15-nov-2026, 09:56 AST

**Trabajo documental ejecutado; no es un sprint de gameplay.** Revisión independiente de la API GitHub y de los fuentes en el repo público `meliodr/juego-`. Base congelada: [PR #19 / `1d01db54b63250ed0aacb0704a5ac16290324e32`](https://github.com/meliodr/juego-/pull/19). Plan original: [PR #17](https://github.com/meliodr/juego-/pull/17) `docs/roadmap-sprints-02-08-20261115` / `bb97a37df78cf29324e8bdfe7b6e17c50cc2cffe`. Auditoría previa: [PR #21](https://github.com/meliodr/juego-/pull/21) `c8708f7d7d0cafb4fdff273ce76e028fac44bd1f`. **No se ha accedido al disco Windows, Unreal Editor, Epic Launcher, Steam ni a chats de Codex.**

## Resultados comprobados, con alcance

API GitHub: **22 ramas**, **19 PRs**, incluyendo #20 (una ficha de candidato CC0 `Shrub 03`, no una malla integrada) y #21 (auditoría + gate Git-only, no gameplay); **no hay ninguno de los cuatro heads previstos** `prototype/ib-s25-realism-lab`, `prototype/ib-s3-planet-lab`, `prototype/ib-s35-interactions-lab`, `prototype/ib-s4-response-lab`. La consulta de Actions muestra **0 workflow runs**; status y workflow runs del SHA de S2 están vacíos. Esto significa **sin entrega pública verificable, no evidencia de ausencia de trabajo sin push**.

El baseline S2 **sí contiene implementación C++/uasset**, no solo planes. `compare` contra `prototype/ib-urban-mvp` muestra 2 commits y **40 archivos** (incluyendo Character, Circuit, poses de vuelo, `ImpactFlightFeedback`, `ImpactRadialLab.cpp` y `Author-Sprint02World.py`). `main` es una rama distinta, principalmente documentación; no se debe hacer cherry-pick a ciegas.

| Frente | Comprobado en GitHub | No demostrado |
|---|---|---|
| S2.5 | PR #16 aporta fachadas del distrito, PR #20 documenta un arbusto Poly Haven, PR #19 contiene geometría S2 | Rama del laboratorio, importación nueva legal, cinco capturas A/B, DX12 visible, coches realistas o vegetación final |
| S3 | `ImpactRadialLab.cpp` y contrato de región/gravedad de `IB_SPRINT02_PLANET_LAB.md` | 4 ciudades transitables, streaming planetario, precarga, guardar/restaurar estado y 10 retornos |
| S3.5 | `ImpactCircuitComponent`, pruebas del circuito y cápsula de S2 | Barrido **del volumen completo de la carga** a 320/368 m/s, cinco clases, 30 agarres por clase, punches/dedup/IK visual |
| S4 | Prompt y contrato de respuesta/daño `HealthDelta=0` | Policía humano en gameplay, bala sin daño a H01, tanque con misil no letal+knockback+recuperación, robot, telequinesis/láser, suelo con colisión |

**Escalera de evidencia:** sin rama = `AUSENTE EN GITHUB`; documento = `DOCUMENTADO`; C++/.uasset = `CÓDIGO`; log de compilación local reproducible = `COMPILADO`; prueba sin pantalla = `HEADLESS`; captura/métrica DX12 en pantalla con sonido = `DX12 VISIBLE`; prueba con mando real = `HUMANO`. Ningún laboratorio nuevo supera `AUSENTE EN GITHUB`. El gate del PR #21 reporta 3/3 tests sintéticos y verifica solo genealogía/propiedad, **NO** compilación ni funcionalidad. La auditoría **no reprodujo** por sí misma los benchmarks de S2.

### Acertó

1. S2 dejó una base para vuelo de 5 niveles, turbo, lanzamiento, ruta de costa y C++ con test radial; el código está versionado.
2. Reutilización de fachadas Poly Haven CC0 y componentes Unreal reduce coste frente a crear todo desde cero.
3. Hay un punto de comparación real (PR19 SHA fijo), rutas de propiedad exclusivas y PR #21 publica un gate de Git reproducible para futuros laboratorios.

### Falló / evidencia insuficiente

1. `Tools/Author-Sprint02World.py` usa `Cube` (autovía), `Cylinder` (mirador/playa y troncos), `Sphere` (colinas), `Plane` (agua) y `Cone` (copas); además `Kit Cidade Low Poly` para coches. El nivel puede tener buen FPS sin cumplir el aspecto City Sample.
2. PR19 documenta **143.94 FPS de media, p95 8.53ms, p99 9.59ms, VRAM 2514MiB** en una medición DX12 1080p con audio WASAPI, pero render offscreen, entradas simuladas, saltos de posición y dos cuadros >50ms cerca de recarga. En PR18, el audio de su benchmark final estaba desactivado; **no mezclar condiciones**. Sin imagen de gameplay + mando físico no hay aceptación artística ni de sensación.
3. La prueba de choque de cápsula a **320/368 m/s** no prueba colisión de un auto transportado por delante/al lado. Tampoco hay evidencia de contrato de daño implementado S4, de streaming/persistencia S3 o de colisión de cráteres.
4. **Corrección de fuente oficial Fab:** [Buildings](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5) figura **Free, UE-Only, «Allows usage with AI: No»**; [Vehicles](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f) figura **Free, UE-Only, «Allows usage with AI: Yes»**. La auditoría PR21 y selección previa marcaban Vehicles como NoAI: **era incorrecto en la ficha consultada a este corte**. No confundir ese selector con licencia para redistribuir el paquete. La [Fab Standard License](https://www.fab.com/eula) permite incorporar a un juego, pero prohíbe redistribución standalone. Las dos fichas públicas indican `Add to My Library`: **no prueba compra/reclamación de esta cuenta**, ni tamaño descargable por módulo. Poly Haven y ambientCG declaran CC0; registrar URL exacta, licencia y hashes locales.

## Tres refinamientos P0 por valor jugable/visual

**P0-01 · Vertical slice visual de una sola manzana, antes de expandir ciudad/planeta.** `IB_S25` aislado con **4 fachadas reutilizadas + 2-3 coches con malla convincente + vegetación real + acera/asfalto PBR + costa sin mar Plane en primer plano**. Reutilizar primero assets ya en `/Game/IB_Donors`, ConceptCar y mallas Poly Haven/ambientCG CC0 verificados. Comparar 5 vistas con FOV/hora/posición idénticos. No descargar City Sample entero; Buildings/Vehicles solo si ya reclamados/locales y aceptados, sin copiar fuentes sueltas a GitHub público. Medir 1080p DX12 visible con audio/mando; objetivo p95 <=16.7ms, mostrar p99 y VRAM; si falla, mejorar calidad/LOD sin escalar a 8 ciudades. Prompt: [IB_REFINAR_S25_VISUAL.md](IB_REFINAR_S25_VISUAL.md).

**P0-02 · Barrido de carga física a velocidad superhumana.** Ensayar carga de coche/container/pole/rock/loose part independiente del CapsuleComponent: sweep desde posición previa a destino, normal de impacto, safe offset, detach de lanzamiento, reset, estado IK, rebote y no atraviesos. Casos de pared de 4cm, esquina, techo, aterrizaje, cambio de sentido a 320 y 368 m/s; **30 ciclos por categoría**, no falsear cuando el editor no está disponible. Prompt: [IB_REFINAR_S35_FISICA.md](IB_REFINAR_S35_FISICA.md).

**P0-03 · Respuesta jugable no letal de policía y tanque.** Laboratorio S4 con 1 policía, 1 tanque y 1 blanco, sin nueva IA global: `HealthDelta=0` para bala y misil sobre H01, misil añade impulso acotado, estado recuperable con protección antistunlock, impactos repetidos deduplicados y sin daño a civiles durante tests. Registrar secuencia vídeo/capturas y **20 disparos de policía + 20 misiles** con resultado. Prompt: [IB_REFINAR_S4_RESPUESTA.md](IB_REFINAR_S4_RESPUESTA.md).

**S3 es prerequisito técnico, no se descarta:** antes de prometer cuatro ciudades, demostrar 2 regiones y retorno con estado en 10 ensayos, polos y origen global en doble precisión, colisión y streaming explícito; luego ampliar hubs. Prompt: [IB_REFINAR_S3_PLANETA.md](IB_REFINAR_S3_PLANETA.md).

## Decisiones operativas

- Cuatro entregas posibles **sólo** en branches independientes desde SHA S2, PR draft contra `prototype/ib-sprint02`, owned paths según PR17 y `Tools/IBAudit_ValidateLabs.py` (PR21). `Character`, `Circuit`, `Build.cs`, `Lvl_IB_C00`, `DefaultEngine.ini`, `uproject` y mapas raíz intocables por laboratorios. Parche de integración opcional como texto NO aplicado.
- Una sola instancia Unreal y cook por equipo. No asumir 8 sesiones Codex lanzadas por existir 8 prompts. Esta ejecución **no dispone de despachador Codex local**; se dejan instrucciones autocontenidas en rama documental.
- **No merge a main**, nada de instalar plugins ni reclamar/aceptar licencias Fab; no subir `.uasset` UE-Only crudos a GitHub público; solo fotos de prueba permitidas y métricas sin extracción de contenido.
- El objetivo del usuario de terminar hacia S7 antes de 12:30 AST es un objetivo, **no un estado verificado**. Avanzar a S5–S7 solo tras cuatro gates con SHA, compatibilidad C++ y sesiones DX12/humano.
