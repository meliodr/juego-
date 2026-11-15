# IMPACT: BEYOND — Decisión de plugins Unreal v1.4
Actualizado: 2026-11-15. Estado: SELECCIONADOS PARA USO O EXPERIMENTO, NO INSTALADOS NI PROBADOS. Windows/Unreal Engine 5.8 como plataforma inicial; un jugador offline; cooperativo muy lejano; compra de assets y plugins RD$0; equipo conocido 64 GB RAM y RTX 4070 8GB VRAM. La captura anterior mostraba UE 5.8 instalándose, Fab UE Plugin y Quixel Bridge en cola. No hay prueba de editor operativo ni de proyecto compilado.

## Objetivo
Seleccionar pocas herramientas útiles para caminar/correr, vuelo, poderes, animaciones, ciudad, destrucción y automatización con Codex/Robert. Usar una sola solución principal por sistema. No activar todo desde el comienzo ni confundir skills de agentes, contenido/plantillas, ejemplos de videojuegos y plugins runtime.

## A. Plugins incluidos en Unreal Engine 5.8: aprobados por fase

| Momento | Herramienta | Decisión |
|---|---|---|
| IB-W01 / primer input | Enhanced Input | USAR si no viene ya activo en Third Person. Contextos de caminar/sprint/flight/agarre/ataques; comprobar mapeos reales antes de agregar otros plugins |
| Rig/animación | IK Rig / IK Retargeter | USAR para pasar clips legales entre rigs, solo cuando haya 2 rigs/clip importado. No activar decenas de retarget systems |
| Rig y agarres | Control Rig | USAR cuando sea necesario para manos, brazos y poses de transporte; no full body IK en toda multitud |
| Efectos vuelo/combate | Niagara | USAR estelas, golpe, energía verde y teleport. Sus partículas no son daño ni objetos agarrables |
| Pared perforable | Chaos + Fracture Mode/Geometry Collections | USAR baseline nativa, confirmar disponibilidad en editor. ChaosEditor/GeometryCollectionPlugin incluyen partes beta; habilitar solo lo necesario |
| Ciudad ampliable | PCG nativo | USAR después de una manzana construida/probada. No generar toda ciudad antes del primer vuelo |
| Golpes/agarres | Motion Warping | USAR únicamente si se demuestra que mejora alineación en distancia válida; no permite atravesar paredes o atraer objetivos injustamente |
| Ciudadanos | StateTree + Smart Objects | USAR después de tener pocas personas y casos de refugio/reacción; MassAI/Crowd experimental queda pospuesto |
| Superpoderes complejos | Gameplay Abilities (GAS) | RESERVA: comparar con componentes simples después de primer sandbox; no requerirlo para caminar/correr/volar ni E1/E2 |
| Modelado procedural | Geometry Scripting | RESERVA/BETA, evaluar si necesitamos malla dinámica y RDM/Chaos no bastan |
| Optimización multitudes | Animation Sharing / Animation Budget Allocator | RESERVA tras medir personajes activos, no activar sin necesidad |
| IA y editor | Unreal MCP (ModelContextProtocol) + AllToolsets | EXPERIMENTO de editor local: Epic lo marca experimental para UE 5.8, hay APIs incompletas. Solo si editor funciona y hay agente local; host 127.0.0.1, no abrir puertos; asegurar que ejecutable distribuible no arranque el servidor MCP |

Unreal Core/CharacterMovement y Chaos runtime no son un plugin superheroico adicional. Comprobar si la plantilla ya habilita Enhanced Input/Niagara. Todas estas herramientas son del motor, pero alguna exige habilitar plugin y reiniciar por proyecto.

## B. Plugin externo 1 — PCGEx: prioridad ciudad, pero NO instalar ahora
Repo: https://github.com/PCGEx/PCGExtendedToolkit . Licencia MIT, estable por versión del motor. Se inspeccionó la rama 5.8 en GitHub, commit 7b2ac324a9df1739502830ce2ad9d2f5fe550510 fechado 2026-11-14. El descriptor PCGExtendedToolkit.uplugin anuncia EngineVersion 5.8.0, VersionName 0.79.6, Runtime Win64 y Editor Win64. Funciones: grafos/Voronoi, rutas, parcelas, filtros y selección de assets. Es un proyecto extenso, no es kit de edificios ni plugin de destrucción. La rama main se considera inestable por el propio proyecto; si se descarga, fijar revisión de 5.8.
Decisión: seleccionado para TEST DESPUÉS de probar PCG nativo. Usar si necesitamos avenidas, redes y parcelas más complejas. Test: 3 vías, 12–20 lotes, edificios del kit gratuito, tiempo de generación, hitches y paquete Windows. Si PCG nativo resuelve, PCGEx no entra al juego. PCGEx Pro de pago descartado. Docs: https://pcgex.gitbook.io/pcgex

## C. Plugin externo 2 — Realtime Destructible Mesh: prioridad para perforación libre, TEST AISLADO
Fab: https://www.fab.com/listings/46f4e8fc-c95c-4e1c-be61-17439bcbbd1b . Fab muestra Free y Allows usage with AI Yes. Repositorio público de documentación: https://github.com/lazydevelopers24/RealtimeDestructibleMesh (NO asumir código abierto). El autor indica UE5.7 o posterior, Windows, proyecto C++ para personalización; booleanos en threads, huecos/colliders actualizados, integridad de malla y agrupación de trabajos. Términos Fab Standard, NO MIT. Comprobar que el launcher muestra compatibilidad 5.8 antes de adquirir y que usuario acepte la licencia; no subir el paquete a GitHub público.
Decisión: Chaos prefractura es BASE. Si queremos perforar arbitrariamente una fachada, comparar RDM contra Chaos en laboratorio separado. 50 impactos, agujeros visuales y reales, latencia de colisión, resets, hitboxes, CPU/GPU/VRAM p95/p99. Aceptar SOLO si demuestra ventaja jugable sobre Chaos y compila/cocina correctamente. No combinar dos sistemas que gestionan la misma pared.

## D. Plugin externo 3 — Realtime Mesh Component Core: alternativa de geometría editable, RESERVA
Repo: https://github.com/TriAxis-Games/RealtimeMeshComponent . Core MIT, versión compatible 5.5–5.8 según README, con LOD/secciones/colisión/malla procedural en runtime. NO crea cortes de paredes por sí solo; requiere algoritmos y diseño de interacción. La edición Pro es de pago y está fuera del presupuesto.
Decisión: no instalar mientras Chaos y RDM cubran necesidades. Solo plantearlo si surge requisito de generar o actualizar geometría del mundo durante la partida que esas opciones no atienden, sin duplicar el pipeline de mallas.

## E. Descartes y reservas explícitos
- Niagara Destruction Driver (https://github.com/eanticev/niagara-destruction-driver), MIT, autor declara pruebas UE5.5.4, sin 5.8 comprobado; solo escombros cosméticos GPU. RESERVA después de profiling.
- Grid Destruction (https://www.fab.com/listings/3a41d8b7-a4e0-4730-8df7-5ba2ff4fd9e0), declarado compatible 5.7/5.8 Win64, pero Fab muestra licencia con Comprar ahora y no se verificó oferta gratuita; DESCARTADO por presupuesto y orientación multiplayer no prioritaria.
- Plugins de cortes universales de cuerpos para kaijus, MassCrowd experimental, suites de planetas con muchas dependencias, PCGEx Pro, RMC Pro/Studio y plugins de licencias desconocidas: NO INSTALAR en fase inicial.
- Fab UE Plugin y Quixel Bridge (en cola): son accesos a contenido, no movimiento ni superpoderes; no bloquean W00/W01. El uso de Fab requiere licencia/edición concreta de cada asset.
- GASam/ProjectAscend son proyectos de ejemplo (no plugins de poderes listos); evitar incorporarlos completos.
- PlotBuilder y REPIRK son proyectos/componentes auditados, no plugins a instalar globalmente. Mantener evaluación aislada por su propio ticket.

## F. Orden de activación y reversión
1. Confirmar UE5.8 en Windows funcionando y proyecto Third Person. Revisar lista Edit > Plugins: Enhanced Input basta para iniciar.
2. Prueba tierra/aire: IK Rig/Control Rig solo si importamos otras animaciones; Niagara al añadir efectos. Sin dependencias externas nuevas.
3. Crear primera pared Chaos/Fracture; si no satisface perforación dinámica, abrir un sandbox nuevo con RDM Free compatible 5.8, sin modificar juego principal.
4. Tras una manzana manual, PCG nativo; si faltan rutas/parcelas avanzadas, sandbox PCGEx en branch/revisión 5.8.
5. Más adelante Motion Warping (agarres/combate), StateTree/SmartObjects (ciudadanos), GAS (habilidades complejas), complementos de rendimiento tras medición.
6. Unreal MCP editor solo cuando haya acceso real de agente local; prueba crear cubo y guardar escena, auto-start inicialmente OFF, endpoint local y verificación del ejecutable sin MCP. Si no funciona, editor scripting/C++/Blueprint habitual.
7. Cada plugin cambia un archivo de proyecto/branch solo si hay test; registrar licencia, versión, dependencia y rollback. Si falla C++/cook/packaging o empeora FPS, desactivar, revertir al checkpoint y documentar por qué. No mantener a la vez opciones competidoras.

## G. Pruebas y veredictos
PLUGIN-01 proyecto con EnhancedInput y ciclo caminar/correr/saltar/vuelo (cuando exista).
PLUGIN-02 IK/ControlRig: 20 transiciones sin poses bruscas o manos absurdas en objeto.
PLUGIN-03 Niagara: 30 golpes/impulsos en ciudad, culling y no hitboxes falsas.
PLUGIN-04 Chaos vs RDM: 50 impactos y 20 reinicios, hueco transitable, sin colisión fantasma, peak VRAM y p95/p99.
PLUGIN-05 PCG vs PCGEx: 3 vías/12 parcelas y rendimiento de streaming/cook.
PLUGIN-06 MCP: agente local inspecciona/crea/edita/guarda un actor, rollback y juego empaquetado sin servidor. Opcional, no bloquea juego.
Estado por candidato: SELECTED_FOR_USE / EVALUATE_LATER / TEST_IN_SANDBOX / ACCEPTED_AFTER_RUNTIME / REJECTED; ninguna opción externa es ACCEPTED_AFTER_RUNTIME hoy.

## Fuentes principales, consultadas 2026-11-15
Epic working with plugins: https://dev.epicgames.com/documentation/en-us/unreal-engine/working-with-plugins-in-unreal-engine
Enhanced Input: https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine
Chaos GeometryCollections: https://dev.epicgames.com/documentation/en-us/unreal-engine/geometry-collections-user-guide
IK Rig: https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine
Motion Warping: https://dev.epicgames.com/documentation/unreal-engine/motion-warping-in-unreal-engine
Niagara: https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-visual-effects-in-niagara-for-unreal-engine
Unreal MCP experimental: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8
PCGEx MIT and 5.8: https://github.com/PCGEx/PCGExtendedToolkit
RDM license/restrictions: https://github.com/lazydevelopers24/RealtimeDestructibleMesh
RMC Core MIT: https://github.com/TriAxis-Games/RealtimeMeshComponent
