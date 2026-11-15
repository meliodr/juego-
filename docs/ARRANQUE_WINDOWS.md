# IMPACT: BEYOND — Comprobación rápida de Windows

2026-11-15. Complemento de [WINDOWS_INICIO_V12.md](WINDOWS_INICIO_V12.md), que conserva los tickets vigentes: **W00 = editor y herramientas; W01 = proyecto C++; W02–W04 = movimiento y vuelo**. Este checklist no crea otra secuencia ni reemplaza ese plan. Windows/Win64 primero; Linux y cooperativo aplazados.

## Evidencia actual

La captura proporcionada muestra Unreal Engine 5.8 instalándose y Fab/Quixel Bridge en cola. No acredita instalación terminada, compilador disponible, editor abierto ni build. Hardware declarado: 64 GB RAM y RTX 4070 8 GB VRAM. El acceso local no quedó habilitado; esta preparación en GitHub no es ejecución en la PC y no equivale a tarea enviada a Robert.

## W00 — Lo primero que comprobar

- Epic Launcher: UE5.8 listo para iniciar y editor capaz de abrir. Registrar parche real; no inferirlo ni cambiar a Preview.
- Visual Studio Installer: versión y cargas ya instaladas. UE5.8 admite **VS2022 17.14+** y **VS2026 18.0+**, según Epic. No instalar dos versiones por costumbre.
- Cargas/componentes: **Desarrollo de juegos con C++**, herramientas de escritorio/MSVC x64-x86 compatibles y Windows SDK. Visual Studio Tools for Unreal Engine es la integración aconsejada. Epic lista para UE5.8 SDK mínimo 10.0.22621.0 y predeterminado 10.0.26100.0. Verificar el compilador con UnrealBuildTool, no solo la presencia de la aplicación.
- Registrar CPU, GPU exacta, Windows/driver, SSD/espacio y ruta del motor mediante revisión local autorizada. No repetir preguntas de RAM/GPU ya contestadas ni modificar drivers automáticamente.

## W01 — Proyecto mínimo

Confirmar checkout y cambios locales antes de crear archivos. Ubicación propuesta: `<checkout>/Game/ImpactBeyond/ImpactBeyond.uproject`.

Configuración propuesta del selector: **Games/Juegos → Third Person → C++ → Desktop**, nombre **ImpactBeyond**, variante básica para comprobar arranque, calidad Scalable si se ofrece, sin ray tracing inicialmente. No inventar opciones ausentes en la versión instalada. Son elecciones de nuestro prototipo, no requisitos universales del motor.

Con toolchain listo: crear y compilar; Play para comprobar movimiento base, salto, cámara y colisión; después empaquetar Development/Win64 y abrir fuera del editor. Caminar lento, sprint y vuelo propio se añaden en las siguientes tareas, no quedan implementados por generar la plantilla.

Si C++ está bloqueado, una prueba Blueprint puede verificar editor/entrada, pero **no aprueba W01 ni demuestra compilación C++**. Registrar el bloqueo preciso.

Fab y Quixel Bridge no son condiciones de aceptación de esta primera prueba. No importar City Sample, ManOfSteel entero, planetas ni librerías completas. No cancelar colas, instalar programas o cambiar configuración global sin autorización.

## Resultado que se exige

Distinguir instalación observada, instalación confirmada, editor abierto, compilación y ejecución. Reportar versiones, comandos, errores, captura y pruebas reales. No subir datos personales, motor, Binaries, Intermediate, Saved o DerivedDataCache al repo. No afirmar FPS sin hardware/resolución/perfil medidos.

## Fuentes consultadas el 2026-11-15

- Epic, Visual Studio: https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine
- Epic, UE5.8 Platform SDK Upgrades: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes?lang=en-US
- Microsoft, carga e integración: https://learn.microsoft.com/es-es/visualstudio/gamedev/unreal/get-started/vs-tools-unreal-install
- Epic, selector de proyectos: https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-new-project-in-unreal-engine

Consultar la fila/documentación de UE5.8 aunque una página genérica tenga encabezado de otra versión. Ninguna de estas fuentes confirma instalaciones en el equipo del usuario.
