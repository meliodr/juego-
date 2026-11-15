# IMPACT: BEYOND — Inicio real en Windows (v1.2)

Actualizado: 2026-11-15. **Decisión vigente: Windows como plataforma principal del editor y primer ejecutable.** Linux queda como posibilidad de port posterior, no gate actual. Un solo jugador offline; cooperativo muy lejano. Compras de assets/plugins RD$0. El proyecto Unreal todavía **no** se ha creado, abierto ni compilado en este entorno.

## Evidencia disponible
En la captura del usuario se ve Epic Games Launcher instalando **Unreal Engine 5.8**, con descarga/integración en progreso (335.76 MB de 11.54 GB en el momento fotografiado). Quixel Bridge (UE 5.8) y Fab UE Plugin (UE 5.8) aparecen **en cola**. No deducir que terminaron de instalarse, que se ejecutaron o que ya existe un proyecto.
Hardware reportado: RAM 64 GB; GPU «4070 con 8 GB VRAM». Verificar modelo exacto de tarjeta, CPU, driver, DirectX 12, SSD y espacio libre. No volver a preguntar datos ya proporcionados.

## Decisiones del primer sprint
1. **W00 — Comprobar Windows, Unreal y compilador.** Abrir Epic Launcher cuando finalice, iniciar UE 5.8 y confirmar que funciona su editor. Revisar espacio, drivers, CPU y Visual Studio / MSVC / Windows SDK de forma no destructiva antes de instalar o modificar software.
2. **W01 — Proyecto mínimo para desarrollar.** Games → Third Person (evaluar variante Combat solo si está realmente disponible), proyecto **ImpactBeyond** en ruta de usuario elegida, C++ si el compilador está listo. Si el toolchain falta, primero usar plantilla Blueprint para validar editor/entrada y registrar el bloqueo de C++ sin improvisar una instalación; migración a C++ cuando se confirme.
3. **W02 — Primer circuito de movimiento terrestre.** Un personaje, teclado/ratón, idle/caminar/correr/esprintar/saltar/caer/aterrizar, cámara y colisión. Compilar y ejecutar fuera del editor cuando la herramienta esté lista.
4. **W03 — Vuelo ManOfSteel adaptado.** Revisar C++ MIT de ManOfSteel y extraer solo perfil de vuelo, inclinación y cámara; no su personaje, Blueprints incompletos ni assets de Superman. Mantener movimiento CMC con colisión barrida. Animaciones Quaternius Standard/permitidas o maniquí de plantilla hasta poder importar clips reales.
5. **W04 — Recorrido de una manzana gris.** 30 ciclos caminar→correr→saltar→despegar→frenar→aterrizar→correr, 10 minutos de vuelo en un mapa sencillo, medir y corregir antes de traer carros, edificios, robots o planetas.

## Dependencias de Windows según Epic UE 5.8
Notas de UE 5.8: IDE recomendado Visual Studio 2026; **mínimo compatible Visual Studio 2022 v17.14**. MSVC mínimo 14.38, recomendado 14.50. Windows SDK mínimo **10.0.22621.0**; por defecto **10.0.26100.0**. La documentación de Epic indica que el Launcher instala algunos prerrequisitos de ejecución, **pero eso no asegura que estén las herramientas de desarrollo C++**.
Fuentes: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes?lang=en-US
https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine
https://dev.epicgames.com/documentation/unreal-engine/installing-and-setting-up-a-project

## Quixel Bridge / Fab
No dependemos de Bridge o Fab para W00-W04. No instalar packs, iniciar sesión en servicios extra ni aceptar términos de activos por automatización sin que el usuario lo decida. Si Fab queda instalado, usarlo solo al necesitar un asset concreto con licencia/edición verificada. Para primer ejercicio usar mallas de plantilla.

## Activación y estado
Esta ficha es una **orden de trabajo preparada, no ejecutada**. No hay acceso directo confirmado al ordenador Windows desde este chat ni una tarea enviada/aceptada por Robert. No afirmar que Unreal terminó, que VS está instalado o que W00/W01 pasaron sin pruebas locales.
Prioridad de aceptación W00: editor abre, se reconoce DX12/GPU, proyecto de plantilla abre y permite mover personaje. W01: herramienta de compilación válida y proyecto C++ compila; si falla registrar salida. W02-W04 requieren vídeo/captura o logs y verificación humana. Un proceso sin compilador sigue BLOQUEADO para C++ aunque editor Blueprint funcione.

## No hacer ahora
No migrar a Linux, no instalar varias versiones de UE, no importar una ciudad gigante/ManOfSteel entero/City Sample, no activar MassCrowd ni Chaos masivo, no desplegar servidores ni empezar cooperativo, no construir planetas. Esas tareas dependen del bucle jugable inicial.

## Evidencia del ejecutor
Cada ticket: versión UE, versión VS/MSVC/SDK, CPU/GPU/driver, ruta de proyecto sin datos personales, commit, prueba, resultado real, log/captura, limitación y siguiente paso. Para 4070 8 GB se aspira a 1080p/60 FPS en calidad ajustable, nunca se declara conseguido antes de medir.
