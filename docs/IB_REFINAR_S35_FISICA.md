# ENCARGO CODEX P0-02 — S3.5: colisión del objeto cargado a velocidad extrema

**Modelo sugerido:** Codex orientado a C++/física con esfuerzo HIGH o EXTRA HIGH si aparece, no configurado desde este documento.

**Ejecución local condicionada:** actúa como un SOLO agente Codex dentro de Windows/Unreal; esta ficha publicada NO significa que el chat esté iniciado. Antes de actuar lee `git status`, ramas/worktrees actuales, PR #19 SHA `1d01db54b63250ed0aacb0704a5ac16290324e32`, las cinco fuentes PR #17 (`docs/PLAN_PARALELO_S25_S3_S35_S4_20261115.md`, cuatro `PROMPT_CODEX_*.md`) y `docs/IB_AUDITORIA_REFINAMIENTO_0956_20261115.md` en la rama documental `docs/ib-refine-evidence-20261115-0956` por lectura remota (no merge). No intentes usar el HEAD de `main` como base jugable.

**Repositorio:** https://github.com/meliodr/juego- . Todas las ramas de laboratorio nuevas nacen *exactamente* del commit S2 fijo. Verifica si la rama de este encargo ya existe local/remoto antes de crearla; no resetees ni sobreescribas trabajo. En worktree propio y rama exclusiva, **no alterar** `ImpactBeyondCharacter.cpp/.h`, `ImpactCircuitComponent.cpp/.h`, `ImpactBeyondGameMode.cpp/.h`, `ImpactBeyond.Build.cs`, `ImpactBeyond.uproject`, `Config/**`, `Content/IB_C00/Lvl_IB_C00.umap`, `AGENTS.md`, `.codex/**`, `docs/PLAN_ACTIVO.md` ni paths de otros laboratorios. Cualquier conexión al juego base será **propuesta de parche en tu carpeta**, no aplicada a esos archivos. No correr dos Unreal Editor/cook simultáneos. No main merge, force push, borrado, nuevos plugins, cambios Windows/UE, compras ni reclamaciones Fab.

**Entrega exigida:** PR **draft** contra `prototype/ib-sprint02`, SHA reproducible, lista de cambios por archivo, datos de licencia/archivo original/ubicación, comandos de build/test reales, capturas si se abrió UE, métricas separadas, bloqueos, regresiones y estado `DOCUMENTO|CÓDIGO|COMPILADO|HEADLESS|DX12_VISIBLE|HUMANO`. Ejecutar `Tools/IBAudit_ValidateLabs.py` del PR #21 tras incorporar la herramienta de lectura/QA de forma que no entre en el diff del laboratorio; el test es solo de propiedad y lineage. Nunca declarar PASS de editor, DX12, mando o sonido si no se midió con evidencia. Si no hay herramienta local de Unreal o permisos, dejar preparación útil y registrar qué falta. Presupuesto RD$0.

**Branch:** `prototype/ib-s35-interactions-lab`. **Worktree:** `../impact-s35-interactions`. **Únicos archivos nuevos/modificados:** `ImpactBeyond/Source/ImpactBeyond/IB_S35/**`, `ImpactBeyond/Content/IB_S35/**`, `Tools/IBS35_*`, `docs/IB_S35_*`.

## Problema concreto
El PR #19 reporta barridos del personaje a 320/368 m/s, pero no demuestra que el volumen del vehículo que transporta por delante del héroe siga sin atravesar paredes o techos. Una prueba a baja velocidad del objeto no es equivalente. El C++ de `ImpactCircuitComponent` existe en baseline, pero no se puede editar en este laboratorio sin invadir a otros agentes.

## Trabajo
1. Inspeccionar sin tocar las reglas existentes de `ImpactCircuitComponent`, el perfil de colisión del coche, IK, rigid bodies y sweep de `CharacterMovement`. Explicitar un **contrato puro** para `Car`, `Container`, `Pole`, `Rock`, `LoosePart`: `ObjectId`, masa, bounds de colisión, punto de agarre, ownership, fase `Idle/Carried/Thrown/Settling`, resultado de sweep, invalidación por despawn y resets.
2. Implementar adaptador/prototipo aislado en `IB_S35`: cada frame/step mover **el volumen de carga** desde `Prev` a `Next` con sweep, no simplemente set-location de malla; registrar hit actor, normal, instante, stop/safe gap y alternativa drop/slide. Evitar tunneling a velocidades extremas y objetos dormidos atascados. El héroe y la carga NO deben atravesar techo/pared ni provocar velocidad infinita. No inventar un nuevo sistema global duplicado.
3. Añadir dos ataques de puño (ligero/fuerte) desacoplados de VFX con ventana de hit, deduplicación target por ataque y cooldown. Producir adaptador que S4 pueda consumir sin reescribir sus archivos; si exige editar `Build.cs`, dejar patch propuesto en tu propia ruta, NO aplicarlo.
4. Crear mapa de test de pasillo/obstáculo aislado, si el editor está libre. Sin nuevos assets externos (primitivas de prueba aquí sí están permitidas).

## Verificación exigida
- Muestra de pared de 4cm, esquina 90°, techo, suelo en aterrizaje, giro brusco de cámara, abandono de carga, cambio de mapa/reset; correr 30 ciclos **por tipo de objeto**, 30 golpes ligeros y 30 fuertes sin doble hit y sin atravesar collider.
- Probar 12, 90, 180, 320 y 368m/s. A cada régimen, registrar `target speed`, `measured speed`, dt de física, sweep (True/False), fallos, clipping, hitches y exit reason. Si la física discreta no cubre 368m/s, reportar `FAIL` y proponer CCD/substepping/sweep predictivo sin afirmar resuelto.
- Contrastar una secuencia sin cargar objeto vs otra con objeto en misma ruta; marcar si el problema es `H01 capsule` o `held payload`. Distinguir unit tests puros, headless Unreal y demo humana.
- Publicar PR draft contra S2 con SHA, pruebas reproducibles, rutas propias y propuesta textual de integrador. No fusionar.
