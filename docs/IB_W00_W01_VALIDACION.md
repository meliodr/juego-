# IB-W00 e IB-W01 — Validación local de Windows

## IB-W00 · Aceptado el 15 de noviembre de 2026

| Comprobación | Resultado observado |
|---|---|
| Equipo | Windows 11 Home build 26200, Intel Core i9-14900HX (24 núcleos y 32 hilos), 63,7 GiB de RAM |
| Almacenamiento | SSD WD Green SN350, 115,7 GiB libres antes de crear el proyecto |
| GPU | RTX 4070 Laptop GPU, controlador 32.0.16.2114. El editor informó 7948 MB de memoria de vídeo dedicada y seleccionó este adaptador |
| Editor | UE 5.8.4, CL 58902052. Se observó la ventana de inicio del editor y se abrió el selector de plantillas |
| DX12 | El arranque del editor registró la creación del RHI D3D12 y seleccionó NVIDIA. El siguiente arranque, con el proyecto, registró D3D12 SM6 |
| Visual Studio | Community 2026 18.9.12112.369 y Build Tools 2022 17.14.37614.0 |
| Compilación | UBT seleccionó MSVC 14.44.35228 (directorio 14.44.35207) con Windows SDK 10.0.26100.0. Unreal Turnkey informó Win64 SDK válido |

Se revisó y ejecutó el script de solo lectura del PR #2 (59a12c3). El inventario inicial no enumeró el SDK y omitió el estado Git del worktree. Se comprobó el árbol Git con Git directamente y el SDK con Turnkey/UBT. El navegador de proyectos mostró errores de pruebas internas en el registro de inicio, pero el proceso completó la inicialización y abrió la pantalla de plantillas.

Requisitos de referencia de Epic: [Unreal Engine 5.8 Release Notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes).

## IB-W01 · Aceptado el 15 de noviembre de 2026

Proyecto `ImpactBeyond/ImpactBeyond.uproject`, creado desde el asistente Third Person de Epic en C++, Desktop y variante Ninguna. Se conservan los ejemplos de variantes que genera la edición C++ de la plantilla. El mapa de arranque sigue siendo Third Person. Se reutilizan sus acciones Enhanced Input, cámara, colisión y maniquí, y la configuración DX12 Desktop de la plantilla.

Abrir con UE 5.8.4. Para la prueba local, entrar al mapa Lvl_ThirdPerson y pulsar Play. WASD desplaza, el ratón gira la cámara y Espacio salta. Comprobar que el personaje se detiene al avanzar contra un bloque.

Los ejemplos suministrados en Templates se documentan bajo las secciones 1 y 5(b) de la [licencia de Unreal](https://www.unrealengine.com/eula/unreal). El repositorio contiene proyecto, código y contenido generado por esa plantilla.

### Compilación y empaquetado

Unreal Build Tool terminó con código 0 para `ImpactBeyondEditor Win64 Development` e `ImpactBeyond Win64 Development`. El empaquetado usa AutomationTool (`BuildCookRun`, Win64 Development, mapa `/Game/ThirdPerson/Lvl_ThirdPerson`, Pak/IoStore). Los binarios y registros de máquina quedan fuera del repositorio.

AutomationTool completó compilación, preparación del mapa, stage, paquete y archivo Win64 con código 0. El paquete Development ocupa 1,02 GiB. El ejecutable arrancó fuera del editor y cargó Lvl_ThirdPerson con D3D12 SM6 en la RTX 4070. El editor también mostró el mapa en Play-in-Editor y su verificación de mapa devolvió 0 errores y 0 advertencias.

**Prueba visual del ejecutable:** Espacio produjo salto y aterrizaje. El ratón giró la cámara. Con W el personaje dejó la plataforma inicial, avanzó hacia el muro y quedó bloqueado por él al repetir entrada. Para facilitar las pulsaciones breves de la herramienta se usó temporalmente `slomo 10` en el tramo de desplazamiento y choque, y luego se restauró `slomo 1`. No se midió rendimiento ni se evaluó la sensación de control.

Se usaron los componentes existentes. El aviso de red de Windows no impidió verificar el juego local. Para reproducir, ejecutar el paquete Win64 Development desde su carpeta completa (no mover el exe por separado).
