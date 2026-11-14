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
