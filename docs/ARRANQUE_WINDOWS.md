# IMPACT: BEYOND — Arranque en Windows

Actualizado: 2026-11-15. Plataforma principal y primer ejecutable: **Windows/Win64, un jugador offline**. Linux queda como posible destino posterior, sin bloquear ninguna entrega. Esta decisión sustituye las condiciones de plataforma Linux de PLAN_ACTIVO, TAREAS_ACTIVAS y skills anteriores; no cambia el alcance jugable ni reactiva cooperativo.

## Evidencia y acceso

El usuario mostró Epic Games Launcher con Unreal Engine **5.8 instalándose**, y Quixel Bridge/Fab UE Plugin en cola. Esa captura no demuestra instalación terminada, apertura del editor, existencia de MSVC/SDK ni compilación exitosa. No fijar parche de UE por inferencia. Hardware declarado: 64 GB RAM, RTX 4070 de 8 GB VRAM; verificar CPU, modelo exacto, drivers y SSD en el entorno real.

El intento de habilitar ejecución local no se aceptó. No hay acceso a la computadora confirmado desde este chat. La preparación en GitHub sí puede continuar; no afirmar que se abrió Unreal o que Robert/Codex ya recibió y ejecutó la tarea.

## IB-W00 — Reemplaza IB-L00 en la cola activa

**Resultado:** crear, compilar y ejecutar un proyecto mínimo Third Person C++ en Windows, y después empaquetarlo como Win64. Ninguna mecánica propia se considera implementada por completar el arranque.

1. Confirmar que Unreal 5.8 aparece listo para iniciar y que el editor abre. Registrar versión/parche real; no cambiar a Preview.
2. Revisar Visual Studio Installer y herramientas existentes antes de instalar otra versión. Para UE5.8, Epic documenta VS2022 17.14 o superior, y VS2026 18.0 o superior. Si no existe instalación compatible, preferir Community estable compatible; no instalar ambas por costumbre.
3. Revisar las cargas **Desarrollo de juegos con C++** y **Desarrollo para el escritorio con C++**, MSVC x64/x86, Windows SDK compatible y Visual Studio Tools for Unreal Engine. Epic lista SDK mínimo 10.0.22621.0 y recomienda 10.0.26100 o posterior para 5.8. Contrastar los componentes elegidos con la instalación real y su UnrealBuildTool; no dar toolchain por listo porque Visual Studio abra.
4. Confirmar checkout de meliodr/juego-, rama y cambios locales. No sobrescribir otro proyecto. Ubicación propuesta: `<checkout>/Game/ImpactBeyond/ImpactBeyond.uproject`.
5. En el selector del editor: Games/Juegos → Third Person → C++ → Desktop. Nombre `ImpactBeyond`. Para esta prueba usar variante básica, calidad Scalable si se ofrece, sin ray tracing y sin contenido adicional que no haga falta. Son decisiones de alcance, no requisitos de Epic. Si una opción no existe en esa versión, documentar lo mostrado sin inventar botones.
6. Crear y compilar. Abrir el mapa de prueba y Play: verificar desplazamiento, salto, cámara y colisiones. La caminata lenta, sprint y vuelo propios llegan en GND00/F01 después de validar la base; no afirmar que ya están hechos por usar la plantilla.
7. Empaquetar Development/Win64 y abrir fuera del editor. Registrar ruta local solo en evidencia privada cuando contenga datos personales, logs relevantes, versiones y resultado. No subir Binaries/Intermediate/Saved/DerivedDataCache ni archivos del motor al repo.

**Quixel Bridge y Fab:** no son condiciones de aceptación de esta primera prueba. No descargar City Sample, planetas, plugins de terceros ni bibliotecas completas todavía. No cancelar instalaciones ni cambiar drivers por cuenta propia.

**Siguiente orden:** W00 → FLT-AUD/GND00 → F01 movimiento suelo-aire → C00/P01/D01 → E01/S01. Pruebas visuales y rendimiento se realizan en Windows/Win64; DirectX se verifica en el equipo, sin exigir Vulkan. Meta 1080p/60 FPS ajustable, no resultado medido. Cooperativo continúa muy lejano.

## Cierre honesto

Estados: preparación del repositorio, instalación observada en captura, instalación confirmada, editor abierto, compilado, ejecutado y empaquetado son evidencias distintas. En esta actualización solo se completó la preparación documental. Si falta acceso, compilador o SDK, indicar el bloqueo exacto; no convertirlo en una tarea completada.

## Fuentes oficiales consultadas el 2026-11-15

- Epic, compatibilidad Visual Studio/UE y componentes: https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine
- Epic, notas UE5.8, Platform SDK Upgrades: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes?lang=en-US
- Microsoft, instalación Visual Studio Tools para Unreal: https://learn.microsoft.com/es-es/visualstudio/gamedev/unreal/get-started/vs-tools-unreal-install
- Epic, selector y creación de proyecto: https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-new-project-in-unreal-engine

Las páginas genéricas pueden mostrar el motor más reciente en su encabezado; usar la fila y documentación correspondientes a 5.8 y confirmar el toolchain en el build real.
