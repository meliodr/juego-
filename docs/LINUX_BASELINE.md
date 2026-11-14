# IMPACT: BEYOND — Base Linux

Fecha: 2026-11-14. Decisión del usuario: trabajar en Linux. Interpretación de trabajo: desarrollo y primera versión jugable nativos en Linux. No se ha inspeccionado su equipo ni ejecutado Unreal en él durante esta actualización.

## 1. Decisiones

- Mantener Unreal Engine 5; no migrar de motor solo por cambiar de sistema operativo.
- Editor y ejecutable Linux nativos; Vulkan como ruta gráfica de referencia. Wine/Proton no serán requisitos del proyecto.
- Empezar con la distribución que tenga el usuario si satisface los requisitos de la versión elegida. No formatear, cambiar de distribución ni instalar drivers sin autorización.
- Preferir una distribución precompilada oficial del editor cuando sea adecuada; compilar todo el motor no es un objetivo en sí mismo.
- Fijar versión/parche del motor y toolchain después de inspeccionar el entorno y compilar/empaquetar la base. No tomar un número dicho previamente en el chat como compatibilidad demostrada.
- No exigir DLSS, Reflex, ray tracing ni otra función de un único fabricante. Mantener un perfil escalable para priorizar movimiento y destrucción.
- Ningún plugin del catálogo está certificado para Linux por esta investigación. Fuente disponible es requisito útil, no prueba de portabilidad.

Epic documenta instalaciones Linux precompiladas y desde fuentes, Vulkan y empaquetado nativo [S1, S2]. Estos son hechos de plataforma; las selecciones anteriores son decisiones del proyecto.

## 2. Inspección inicial: tarea para el entorno real

Registrar distribución/versión, arquitectura de CPU, CPU, RAM, GPU/VRAM, driver Vulkan, espacio libre, sistema de ventanas cuando afecte la entrada, ruta/versión de Unreal y toolchain de esa versión. No inferir hardware por una conversación antigua ni confundir un contenedor de chat con la laptop del usuario.

Usar primero comprobaciones de solo lectura. Instalar componentes solo cuando se confirme que faltan y exista autorización. Guardar evidencia con rutas privadas y datos sensibles omitidos antes de publicar al repositorio.

La documentación rápida y las tablas de requisitos pueden referirse a versiones distintas del compilador. Elegir el toolchain indicado para la versión real de UE; no sustituirlo automáticamente por el clang más reciente del sistema [S1, S2].

## 3. Flujo de desarrollo

Git -> proyecto local -> Codex en terminal/editor disponible -> compilación nativa -> prueba en editor -> empaquetado Linux -> ejecución fuera del editor -> evidencia.

Codex puede trabajar desde la terminal según su documentación [S5]. El acceso, límites y permisos reales de la cuenta no se dan por ilimitados. No contratar APIs adicionales ni generar assets mediante servicios de pago para el gameplay.

Unreal MCP es una ayuda opcional del editor, no una dependencia de ejecución del juego. La documentación de UE 5.8 lo presenta como experimental y admite configuración para Codex [S4]. Comprobar presencia y funcionamiento en la distribución Linux instalada. Prueba mínima: listar actores, crear/mover/eliminar un cubo de una escena de laboratorio y guardar. No afirmar que cualquier Blueprint/asset se puede producir automáticamente.

Si MCP no está disponible o falla, continuar con código, herramientas del editor y scripts documentados que sí existan, registrando los pasos manuales pendientes. No actualizar a un motor preliminar solo por una herramienta experimental. No exponer MCP a Internet; limitarlo a la máquina local y revisar scripts de terceros antes de ejecutarlos.

Separar herramientas de editor de módulos runtime. Un paquete jugable no debe necesitar el agente, MCP, credenciales ni conexión a IA. La prueba offline se realiza sobre el ejecutable, no desconectando herramientas durante una compilación.

## 4. Filtro obligatorio de dependencias

Antes de añadir un candidato:

1. Confirmar licencia, coste cero y acceso a los archivos necesarios.
2. Inspeccionar `.uproject`, `.uplugin`, `Build.cs`, `Target.cs` y fuentes necesarias. Revisar listas de plataformas, bibliotecas externas, rutas Windows, includes con mayúsculas distintas y dependencias exclusivas del editor.
3. Exigir fuente o binarios adecuados a Linux y a la versión del motor, según el tipo de módulo. Un `.dll` para Windows no satisface la entrega Linux.
4. Compilar en la base mínima Linux; después cocinar/empaquetar. La compilación del editor no certifica el paquete runtime.
5. Abrir el ejecutable con Vulkan y ejecutar la prueba real de esa mecánica. Probar sonido, entrada y materiales, no solo ausencia de crash al arrancar.
6. Registrar resultado y adaptación. Si la portabilidad obliga a una reconstrucción amplia, seleccionar la alternativa nativa ya contemplada.

Unreal distingue carpetas Source, Binaries y módulos de editor/runtime [S3]. No eliminar una restricción de plataforma del descriptor y marcar el plugin compatible sin comprobar el código.

## 5. Revisión de la selección previa

| Candidato | Decisión Linux | Evidencia y prueba pendiente |
|---|---|---|
| R02, agarre REPIRK | Candidato condicionado, no base obligatoria | Catálogo: README orientado a UE 5.7/Win64. Build.cs inspeccionado declara módulos de Unreal; esa lista no prueba el resto del código. Portar solo agarre y compilar. Alternativa: interacción mínima nativa. |
| Chaos nativo | Primera vía para la pared del prototipo | Preparar contenido y verificar colisión, fracturas y paquete Vulkan. No se marca ejecutado por estar incluido en el motor. |
| R03, Realtime Destructible Mesh | Reserva hasta validar acceso, términos, fuentes y Linux | No bloquear el primer bucle ni basar toda la ciudad en una compatibilidad sin comprobar. |
| R04, Niagara Destruction Driver | Reserva visual | Ensayar comportamiento GPU/materiales bajo Vulkan; no confundir escombros cosméticos con cuerpos agarrables. |
| R11–R13, Quaternius Standard | Recursos a importar, no un framework | Comprobar contenido del ZIP, formatos, escala, materiales y animación en la versión instalada. |
| R01, Ephemera | Laboratorio orbital separado | Su descriptor inspeccionado limita Reflex, PixelStreaming2 y DLSS a Win64. Eso afecta esas integraciones; no demuestra que todo el proyecto sea incompatible ni que el núcleo compile en Linux. Revisar módulos/shaders y preparar flujo de construcción nativo sin esas dependencias de Windows. |
| R05, daño estilizado | Posterior al sandbox | Inspeccionar descriptor/fuentes y probar un personaje preparado en Linux. No bloquear vuelo/agarres. |
| Binario solo Windows sin fuente necesaria | Fuera de la entrega nativa | No introducir Wine como obligación ni prometer portabilidad por cambiar extensiones. |

Evidencia técnica consultada:

- REPIRK `ModularMechanicsToolkit.Build.cs`, blob `8058895d61cb1b49a708a20c8bca67cd2967ac02`: módulos Core, Engine, InputCore, Chaos/GeometryCollection/Physics, Niagara, audio y UI. Solo se inspeccionó este archivo en la revisión Linux; no se compiló.
- Ephemera `unreal/Ephemera.uproject`, blob `cb2effbfc33380f09e79d8993f08e91e3b3fbdf7`: EngineAssociation 5.8 y filtros Win64 en las tres integraciones citadas. Otros módulos y shaders requieren revisión independiente.

## 6. Repositorio y automatización

- Mantener `UE_ROOT` como configuración local, no fijar rutas de una máquina en el código.
- Preferir scripts shell/Python portables para tareas propias; no convertir un script PowerShell del donante en requisito de Linux.
- Mantener exactamente las mayúsculas de archivos/includes. Revisar permisos de ejecución y finales de línea de scripts.
- No versionar Binaries, Intermediate, DerivedDataCache, Saved, credenciales ni paquetes de terceros con redistribución no autorizada.
- Git LFS se evalúa solo para binarios propios o redistribuibles; límites de almacenamiento/transferencia deben comprobarse antes de subir grandes assets. No se supone almacenamiento gratuito ilimitado.
- CI puede validar documentos y código auxiliar sin motor. Pruebas de Unreal requieren una instalación apropiada; validar cámara/rendimiento necesita además una sesión gráfica/GPU real. No llamar a un chequeo sin render una prueba visual.
- No publicar el código del motor ni recursos protegidos para simplificar una automatización.

## 7. Gate L00: resultado mínimo para comenzar gameplay

Un proyecto base debe compilar, cocinar/empaquetar para Linux y arrancar desde su ejecutable fuera del editor. Probar ratón/teclado, sonido, colisión y salida limpia; repetir después de cerrar sesión del editor. Registrar versión, entorno y comandos reales usados.

Después se ejecuta D0 de GAMEPLAY_SPEC.md: vuelo controlable. No aprobar solo porque el proceso de compilación termina con código cero.

Objetivo de diseño: 60 FPS con perfil ajustable sobre un hardware de referencia todavía no identificado. Registrar resolución/calidad, GPU/driver, tiempo de física, percentiles y tirones. Medir primera ejecución y recorrido repetido por separado. Bajar coste visual/población antes que recortar la respuesta del vuelo.

## 8. Fuentes primarias consultadas

[S1] Epic — Linux Development Quickstart:
https://dev.epicgames.com/documentation/en-us/unreal-engine/linux-development-quickstart-for-unreal-engine

[S2] Epic — Linux Development Requirements:
https://dev.epicgames.com/documentation/unreal-engine/linux-development-requirements-for-unreal-engine?lang=en-US

[S3] Epic — Plugins in Unreal Engine:
https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine

[S4] Epic — Unreal MCP, documentación UE 5.8:
https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8

[S5] OpenAI — Usar Codex con tu plan de ChatGPT, sección terminal:
https://help.openai.com/es-419/articles/11369540-using-codex-with-your-chatgpt-plan

[S6] Fuente REPIRK:
https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5/blob/main/ModularMechanicsToolkit.Build.cs

[S7] Descriptor Ephemera:
https://github.com/0xWerz/ephemera/blob/main/unreal/Ephemera.uproject

Las fuentes pueden cambiar. Fijar commit de cada donante al importarlo y usar documentación correspondiente al motor seleccionado. Todo estado de ejecución sigue pendiente.
