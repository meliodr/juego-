# IMPACT: BEYOND — Referencias de Roblox/Minecraft y eventos de sandbox v0.7

Fecha 2026-11-14. Estado DOCUMENTADO, NO IMPLEMENTADO/NO EJECUTADO. Investigación de fichas de los creadores, descripción de mods, guías y descripciones de vídeo; NO significa que se haya jugado o auditado el código interno de esas experiencias. Sin permiso de reutilización de archivos de franquicias, nada se importa directamente.

## 1. Referencias verificadas y qué replicaremos CON IMPLEMENTACIÓN PROPIA
| Fuente | Función declarada | Lección para IMPACT | Qué no extrapolar |
|---|---|---|---|
| Roblox Project Power (ficha oficial) https://www.roblox.com/games/103072644129928/Project-Power | Toggle flight, boost, lock-on, dodge, combos Breaker/Clamp tras golpe/agarre, recover | Aerial combat en flujo permanente; golpe->control/agarre->lanzamiento y recuperación rápida. Mantener control jugador. | No copiar scripts/modelos ni afirmar que su balance funciona igual en UE. |
| Roblox Viltrumite Time https://www.roblox.com/games/122170231721791/Invincible-Viltrumite-Time | Fly, dash, grab y lift/throw vehicles por teclas distintas | Objetos del entorno como arma natural; entradas rápidas sin inventarios. | No inventar destrucción total por estar en ficha. |
| Roblox Viltrum Battlegrounds https://www.roblox.com/games/95374615431829/Viltrum-Battlegrounds | Destrucción, vuelo 3D arriba/abajo, dash, levantar y lanzar, mapa configurable | Los usuarios quieren caos sandbox y mapa reiniciable; combate a 3 dimensiones. | Ficha promocional no prueba sistema estructural real ni FPS. |
| Roblox Project Power vídeo 2026-06-07 https://www.youtube.com/watch?v=f4V6S06Qa6I | Descripción/minutaje: invasión viltrumita (02:25-10:00), jefe y viaje nuevo planeta | Eventos invasión escalonados con jefe final; grandes eventos raros separados de combate normal | Video metadata, no código reutilizable; no decir visto fotograma a fotograma. |
| Roblox Project Power vídeo 2026-07-26 https://www.youtube.com/watch?v=Aw4Zgq_sJNk | Showcase nuevo planeta y NPCs, según descripción del creador del vídeo | Minipuntos de interés en mundos pequeños pueden añadir variedad, no mundos sin contenido | Información de descripción, no verificación directa del gameplay. |
| Minecraft Viltrumite Flight https://modrinth.com/mod/viltrumite-flight/ | NONE/HOVER/CRUISE/SONIC, poses, cámara dinámica y sonido, speed lock | Flight FSM sencillo + animación relacionada con velocidad; Sonic cerca de 80% del máximo; libertad con diferentes regímenes | Derechos reservados; no portar Java/code o extraer texturas. |
| Minecraft Viltrumite Reimagined https://www.curseforge.com/minecraft-bedrock/addons/viltrumite-reimagined | Anuncia vuelo con block breaking opcional, grab/carry y efectos | Destrucción opcional/configurable según modo sin cambiar vuelo; interacción inmediata | No afirmar exactitud técnica del addon sin código/permisos. |
| Minecraft Invincible Incorporated https://invincibleinc.net/changelogs/ | Pose/lean/noise drivers, camera FOV/shake, reentry glow, shockwave landing, animation keyframes, NPC y vendettas | El "feel" se fabrica con pose+sonido+FX+camara; eventos pueden crear rivales persistentes | Algunos elementos anunciados en changelog/roadmap. Derechos reservados, solo concepto. |
| Minecraft Fisk's Superheroes https://modrinth.com/mod/fiskheroes | >90 poderes, hero packs modulares, vuelo, teletransporte a Luna | Héroes como configuración de habilidades y animaciones comunes, no tres frameworks | Derechos reservados; no importar mod. |

Los juegos y mods con assets de Marvel/Invincible son referencias educativas, NO licencia para reutilizar su arte/modelos. Cuando estemos listos para evaluación subjetiva, grabar sesiones de 2-3 minutos en los juegos si están disponibles y autorizan jugar; puntuar vuelo, cámara, recuperación, legibilidad/latencia, destrucción. Si juego no está disponible, sólo evaluar documentación y vídeos públicos y declarar límite.

## 2. World Event Director: sin historia, sin spams
Un director por región decide eventos orgánicos: robo, asalto robot, meteorito, invasión, kaiju, accidente/evacuación, rival/duelo. Los eventos NO son misiones obligatorias y jugador puede ignorarlos, sumarse, provocar consecuencias o reiniciar zona.
Cada evento es Data Asset con EventId, BodyId/RegionTags, DistrictTags, ThreatTier, EligibleModes, Cooldown, Weight, MinDistance, MaxActive, SpawnBudget, Prefab/Actors, WitnessRules, Phases, Success/Failure/IgnoreOutcomes y NPC/WorldData dependencies.
Flujo Propuesto: IDLE -> CHECK_WORLD -> SELECT_CANDIDATES -> RESERVE_BUDGET -> PREPARE_REGION -> TELEGRAPH -> ACTIVE -> RESOLVE -> CLEANUP/COOLDOWN. Evento no inicia si streaming/collision destino no está lista; no aparecer enemigos frente a cámara ni generar meteorito en interior por defecto.
Evento se activa con una mezcla de reloj suave, amenaza/karma/testigos, exploración/densidad y enfriamiento, con semilla depurable. Evitar script on-tick que consulta cada objeto; evaluación a intervalos y actores pool.
Respetar 3 modos:
- Creativo: usuario genera manualmente eventos, puede desactivar daños/NPC/alerta y velocidad del tiempo.
- Ciudad viva: aparecen eventos ocasionales sin atascar el vuelo; se mantienen consecuencias y alertas.
- Desafíos: presets concretos o torneos con condiciones.
No convertir karma bajo en invasiones interminables ni castigar a quien combate kaiju. El ThreatDirector decide amenaza independiente; AlertDirector decide respuesta del gobierno/población; Karma atribuye acciones concretas por IncidentId.

## 3. Catálogo de eventos inicial
| EventID | Situación | Fases observables | Reciclaje de sistemas | Karma/alerta |
|---|---|---|---|---|
| EVT-ROB | ROBO / atraco comercial | un ladrón/robot entra a una zona, testigo alerta, huye por calles/azotea, el héroe puede interceptar o ignorar | NPC simple StateTree, SmartObject tienda, IA de huida, 1 marcador contextual | detener/rescatar +karma; alarma de evento, no perseguir héroe si no culpable |
| EVT-MET | METEORITO | aviso luminoso en cielo, trayectoria, ruido, impacto en zona permitida, cráter simulado con panel/mesh dañado, fragmentos grandes usables | proyectil, sonido, Niagara trail, Chaos preparado, incident ledger, spawner de rocas | calamidad externa: 0 karma héroe; daños por lanzamiento secundario sí atribuibles |
| EVT-DRONE | PATRULLA ROBÓTICA / ataque | drones sobrevuelan, fijan objetivos, persiguen en 3D y se retiran/son desarmados | robot/dron existente, Flight AI, desarme módulos | respuesta depende del causante; no spawnear en vista |
| EVT-RAID | INVASIÓN | avistamiento orbital, advertencia, oleada drones 3-6, robot pesado, amenaza élite opcional, evacuación, epílogo | mismos robots/AI/spawners, landing VFX, alarma, destrucción, pocos actores | evento global de amenaza y recompensas por protección; no culpar automáticamente al héroe |
| EVT-KAIJU | EMERGENCIA KAIJU | temblor, aparición en cantera/puerto, ataques contra estructuras, desarme de partes, retirada/caída | kaiju modular, destrucción selectiva, NPC refugiados | severidad de amenaza alta, karma por rescate |
| EVT-RIVAL | SUPERHUMANO HOSTIL | contacto aéreo opcional, persecución por distrito, posible duelo que termina sin misión | rival único, aerocombate, agarre, ragdoll | reputación según daño causado, no automáticamente baja por luchar |
| EVT-CRASH | ACCIDENTE/RESCATE | humo y solicitud, objeto caída/vehículo detenido, traslado a refugio | prefab carro+NPC/SmartObject, ciudad ciudadana | un rescate validado por incidente, anti-farming |
| EVT-STORM | TORMENTA DE ESCOMBROS / basura orbital (posterior) | aviso orbital, fragmentos impactan luna/estación, evasión y destrucción localizada | misma trayectoria y prefab meteorito, soporte BodyId | solo después W01 |
| EVT-CONVOY | CONVOY DE TRANSPORTE (posterior) | convoy robótico escoltado, objetivo móvil, trayectorias de huida, alarma | 2-3 robots, vehículos, ruta, destrucción | no iniciar antes de tráfico básico |
| EVT-DUEL | ENFRENTAMIENTO COOPERATIVO (posterior) | superhumano desafía a ambos, intercepción, huida/return | IA y red ya probadas | alerta y karma individual |

**Prioridad real:** implementar SOLO EVT-ROB (1 humanoide que huye), EVT-MET (proyectil a suelo preparado), EVT-DRONE (enemigo existente) y EVT-RAID pequeño después de S01. Kaiju después del gate K01, otros eventos al reutilizar sistemas comprobados.

## 4. Reglas técnicas para meteorito realista en 8 GB VRAM
No hacer booleanos de terreno universales, campos de cientos de chunks, simulación física orbital de cada fragmento ni incendios volumétricos permanentes.
- elegir zona con sky visibility/espacio libre y receptor preparado;
- Spawn visual de piedra a distancia al evento, trayectoria controlada con sweep y suelo cargado;
- FX estela, sonido y señal de dirección; al choque se aplica un prefab crater/decal/cambio de panel de terreno con colisión actualizada, promueve 2–5 fragmentos grandes como objetos agarrables y emite polvo temporal;
- generar ONE IncidentId con causa meteor y DamagePartIds; civiles huyen, alertas suben por amenaza, karma jugador no cambia sin acción.
- Si la región no permite impacto o está en celda no cargada, rechazar, aplazar o escoger zona alternativa; no permitir atravesar edificio sin efecto/colisión.
- en dos jugadores servidor decide lugar/impacto/piezas; partículas cosméticas locales.

## 5. Presupuesto y frecuencia propuestos, sólo para balance
- Mapa V0: como máximo **un gran evento simultáneo por región**, y uno secundario pequeño si física/IA GPU lo permite; sin eventos grandes durante primeros 2 minutos de prueba de vuelo salvo manual.
- Primer experimento ciudad viva: evento menor cada 3–7 min aproximadamente y mayor cada 12–25 min si el jugador no está bajo presión, con enfriamiento y toggles. Estos son valores de TEST, no diseño definitivo.
- Limitar robots activos por evento (3–6 drones + 1 pesado por invasión inicial) y colapsos grandes (0–1 al mismo tiempo). Los spawns no ocultan edificios ni colisiones.
- Bajo 8GB VRAM: instancias preinstanciadas, pooling, Niagara breve, HLOD, pocos NPC con física real, no 100 meteoritos.
- Lógica de karma/alerta no corre en Tick; usa eventos de daño normalizados y periódico cooldown con tiempo fijo. Estado de distrito se guarda por región solo cuando importa.

## 6. Arquitectura propuesta
- UImpactWorldEventDirectorSubsystem (o Actor Coordinator por Level, decidir en implementación): no generador de historia, administra spawn budgets, pools, slots y reglas por zona.
- UImpactEventDataAsset: tipos, requisitos, phases, scores, assets y frecuencias por modo.
- FImpactEventInstance: EventId, InstanceGuid, DistrictId, BodyId, Phase, Participants, Seed, StartTime, Instigator, OriginatingIncidentId.
- Interfaz IWorldEventParticipant para enemigo/NPC, que recibe OnEventStarted/Phase/Completed, no crea una nueva IA por cada evento.
- Reutilizar StateTree/SmartObjects para testigo, ladrón, peatón y protector; usar NavMesh caminable para robo, steering 3D para drones, nunca asumir NavMesh sirve para volar.
- Reutilizar UImpactIncidentLedger + UImpactDistrictAlert + UImpactReputation: amenaza/alarma/karma independientes. Todos con IDs estables/dedup.
- Replicación futura: servidor crea/termina eventos, envía fase+resultados, daños relevantes por actores, cliente crea VFX; snapshots para unión tardía. No sincronizar cada partícula.
- Telemetría: IncidentsPerMinute, EventsStarted/Cancelled, EventBudget, FpsP99, VRAM, NPCActive, ChaosActive, TimeToSafeLoad.

## 7. QA obligatoria
- EVT-01 30 selecciones con semilla fija: pesos y cooldown comprensibles; no 2 grandes simultáneos si presupuesto=1.
- EVT-02 meteor en suelo/edificio/pared/techo: solo zona válida, agujero/cráter coherente, 30 impactos sin colisión fantasma, restos agarrables.
- EVT-03 robo: huida visible, ciudadano testigo, oportunidad de intervenir, fin aunque jugador ignore, sin teleport delante del jugador.
- EVT-04 invasión: oleadas respetan tope, respuesta de policía no culpa jugador, derrota/ignorar/cancelar limpian actores.
- EVT-05 1 meteor produce 1 incidente lógico pese a N escombros; karma de jugador sin cambio salvo su acción.
- EVT-06 mientras vuelo rápido y en órbita no spawnear grupos en celdas no listas; al irse región suspend/coalesce sin perder estado.
- EVT-07 modo creativo sin alertas sigue permitiendo invasión manual y reset de edificios.
- EVT-08 en dos jugadores un solo InstanceGuid compartido, no doble meteor/karma/recompensa.
- PERF-EVT: ejecutar en Linux RTX4070 8GB VRAM con preset Medium 1080p, medir p95/p99 y comparar sin eventos/meteor/invasión. No afirmar rendimiento sin test.
- FUN-EVT: usuario puntúa sorpresa, claridad, libertad para ignorar, repetición y variedad; no aprobar eventos solo por compilar.

## 8. Fuentes públicas y condición
Gameplay Roblox/Minecraft URLs arriba; eventos de Project Power video descritos en metadata YouTube, no asumimos ver/comprobar todos sus frames. UE StateTree/Gameplay Tags: https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-tags-in-unreal-engine y https://dev.epicgames.com/documentation/unreal-engine/external-statetree-quick-start-guide . Todo los nombres de clases son contratos propuestos, no C++ existente.

## 9. Tickets después de gate S01/U00
- IB-EVT00: director con lista DataAssets, selección/cooldown/budget y debug en mapa gris, solo sin NPC.
- IB-EVT01: robo con un ladrón, testigo y refugio, sin economía.
- IB-EVT02: meteor con 1 trayectoria, crater prefab y fragmentos usables; sin ciudad entera de caos.
- IB-EVT03: miniinvasión de drones existentes y respuesta de ciudadanos; QA 8 casos.
- IB-EVT04 (posterior): kaiju/estación/orbita y variantes en co-op.
Aceptar solo tras pruebas Linux, mediciones y valoración de diversión. NO implementar eventos todavía si no pasaron vuelo/carro/pared.
