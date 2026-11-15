# IMPACT: BEYOND — Corte de integración en rama de ensayo (15/11/2026)

**Estado verificable: ramas S2.5/S3/S3.5/S4 publicadas; código de S3/S3.5/S4 COPIADO ADITIVAMENTE a una rama de ensayo sin activar gameplay. NO hay compilación combinada UE, ni nivel S2.5 portable en esta rama, ni Sprint 7 integrado.** Esta documentación es el registro de operaciones; NO sustituye pruebas Windows/DX12 humanas.

## 1. Secuencia y sorpresa importante entre revisiones

Se leyeron ANTES de integrar las dos revisiones previas:

- [Auditoría 08:45, PR #21](https://github.com/meliodr/juego-/pull/21): las ramas S2.5–S4 no tenían entregas remotas durante su corte; auditoría de propiedad de rutas con 3 pruebas Python según su reporte.
- [Ocho frentes 10:30, PR #23](https://github.com/meliodr/juego-/pull/23): publicó ocho **prompts** de S5–S7, no ocho chats Codex ejecutados. [PR #22](https://github.com/meliodr/juego-/pull/22) documentó divergencias; [PR #24](https://github.com/meliodr/juego-/pull/24) creó gate de pruebas en una rama reversible desde S2.
- **CAMBIO REAL POSTERIOR:** desde las 16:14–16:16 UTC (12:14–12:16 AST) aparecieron cuatro PRs de **código**, todos draft y apuntando a S2 `1d01db54b63250ed0aacb0704a5ac16290324e32`. La ausencia de ramas en auditorías anteriores era correcta *en aquel momento*, pero no debe repetirse como estado actual.

| Módulo / PR | Remoto verificado | Qué significa realmente |
|---|---|---|
| [S2.5 #25](https://github.com/meliodr/juego-/pull/25) | `ba964d9cf9a9dbd87cd89c6368107f0bd32194fe`, 39 rutas nuevas; 5 PNG de 1920×1080 + antes; mapa `IB_S25` publicado | Editor UE5.8.4 DX12 y audio **reportados ejecutados** por el autor; realismo **no aceptado**, benchmarking no realizado. Un clon sin activos importados locales no reproduce el mapa. |
| [S3 #27](https://github.com/meliodr/juego-/pull/27) | `2f08add14a341bfef1f3fd91a1c3c0c6b7e56a12`, 21 rutas nuevas | Geometría radial, manifiesto/revisión de saves, 4 hubs, UMG y tests diseñados. 336.251 + 100.172 aserciones **MSVC matemáticas** reportadas; sin mapa generado, UHT, física real, reentrada o save en Unreal. |
| [S3.5 #26](https://github.com/meliodr/juego-/pull/26) | `1ffe10085bb8c7e3afc44ea14a863fa05816d15a`, 30 rutas nuevas | Cinco clases agarrables, puños, ledger, barrido/carga implementados como laboratorio. 2.613 assertions nativas + 280 comparativas reportadas; sin compilación Unreal, sin 30 agarres Chaos, sin animación IK probada. |
| [S4 #28](https://github.com/meliodr/juego-/pull/28) | `7755e366d8e06c23150e37a71b21120e844a857c`, 27 rutas nuevas | Código de policía, misil, H01 inmune a bala/misil, tanque proxy, daño estructural preparado, láser/telequinesis. 2.010 checks nativos reportados; no UHT, mapa, NPC visible, DX12 ni daños/FX físicos verificados. |
| S5–S7 [#23](https://github.com/meliodr/juego-/pull/23) | 16 prompts en dos series de ocho dentro del PR | **Preparados, no hay ocho ramas de implementación/ejecuciones S5–S7 demostradas.** |

`main` `aaad4491ca3610ba22d12f764d1ed0e35ca0356f` continúa orientado a preparación/documentación y NO es la cabeza jugable. La base de ensayo es PR #19 `prototype/ib-sprint02` (`1d01db54`); su PR describe 143,94 FPS offscreen con inputs simulados, **no valida 1080p DX12 visible + audio + mando 60FPS en realismo S2.5**. API GitHub Actions: **0 ejecuciones** en el inventario de esta revisión; ausencia de CI no significa PASS.

## 2. Operación aplicada a GitHub, sin tocar ramas ajenas

Rama NUEVA: **`integration/ib-s2-s4-isolated-labs-20261115`** desde S2 SHA completo `1d01db54b63250ed0aacb0704a5ac16290324e32`.

Se creó un árbol Git sobre la base S2 mediante **SHA exactos de blobs de las ramas de autor**, sin rebases ni merges de sus ramas ni modificación de `main`. Se incorporaron solo rutas nuevas:
- 20 archivos S3: `IB_S3/`, `Tools/IBS3_*`, `docs/IB_S3_*` y README del contenido;
- 29 S3.5: `IB_S35/`, `Tools/IBS35_*`, `docs/IB_S35_*`;
- 26 S4: `IB_S4/`, `Tools/IBS4_*`, `docs/IB_S4_*`;
- 3 gates QA: `Tools/IBAudit_ValidateLabs.py` y 2 tests bajo `Tools/tests/` del PR #24.

**Total: 78 archivos copiados, sin modificación previa, y excluidos los documentos `*_PR_DRAFT.md` por obsolescencia.** En el árbol reunido se comprobaron sin coincidencias de ruta entre módulos/base: no se copiaron nuevos `.umap`, `.uasset`, `.uproject`, `Config`, `Build.cs`, `ImpactBeyondCharacter.*`, `ImpactCircuitComponent.*` ni `IB_C00`. Se revisaron diffs del código: 29 tipos `IMPACTBEYOND_API` con nombres únicos, sin includes propios `IB_S3/IB_S35/IB_S4` faltantes o headers `.generated.h` mal nombrados en el conjunto de fuentes; **esto es inspección estática, no validación de UHT o linking**.

La compilación C++ del módulo escanea automáticamente estas nuevas fuentes; la rama es un **candidato de compilación**, NO una build que haya pasado. Ningún GameMode/mapa/H01 de S2 se enlazó a los nuevos sistemas. No se fusionaron los receptores de S3.5 y S4. Tampoco se incorporó el `IB_S25` binario, pues el propio PR #25 declara referencias a importaciones `.uasset` locales excluidas del repositorio: cherry-pick del mapa solo produciría una prueba visual rota en un clone limpio. El laboratorio S2.5 original y sus capturas permanecen intactos en PR #25.

## 3. Matriz de dependencias y propiedad de archivos

| Orden | Módulo | Aporta ahora | Depende de / conflicto | Estado para entrega |
|---|---|---|---|---|
| S2 | H01, Circuit, IB_C00 mapa original | Caracter, GameMode, Circuit y mapa binario como dueño único | Geometría costera low-poly, cámara/carga y input visibles pendientes | **BASE CONSERVADA; no aceptada 60FPS visible** |
| S2.5 | Escena aislada con 5 vistas, PBR/árbol/coche | `IB_S25`, scripts/reviews en #25, **no importados aquí** | Texturas y 66,3MB árbol CC0 locales fuera del Git + árbol LOD0 de 1.599.403 triángulos; VRAM/FPS y cámara faltan | **ARTE NO ACEPTADO; no se entrega mapa portable** |
| S3 | C++ bajo `IB_S3`, geometría y 4 hubs | Único dueño de S3/manifest/save; requiere creacion `IB_S3` en Editor | Revisión 2 invierte coordenada este/oeste (+ longitud = -Y), save rev1 no migrado; streaming World Partition NO implementado | **CODIGO EN RAMA DE ENSAYO; UHT/UE pendiente** |
| S3.5 | C++ bajo `IB_S35`, Grip, Punch, Receiver | Lab actor derivado de H01; instancia nativa de lease | Volumen del vehículo a 368m/s, 20 paredes y agarre 30× por clase sin Chaos real | **CODIGO EN ENSAYO; integración al H01 bloqueada** |
| S4 | C++ bajo `IB_S4`, Response, misil/tanque/policías/robot proxy | LabPawn independiente, receptor y ataque propio | NO conectar S3.5 directamente: enum, signo de HP, unidades, tiempo, ID/ledger incompatibles | **CODIGO EN ENSAYO; QA UHT/colisiones pendiente** |
| S5 | Ciclo solar y civiles | Ideas/prompts PR #23 | S3 regions/time y S4 alarmas contractuales pendientes | **SOLO PROMPTS** |
| S6 | H02, H03 y selector | Ideas/prompts PR #23 | API de poderes/daño/respawn/H01 no acordada; assets/licencias | **SOLO PROMPTS** |
| S7 | Robot/kaiju/rueda | Ideas/prompts PR #23 | S4 amenazas/S6 selector, respuesta y rendimiento | **SOLO PROMPTS** |

### Contrato crítico S3.5 ↔ S4: BLOQUEADO

No conectar por cast numérico: S3.5 `QuickPunch=0` se convertiría en bala S4 y `HeavyPunch=1` en misil S4, volviendo inmunes a golpes y aplicando un empuje incorrecto. Las conversiones también deben tratar:
1. `HealthDelta <= 0` (S3.5) ↔ `Amount >= 0` (S4), y pérdida *efectiva* de salud ↔ pérdida solicitada para romper estructura. Un receptor muta una vez, nunca encadenar ambos.
2. Impulso S3.5 **kg·cm/s** ↔ `DeltaVelocity` S4 **cm/s**; convertir explícitamente por masa, no duplicar Chaos si `bPhysicsResolved` ya es true.
3. GUID de 128 bits ↔ serial int64, edad relativa de ataque/lanzamiento ↔ tiempo absoluto, ventanas 10s vs 2s, ledger 4096/vida vs 256 con marca de serial retirado; acordar identidad/epoch/dedup ANTES del puente.
4. Pieza física exacta `TargetPart`, emisor real, salud original, reinicio de vida y perfiles de inmunidad H01. No ignorar torso al arrastrar cañón, ni regenerar AttackId en cada contacto.

**Requisito de gate S4:** veinte balas humanas y veinte misiles separados/rápidos deben producir `HealthDelta=0` para H01 y un solo impulso limitado por cooldown, nunca bucle de stunlock o muerte; para blancos normales deben producir daño. Los checks actuales son política numérica, no prueba de proyectiles/impactos/animación/sonido bajo Unreal.

## 4. Realismo City Sample, activos y rendimiento

[Cinco capturas reales S2.5 y revisión del autor](https://github.com/meliodr/juego-/blob/prototype/ib-s25-realism-lab/docs/IB_S25_VISUAL_REVIEW.md) muestran fachadas PBR, coches ConceptCar y árboles CC0, pero aún repetición, azoteas poco convincentes, parque vacío, ventanas oscuras, solo modelo de coche en primer plano y hojas dependientes de LOD0. Su comparación A/B es dentro del laboratorio **antes/después de correcciones**, no frente a S2 ni una imagen oficial de City Sample. No proclamar calidad \"mitad de City Sample\" o benchmark hasta reproducir mismo encuadre/iluminación. **Arte final low-poly queda rechazado**: S2 carretera/tierra con cubes y árboles de cones/cylinders pueden existir como colisión/proxy de desarrollo, no mostrarse como entrega final.

Procedencia declarada S2.5: Poly Haven CC0 asfalto/acera/Island Tree (66.337.268 bytes árbol, 44.243.757 bytes texturas) con hashes y manifiestos; donor UE preexistente de fachadas/ConceptCar. City Sample Buildings/Vehicles y Niagara packs NO se descargaron, ni se reclamaron licencias nuevas, ni se pasó contenido NoAI como entrada a modelos. La licencia UE-Only de Fab puede permitir su uso en proyecto UE según términos pero **no permiso para redistribuir paquetes fuente**, verificarlos antes de cualquier commit público de assets. No instalar plugin ni modificar Windows.

Unreal del autor S2.5 reportó **UE5.8.4 CL58902052 con DX12 RTX 4070 Laptop y audio inicializado**, cinco capturas, pero **0 muestras de FPS**, sin evidencia de Steam Input/mando, VRAM de nivel realista o prueba de colisión. S2 offscreen 143,94fps de la documentación previa **no** sirve como benchmark del nuevo arte.

## 5. Pruebas efectivamente observadas y gates pendientes

**Ejecución nueva realizada desde la auditoría GitHub:** comparación de cinco ramas contra base (todas ahead/behind=0, merge-base exacto), inventario de diffs; cruce de 78 rutas/blob IDs en árbol, ausencia de colisiones; revisión de 44 archivos C++/headers por diffs y 29 tipos únicos/inclusiones propias completas. Se creó el commit de ensamblaje y actualizó solo la rama nueva. **NO se ejecutó en este turno:** Python unit tests, nativos C++, UHT/UBT, UnrealEditor, cook, Chaos, DX12, audio, gamepad, perfilador ni QA de imágenes. Los conteos MSVC y S2.5 Editor son evidencias reportadas por autores en sus PRs, no pruebas repetidas por el integrador.

**Puerta de aceptación para Windows (seriada, sin limpiar worktrees/caches):**

1. Sin tocar tu worktree original: `git fetch origin integration/ib-s2-s4-isolated-labs-20261115` y `git worktree add "..\impact-labs-trial" --detach FETCH_HEAD` en un destino inexistente con espacio suficiente. Verificar `git status --short`, `git rev-parse HEAD`. Trabajar con Unreal solo en este worktree de ensayo; jamás reset o limpieza agresiva del original.
2. Comprobar que **UE instalado exacto 5.8.4** en `Engine/Build/Build.version`, disco, VS/MSVC y ausencia de UnrealEditor/UBT/cook de otros equipos. Obtener turno exclusivo antes de lanzar nada. No reinstalar ni habilitar plugins a ciegas.
3. Ejecutar desde raíz `python -m unittest discover -s Tools/tests -p 'test_IBAudit_*.py' -v` y `python Tools/IBAudit_ValidateLabs.py --help`. En cada lab, seguir las instrucciones propias `Tools/IBS3_TestCore.ps1`, `Tools/IBS35_TestPolicy.ps1`, `Tools/IBS4_Check.py`. Estos tests verifican solo reglas/estructura.
4. Con **turno exclusivo y autorización local**, compilar `ImpactBeyondEditor Win64 Development` con UBT de esa instalación apuntando a `ImpactBeyond/ImpactBeyond.uproject`. Capturar log de UHT y C++ íntegro, reportar primero error real, no forzar cambios a `Build.cs`/Character para simular un PASS.
5. **Solo si UHT/UBT pasa:** ejecutar por separado autores `IBS3_AuthorLab.py`, `IBS35_AuthorLab.py` y `IBS4_AuthorLab.py` siguiendo sus README/scripts en Editor, nunca simultáneos. Hacer 30 agarres/clase, 20 barridos de carga contra muro fino, 20 balas+20 misiles a H01, 10 reentradas y saves/restarts de S3. Registrar logs y fallos. El mapa S2.5 no se abre de forma reproducible en esta rama sin construir assets, así que revisarlo únicamente desde su trabajo aislado.
6. Ejecutar QA visual **con ventana DX12, salida de audio, mando físico/Steam Input**: 1920×1080, 100% y prueba B 83% (TSR), 10–15min con vuelo 90–368m/s, carga 250kg+ y aterrizajes, captura cinco cámaras comparables S2/S2.5/City Sample, p95/p99 frametime, VRAM/streaming/hitches y reinicio. Solo atribuir 60FPS si la captura/traza real confirma y el arte está aceptado. Steam Input no puede inferirse del plugin ni del test simulado.
7. Resolver adaptador S3.5/S4 y save S3 solo en una **rama sucesora** tras UHT/UE rojo→verde. No conectar mapas ni cambiar el Character base hasta entonces. Nuevos sprints S5–S7 permanecen aparte.

## 6. Riesgos prioritarios y próximo cambio seguro

1. **Gate compilación faltante**: las nuevas clases se incluyen automáticamente en un build de módulo. Un fallo UHT puede bloquear el proyecto entero aunque los tres labs no estén en GameMode. La rama queda draft; no pedir merge sin compilar y corregir en rama sucesora.
2. **Interfaz de daño/colisión**: S3.5 y S4 son independientes pero **NO semánticamente integrables** aún. Crear diseño de adaptador `IBCombatHitV1` y matriz de pruebas negativos antes de conectar Circuit, Character o policía. Mantener una sola fuente de verdad para salud por blanco.
3. **Dependencias artísticas y mapa S25**: primera meta es escena callejera portable y 60FPS p95 sin LOD0 1,6M tríangulos por árbol. Sustituir árboles en LOD controlado/material, agregar variedad de fachadas y props, elegir únicamente assets que se necesiten.

**No se modifica `main`, no se fuerzan pushes, no se hace merge al PR de autor, no se instala nada, no se descargan assets nuevos ni se borra nada local.** El estado de integración real es *código aislado reunido en candidato de compilación*; el videojuego jugable más verificable permanece en la rama S2 y sus pruebas DX12 offscreen son históricas.
