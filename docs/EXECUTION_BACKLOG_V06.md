# Actualización v1.0: caminar/correr + transición a vuelo

**OBLIGATORIO**: tras L00 y FLT-AUD, antes de cerrar F01, tener mismo personaje **Idle/Walk/Jog/Sprint/Jump/Fall/Takeoff/Hover/Cruise/Sonic/Brake/Land/Recover**. No dos personajes para tierra y aire. Fuente: docs/GROUND_FLIGHT_CREATORS_V10.md. Mantener co-op deferred según v0.9.

- **IB-GND00 (P0)**: Third Person Combat base; añadir caminar y sprint según DataAssets, BlendSpace, arranques/paradas simples y giro, sin importar ALS completo. Gate GND-01/02: 10min y 30 secuencias terrestres.
- **IB-GND01 (P0)**: ManOfSteel Flight/Lean parcial con locomoción terrestre; transiciones Run/Jump/Flight/Land/Run, 30 ciclos en calles y techos. Gate GND-03.
- **IB-GND02 (P1)**: si hay deslizamiento/snaps, GASP 5.8 para clips de start/stop/pivot bajo licencia permitida, Gorka Games/Matt Aspland como referencias; gate GND-06 y p95/p99.
- **IB-GND03 (P1)**: con P01, caminar/correr/volar con objeto agarrado y distintas masas, IK controlado; gate GND-04.

Ningún tutorial o GitHub sin LICENSE autoriza copiar assets. Proyecto L00 sigue sin ejecutarse en Linux.

# Actualización de alcance v0.9 — LEER ANTES DEL BACKLOG ANTERIOR

El usuario pospone cooperativo a un horizonte muy lejano. Todos los tickets N0/N1/N02 quedan **DEFERRED y excluidos de prioridades actuales**, aunque aparezcan con P0/P2 en tablas históricas de abajo. No ejecutar test de dos instancias, replicación, sesión LAN, Red o servidor antes de nueva solicitud. Esto anula requisitos de red para cerrar F01/P01/D01.

Prioridad nueva de vuelo: FLT-AUD (ManOfSteel Flight.cpp, LFS/licencias) -> FLT-BASE (proyecto Linux CMC + lean/camera MIT mínima) -> FLT-ANIM (1 rig con poses permitidas) -> FLT-QA (10min vuelo y 20 transiciones reales). Documentación: docs/FLIGHT_REUSE_AUDIT_V09.md. L00 Linux sigue siendo primer gate de ejecución. Después ciudad, carro, pared, enemigos, sistema cívico.

# IMPACT: BEYOND — Backlog de inicio y dependencias v0.6
Estado: listo para **revisión del plan**, NO se autoriza por sí mismo ejecutar el motor en la máquina del usuario. Equipo informado: 64GB RAM + RTX4070 8GB VRAM. Datos pendientes: CPU, Linux distro/kernel, driver NVIDIA/Vulkan, Unreal instalado, SSD/espacio.

## Regla de prioridad
P0 = debe quedar jugable antes de expandir; P1 = después del bucle físico; P2 = expansión. Trabajar un ticket con cambio técnico principal por vez. Los experimentos de red N0/N1 se ejecutan tempranamente en laboratorio pero la sala cooperativa no bloquea el sandbox individual. W01 órbita no bloquea los primeros P0.

| ID | P | Resultado exacto | Rol implementa / rol revisa | Dependencia | Cierre verificable |
|---|---|---|---|---|---|
| IB-L00 | P0 | UE Linux base con prueba de GPU/driver y paquete Third Person ejecutable | Codex Integrador / Codex QA | autorización acceso Linux y toolchain | launch/cook/package fuera editor, log GPU+CPU+FPS |
| IB-F01 | P0 | vuelo, boost, freno, cámara, test de colisiones | Codex Gameplay / QA+usuario | L00 | 10min de vuelo, pruebas FPS variados, valoración de control |
| IB-N0 | P0-investigación | dos instancias con 2 héroes volando en mapa gris | Codex QA/Networking | F01 | posiciones estables, autoridad de input y cámara de cada jugador |
| IB-C01 | P0 | 1–4 manzanas urbanas densas con rutas verticales y visual LOD | Codex World / QA+usuario | L00,F01 | vuelo/carga sin paredes faltantes; assets licenciados |
| IB-P01 | P0 | carro agarrable, transportar, soltar y lanzar en suelo/aire | Codex Gameplay / QA | F01 | 30 ciclos, carga no atraviesa paredes |
| IB-D01 | P0 | fachada perforable con hueco y grandes fragmentos agarrables | Codex Gameplay/Destruction / QA | P01,C01 | 50 impactos, reset, colisión real, memory |
| IB-N1 | P0-investigación | dos clientes ven mismo estado carro/hueco | Codex QA/Networking | P01,D01 | un portador único y estado replicado |
| IB-E01 | P1 | robot con brazo/propulsor desarmable + rival superhumano de vuelo | Codex Gameplay/AI / QA | D01 | desarme altera ataque; combate con recuperación |
| IB-S01 | P1 | spawner, modos Creativo, reinicio, sonido, sandbox jugable | Codex Gameplay/UI / QA+usuario | E01 | sesión 15min humano >=4/5 en vuelo/control/deseo repetir |
| IB-U00 | P1 | incident ledger con atribución+dedup y test de daño/kaiju/rescate | Codex City / QA | D01,E01 | 30 escombros no puntúan 30 veces |
| IB-U01 | P1 | 3–6 ciudadanos StateTree, huida a 1 refugio, testigo | Codex City / QA+usuario | U00,C01 | amenazas reconocidas, no atasco/caída sin retorno |
| IB-U02 | P1 | alerta local 0–5, decay, respuesta por umbrales, Creativo on/off | Codex City / QA | U01 | alarma no omnisciente, cooldown/spawn limit |
| IB-U03 | P1 | karma regional/persona -100..100, rescate/colateral, anti-farming | Codex City / QA | U00,U02 | daños de robot no penalizan héroe, rescue no farming |
| IB-H02 | P1 | genio esmeralda: escudo, mano, puño/martillo/pinza prefabricados | Codex Gameplay / QA+usuario | P01,D01,E01 | mismos objetos/muros, control fluido |
| IB-H03 | P1 | blink, evasión, ataque post-blink seguro | Codex Gameplay / QA | F01,C01 | 50 destinos, nunca dentro sólido |
| IB-U04 | P2 | tiers A/B/C, pooling, Mass opcional | Codex City/Perf / QA | U01, GPU profiling | NPC counts/frametime documentados |
| IB-K01 | P2 | kaiju mediano con 3 piezas separables y cambios de habilidad | Codex Enemies / QA | E01,D01 | 3 desmontajes y test de reset |
| IB-N02 | P2 | 2 jugadores LAN PvE con spawn/alertas/karma/objetos | Codex Networking / QA+usuario | N0,N1,U03,H02 o H03 | 2 paquetes Linux y test latencia/desconexión |
| IB-W01 | P2 | planeta proxy, atmósfera, Sol visible, órbita y regreso | Codex World/Engine / QA | F01,C01 | 10 viajes ida/vuelta con estado estable |
| IB-W02 | P2 | luna y dos planetas cada uno con área local jugable | Codex World / QA | W01 | aterrizar/explorar/interactuar/regresar; no solo esfera visual |

## Tareas añadidas por v0.7 — animación, arte, sucesos aleatorios

Estos tickets se ejecutan después de la validación técnica necesaria; **no retrasan IB-L00/IB-F01 ni el primer carro y pared**. Fuentes de verdad: ANIMATION_CHARACTER_PIPELINE_V07.md y REFERENCES_DYNAMIC_EVENTS_V07.md.

| ID | Prioridad | Dueño propuesto | Resultado/Dependencia | Aceptación |
|---|---|---|---|---|
| IB-ANIM00 | P1 | Codex auditor recursos + integrador Unreal | tras L00; verificar ZIP Standard Quaternius UBC/UAL, 15 mocaps Rokoko, licencias/rig | manifiesto de archivos concretos, clips, términos, imports de 1 humanoide en Linux |
| IB-ANIM01 | P1 | Codex Gameplay/Arte técnico + QA | tras F01/ANIM00; 6-10 poses Hover/Cruise/Sonic/Brake/Takeoff/Land/Recover con IK de manos | paquete Linux con 20 transiciones sin snaps y 30 agarres; prueba humana de cámara/velocidad |
| IB-ART01 | P1 | Codex Blender/Arte técnico + QA | tras ANIM01; traje original H01 y base compartida, variantes H02/H03 después | arte exportable rigged, sin clipping grave, licencia y rendimiento evaluado |
| IB-ROBOTART | P1 | Codex Arte/Enemies + QA | tras E01 y auditoría Sci-Fi Standard | un robot con 2-3 piezas separables, IA cambia al perderlas |
| IB-KAIJUART | P2 | Codex Creature/Rig + QA | tras K01; Quaternius Monster/Mesh2Motion | una criatura animada y tres sockets/piezas con impacto y colisión; no gore |
| IB-EVT00 | P1 | Codex City/Event Designer + QA | tras S01/U00; director DataAssets, semilla, cooldown, slots y debug | 30 selecciones reproducibles; máximo 1 evento grande y limpieza |
| IB-EVT01 | P1 | Codex City/AI + QA | tras U01/EVT00; robo con ladrón, testigo y huida | evento se detecta, puede ignorarse o detenerse, termina sin pop-in |
| IB-EVT02 | P1 | Codex Gameplay/Destruction + QA | tras D01/EVT00; meteorito visible y cráter prefab con 2-5 piedras | 30 impactos, un IncidentId, suelo válido, colisión cambiada y piezas agarrables |
| IB-EVT03 | P2 | Codex AI/Response + QA | tras E01/U02/EVT00; mini invasión de robots reutilizados | spawn budget, alarma/evacuación, derrota/retirada y reset sin duplicar karma |
| IB-EVT04 | P2 | Codex World/AI + QA | tras K01/W01; kaiju, meteor lunar/espacial, expansión regional | estabilidad y atribución correcta con un evento importante activo |

Los videos de experiencias Roblox/Minecraft son referencias y NO dependen de importación de recursos ARR. La revisión visual de animaciones y clips originales de Rokoko/GASP requiere abrir/inspeccionar los archivos permitidos; su ficha pública no certifica calidad o pertenencia del clip. No generar modelos 3D reales desde imagen sin rigging.

## Tareas de sensación y optimización añadidas (v0.8)
Leer [ANIMATION_FEEL_OPTIMIZATION_V08.md](ANIMATION_FEEL_OPTIMIZATION_V08.md), sin alterar el orden de vuelo->carro->pared. Son criterios de calidad, **no nuevos requisitos bloqueantes para crear L00**.

| ID | Prioridad | Qué se entrega | Después de | Test de aceptación |
|---|---|---|---|---|
| IB-FEEL01 | P0 | 1 héroe con Takeoff/Hover/Cruise/Sonic/Brake/Recover y cámara suave | F01 + ANIM00 | FEL-01/02: 20 cambios sin snaps, 10min de vuelo y valoración 4/5 |
| IB-FEEL02 | P0/P1 | Agarres IK de varios tamaños, golpe ligero/pesado y reacción física parcial | P01/D01 + FEEL01 | FEL-03/04/05/06: 30 agarres y 20 recuperaciones sin camera snaps |
| IB-FEEL03 | P1 | 4 capas de efectos de golpe, polvo, audio/material y shockwave | S01 | FEL-07: legibilidad, cámaras y rendimiento medido, sin 20 partículas de daño duplicado |
| IB-FEEL04 | P2 | Escalado NPC: Animation Budget Allocator/Sharing/LOD si perfiles justifican | U01 + profiling | FEL-09: p95/p99 y CPU/VRAM con 0/4/8/12 ciudadanos Tier A |
| IB-FEEL05 | P1-investigación | Validar hitstop solo visual, cámara local y golpes en red | N0/N1 + FEEL02 | FEL-10: servidor nunca se congela, clients conservan acciones |
| IB-FEEL06 | P2 | Kaiju con peso/anticipación/daño por pieza y cámara | K01 | 3 puntos de desarme con telegraph y frames/VRAM medidos |

La IA puede crear código/AnimBP y preparar clips, pero el usuario evalúa naturalidad, potencia, mareo y repetibilidad. No comprar mocap para cumplir.

## Ticket de implementación IB-L00 listo para ejecutar CUANDO el usuario autorice
**Objetivo:** comprobar hardware/UE/driver y abrir proyecto base Linux mínimo.
**Entradas:** repositorio y acceso real Linux; distro/CPU/SSD desconocidos se deben inspeccionar, no preguntar por RAM/GPU ya dichas.
**Archivos permitidos:** proyecto mínimo de Unreal en carpeta nueva, Docs/QA, Config, Source; no modificar drivers o UE global ni contenido de terceros.
**Pasos:** auditoría lectura sola -> revisar instalación y toolchain -> elegir versión estable compatible -> base Third Person C++ -> compilar -> empaquetar -> ejecutar Vulkan -> medir -> registrar hallazgos.
**No hacer:** importar ciudad/planeta, comprar assets, activar MCP remoto, editar otro repositorio, crear servidores.
**Gates:** build/package/start Linux de verdad, input/mouse/camera, salida limpia, GPU/CPU/VRAM/driver en log; si no hay Linux accesible marcar BLOQUEADO.

## Ticket IB-F01 planificado (no comenzar sin L00)
**Objetivo:** personaje temporal vuela y acelera con control. **Recursos:** CharacterMovement/EnhancedInput nativos. **No hacer:** planetas, multitudes, 3 héroes. **Cierre:** 10 minutos de vuelo, freno, 30/60fps si alcanzables, colisión con estructuras finas, feedback de control del usuario.

## Ticket IB-U00 planificado (sin crear NPC masivos)
**Objetivo:** impacto, daño colateral, rescate y amenaza generan FImpactIncident de causa única. Implementar ledger con IDs y deltas deterministas configurables. Test de 30 fragmentos, daño robot/kaiju y daño del héroe. Sin datos para NPC aún; no generar un sistema de policía completo ni karma visible hasta U02/U03.

## Verificación de Ready antes de empezar (manual)
- [ ] Director creativo revisa y aprueba alcance MVP, tres modos y cómo reaccionará ciudad/karma.
- [ ] Se conoce CPU, Linux/distro, NVIDIA driver, SSD/espacio y estado de Unreal local (auditar en L00).
- [ ] Codex conectado a repo real, con permisos acotados; las cinco skills están disponibles o pueden leerse manualmente desde .agents/skills.
- [ ] La licencia de cada archivo del primer pack está documentada y solo se importan recursos necesarios.
- [ ] No hay tareas gigantes o dependencias de pago ocultas en L00/F01.
- [ ] Se conserva un punto de rollback y formato de reportes PASS/FAIL/BLOCKED.
Nada de esto se marca completado por la mera existencia de este archivo.

## Fuentes de verdad
Ver docs/PRODUCTION_CONTRACT_V06.md, docs/CIVILIANS_ALERTS_KARMA_V06.md, docs/MASTER_PLAN_V05.md, docs/CODEX_QA_PLAYBOOK_V05.md, docs/FREE_RESOURCE_AUDIT_V05.md y AGENTS.md.
