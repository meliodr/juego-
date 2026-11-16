# Ejemplos oficiales Epic: selección por utilidad
Corte: 15 de noviembre de 2026.

## Decisión de Jarry
Usar **Fab de Epic como fuente de recursos gratuitos para IMPACT: BEYOND**. Presupuesto **RD$0**. Esta decisión no selecciona ni autoriza instalar todos los paquetes: se revisarán licencia, condiciones NoAI, tamaño, dependencias y utilidad de cada recurso antes de incorporarlo. Los candidatos de otras fuentes documentados aquí quedan como referencias o alternativas, no como una lista de descargas.

## Orden propuesto de evaluación
1. **Game Animation Sample (GASP)**: primera referencia para locomoción y ragdoll. [Epic confirma actualización a UE 5.8](https://www.unrealengine.com/tech-blog/download-the-latest-game-animation-sample-project-now-updated-for-ue-5-8). Es el único candidato de esta lista cuya compatibilidad 5.8 se ha verificado documentalmente. No acredita integración realizada en IMPACT ni incluye automáticamente vuelo superheroico.
2. **Content Examples**: consultar demostraciones concretas, en particular Chaos. [Documentación oficial](https://dev.epicgames.com/documentation/en-us/unreal-engine/content-examples-sample-project-for-unreal-engine). Extraer aprendizaje y evaluar módulos, no importar el proyecto entero por defecto.
3. **City Sample Buildings** y **City Sample Vehicles**: ciudad y vehículos por separado cuando hagan falta en la prueba. [Edificios en Fab](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5), [vehículos en Fab](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f). Un modelo de edificio no garantiza interiores o destrucción; un coche requiere comprobar física, colisiones y uso de agarre/lanzamiento.
4. **Stack O Bot**: referencia pequeña de estructura y mecánicas para estudiar, sin cambiar el protagonista aprobado. [Fab](https://www.fab.com/listings/b4dfff49-0e7d-4c4b-a6c5-8a0315831c9c).
5. **Lyra**: evaluar más adelante si sus sistemas resuelven una necesidad concreta; no imponer su complejidad al prototipo offline. [Documentación oficial](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine).
6. **City Sample completo**: pospuesto; preferir módulos y piezas concretas antes que la ciudad completa. [Documentación oficial](https://dev.epicgames.com/documentation/en-us/unreal-engine/city-sample-project-unreal-engine-demonstration).

No usar la antigua demostración Chaos de UE4 que requiere compilación de fuentes como base del proyecto actual.

## Licencia y NoAI
Revisar el [Epic Content License Agreement](https://www.unrealengine.com/eula/content) y la ficha/licencia vigente de la edición adquirida. En esta revisión, GASP, Content Examples, Stack O Bot, Lyra y edificios se señalan **NoAI**: no cargar sus assets en IA generativa. Registrar las condiciones de vehículos por separado; no extrapolar permisos entre paquetes. Programar código propio con asistencia IA no convierte automáticamente todos los archivos de terceros en material permitido para esas herramientas.

No copiar assets a GitHub como documentación ni redistribuir paquetes fuente. Conservar avisos y comprobar límites UE-only cuando correspondan.

## Validación pendiente
- Tamaños de descarga e instalación **no verificados**.
- Compatibilidad UE 5.8 **no verificada** para candidatos distintos de GASP.
- No se ha descargado, instalado, importado, compilado ni ejecutado ninguno en esta investigación.
- Tras los prerrequisitos W00/W01, ensayar primero locomoción, despegue, vuelo y aterrizaje; después interacción física. Un efecto Niagara no sustituye una abertura física o una colisión correcta.
- Cada incorporación requiere edición, licencia, espacio disponible, dependencias y resultado de prueba. La propuesta no modifica la cola activa ni prueba que el juego ya funcione.
