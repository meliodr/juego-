# ENCARGO CODEX S3 — planeta: validar transición entre 2 regiones antes de 4 ciudades

**Modelo sugerido:** Codex con mayor capacidad disponible para geometría/origen global y UE5, esfuerzo EXTRA HIGH, no seleccionado desde esta ficha.

**Ejecución local condicionada:** actúa como un SOLO agente Codex dentro de Windows/Unreal; esta ficha publicada NO significa que el chat esté iniciado. Antes de actuar lee `git status`, ramas/worktrees actuales, PR #19 SHA `1d01db54b63250ed0aacb0704a5ac16290324e32`, las cinco fuentes PR #17 (`docs/PLAN_PARALELO_S25_S3_S35_S4_20261115.md`, cuatro `PROMPT_CODEX_*.md`) y `docs/IB_AUDITORIA_REFINAMIENTO_0956_20261115.md` en la rama documental `docs/ib-refine-evidence-20261115-0956` por lectura remota (no merge). No intentes usar el HEAD de `main` como base jugable.

**Repositorio:** https://github.com/meliodr/juego- . Todas las ramas de laboratorio nuevas nacen *exactamente* del commit S2 fijo. Verifica si la rama de este encargo ya existe local/remoto antes de crearla; no resetees ni sobreescribas trabajo. En worktree propio y rama exclusiva, **no alterar** `ImpactBeyondCharacter.cpp/.h`, `ImpactCircuitComponent.cpp/.h`, `ImpactBeyondGameMode.cpp/.h`, `ImpactBeyond.Build.cs`, `ImpactBeyond.uproject`, `Config/**`, `Content/IB_C00/Lvl_IB_C00.umap`, `AGENTS.md`, `.codex/**`, `docs/PLAN_ACTIVO.md` ni paths de otros laboratorios. Cualquier conexión al juego base será **propuesta de parche en tu carpeta**, no aplicada a esos archivos. No correr dos Unreal Editor/cook simultáneos. No main merge, force push, borrado, nuevos plugins, cambios Windows/UE, compras ni reclamaciones Fab.

**Entrega exigida:** PR **draft** contra `prototype/ib-sprint02`, SHA reproducible, lista de cambios por archivo, datos de licencia/archivo original/ubicación, comandos de build/test reales, capturas si se abrió UE, métricas separadas, bloqueos, regresiones y estado `DOCUMENTO|CÓDIGO|COMPILADO|HEADLESS|DX12_VISIBLE|HUMANO`. Ejecutar `Tools/IBAudit_ValidateLabs.py` del PR #21 tras incorporar la herramienta de lectura/QA de forma que no entre en el diff del laboratorio; el test es solo de propiedad y lineage. Nunca declarar PASS de editor, DX12, mando o sonido si no se midió con evidencia. Si no hay herramienta local de Unreal o permisos, dejar preparación útil y registrar qué falta. Presupuesto RD$0.

**Branch:** `prototype/ib-s3-planet-lab`. **Worktree:** `../impact-s3-planet`. **Únicos archivos nuevos/modificados:** `ImpactBeyond/Source/ImpactBeyond/IB_S3/**`, `ImpactBeyond/Content/IB_S3/**`, `Tools/IBS3_*`, `docs/IB_S3_*`.

## Problema comprobado
El PR19 tiene `ImpactRadialLab.cpp` y un contrato de regiones de docs, **NO** un planeta jugable con cuatro ciudades. En la ciudad y costa actuales todo ocurre en un nivel urbano; el regreso en esa sesión **NO** es persistencia en disco ni streaming planetary. No crear cuatro ciudades vacías y llamarlas sprint 3 terminado.

## Trabajo pequeño y medible
1. Leer `docs/IB_SPRINT02_PLANET_LAB.md`: `BodyId`, `RegionId`, posición global doble precisión, marco local tangente con +Z radial, normal gravitatoria, banda de histéresis X=600–640m y precarga por predicción hasta 320–368m/s. Inspeccionar `ImpactRadialLab.cpp` y anotar límites del test.
2. Implementar en `IB_S3` una prueba A/B **de dos regiones** con centros de mundo/gravedad coherentes, transformación de pose y velocidad, cambios de origen sin saltos, retención temporal de colisión y precarga con horizonte 1.5s mínimo. Nada de modificar `Lvl_IB_C00` ni colocar un planeta gigante en el nivel base.
3. Añadir actor de cambio persistente con `ObjectId` estable: mover objeto, entrar segunda región, regresar 10 veces y reiniciar aplicación/guardado una vez. Estado serializado explícito, no solo mismo nivel cargado. Evitar duplicados cuando un objeto migró entre regiones. Descartar escombros efímeros.
4. Probar latitud ecuatorial, cercanía a polos, orientaciones, cuantización a grandes distancias, gravedad y aterrizaje sobre curvatura; generar un mapa/minimapa técnico de dos regiones y telemetría de regiones, no venderlo como mapa de planeta final. Las cuatro hubs/cidades son ETAPA POSTERIOR tras superar estos gates.

## Pruebas y reportes
- 10 idas y vueltas dos regiones manteniendo (actor moved, position, state), 10 cambios de frontera en histéresis sin rebotar, punto de polo y ecuador, 10 velocidades/rotaciones exportadas sin mezcla de marcos. Añadir test de carga a 320/368 m/s y medir hitch/collider antes de declarar streaming aceptado.
- Para persistencia, registrar resultados de estado tras cerrar/reabrir, versión de save y checksum de IDs. Para gameplay, recorrer a pie/vuelo y aterrizar, con DX12 visible, audio y mando cuando disponible. Si no hay editor, entregar tests de matemáticas con señal `CODE/HEADLESS`, jamás ciudades terminadas.
- Guardar en rama y PR draft propios, con SHA, ruta de prueba, métricas y lista de faltantes. No integrar a main ni tocar a otros agentes.
