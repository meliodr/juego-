# IMPACT: BEYOND — Ciudadanos, alertas, karma y respuesta del mundo (v0.6)
Fecha: 2026-11-14. **ESTADO: diseño propuesto, no implementado ni ensayado**. Linux, RTX 4070 8 GB VRAM y 64 GB RAM declaradas por el usuario; CPU, distribución Linux, controlador Vulkan, SSD y versión concreta de Unreal aún sin comprobar. Sigue sin existir un paquete jugable.

## 1. Objetivo
Que la ciudad sea **un lugar vivo para explorar y transformar**, no una ciudad vacía ni un sistema policial que impida divertirse. Los ciudadanos pasean, trabajan, usan zonas de actividad, reaccionan a golpes/kaijus/robots, alertan y pueden ser auxiliados; el mundo recuerda cómo te comportas. El sandbox ofrece libertad absoluta de movimiento y destrucción controlada por modos de juego.

Tres sistemas diferentes:
1. **Alerta local (Heat/Alarm, 0–5):** urgencia y respuesta operativa de esta zona, temporal y sensible a presencia/testigos.
2. **Karma/Reputación (−100..100):** percepción del jugador en una región y un resumen global, persistente entre sesiones si se activa guardado.
3. **Amenaza mundial/incidente:** gravedad objetiva de un robot/kaiju/superhumano o desastre. Un kaiju NO se culpa automáticamente al héroe ni convierte al héroe en perseguido.
Separar karma y alerta evita castigar al jugador que salva ciudadanos durante una pelea destructiva.

## 2. Modos, prioridad de libertad
- **Creativo / Caos:** generar NPC, enemigos, vehículos y estructuras; karma y respuesta institucional desactivables por separado; vida/energía infinita, reinicio de sectores. No cortar vuelo ni poderes. Predeterminado para primeras sesiones de prueba.
- **Ciudad viva:** ciudadanos, testigos, reputación y alertas encendidas, respuesta narrativa sin bloqueo de poderes.
- **Desafío:** reglas más exigentes, respuesta coordinada y retos de evacuación/rescate; sigue siendo sin campaña obligatoria.
Destrucción del mundo configurable: normal, parcialmente regenerable o reset por zona. Un solo interruptor no debe cambiar daño a NPC y daño de estructuras de forma confusa.

## 3. Sistema de alerta, niveles y contrajuego
Heat interno 0..100 por **DistrictId**, visualizado en niveles ordinales. Umbrales iniciales para pruebas: 0 = [0,10), 1 = [10,25), 2 = [25,40), 3 = [40,60), 4 = [60,80), 5 = [80,100]. Ajustables por Data Asset, nunca hardcoded en UI.
| Nivel | Respuesta visible sin frustración |
|---|---|
| 0 Tranquilo | Rutinas normales, radio ambiental, calles abiertas |
| 1 Alerta vecinal | Miradas, grabaciones/llamadas simuladas, peatones se alejan |
| 2 Emergencia | Civiles buscan cobertura/evacúan; marcadores discretos; tráfico intenta evitar zona |
| 3 Contención ligera | Drones/unidades ordinarias se acercan SOLO a evento detectado; no spawns instantáneos frente a cámara |
| 4 Respuesta especializada | Robots de contención, prioridad de rescate de civiles, rutas evacuadas y emisores de advertencia |
| 5 Crisis | Unidad élite/heavy desplegada con aviso previo; amenazas independientes (kaiju) pueden coexistir, sin generarse siempre por culpa del jugador |

Heat depende de eventos **atribuidos** (root instigator), gravedad, zona, confirmación de testigo/sensor, persistencia y cooldown. Para evitar omnisciencia, respuestas necesitan reporte validado, observador o sensor confiable; testigos pueden correr primero y reportar después. Eventos explícitos de ataques en zonas de emergencia pueden elevar alerta inmediatamente aunque no haya ciudadano visible si un sensor existe.
No castigar por velocidad sin daño ni por volar normalmente. Ante una zona peligrosa, los ciudadanos deben reaccionar a amenaza enemiga sin convertirlo en agresión del héroe.
Decaimiento con tiempo seguro/sin ataques, y modificación por refugiarse o salir del distrito. No reset instantáneo al cruzar una esquina. Sin oleadas infinitas obligatorias: disparadores con presupuesto, cooldown y opción de desactivar respuesta.
Evitar que una explosión con 20 fragmentos compute 20 incidentes completos: agrupar/capsular eventos mediante **IncidentId / CausationId**, origen del impulso, ventana temporal y límites. Dos jugadores: alerta compartida por distrito, contribuciones por jugador, ninguna doble atribución por replicación.

## 4. Karma: reputación con ciudadanos (no árbol moral)
Valor por jugador y región, [-100,+100], opcional resumen global ponderado. El estado y sus reglas deben ser datos, no etiquetas fijas de bueno/malo.
| Evento | Cambio propuesto inicial | Condiciones |
|---|---:|---|
| Ayudar a un civil en peligro real | +5 | una sola vez por IncidentId; peligro preexistente y no fabricado para explotar sistema |
| Evacuar/rescatar un grupo | +8 | recompensa limitada por evento, no +8 por persona |
| Defender un sector de robot/kaiju | +6 | resultado confirmado y contribución relevante; no solo estar cerca |
| Dañar propiedad no implicada | −2 a −8 | escalar por gravedad, causa atribuida y contexto de pelea |
| Poner un civil en peligro por acción propia | −8 | distinguir daño potencial y daño confirmado |
| Agredir deliberadamente a civiles | −15 | respuesta clara, sin detalle gráfico obligatorio |
| Destrucción por enemigo | 0 para el héroe | solo atribuirle la porción causada por él |
| Sacrificio/daño colateral para salvar gente | ponderación contextual | rescates y daños se registran por separado, evitando absolutos |

Factores: **propiedad de causa**, gravedad, advertencia, amenaza activa, testigos, intervención y repetición. Los valores exactos se balancearán tras sesiones humanas. Si el jugador crea el peligro y lo “resuelve” no obtiene karma positivo neto; evitar granjas de rescates.
Efectos de reputación sobre comportamiento, sin economía obligatoria: ciudadanos se acercan y reconocen al héroe, algunos piden ayuda; si reputación baja se apartan, llaman antes, y autoridades intensifican escrutinio. No eliminar rutas, mundo, poderes o aliados jugables. Persistencia por BodyId/RegionId para planeta principal; otros planetas usan facciones/relaciones apropiadas (no ciudadanos humanos idénticos en todos los cuerpos).
En Creative todo esto opcional; karma visible opcional HUD minimalista. Presets y controles accesibles en menú.

## 5. Ciudadanos por niveles de simulación
Roles iniciales: peatón, trabajador/operario, conductor visible o simulado, testigo, persona que requiere rescate, socorrista NPC opcional. Apariencias seleccionadas de paquetes permitidos o proxies; no crear 50 personajes/modelos únicos para MVP.
**Tres niveles técnicos**:
- **Tier A (cerca/participante):** ACharacter liviano con colisión, respuesta a golpes, opciones rescate/ser transportado, reacción de voz/animación, StateTree sencillo. Pool pequeño y no todos tienen ragdoll completo.
- **Tier B (media distancia):** movimiento simplificado por caminos/ZoneGraph o Mass si funciona en Linux, animación y colisión reducidas, sin habilidades físicas complejas.
- **Tier C (fondo/alta velocidad):** instanced mesh o silueta/grupos animados, ninguna simulación individual a larga distancia y culling al subir a órbita.
Al acercarse o fijar objetivo, **promoción** de representación/identidad B->A; al alejarse, despromoción A->B conservando solo estado importante. No mantener 100 ragdolls como 100 actores completos. Cuando NPC de fondo no sea interactivo, que no aparente ser agarrable a distancia.
**Presupuesto de prueba para GPU de 8GB (NO capacidad garantizada)**: activar al comienzo ~6–12 ciudadanos Tier A, ~15–35 Tier B, ~60–120 Tier C por distrito visible; comparar con 0, 4, 8, 12 A y medir frame time, CPU, draw calls y VRAM. Bajar según CPU no conocido. Decisiones definitivas solo después de profiling. 60 FPS a 1080p es objetivo, no resultado; primero perfil bajo/medio, 4GB–6GB de recursos gráficos propios como presupuesto orientativo con margen a editor/SO/driver, no una asignación exacta del motor.
**AI nativa**: StateTree para comportamiento; Smart Objects para bancos/paradas/refugios/puestos; NavMesh (y ZoneGraph cuando proceda) para caminar; MassEntity/MassGameplay/MassCrowd solo **después de prototipo pequeño** porque MassAI/MassGameplay/ZoneGraph están etiquetados experimental en 5.8. No usar Mass solo para conseguir 10 peatones.
**Comportamiento StateTree**: Idle -> Wander/UseSmartObject -> NoticeThreat -> FleeToShelter -> Wait/Report -> Resume, con rama Aid/NeedsRescue. No teletransportar ciudadanos fuera de vista para fingir una evacuación sin contabilizar lo que significa para el juego. Para rendimiento sí se puede desactivar la simulación lejos de jugadores sin efecto jugable.
**Percepción**: amenazas locales con radio/oclusiones, audición de impactos/sirenas, aviso de testigos; no consulta global de posición perfecta al héroe.
**Rescate**: agarrar a un ciudadano herido para llevarlo a zona segura, o despejar ruta; control asistido para que transporte sea estable a velocidad moderada; no forzar a tratar un NPC como arma. En modo caos pueden existir física y golpes estilizados, con repercusiones de karma opcionales.

## 6. Sistema de incidentes: una única fuente de verdad
Propuesta de struct:
- IncidentId/Sequence, Timestamp, BodyId, RegionId, DistrictId
- InstigatorActorId, OwnerPlayerId, SourceObjectId, SourceAbilityId, CausationId (provenance limitada), VictimId/GroupId
- Category (property, threat, rescue, civilian_harm, robot_attack, kaiju_attack), Severity, Position, Witnessed(bool/confidence), AuthorityConfirmed
- ScoreDeltaAlert, ScoreDeltaKarma
**Pipeline**: Hit/AI event -> ImpactEventNormalizer (agrupa fragmentos/frames) -> AttributionResolver -> IncidentLedger (dedup, autoridad) -> AlertSubsystem y ReputationSubsystem -> CivilianReaction/ResponseDirector -> presentación HUD/sonidos. Separar causas concretas de polvo/partículas; FX nunca calcula karma.
Para desarrollo y cooperativo: daños y incidentes relevantes nacen/validan en autoridad; clientes solo muestran estado y VFX, no crean doble recompensa. Historial corto en memoria (TTL), snapshot regional persistente; no almacenar cada cristal roto para siempre.

## 7. Ejemplo de simulación
Un robot ataca un rascacielos: amenaza objetiva sube; la gente cerca huye; **el karma del jugador no cae**. H01 lanza un carro contra el robot, abre una fachada y provoca daño colateral moderado: incidente atribuido a H01; alerta local puede subir, karma ajusta según daño. H02 crea escudo y salva grupo: una bonificación por grupo validado. Si el incidente atrae unidad de contención, puede reconocer quién inició el peligro; no necesariamente atacar a ambos héroes. H03 teletransporta para rescatar civil desde azotea: la zona debe estar precargada y el destino validado.
Si alertas están desactivadas, la pelea sigue siendo totalmente jugable.

## 8. Plan de entrega del sistema urbano
- **U00: datos + depuración sin NPC**: eventos de 2 carros, 1 pared y 1 robot; ledger y pruebas de origen/dedup. HUD de debug. Aún sin alertas visibles.
- **U01: ciudadano básico**: 3–6 NPC, StateTree caminar/huir, SmartObject refugio, detectar golpe; probar Linux en mapa pequeño.
- **U02: alerta local**: 0..5, testigo/sensor, decaimiento, spawner con techo; depuración.
- **U03: karma**: rescates reales, daños propios/enemigo, guardado; no farming.
- **U04: densidad**: promover/despromover NPC y evaluar Mass si lo justifica la medición.
- **U05: dos jugadores**: IncidentId y karma individual, alerta compartida, cliente tardío sincroniza estado; pasar antes de activar multijugador ampliado.
No poner U04 por delante del vuelo / carro / pared: población no debe retrasar el sandbox físico.

## 9. QA / aceptación
- INC-01: 30 daños con escombros provenientes de un único impacto -> un incidente lógico con causa apropiada, no 30 puntuaciones.
- INC-02: daño de kaiju sin acto del héroe -> alerta/emergencia sí, karma del héroe sin cambio.
- INC-03: jugador causa peligro, rescata mismo objetivo -> no puede ganar karma ilimitado; repetir 20 veces.
- INC-04: 20 casos de daño colateral/evacuación/acción heroica, con deltas esperados registrados.
- NPC-01: peatón detecta amenaza y busca refugio válido; no corre hacia zona de daño ni queda atrapado indefinidamente; restauración al terminar peligro.
- NPC-02: promoción/despromoción no duplica ciudadanos ni memoria; al desactivar física fuera de zona, nadie queda permanentemente agarrado.
- ALERT-01: escalada, cooldown, respuesta, salida de distrito, vuelta; no aparición visible instantánea ni oleada sin presupuesto.
- COOP-INC: dos jugadores causan un mismo evento; sin doble penalidad, alerta consistente y sincronización de reputación por actor.
- PERF-CIV: comparar perfiles sin/6/12 ciudadanos Tier A, con diferentes cantidades B/C, incluyendo combate y Chaos; medir p95/p99 CPU/GPU y VRAM sobre Linux 4070 8GB, RAM64GB con CPU real registrado.
Pruebas observables y logs: no afirmar gates aprobados por escribir documentos.

## 10. Fuentes técnicas actuales
- UE StateTree: https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-state-tree-in-unreal-engine
- UE SmartObjects: https://dev.epicgames.com/documentation/en-us/unreal-engine/smart-objects-in-unreal-engine---overview
- UE MassGameplay overview: https://dev.epicgames.com/documentation/unreal-engine/overview-of-mass-gameplay-in-unreal-engine?lang=es-ES
- UE MassAI experimental 5.8: https://dev.epicgames.com/documentation/unreal-engine/API/PluginIndex/MassAI
- UE MassCrowd plugin: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/MassCrowd
- UE Linux requirements: https://dev.epicgames.com/documentation/unreal-engine/linux-development-requirements-for-unreal-engine?lang=en-US
