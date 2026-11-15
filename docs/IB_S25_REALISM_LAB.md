# S2.5 — laboratorio de recuperación visual

Estado del 2026-11-15: **CÓDIGO y pruebas ligeras PASS; Unreal, mapa, capturas, DX12 y aceptación artística PENDIENTES.** No es una entrega fotorealista terminada ni integración global/Sprint 7.5. RD$0, sin compras, plugins nuevos, cook, cambios de seguridad o merge.

Base exacta: `1d01db54b63250ed0aacb0704a5ac16290324e32`. Rama: `prototype/ib-s25-realism-lab`. Worktree: `task/impact-s25-realism`, con repositorio auxiliar `task/impact-s25-repo`; se usó la carpeta autorizada porque las raíces originales no permitían operaciones Git en el sandbox. La auditoría posterior de solo lectura confirmó el checkout original en la misma base y una carpeta no versionada `ImpactBeyond 5.8/`. No se sobrescribió. La configuración documental de [coordinación](https://github.com/meliodr/juego-/blob/docs/roadmap-sprints-02-08-20261115/docs/PLAN_PARALELO_S25_S3_S35_S4_20261115.md) se leyó sin mezclar esa rama.

## Lo entregado y lo que falta

| Archivo | Función / evidencia |
|---|---|
| `Tools/IBS25_recipe.py` | Receta determinista y OBJ arquitectónicos en cm/Z-up; UV con escala física. Ejecutado: 5 archivos, 49.614 bytes, 1.288 módulos de fachada. |
| `Tools/IBS25_generated/` | Cuatro OBJ propios (asfalto, aceras biseladas, marcas y azoteas) y `scene.json`. No son capturas ni un `.umap`. |
| `Tools/IBS25_pbr.py` | Obtención acotada e importación opcional de seis mapas CC0 2K. Descarga real: 44.243.757 bytes; SHA-256 y dimensiones comprobados. Importación UE pendiente. |
| `Tools/IBS25_sources/manifest.json` | URLs originales, licencia, formato, dimensiones, bytes y hashes. Los seis archivos de imagen quedan locales, excluidos de Git. |
| `Tools/IBS25_author.py` | Adaptador Unreal; preflight por defecto. Autoría solo con dependencias completas y `IBS25_EXECUTE=1`. Reutiliza `ImpactFacade`; no altera código compartido. NO ejecutado en Unreal. |
| `Tools/IBS25_capture.py` | Cinco capturas secuenciales, calentamiento por cámara, verificación de PNG y metadatos. NO ejecutado. |
| `Tools/IBS25_benchmark.ps1` | Comando revisable por defecto; lanzamiento opt-in, turno coordinado, guard de procesos, mutex cooperativo y muestreo de memoria de GPU. NO se lanzó Unreal. |
| `Tools/IBS25_metrics.py` | FrameTime/FPS/p95/p99 de CSV reales, calentamiento explícito y rechazo de datos inválidos. Probado con datos sintéticos, no rendimiento del juego. |
| `Tools/IBS25_test.py` | 11 pruebas ligeras PASS; no sustituyen validación de APIs Unreal/render/colisiones. |

La receta dispone seis edificios de 5–8 pisos en tres manzanas, cuarta manzana de plaza, parque de transición, seis ConceptCars, dieciséis posiciones para árboles reales y ruta aérea central. Fachadas con entradas y cornisas existentes; 72 paneles como máximo usan el interior mapping existente. Árboles con escala uniforme; coches ajustados uniformemente a longitud 4,5 m. No se distorsionan los coches por eje. Ninguna malla de Kit Cidade aparece en primer plano. El único Cube del adaptador es colisión invisible; no se generan árboles de cilindro/cono ni colinas esféricas. No hay costa/agua en esta variante: se eligió **transición a parque**, permitida por el encargo.

La vegetación continúa bloqueada: en el Content auditado no apareció una malla real de árbol. `tree: null` es una dependencia pendiente deliberada. El generador no guarda un mapa incompleto con árboles falsos. Se ha producido código y geometría útil, **no se alcanzó el objetivo perceptual City Sample**. Requiere revisión de escala, orientaciones glTF/OBJ, juntas, colisiones, materiales y composición en el motor.

## Reutilización, licencia y bytes

| Necesidad | Principal | Respaldo / estado |
|---|---|---|
| Fachada | Poly Haven Modular Urban Apartments Facade, cuatro módulos ya importados | City Sample Buildings solo si bytes y derechos locales están confirmados; no descargar/reclamar automáticamente. |
| Coches | `/Game/Car/SM_AutomotiveTP_Car`, ConceptCar ya usado por el proyecto | City Sample Vehicles, pendiente de bytes/licencia; Kit Cidade solo QA/fondo, excluido de este primer plano. |
| Vegetación | Una malla real local con procedencia/licencia registrada; falta asignación | Un árbol CC0 oficial seleccionado y probado en sesión posterior; NO sustituir por primitivas. |
| Calle/acera | [Asphalt 02](https://polyhaven.com/a/asphalt_02) (3 m) + [Concrete Pavement](https://polyhaven.com/a/concrete_pavement) (1,8 m), CC0, seis mapas 2K | Sin downgrade silencioso a material plano; la autoría falla si faltan texturas. |
| Ventanas | `/Game/IB_C00/Interiors/M_MVP_WindowInteriors` | Mantener vidrio original en los restantes módulos; no cambiar el material donante. |

Inventario local medido: `UrbanFacades`: 24 `.uasset`, **24.829.802 bytes**; `Car`: 8 `.uasset`, **11.945.721 bytes**; `Interiors`: 1 `.uasset`, **15.580 bytes**. Son dependencias del proyecto, no archivos nuevos republicados por este PR. Según [la licencia oficial de Poly Haven](https://polyhaven.com/license), sus assets son CC0 y permiten redistribución; las imágenes nuevas siguen locales por economía de repositorio. El manifiesto permite verificarlas. Los OBJ/receta son autoría mínima propia porque no se halló geometría de calle detallada reutilizable en el subconjunto local; no pretenden reemplazar edificios o vegetación por primitivas.

ConceptCar: la procedencia local de plantilla se documentó en `docs/IB_MVP_URBANO.md`; este PR referencia la ruta y no añade sus bytes. City Sample completo/Buildings/Vehicles, Electric Dreams y Automotive Materials no se importaron ni se publicaron. Ningún contenido NoAI se cargó como entrada al modelo. No se afirma posesión por figurar Free en Fab. Un clone de Git **no basta** para abrir el laboratorio: faltan el `.umap` generado, la malla vegetal autorizada, imports y módulos compatibles.

UE instalado verificado leyendo `Build.version`: **5.8.4, CL 58902052**. Los manifiestos de Launcher consultados muestran motor/Bridge/Fab, pero no City Sample. Las raíces habituales de proyecto consultadas no confirmaron `Small_City_LVL`; esto es un inventario acotado, no prueba de ausencia en todo el disco. No se inició su descarga. Terra aparece en la caché de modelos de Codex; ni esa caché ni el prompt prueban modelo/esfuerzo activos. No se cambió configuración de modelo.

## PCG seleccionado con alcance explícito

Se consultó la [documentación oficial City Sample PCG](https://dev.epicgames.com/documentation/unreal-engine/city-sample-pcg-for-unreal-engine?lang=en-US). La URL sin versión consultada se presenta como documentación 5.9; describe la demo MCP/PCG de UE5.8. No se afirma que sus grafos estén instalados ni compatibles por haber leído la página.

Aplicación conceptual al laboratorio: separar huellas de manzana, calzada, parcela, fachada y parque; dejar exclusiones para plaza/vuelo. Los candidatos de reutilización selectiva son Roads → Lots → Buildings y CentralPark, con shape grammar de carril/bordillo/acera y módulos de fachada. No se copiaron los 18 grafos ni se activó PCG. La demo requiere partición explícita y streaming activado; un nivel World Partition por sí solo no lo resuelve. Si se migra posteriormente, particionar elementos con celdas iniciales de 128 m, edificios por actor, verificar colisión y frontera de carga. Este laboratorio compacto es **no-WP**, con HISM separados por edificio; no se declara streaming implementado o rendimiento mejorado por HISM sin A/B.

## Reproducción sin editor

Desde el worktree:

```powershell
python -B Tools/IBS25_recipe.py --output Tools/IBS25_generated
python -B Tools/IBS25_pbr.py
python -B Tools/IBS25_test.py
```

La receta es idempotente por contenido; aborta si encuentra un archivo generado diferente. El downloader conserva archivos existentes, exige PNG/JPEG 2048×2048, limita cada archivo a 32 MiB y comprueba la reserva acordada de 40 GiB antes de descargar. La primera pasada transfirió 42,19 MiB. Las descargas posteriores comprueban la recepción local. No instala paquetes Python. Las pruebas usan temporales dentro de `Tools/IBS25_test_work`; el primer ensayo en el Temp virtual del sandbox falló por permisos y se corrigió sin cambiar ACL.

## Autoría en un turno seguro de Unreal — PENDIENTE

1. Coordinar un turno exclusivo. Un listado sin procesos no reserva la laptop. No se obtuvo dicho turno durante esta pasada, por lo que no se abrió editor ni cook. El launcher de benchmark exige confirmación del turno y no mata procesos ajenos.
2. Materializar en este worktree los assets/base necesarios y módulos compatibles con UE5.8.4. Es un checkout sparse de código/documentación; no copiar carpetas completas de proyectos/cache ni tocar el checkout original. Cualquier build necesario se coordina aparte. Python/editor scripting debe estar disponible en la sesión autorizada; no se instala/activa automáticamente ningún plugin.
3. Ejecutar `IBS25_pbr.py` en el editor con `IBS25_IMPORT_PBR=1`. Importa únicamente a `/Game/IB_S25/Textures`, escribe `Tools/IBS25_sources/assets.json`, configura sRGB solo en color, normal DX y límite 2K. No reimporta ni sobreescribe texturas ajenas.
4. En una copia local propia de `assets.json`, añadir `"tree": "/Game/.../MallaVegetalAutorizada"` con la licencia/inventario registrado. El archivo de ejemplo no inventa rutas Fab. Usar un árbol real con proporciones adecuadas; probar colisión de tronco si se convierte en objeto jugable (en esta autoría la vegetación decorativa no colisiona).
5. Con editor limpio (sin mapas sin guardar), preparar variables y ejecutar el adaptador en su consola Python. `exec(compile(open(script, encoding='utf-8').read(), script, 'exec'), {'__name__':'__main__', '__file__':script})` conserva `__file__` para todos los scripts.

```python
import os
script = r"<worktree>\Tools\IBS25_author.py"
os.environ['IBS25_ASSETS'] = r"<worktree>\Tools\IBS25_sources\assets.json"
os.environ['IBS25_GENERATED'] = r"<worktree>\Tools\IBS25_generated"
# Primera ejecución sin IBS25_EXECUTE: inventario/preflight, no mutaciones.
exec(compile(open(script, encoding='utf-8').read(), script, 'exec'), {'__name__':'__main__', '__file__':script})
# Tras resolver faltantes, la operación ya autorizada se habilita explícitamente:
os.environ['IBS25_EXECUTE'] = '1'
exec(compile(open(script, encoding='utf-8').read(), script, 'exec'), {'__name__':'__main__', '__file__':script})
```

Destino único `/Game/IB_S25/Lvl_IB_S25_RealismLab`. Marcador de autoría y hash de receta/config impiden reutilizar un mapa diferente. Los assets nuevos se crean solo bajo `/Game/IB_S25`. El adaptador no carga/guarda `IB_C00`, no muta donantes, no ejecuta `save_all`. Una importación fallida puede dejar assets parciales propios: conservarlos y diagnosticar; no eliminar trabajo para ocultar fallos. Repetir dos veces, comparar número de actores/instancias y hashes antes de marcar idempotencia **Unreal** PASS.

## Cinco vistas comparables

| Vista | Posición XYZ, m | Pitch/Yaw/Roll, grados | FOV horizontal | Evidencia |
|---|---|---|---|---|
| Avenida a pie | -92, -2, 1,7 | 2 / 0 / 0 | 65 | PENDIENTE |
| Esquina/coches | -13, -13, 1,7 | 3 / -135 / 0 | 60 | PENDIENTE |
| Fachada a 10 m | -68, -13, 6,5 | 0 / -90 / 0 | 55 | PENDIENTE |
| Azotea/centro | -68, -39, 34 | -20 / 42 / 0 | 70 | PENDIENTE |
| Parque | 157, -52, 1,7 | 5 / 135 / 0 | 65 | PENDIENTE |

Sol previsto: pitch -32°, yaw -35°, 65.000 lux. EV100 fijo 14 requiere el rango de luminancia extendido existente; el adaptador aborta si falta, no edita Config. Resolución 1920×1080. Usar el mismo sol/FOV/altura/distancia relativa en la referencia; una imagen promocional sin metadatos es **referencia cualitativa**, no comparación idéntica. No se inventan las posiciones de Small City.

Con el lab visible, definir un `IBS25_RUN_ID` nuevo y ejecutar `IBS25_capture.py` mediante el mismo patrón `exec/compile`. Espera diez segundos por cámara sin bloquear el game thread y verifica cada PNG antes de registrar captura. No sobrescribe una sesión previa. **No hay screenshots entregadas todavía.** Para cada una rellenar siluetas, variedad PBR, iluminación, coche y vegetación como `PASS / CORREGIR / N/A justificado`, con enlace a referencia y captura. Nada se marca fotorealista a partir de que el script termine.

## A/B de coste DX12 — PENDIENTE

`IBS25_benchmark.ps1` prepara dos ejecuciones con Lumen, TSR, shadow maps convencionales y audio habilitado: **A = escala 100%; B = 83%**, mismas posiciones/FOV, sombras y pool de texturas. Son presets de ensayo, no garantía de 60 FPS. No se cambian `.ini`. Usar alimentación/perfil térmico estables y registrar driver/GPU/versión. El wrapper no inicia compilación cuando falta el módulo.

```powershell
# Revisar el comando primero; sin -Launch no abre nada.
./Tools/IBS25_benchmark.ps1 -Editor '<UE5.8>/Engine/Binaries/Win64/UnrealEditor.exe' -Project '<worktree>/ImpactBeyond/ImpactBeyond.uproject' -Variant A
# Con turno coordinado y mapa/módulo ya verificados, agregar -Launch -EditorTurnConfirmed.
```

En cada ejecución, confirmar DX12 en log, ventana visible, audio device activo y sonido audible del juego (audio habilitado no prueba audio audible). Usar `viewactor <nombre_runtime>` de cada CameraActor registrado por el generador; verificar que adopte su FOV. Si no lo hace, detener la comparación y corregir la ruta, no medir cámaras distintas. Esperar 30 s iniciales y 10 s por cámara, ejecutar `csvprofile start`, permanecer 20 s en esa cámara, `csvprofile stop`. Mantener screenshots fuera de las ventanas de medición. Repetir las cinco cámaras, primero vuelta fría y después caliente, y cerrar A antes de iniciar B. Registrar nombre/variante/vista de cada CSV. Esta prueba de editor `-game` conserva sobrecostes del editor y no certifica un paquete final.

```powershell
python -B Tools/IBS25_metrics.py '<captura-A.csv>' '<captura-B.csv>'
```

El launcher muestrea `nvidia-smi` cada ~0,5 s si existe. `memory.used` es memoria de **toda la GPU**, incluye escritorio/otros procesos y puede perder picos; no se etiqueta VRAM exacta del proceso. Adjuntar además `stat streaming`, `stat RHI`, mips pendientes y estado de sombras; si falta telemetría, marcar PENDIENTE. El analizador no infiere audio, render visible, DX12 ni VRAM desde FrameTime. No aplicar cifras heredadas del Sprint 2 a este lab.

## Validación y entrega al integrador

Ver `IB_S25_EVIDENCE.json` para evidencia de esta pasada. Revisión e implementación realizadas por el mismo agente; no se simula QA independiente. Gates:

| Gate | Resultado |
|---|---|
| Receta/OBJ y rechazo de sobrescritura | PASS, Python local |
| Fuente CC0, hashes y tamaño 2K | PASS, seis archivos reales |
| Sintaxis Python y PowerShell | PASS, sin ejecutar APIs Unreal |
| Autoría/idempotencia UE, colisiones/transitabilidad | PENDIENTE |
| `.umap`, 5 PNG comparadas, aceptación artística | PENDIENTE |
| DX12/audio visible A/B, p95/p99/VRAM/streaming | PENDIENTE |
| Gamepad, jugabilidad, empaquetado, integración | NO EJECUTADO / fuera de esta primera pasada |

Rollback: mantener la rama del laboratorio aislada; no aplicar sus scripts al mapa principal. El integrador decide qué adaptar. No se propone merge automático ni se altera el snapshot común.
