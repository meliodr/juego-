# AGENTS.md — IMPACT: BEYOND

## Actualización de recursos y estado jugable — 18/11/2026

Leer **docs/FAB_AUDITORIA_LOTE03_2026-11-18.md** además de **docs/FAB_SELECCION_ACTIVA.md** y **docs/FAB_CATALOGO_MAESTRO.csv** (82 fichas: 12 seleccionadas, 10 por probar, 40 reservas y 20 descartes). Los números anteriores de 52/8/5/27/12 que se ven más abajo son un registro del 15/11, no actuales. **No inferir propiedad**, el acceso de navegador conectado a My Library no está operativo desde este chat; 3 promociones del 17/11 al 1/12 (Absorption FX, Post-Apocalyptic Factory, Advanced Companion Drone). No comprar/reclamar/descargar/instalar ni enviar contenido NoAI a programas de IA generativa.

**La versión S1–S7 fue integrada y validada como build funcional según docs/PLAN_ACTIVO.md y docs/IB_S7_CLOSURE_REVIEW.md al 16/11/2026.** Ya no hablar de W00/W01 como pendientes actuales. Si se evalúan nuevos recursos, hacerlo con un solo ensayo aislado de ciudad/vegetación/malecón/agua por necesidad y priorizar consistencia realista/perfil GPU, sin alterar la build S7 que funciona.


## Reglas de selección Fab — catálogo activo (2026-11-15)

Antes de crear un personaje, animación, vehículo, escenario, robot, kaiju, VFX o sonido, leer **docs/FAB_SELECCION_ACTIVA.md** y consultar **docs/FAB_CATALOGO_MAESTRO.csv**. Consultar lotes 01/02 para detalles por ficha. Los 52 productos solo están confirmados como **Gratis/Free en listados públicos**, NO como reclamados o instalados. No inferir «lo tengo» ni «no lo tengo» sin acceso real a Mi biblioteca; no descargar, comprar, reclamar o instalar nada automáticamente. Clasificación vigente: 8 SELECCIONAR, 5 PROBAR, 27 RESERVAR, 12 DESCARTAR. Una categoría no implica funcionalidad lista: un modelo de robot no es robot desarmable hasta la prueba.

**No entrenar ni desarrollar ningún modelo IA para este juego.** IA como asistente de programación de Codex es diferente de entrenar una IA. Aplicar [Epic Content License sección 17](https://www.unrealengine.com/eula/content): **los assets con etiqueta NoAI no pueden proporcionarse como entrada a programas de IA generativa**; mantener los bytes de esos recursos fuera de prompts, subidas o análisis de modelos. Su uso dentro del juego debe seguir términos de su licencia, y nunca subir paquetes a GitHub público. «Generated with AI: Yes» es dato de origen, no requisito de entrenar. La edición Fab Standard permite uso privado, con restricciones a redistribución standalone; verificar la licencia real.

Automatización «Auditoría Fab de IMPACT» programada cada 3 días desde 18/11/2026 para intentar revisar y actualizar GitHub, con reporte de cambios y bloqueos. **No presumir futuras ediciones ni acceso permanente**, ni delegar en la automatización decisiones de compra. Estado de hoy: ninguno de los assets ha sido importado/probado desde este chat. Seguir trabajando W00/W01 y el personaje que camina/corre/vuela como prioridad.


## Regla principal e innegociable — REUTILIZAR AL MÁXIMO

**«Reutilizar lo más que se pueda y crear lo mínimo que se pueda».**

Antes de implementar cualquier mecánica, personaje, animación, mapa, material, efecto, sistema o herramienta:
1. **Buscar primero lo existente:** Unreal Engine 5.8 (plantillas, componentes y plugins integrados), biblioteca Fab del usuario, assets gratuitos autorizados, y código/proyectos abiertos relevantes ya investigados.
2. **Verificar que se pueda aprovechar:** archivos reales completos, edición gratuita, licencia por código/arte, compatibilidad Windows/UE 5.8, dependencias, calidad y coste de integración/rendimiento. No confundir un vídeo o puntero LFS con un recurso utilizable.
3. **Preferir reutilizar → configurar → adaptar → combinar.** Crear código, modelos o animaciones nuevos **solamente para cubrir una necesidad que no resuelva adecuadamente lo disponible** o cuando adaptar resulte más costoso o inestable que una solución mínima propia.
4. **Importar solo lo imprescindible**, no proyectos gigantes por anticipación. Elegir un candidato y un respaldo por necesidad; probar el resultado en una escena pequeña y abandonar alternativas cuando una funcione bien.
5. **Justificar toda creación desde cero:** explicar qué se buscó, por qué no sirve, qué parte mínima se creará, cuánto añade en complejidad y cómo se probará.
6. **Mantener calidad y derechos:** no sacrificar fluidez, control ni mantenimiento por forzar la reutilización; no copiar recursos sin permisos aunque el juego sea privado. Conservar avisos de licencia y procedencia.

Esta regla manda para Codex, Robert cuando tenga acceso, y cualquier colaborador futuro. **La prioridad no es producir más código ni más assets: es obtener más juego funcional con el mínimo trabajo nuevo.**


## Plugins: decisiones activas 2026-11-15

Leer **docs/PLUGIN_SELECTION_V14.md** antes de instalar o activar cualquier plugin del motor o tercero. Selección nativa por fase: Enhanced Input → IK Rig/Control Rig al importar clips → Niagara al añadir VFX → Chaos/Fracture al probar pared → PCG al distribuir ciudad; Motion Warping, StateTree/SmartObjects y GAS solo si un ticket posterior lo justifica. Unreal MCP/AllToolsets editor EXPERIMENTAL, solo local y sin servidor en build distribuible. Externos en sandbox: PCGEx MIT rama 5.8 para grafos/parcelas después de PCG nativo; Realtime Destructible Mesh gratuito Fab Standard para comparar huecos frente a Chaos; Realtime Mesh Component Core MIT reservado para malla runtime. NO instalar ningún externo hoy sin proyecto/compilación y prueba, no habilitar suite completa por curiosidad, no adquirir Fab automáticamente ni copiar sus binarios a GitHub. Cooperativo permanece aplazado. Mantener siempre una opción nativa de respaldo.


## Decisión de plataforma vigente — Windows, 2026-11-15

**El usuario cambió de Linux a Windows como plataforma principal y acaba de mostrar Unreal Engine 5.8 instalándose en Epic Games Launcher.** Quixel Bridge y Fab UE Plugin están en cola; no afirmar que están listos. Leer docs/WINDOWS_INICIO_V12.md antes de ejecutar cualquier gate. La primera tarea es **IB-W00** (verificar editor y toolchain Windows), seguida de IB-W01 (crear proyecto). En el plan histórico, toda instrucción de «debe compilar en Linux», «Vulkan como primer renderer», «L00» o «primer ejecutable Linux» queda pospuesta. Windows y DX12 son la ruta de referencia; Linux solo port futuro. 
La instalación visualizada no confirma que el editor se haya iniciado ni Visual Studio/MSVC instalado. Para UE 5.8, VS 2022 v17.14 o VS 2026, MSVC/Windows SDK compatibles; comprobar antes de crear proyecto C++ y no instalar ni cambiar herramientas sin autorización. Cooperativo continúa extremadamente lejano, fuera del plan activo.


## Fuentes de verdad y estado

1. Leer `docs/PLAN_ACTIVO.md` y el ticket correspondiente de `docs/TAREAS_ACTIVAS.md`.
2. Leer solo la skill pertinente y los documentos temáticos que pide esa tarea.
3. Los planes versionados anteriores son investigación y antecedentes. No reactivan tareas de red, nombres descartados ni otros alcances sustituidos.

Estado a 2026-11-14: repositorio de planificación y skills. En la revisión de `main` anterior a esta consolidación no había `.uproject`, código de juego ni ejecutable. No se ha verificado el Linux del usuario. No declarar instalado, compilado, jugable, importado o probado lo que no se ha ejecutado. La petición actual es planificar; no autoriza instalaciones, cambios de sistema ni ejecución remota por sí sola.

## Alcance y decisiones firmes

- Sandbox individual, offline, Windows como plataforma principal y compras de assets/plugins RD$0. El cooperativo es MUY LEJANO: no N0/N1, LAN, RPC, replicación, servidor, lobby, EOS, matchmaking ni tests de latencia actuales.
- Prioridad: caminar/correr/saltar y volar con un mismo personaje; luego agarrar/lanzar, destruir con huecos reales, combatir y repetir. No comenzar por planeta, campaña, gran ciudad o infraestructura online.
- Vuelo libre sin combustible ni poderes bloqueados por karma. Movimiento responsivo, cámara legible y configuraciones de efectos/accesibilidad. Las animaciones acompañan al movimiento; no lo sustituyen con teletransportes ocultos.
- H01 fuerza; H02 genio cósmico masculino esmeralda del concepto aprobado, sin fruta del diablo ni traje/icono de Linterna Verde; H03 teletransporte. H01/H02/H03 son IDs internos. No usar VECTOR o PRISMA como nombres de héroes ni recuperar al enemigo constructor genérico. No confundir arte 2D con modelo riggeado listo.
- Robots, kaijus y superhumanos; desarme de piezas preparadas con consecuencias funcionales y sin gore explícito. Primero un robot pequeño y un rival, no diez rigs.
- Ciudadanos, amenaza, alerta y karma son sistemas distintos. No atribuir automáticamente daño enemigo al jugador. Modos creativos permiten desactivar consecuencias.
- La visión espacial conserva planeta principal pequeño/continuo visualmente, Luna, dos planetas visitables por regiones y Sol solo visible. Se integra después del sandbox urbano estable, no física detallada de todos los cuerpos.

## Entorno conocido y límites de acceso

Hardware reportado: **64 GB RAM, RTX 4070 de 8 GB VRAM**. Inspeccionar CPU, modelo exacto de GPU, versión de Windows, driver/DirectX 12, SSD/espacio y motor/toolchain Windows (Visual Studio/MSVC/SDK). No repetir la pregunta de RAM/GPU ni confundir el contenedor del chat con la máquina del usuario.

Motor/parche y toolchain se fijan al pasar IB-L00, no por una versión citada en documentación cambiante. No actualizar a Preview ni modificar driver/distribución sin autorización. Unreal MCP es opcional y debe comprobarse en el editor local; no exponerlo a Internet ni convertirlo en dependencia del ejecutable.

Robert es el Dot del usuario, no una herramienta cuya conexión o ejecución debamos dar por hecha. Un encargo escrito no significa que fue enviado, aceptado o ejecutado. Los roles de integrador, artista técnico y QA son responsabilidades, no agentes necesariamente activos. No trasladar permisos/roles de otros proyectos.

## Flujo de trabajo

- Una tarea de implementación activa. Auditoría paralela solo sin modificar los mismos archivos. Un responsable de integración; no editar simultáneamente el mismo Blueprint/mapa.
- Trabajar en rama/checkpoint seguro y revisar cambios previos. No force-push, resets destructivos, sobrescritura de trabajo ajeno, cambios en otros repositorios o merges no autorizados.
- Antes: objetivo, archivos permitidos, dependencia, procedencia, prueba, límite y rollback. Después: commit, resultado observado, evidencia, problemas y siguiente decisión.
- Primera tarea operativa: IB-W00 y después IB-W01. Luego FLT-AUD/GND00/F01 → C00/P01/D01 → E01/S01. Consultar la cola; no abrir trabajos de planetas por iniciativa propia.
- No añadir otra versión de plan encima de esta guía. Actualizar el plan activo y su cola, preservando decisiones y registrando cambios. Git conserva el historial previo de README/AGENTS.

## Reutilizar antes de crear

Consultar auditorías de vuelo y ciudad/destrucción/planetas. ManOfSteel: evaluar solo C++ y clips realmente disponibles/permitidos, no asumir que punteros LFS sean assets. REPIRK: solo agarre mínimo. PlotBuilder: ensayo del soporte en estructura pequeña contra alternativa Chaos nativa; no importar inventario/tienda. Ephemera: laboratorio espacial separado.

Una necesidad, un candidato y una alternativa nativa. Inventariar bytes, permisos, versión y dependencias antes de integrar. Si exige migrar un framework grande, volver a la alternativa; dejar de buscar cuando una opción supera la prueba. Conservar una sola implementación principal de movimiento/agarre/destrucción.

## Reglas técnicas de juego

- Para cualquier sesión de prueba que se abra al usuario en Windows, actualizar el acceso existente ImpactBeyond al paquete vigente y abrirlo desde Steam. En Propiedades > Mando, exigir Activar Steam Input para este juego. Si el lanzador del paquete avisa de Visual C++ a pesar de que el runtime ya está instalado, usar el ejecutable de juego en ImpactBeyond/Binaries/Win64 y verificar el arranque real por Steam.

- Diseñar cada interacción jugable con teclado/ratón y mando desde el inicio. Mantener el esquema DualSense de PlayStation 5 y comprobar los eventos de gamepad en cada entrega. Registrar aparte cualquier prueba que requiera el mando físico. Agrupar avances jugables y reservar las pruebas del usuario para sensación de control o hardware que no se pueda verificar desde el entorno.

- Separar locomoción, acción y presentación. Datos editables para velocidades/daño/efectos; pocos componentes, no un framework gigante antes de jugar.
- Barrer también el volumen del objeto transportado; no pegar un carro a las manos e ignorar su colisión.
- Un dueño lógico por objeto, un desprendimiento por PartId y daño no duplicado por AttackId/TargetId. Restaurar estados al soltar, destruir o reiniciar. Son reglas offline, no trabajo de networking.
- Pared rota implica colisión transitable; polvo o Niagara no sustituyen el hueco. Solo pocos fragmentos significativos físicos/agarrables, resto cosmético.
- Desarme elimina la capacidad correspondiente. Teleport valida cápsula, carga y región lista. Un efecto no decide daño, karma ni movimiento.
- No penalizar al usuario por cada fragmento del mismo impacto ni premiar rescates fabricados indefinidamente.

## Licencias y seguridad

Verificar código, modelos, clips, audio y plugins por separado; público/gratuito no equivale a permiso universal. Mantener LICENSE, avisos y procedencia. Revisar edición Standard/Source y términos concretos; no comprar ni contratar APIs para completar tareas. El juego privado no justifica subir assets protegidos a GitHub público.

No publicar credenciales, rutas privadas, datos del usuario ni archivos de terceros sin permiso. No redistribuir el motor, paquetes de Fab u otros recursos como bibliotecas sueltas. No contactar autores, comprar, alterar visibilidad/permisos o abrir puertos sin autorización. Inspeccionar scripts antes de ejecutarlos.

## Skills existentes

- `impact-linux-gate`: acceso, toolchain y paquete Linux.
- `impact-license-audit`: procedencia, integridad y permisos.
- `impact-gameplay-gate`: una mecánica y su prueba.
- `impact-character-animation`: rig/retarget/poses/IK/cámara.
- `impact-qa-review`: regresión, perfiles y evidencia.
- `impact-civic-systems`: ciudadanos/alerta/karma cuando toque.
- `impact-world-events`: eventos cuando existan sus participantes.

Si una skill histórica menciona red, aplicar el alcance offline actual y omitir esa rama. Las skills no conceden herramientas, acceso o permisos adicionales.

## Evidencia y cierre

Estados: PLANIFICADO, EN CURSO, IMPLEMENTADO SIN VALIDAR, COMPILADO, EJECUTADO, ACEPTADO o BLOQUEADO. No inventar porcentajes, benchmarks ni tests. Un build sin render no valida DX12, cámara, arte o diversión. Un vídeo ajeno tampoco.

Registrar versiones reales, instrucciones, mapa, resultados/fallos, logs, capturas cuando sean pertinentes y rendimiento con hardware/resolución/calidad. Separar primera ejecución del recorrido repetido. Meta 1080p/60 FPS configurable, no garantía para el equipo. Conservar build anterior jugable. El usuario evalúa control/potencia/deseo de repetir; no sustituirlo por una afirmación de IA.
