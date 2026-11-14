# IMPACT: BEYOND — Playbook de Codex, Linux y QA v0.5
2026-11-14. Este documento describe cómo trabajar CUANDO se disponga de un Linux real con Unreal. No confirma que el agente tenga acceso al equipo, a una GPU o al editor en el chat actual.

## Regla principal
**Codex ensambla, ejecuta pruebas y deja evidencias; el usuario juega y decide la sensación.** Un LLM no sustituye artista de animación, QA humano ni hardware GPU. No construir una arquitectura extensa sin demostrar gameplay.
Antes de cada acción: leer README.md, AGENTS.md, MASTER_PLAN_V05.md, FREE_RESOURCE_AUDIT_V05.md, LINUX_BASELINE.md, COOP_PLAN.md. Respetar doc versionada vigente. No utilizar nombres rechazados VECTOR/PRISMA.

## Nivel realista de autonomía de IA
- **Bien automatizable:** búsqueda/licencias preliminares; revisión de C++/descriptor, scaffolding, componentes pequeños, código de pruebas, scripts de importación de modelos, generar materiales simples, configurar inputs/actores via MCP o Python editor, ejecutar comandos de build/cook/package, leer logs, crear reportes, corregir errores reproducibles.
- **Automatizable con inspección:** UAssets binarios, Blueprints, animation rigs, Niagara, GeometryCollections, World Partition/PCG, colisiones, validación de compatibilidad de plugins. Si falta una API de MCP, usar script editor documentado o intervención breve con pasos reproducibles. No inventar cambios de editor que no se ejecutaron.
- **No garantizado automáticamente:** “diversión”, control preciso, diseño excelente de ciudad, calidad de cámara a 1000 km/h, aspecto final consistente, ragdoll estable en todos los casos, QA de dos jugadores sobre Internet, física de planeta completo. Test de juego humano obligatorio.
- **No automatizar:** pagos, instalar servicios/API de runtime, modificar la red del usuario, hacer público contenido no autorizado, bajar binarios no inspeccionados, cambiar licencias/hardware, borrar originales sin validación, subir activos comerciales ajenos a GitHub.

## Unreal MCP específico 5.8
- Función **experimental** integrada en Unreal Editor, no licencia para ejecutar el editor remoto arbitrariamente. Requiere plugins ModelContextProtocol y AllToolsets (ToolsetRegistry como dependencia); habilitar localmente tras revisar editor/versión.
- Servidor MCP escucha en loopback local (documentación 5.8 indica 127.0.0.1:8000/mcp si se habilita autoarranque); **no exponer a LAN/Internet**, no abrir firewall, no poner claves. Servidor no sustituye Git ni garantiza todas las operaciones.
- Documentación: ModelContextProtocol.GenerateClientConfig Codex desde consola del editor, tras configuración. Verificar archivo generado; no reemplazar config existente sin inspección; ejecutar Codex desde raíz del proyecto. Las herramientas MCP que modifican editor corren serializadas en game thread: **NO enviar llamadas de herramientas solapadas**.
- Empezar con prueba MCP-00: inspeccionar nivel, crear cubo, mover cubo, guardar copia de nivel, cerrar/reabrir editor, comprobar diferencia. Si falla, fallback shell+Unreal Python Editor y prueba manual mínima.
- Editor MCP no se empaqueta como dependencia del juego. Todo código generado se somete a revisión de compilación real.

## Auditoría Linux L00 — comandos solo en máquina real
No ejecutar en contenedor genérico y presentarlo como portátil del usuario:
- uname -a; cat /etc/os-release
- lscpu; free -h; df -h .
- command -v vulkaninfo && vulkaninfo --summary
- command -v nvidia-smi && nvidia-smi (si GPU NVIDIA)
- command -v clang++; clang++ --version
- localizar UE_ROOT y Engine/Binaries/Linux/UnrealEditor; comprobar licencia/acceso de Epic, version sin forzar update.
El documento de requisitos UE5.8 recomienda Ubuntu22.04/ Rocky8, 32GB RAM, VRAM>=8GB y Clang 20.1.8 para 5.7/5.8; Quickstart ha mostrado ejemplo Clang18.1.0: **preferir tabla de requisitos versionada para toolchain elegido**, no asumir clang del sistema.
Objetivo práctico: paquete Linux ejecutable independiente. Validar Vulkan GPU/driver; tiempos de primera compilación/caché no son FPS en gameplay.

## Pipeline por cambio
1. Tomar UNA tarea (gate/issue), escribir propósito en una frase, máximo dos sistemas afectados, condiciones de aceptación, fuente/licencia/versión.
2. Crear rama de trabajo o checkpoint seguro; verificar estado de Git y archivos no versionados. No sobrescribir trabajo sin consentimiento.
3. Buscar donante autorizado relevante de FREE_RESOURCE_AUDIT, inspeccionar partes realmente descargables y deps. Importar solo subconjunto. Guardar nombre/revisión/licencia/ubicación.
4. Implementar vertical slice mínima y un nivel de pruebas. Evitar grandes renovaciones de arquitectura y dependencias duplicadas.
5. Compilar y ejecutar pruebas unitarias/funcionales, no solo linters. Cuando sea posible ejecutar desde binario Linux fuera del editor.
6. Capturar logs de compilación, screenshot/vídeo representativo de la prueba, FPS/frame-time hardware+config, fallo reproducible. Compilar sin render NO valida arte/cámara/Vulkan.
7. Si falla un cambio, intentar arreglo pequeño con causa documentada; si una dependencia empieza a requerir migrar medio framework, revertir al checkpoint y usar fallback nativo. No generar nuevas dependencias para ocultar incompatibilidad.
8. QA de humano breve del bucle. Juntar feedback cualitativo con fallos reproducibles, actualizar tickets; merge solo tras gates mínimos y sin introducir regresión jugable.
9. Conservar una build anterior funcional. Guardar licencia/créditos del código MIT; recursos QAL no distribuir standalone, CC0 según edición/archivo.

## Contrato de entregas
Cada ticket debe informar:
- Estado: DESCUBIERTO -> FUENTE_INSPECCIONADA -> IMPLEMENTADO -> COMPILADO -> EJECUTADO -> ACEPTADO, o BLOQUEADO.
- Build: commit, UE version, Linux distro, clang, GPU, driver, RAM, VRAM, ruta del proyecto (sin secretos), build/package/run commands REALES.
- Aceptación: nombre mapa/test, entradas, número de ciclos, resultado esperado y observado, error/stack/log completo cuando falla.
- Rendimiento: resolución, perfil Low/Medium/High, promedio y p95/p99 de tiempo de frame, CPU/GPU, picos Chaos y VRAM. Sin objetivo 60 FPS como resultado no medido.
- Jugabilidad: puntuaciones 1–5 de control, velocidad, claridad de impactos, deseo de repetir; fecha y quién la jugó (sin identificar personas innecesariamente).
- Límite pendiente y cuál es el siguiente gate. No dar “80% terminado” sin cobertura funcional.

## Plantilla mínima para pedir una tarea a Codex
Tarea: IB-XXX [resultado observable].
Fuentes: MASTER_PLAN_V05 + FREE_RESOURCE_AUDIT + docs originales.
Trabaja solo: [módulos y mapa].
NO tocar: [otros sistemas, configuración global, red, plugins pagos].
1. Inspecciona versión, archivos y licencias reales.
2. Selecciona opción nativa o recurso gratuito verificado.
3. Implementa [único bucle].
4. Agrega test reproducible y versión empaquetada Linux si entorno lo permite.
5. Ejecuta, registra evidencias, fallo y limits. Nunca inventes screenshots, compilación o uso de MCP.
6. Con fallos no resueltos, informa el bloqueo y conserva una versión funcional.
Resultado: commit + changelog + test/log + instrucciones de reproducción.

## QA prioritaria
- Flight 10 minutos + combinación de cámara/boost/freno/colisión a 30/60 FPS si los alcanza.
- Carro: 30 ciclos agarrar/transportar/lanzar en aire/suelo; objeto no atraviesa fachada indemne.
- Pared: 50 golpes/impactos, hueco transitable y reset sin colisiones fantasma.
- Robot: desconectar brazo/cabeza/propulsor uno por uno, IA pierde habilidad correcta; desprendido no se duplica.
- Kaiju: 3 puntos preparados y cambio de IA/animación/colisión, no cada hueso cortable.
- Blink: 50 destinos válidos/inválidos (en vuelo, bordes, techos y con objetos sujetos), nunca dentro de sólido.
- Planeta: 10 viajes órbita-regreso, referencia persistente y sin cambio equivocado de gravedad.
- Red: dos instancias nativas, una sola posesión por carro, mismo estado final de muro/robot, retransmisión tras join o reconexión básica si implementada, pruebas con latencia/pérdida.
- Nada aprobado solo por demo de donante o respuesta textual de IA.

## GitHub público y arte binario
Versionar C++, Config, scripts de pruebas, docs, manifiestos de procedencia; no Binaries, DerivedDataCache, Intermediate, Saved, logs voluminosos o secretos.
Los assets CC0 propios de terceros solo tras verificar edición; recursos QAL/Fab pueden requerir distribución únicamente dentro del juego, no como asset suelto público. Si duda, scripts de descarga con enlaces y hash/manifest, no incluir ZIP.
Git LFS de assets permitidos puede consumir cuota del usuario; no considerarlo sin límite ni requisito en v0.5. Los archivos .uasset/.umap son binarios, difíciles de fusionar: preferir commits pequeños y propiedad clara.

## Fuentes
Unreal MCP 5.8: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8
Automation Framework: https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-test-framework-in-unreal-engine
Run Automation Tests: https://dev.epicgames.com/documentation/unreal-engine/run-automation-tests-in-unreal-engine
Gauntlet: https://dev.epicgames.com/documentation/unreal-engine/gauntlet-automation-framework-overview-in-unreal-engine?lang=en-US
Linux requirements: https://dev.epicgames.com/documentation/unreal-engine/linux-development-requirements-for-unreal-engine?lang=en-US
