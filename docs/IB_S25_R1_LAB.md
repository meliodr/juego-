# S2.5 — laboratorio de recuperación visual

**Mapa y cinco capturas reales ejecutados; realismo NO aceptado.** [Ver imágenes y revisión](IB_S25_VISUAL_REVIEW.md). No es una integración jugable, benchmark ni entrega final. RD$0.

Rama `prototype/ib-s25-realism-lab`, [PR draft #25](https://github.com/meliodr/juego-/pull/25), base congelada `1d01db54b63250ed0aacb0704a5ac16290324e32` de `prototype/ib-sprint02`. Checkout aislado `task/impact-s25-realism`. No se modifica `IB_C00`, código compartido, Config ni `.uproject` en el diff final. La configuración reescrita automáticamente por el Editor se respaldó y restauró a HEAD.

## Entrega comprobada

- `/Game/IB_S25/Lvl_IB_S25_RealismLab`: seis edificios en tres manzanas, plaza de aterrizaje, transición de parque y corredor aéreo. 79 actores propios, 1.288 instancias de fachada, seis coches y dieciséis árboles reales. La plaza/parque sigue demasiado vacía.
- Cinco PNG 1920×1080 sin edición de imagen, más una captura anterior de la misma sesión. El antes NO es el mapa original del Sprint 2. Transformaciones/FOV, sol previsto, exposición y ajustes de captura están en `IB_S25_IMAGES/capture.json`; hashes en `manifest.json`.
- Autoría + lookdev repetidos en Unreal: mismo estado numérico de actores, FOV, LOD, referencias de materiales/mallas e instancias. [Resultado](IB_S25_VALIDATION.json). No certifica colisiones ni jugabilidad.
- 11 pruebas ligeras PASS. UE 5.8.4 CL58902052 arrancó en DX12/RTX 4070 Laptop, inicializó audio y ejecutó Python. El acceso visual se concedió y se usó. Cierre propio con código 0; procesos de Unreal ausentes y mutex libre al terminar.

## Correcciones observadas

1. El importador OBJ refleja Y incluso con `convert_scene=False`. La escala Y=-1 en los componentes propios alinea las azoteas con sus edificios; se verificó visualmente.
2. ConceptCar tenía cinco slots sin material, una instancia sin padre y dos referencias de textura rotas. Se asignaron materiales existentes a los componentes y se reparó una copia local del material de pintura. Los donantes originales no se guardaron ni cambiaron.
3. `r.EyeAdaptation.CachedLightingPreExposure=8` cubre EV14 según el código del motor instalado. Es un ajuste de sesión, sin editar Config.
4. La cámara de esquina original no mostraba coches. El lookdev cambia su pose a (-18,1,1.7)m, pitch/yaw/roll (3,-138,0), FOV65. Los metadatos conservan tanto la pose de receta como la observada.
5. Se generaron tres LOD del árbol, pero su reducción automática eliminaba follaje a distancia. La revisión visual fuerza LOD0; su coste aún requiere benchmark. No se afirma que esta elección cumpla 60 FPS.
6. `ExecutePythonScript` cerraba el Editor al devolver el script. La API nativa `EditorPythonScripting.set_keep_python_script_alive` mantuvo esta sesión hasta el cierre explícito.

## Dependencias, licencias y reproducibilidad

Solo el mapa propio, scripts, geometría OBJ propia, documentación y renders se publican. Los `.uasset` generados/importados quedan excluidos por `Content/IB_S25/.gitignore`; los logs y respaldos también son locales. Un clone de Git **no basta** para visualizar el mapa sin reconstruir estas dependencias.

| Necesidad | Principal | Respaldo/limitación |
|---|---|---|
| Fachada | Cuatro módulos Poly Haven Urban Apartments ya importados: BayWide, BayDouble, Entry, Cornice | City Sample Buildings requiere bytes/derechos locales; no descargado. La fachada actual no satisface todavía la variedad moderna solicitada. |
| Coches | ConceptCar existente y materiales de `/Game/Car/Assets`, reparados solo en copia local | City Sample Vehicles no importado; Kit Cidade excluido del primer plano. Copia de material de plantilla no publicada como fuente. |
| Árbol | Island Tree 01, Poly Haven CC0; glTF/binario/nueve JPG 1K; 11 archivos, 66.337.268 bytes | Fuente de 1.599.403 triángulos; reducción/LOD visualmente problemática. Shrub 03 solo candidato sin importar. |
| Asfalto/acera | Asphalt 02 y Concrete Pavement, Poly Haven CC0, seis mapas 2K, 44.243.757 bytes | Sin nuevas compras ni paquetes masivos. |
| Ventanas | `M_MVP_WindowInteriors` existente y vidrio donante | La lectura de interiores sigue oscura y repetitiva; requiere revisión artística. |

Procedencia: [Poly Haven CC0](https://polyhaven.com/license), [árbol](https://polyhaven.com/a/island_tree_01), [asfalto](https://polyhaven.com/a/asphalt_02), [acera](https://polyhaven.com/a/concrete_pavement). Texturas: `Tools/IBS25_sources/manifest.json`. Árbol: [recibo público de fuentes y hashes](IB_S25_TREE_SOURCES.json); bytes locales en `Tools/IBS25_sources/tree`. Dependencias previas: UrbanFacades 24 assets/24.829.802 bytes; Car 8 assets/11.945.721 bytes; Interiors 1 asset/15.580 bytes. No se redistribuyen nuevos fuentes Fab/UE-Only ni se usa NoAI como entrada.

## Repetir el laboratorio

1. Coordinar turno exclusivo, comprobar procesos y SSD. El último chequeo previo al arranque mostró 148.377.722.880 bytes libres. No lanzar mientras otra sesión use UE/cook/build.
2. Reutilizar el módulo compatible y assets de la base en este checkout. PythonScriptPlugin/EditorScriptingUtilities existentes se habilitaron solo mediante argumentos del proceso; no hubo instalaciones.
3. `python -B Tools/IBS25_recipe.py --output Tools/IBS25_generated`; `python -B Tools/IBS25_pbr.py`; `python -B Tools/IBS25_test.py`. El downloader PBR verifica hashes/dimensiones y reserva de 40 GiB. Para el árbol, obtener únicamente los archivos oficiales del recibo en la estructura relativa indicada; no está incluido en el downloader PBR.
4. En una sesión autorizada, ejecutar `IBS25_stage.py`: importa texturas/árbol y ejecuta autoría. Después ejecutar `IBS25_lookdev.py` para las correcciones explícitas de componentes y cámara. Nunca aplicar al mapa principal.
5. `IBS25_validate.py` repite autoría + lookdev y compara estado numérico. El primer validador usaba representaciones de structs con direcciones transitorias y dio un falso cambio; se corrigió a componentes numéricos redondeados a 1e-6. El resultado posterior está registrado, sin ocultar el ensayo inicial en logs locales.
6. `IBS25_review.py` captura cinco vistas en una carpeta nueva; acepta `IBS25_NEXT_RUN_ID` o genera uno UTC. `IBS25_capture.py` conserva diez segundos de calentamiento por vista y valida PNG/metadatos. `IBS25_finish.py` solo libera la sesión Python cuando está en este lab y no hay mapas ajenos sucios.

Scripts en la consola Python: `exec(compile(open(script, encoding='utf-8').read(), script, 'exec'), {'__name__':'__main__', '__file__':script})`. La cola `IBS25_live.py` solo ejecuta basenames `IBS25_*.py` de Tools, sin servidor/puerto. Pausar solicitudes antiguas antes de iniciar una nueva sesión.

## Cámaras finales observadas

| Vista | XYZ metros | Pitch/Yaw/Roll grados | FOV |
|---|---|---|---|
| 01_avenue | -92.00, -2.00, 1.70 | 2.00, 0.00, -0.00 | 65.0 |
| 02_corner_cars | -18.00, 1.00, 1.70 | 3.00, -138.00, -0.00 | 65.0 |
| 03_facade_10m | -68.00, -13.00, 6.50 | 0.00, -90.00, 0.00 | 55.0 |
| 04_rooftop | -68.00, -39.00, 34.00 | -20.00, 42.00, 0.00 | 70.0 |
| 05_park | 157.00, -52.00, 1.70 | 5.00, 135.00, 0.00 | 65.0 |

Sol configurado: pitch -32, yaw -35, 65.000 lux; EV10014. Capturas con TSR, screen percentage100, shadow maps convencionales y pool2500MiB. Esto no es una medición de FPS/VRAM.

## Referencia City Sample / PCG

Se consultó [City Sample PCG](https://dev.epicgames.com/documentation/unreal-engine/city-sample-pcg-for-unreal-engine?lang=en-US): se aprovecharon conceptualmente calle/parcela/fachada/parque, sin importar sus grafos ni declarar streaming implementado. Este nivel compacto es no-WP; HISM separados por edificio. Una futura migración PCG requiere partición y streaming explícitos. No se descargó City Sample ni se confirmó Small_City_LVL instalado en la auditoría acotada.

La [documentación oficial de City Sample](https://dev.epicgames.com/documentation/en-us/unreal-engine/city-sample-project-unreal-engine-demonstration) fue accesible; la captura enlazada devolvió un desafío de acceso, que no se eludió. La comparación visual directa contra esa referencia queda pendiente. No se cuantifica ni se declara alcanzado 1/3–1/2 de su calidad.

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


## Estado de aceptación

Mapa, imports y capturas: EJECUTADOS. Repetición autoría + lookdev: PASS. Calidad artística: CORREGIR, con revisión por vista enlazada arriba. Colisiones/transitabilidad, A/B DX12 a 1080p con p95/p99/VRAM, integración global y aceptación humana: PENDIENTES. No hubo cook, paquete, merge ni prueba de mando en esta sesión.
