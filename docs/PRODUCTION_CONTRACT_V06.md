# Fe de erratas y alcance v0.9

Cooperativo explícitamente **MUY LEJANO**. Cualquier FR-13, NFR-06, N0/N1/N02, rol de Networking, Server authority o prioridad LAN descritos abajo son **visión remota NO activa**. No son condiciones para aprobar V0 ni para avanzar a S01. Actualmente diseñar y probar solo un jugador en Linux offline. Fuente superior: docs/FLIGHT_REUSE_AUDIT_V09.md. Reutilizar lógica C++ ManOfSteel si pasa la auditoría/port y reconstruir animación desde fuentes permitidas; no intentar portar proyecto completo.

# IMPACT: BEYOND — Contrato de producción y arquitectura v0.6

Fecha: 2026-11-14. ESTADO: **PLAN PARA REVISIÓN; DESARROLLO NO INICIADO**. Linux nativo; Unreal Engine estable por determinar tras pruebas locales; hardware declarado por usuario: **64 GB RAM y RTX 4070 con 8 GB VRAM**. CPU, Linux, driver, SSD y versión instalada de UE aún no comprobados. Cero compras de assets/plugins.

## 1. Alcance dividido: prototipo vs. visión
**V0 vertical slice jugable:** Linux empaquetado, H01 vuela, pequeña manzana urbana con alturas, carro agarrable/lanzable, fachadas perforables, un robot simple desmontable, rival aéreo, 3–6 ciudadanos reactivos, alarma sencilla, reinicio sin campaña. Esto es el primer resultado divertido, no un juego completo.
**Sandbox extendido:** distrito de 4–8 manzanas, tres héroes H01/H02/H03, robots de diferentes tamaños, kaiju mediano, ciudadanos/karma/alertas, 2 jugadores LAN, órbita, Luna y dos planetas con una zona local jugable cada uno; Sol visual no aterrizable. Son destinos posteriores, no requisitos de V0.
**Principio:** ningún título/feature se declara listo sin ejecutar prueba jugable. Desarrollar en Linux real solo después de L00 aprobado y revisión de diseño.

## 2. Requisitos funcionales y gates
| ID | Requisito verificable | Gate |
|---|---|---|
| FR-01 | Vuelo libre, aceleración/frenado, cámara legible, recuperación aérea | F01 |
| FR-02 | Sujetar carro/objeto/personaje elegible, soltar y lanzar, incluso al volar | P01 |
| FR-03 | Abrir agujero de pared, colisión realmente actualizada y pieza reutilizable | D01 |
| FR-04 | Robot pierde brazo/propulsor, cambia IA/capacidades y deja pieza agarrable | E01 |
| FR-05 | Rival superhumano aéreo que persigue, anuncia ataque, recibe golpes y se recupera | E01 |
| FR-06 | Ciudad de diferentes alturas, avenidas, azoteas y zonas interactivas | C01 |
| FR-07 | Ciudadanos por LOD, huida/rescate y refugios | U01/U04 |
| FR-08 | Alertas locales 0–5 y karma personal −100..100 atribuidos a causa, configurables | U02/U03 |
| FR-09 | Sandbox sin historia, generar enemigos/objetos, reinicio y opciones de juego | S01 |
| FR-10 | H02 genio esmeralda/constructos y H03 blink seguro | H02/H03 |
| FR-11 | Kaiju mediano con 3 partes desmontables que cambien comportamiento | K01 |
| FR-12 | Órbita y retorno; Luna y 2 planetas con una región jugable; Sol solo visible | W01/W02 |
| FR-13 | Cooperativo dos personas con estado consistente de daño, agarre, karma y alerta | N01/N02 |

## 3. Requisitos no funcionales
- **NFR-01 Linux/Vulkan nativo:** ejecutable arranca fuera del editor y no necesita Codex/IA ni conexión de red para modo local.
- **NFR-02 Costo RD$0 de compras:** fuente, edición y licencia concreta de cada recurso verificada; MIT conserva avisos; Fab/QAL no se redistribuyen como packs; no usar GTA/Marvel assets.
- **NFR-03 Rendimiento:** aspiración 1080p Medium 60 FPS en la RTX 4070 8GB; fallback Low y resolución escalada. Solo validar con mediciones de p95/p99 frame time, CPU, GPU y VRAM; nunca garantizar de antemano.
- **NFR-04 Gameplay:** vuelo e interacción no se bloquean por karma, combustible ni misiones. Impactos y movimiento legibles sin motion blur obligatorio.
- **NFR-05 QA:** tests reproducibles, evidencia de compilación y ejecución, rollback y una versión anterior jugable. No aceptar resultados inventados de agentes.
- **NFR-06 Red preparatoria:** daños, estados físicos y alertas diseñados como servidor autoritativo, aunque el modo online venga después.
- **NFR-07 Seguridad del proyecto:** no modificar red, drivers, instalar servicios, comprar plugins o mover otros repositorios sin permiso.

## 4. Roles y responsabilidades (no personas contratadas)
| Rol | Trabajo | Decisiones y entregable |
|---|---|---|
| Director creativo / jugador (usuario) | definir sensación, modos, identidad, aceptar diversión | prioriza y puntúa sesiones; no obligado a programar |
| Productor y arquitecto (ChatGPT) | alcance, dependencias, fuente y documentación, ADR, backlog | decide propuesta técnica, presenta alternativas; no dice que compiló sin prueba |
| Integrador Unreal/Linux (Codex A) | build, C++, entradas, plugins nativos, importación controlada | merge tras pasar gate |
| Ingeniero de gameplay (Codex B) | vuelo, agarres, combate, destrucción, habilidades H01/H02/H03 | implementación y pruebas de mecánicas |
| Ingeniero de mundo/IA (Codex C) | ciudad PCG, NPC, alertas, karma, IA, tiers/LOD | prueba ciudad y incidentes; Mass solo tras mediciones |
| QA/rendimiento (Codex D) | tests, matrices, profiling Linux/Vulkan y pruebas de red | PASS/FAIL/BLOCKED y evidencias |
| Auditor de recursos (Codex/ChatGPT) | licencias, dependencias, versiones, ediciones gratis | manifiesto de procedencia con aceptación explícita |
| Tester creativo (usuario) | prueba 10–15 min de vuelo/destrucción | GO/REWORK/STOP; evaluación subjetiva |

Los roles A–D representan **funciones que puede ejecutar un Codex secuencialmente**, no una afirmación de que haya múltiples agentes ejecutándose. Paralelismo solo con ramas/archivos aislados. Una persona/agente integrador dueño del merge a main.

## 5. Arquitectura propuesta (NO archivos C++ existentes)
~~~text
ImpactBeyond/
  ImpactBeyond.uproject
  Source/ImpactBeyond/
    Core/                 # IDs, GameplayTags, incident ledger
    Flight/               # CMC, EnhancedInput, camera
    Interactions/         # grab/hold/throw + item state
    Combat/               # damage/hits/targeting/recovery
    Destruction/          # prepared walls/Chaos/physical pieces
    Enemies/              # robot limbs/kaiju/superhuman AI
    Heroes/               # H01/H02/H03 Data Assets & abilities
    Civilians/            # NPC state, StateTree, rescue, LOD
    WorldResponse/        # reputation + alert + dispatch
    World/                # PCG districts/streaming/planet bodies
    Networking/           # authority, replication, ownership
    Save/                 # region snapshots & options
  Content/                # UE assets with verified rights
  Config/
  Tests/
~~~
Iniciar en un solo módulo de gameplay organizado en carpetas C++ y Blueprints; NO crear una docena de plugins antes de probar. Usar Data Assets para parámetros de movimiento, daño, alertas, karma y spawn; Gameplay Tags para categorías/estados. GAS completo solo cuando la complejidad de habilidades y red lo justifique, no prerequisito de V0.

### Sistema de eventos compartidos
~~~mermaid
flowchart TD
 A[Input / AI intent] --> B[Rules & validation]
 B --> C[Flight / Grab / Combat / Destruction]
 C --> D[Normalized gameplay incident]
 D --> E[Ledger: attribution + dedup]
 E --> F[District alert 0-5]
 E --> G[Player / region karma]
 E --> H[State persistence]
 F --> I[Civilians & Response Director]
 G --> J[Presentation]
 I --> J
 H --> K[GameState / PlayerState authoritative snapshots]
~~~
Separar intención -> validación -> estado jugable -> VFX. Efectos visuales no cambian el karma. El cliente no puede decidir daño/teletransporte/agarre finales.
- Estado del objeto: ObjectId, HolderPlayerId, estado Libre/Sujeto/Lanzado, secuencia/revisión; transporte barre colisión también para volumen del carro.
- Destrucción: PartId, estado intacto/dañado/ausente, revisión; Chaos genera fragmentos limitados, solo algunos son jugables; huecos y colisiones se reconstruyen desde estado autoritativo.
- UImpactWorldEventSubsystem es concepto local, no objeto replicado por sí solo; datos autoritativos se sincronizan mediante GameState/PlayerState/actores cuando exista cooperativo.
- Alert/Karma consumen IncidentId/CausationId para no penalizar por cada partícula del mismo choque; se guardan por jugador/distrito y separan amenazas enemigas de acciones del héroe.
- Planetas: BodyId + RegionId + coordenadas globales de precisión adecuada, origen local para física; LOD desde distancia y zonas jugables limitadas; no mantener varios mundos físicamente simulados al mismo tiempo.

## 6. Perfil de hardware, VRAM y CPU
- RAM64GB > recomendación32GB; la VRAM 8GB es distinta y puede agotar recursos de texturas/sombras/partículas. Vulkan Linux es sensible a baja VRAM según Epic.
- Empezar 1080p con perfil Medium o Low; sin ray tracing/Lumen hardware forzados; medir qué aporta Nanite selectivo, TSR/escalado y HLOD; conservar preset FPS.
- Presupuesto provisional de ensayo ~5.5–6.5GB para uso de juego si driver/monitor/SO lo permiten, NO promesa de capacidad; medir reserva real y picos. No compilar dos Unreal Editors pesados a la vez.
- Ciudadanos de prueba: TierA 6–12 interactivos, TierB 15–35 simplificados y TierC 60–120 distantes; evaluar con 0/4/8/12 A y no asumir que CPU desconocida los soporta.
- Chaos: 1 colapso importante por instancia de prueba, pocos fragmentos activos y piezas elegibles; no todos los cristales físicos.
- Antes de fijar cifras: CPU, CPU cores/threads, Linux distro, driver NVIDIA, versión Vulkan, almacenamiento libre/SSD, versión del motor, rendimiento medido.

## 7. Fases/puertas y dependencia
~~~text
L00 Revisión Linux + paquete base
  -> F01 Vuelo y cámara
  -> C01 4-8 manzanas densas (primero 1 manzana)
  -> P01 Agarre carro/persona/objeto
  -> D01 Fachada perforable
  -> E01 Robot modular y rival aéreo
  -> S01 Sandbox (spawner/reinicio/audio + playtest)
  -> U00 Event ledger y causas
  -> U01 Ciudadanos
  -> U02 Alertas
  -> U03 Karma
  -> U04 Densidad/LOD/Mass solo si necesario
  -> H02 Genio cósmico, H03 Teleport y K01 Kaiju
  -> W01 Orbita, W02 Luna+2 mundos
  -> N02 Cooperativo LAN jugable (pruebas N0/N1 tempranas desde F01/P01)
~~~
El orden puede adaptar investigación independiente, pero NO bloquearemos F01 con sistemas de mundos ni U04 con Mass. La versión más divertida y estable tiene prioridad sobre añadir contenido.
P01: 30 agarres/transportes/lanzamientos; D01: 50 impactos, agujero transitable, reset; U00: 30 fragmentos no crean 30 sanciones; U01: civil huye a refugio; U02: alerta crece/decae y se limita respuesta; U03: rescates/daños sin farming; N0/N1: dos instancias validadas; S01: sesión humana >=4/5 en control/velocidad/diversión.

## 8. Definition of Ready (DoR) / Definition of Done (DoD)
**DoR**: ID y objetivo, dueño de tarea, archivo/rama, dependencias, fuente/licencia si aplica, aceptación observable, límite de intervención, rollback y entorno disponible.
**DoD**: commit/PR, construcción y ejecución Linux si hay runtime, tests y logs, capturas/vídeo cuando afecta imagen, versiones/hardware/medición, licencia revisada, reporte QA PASS/BLOCKED, valoración de usuario si control/diversión. No declarar 80% ni completado por escribir código sin ejecución.

## 9. Workflow repetible
~~~mermaid
flowchart LR
 A[Idea] --> B[Ticket/DoR]
 B --> C[Audit sources/licenses]
 C --> D[Branch]
 D --> E[One small implementation]
 E --> F[Build/test/profile]
 F -->|fail| G[Fix or revert]
 G --> E
 F -->|pass| H[Human playtest if gameplay]
 H -->|rework| G
 H -->|approved| I[Review/merge/main]
 I --> J[Linux package/release note]
~~~
Un agente dueño de main; no editar simultáneamente mismo UAsset/umap en ramas concurrentes. Publicar un build anterior estable. No modificar sistema local, permisos, VPN, credenciales ni drivers sin autorización. Regla de parada: si integrar un paquete exige migrar un framework completo o es ilegal/incompatible, marcar BLOCKED y usar alternativa mínima nativa.

## 10. Skills de Codex planificadas (archivos se crean aparte)
- impact-license-audit: fuentes, licencias, packs y dependencias.
- impact-linux-gate: diagnóstico nativo, build/package/Vulkan.
- impact-gameplay-gate: integración de una mecánica, QA y revert.
- impact-civic-systems: ciudadanos, incidente, karma, alerta y LOD.
- impact-qa-review: pruebas automatizadas y reporte honesto.
AGENTS.md = principios del repo; docs = arquitectura; skill = procedimiento reusable; issue = instrucción concreta. Skills no conceden permisos ni ejecutan código solas.

## 11. Herramientas y límites
Unreal estable Linux + C++/Blueprint/Enhanced Input/StateTree/SmartObjects/NavMesh/PCG/WorldPartition/Chaos/Niagara/Unreal Insights/Automation. MassEntity/MassGameplay/MassCrowd/ZoneGraph son experimentales en partes 5.8; **no dependencia obligatoria**. Blender gratis opcional para mallas modulares, Git/GitHub, Codex CLI; Unreal MCP experimental local solo si verificado. Quaternius/Kenney Standard gratuitas y licencias por descarga; REPIRK MIT si compila Linux. Sin servidores pagos, IA runtime, assets de franquicias ni decenas de plugins.

## 12. Entradas faltantes antes de gameplay
Falta confirmar **CPU, distribución Linux, SSD/espacio, versión driver NVIDIA/Vulkan, estado de instalación Unreal/SDK**. No preguntar de nuevo por RAM/GPU: 64GB RAM + 4070 8GB se declararon aquí. Antes de empezar: aprobación de dirección creativa de modos Creativo/Ciudad Viva/Desafío y los contratos karma/alerta; se puede planificar usando los valores propuestos sin esperar respuestas. Después ejecutar L00.
