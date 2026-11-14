# IMPACT: BEYOND — Dirección de sensación, animación, epicidad y optimización v0.8

Fecha: 2026-11-14. ESTADO: **DISEÑO Y QA PLANIFICADOS, NO ANIMACIONES IMPLEMENTADAS NI GAMEPLAY PROBADO**. Fuente previa: ANIMATION_CHARACTER_PIPELINE_V07, GAMEPLAY_SPEC, COMBAT_ROSTER_V03, PRODUCTION_CONTRACT_V06, CODEX_QA_PLAYBOOK_V05. Este documento concreta *feel* y rendimiento; no sustituye identidad/controles. Linux nativo; GPU declarada RTX 4070 8 GB VRAM, RAM64GB, CPU/driver Linux no auditados. RD$0 en assets.

## 1. Objetivo perceptual (no confundir con animación espectacular)
El jugador debe poder **despegar -> atravesar la ciudad a enorme velocidad -> frenar -> agarrar un carro/persona -> cambiar de dirección -> lanzar -> perforar una fachada -> perseguir al enemigo -> recibir un golpe -> recuperar vuelo**, en una secuencia sin menús ni cinemáticas obligatorias, sin que la animación arrastre físicamente el pawn fuera de la intención de control. Sensación superheroica original: velocidad brutal con claridad, inercia física comprensible, impactos que cambian el escenario, no copia de Invincible ni de rigs/movimientos protegidos.

Principios:
1. **Inputs mandan, animación sigue.** CharacterMovement/FlightComponent determina posición, trayectoria, colisión y velocidad; AnimationBP consume parámetros y dibuja postura. Animación root motion excepcional solo para ataques y agarres de precisión con ventana/validación limitada; nada de volar con root-motion continuo o SetActorLocation Tick.
2. **La animación no puede bloquear la exploración.** Clips cortos, interrupciones explícitas y recuperación aérea. Combos visualmente contundentes sin quitar cámara/control.
3. **Físicas limitadas y contextuales.** Paradas en sólidos resistentes, atravesar paredes preparadas con energía suficiente, reacción de entorno útil. No ragdoll total en cada roce.
4. **Impactos tienen 3 fases**: anticipación de pose, contacto/sonido/compresión, seguimiento de impulso/polvo/escombros. Evitar partículas y shake que tapen siguiente objetivo.
5. **Calidad concentra recursos cerca.** Héroe local y enemigos en pelea a actualización alta; NPCs lejanos comparten o reducen animaciones; física jugable no depende de si la pose se está actualizando.

## 2. Capas de animación: arquitectura mínima
- **Movement state C++**: Grounded, Takeoff, Hover, Cruise, Sonic, Brake, Land, HitStagger, AirRecovery, Grabbed, Carry, Throw, Blink (H03), Summon (H02). Separar estado locomoción de acción simultánea: no convertir cada combinación en estado único (ej. Sonic+carry+hurt).
- **Animation state**: locomoción base (Ground/Flight), capa aditiva aceleración/lean/bank, capa superior de ataque/agarre/gesto, capa de reacción física parcial, IK de manos/objeto, pose final. AnimBP con propiedades thread-safe/fast path donde proceda, evitar EventGraph pesado por Tick.
- **Control rig / Full Body IK**: para agarrar objeto en sockets conocidos, pies en suelo y reach; solo actores próximos/implicados, no en ciudadanos distantes. Solver necesita coste de CPU medido.
- **Inertialization / Dead Blending**: para transiciones cortas y no extremas; animación muy distinta o cambio rápido repetido requiere una pose intermedia o blend específico. No usar inertialization para arreglar teleports/snap de posición.
- **Motion Warping**: solo animaciones de acercarse/agarrar/golpear en distancia corta y posición validada; no usarlo como autoteleportación ni corregir metros de distancia o atrapar enemigo detrás de muro. Control por gameplay.
- **Physics Control / animación física parcial**: extremidades/hit reaction y transición a ragdoll completo SOLO knockdown/grandes lanzamientos; después pose recuperación alineada a suelo o recuperación aérea. Prioridad del movimiento físico solo una fuente por estado para evitar oscilación.
- **Niagara/Audio/Camera presentation**: recibe eventos del estado de juego, no decide daño ni desplazamiento. En cooperativo no activar slow-motion global; hitstop, si se usa, es cosmético y local, no afecta física/replicación.
- **Retarget**: 1 esqueleto bípede para H01/H02/H03 si compatible, movimientos Quaternius/Rokoko autorizados, animaciones bakeadas/retargeteadas antes de runtime donde compense. Donante GASP 5.8 aislado y términos Fab/NoAI revisados antes del flujo IA.

## 3. Hoja de poses concretas
Primer set (TODO por verificar disponibilidad en paquetes; crear manualmente solo faltantes). No afirmar contenido de Rokoko/Mixamo sin revisar ZIP.
| Categoría | Poses/clips mínimos | Naturalidad |
|---|---|---|
| Tierra | idle respiración, 8-way locomotion via blend, sprint, jump, land, turn start/stop | péndulo torso/contrapeso, pie sin deslizarse, no root motion descontrolado |
| Vuelo | takeoff corto, hover respiración, cruise adelantado, sonic aerodinámico, brake brazos abiertos, bank lateral aditivo, ascender/descender pose derivada, landing, aerial recover | velocidad/acceleración reales alimentan lean y aleteo inexistente: fuerza sin propulsores físicos |
| Combate | jab, hook, uppercut, heavy windup/strike/follow-through, air dash hit, block/evasion, whiff/recovery, receive hit light/heavy | contacto precede reacciones de daño, apuntado y telegraph para contrajuego |
| Agarre | acercar mano, acoplar sockets, carry ligero/pesado, lanzar corto/throw charged, soltar | IK al punto del objeto; cambiar peso/no clipping; sujeción física controlada |
| Estados y derrota | knockback, flinch por parte, airborne ragdoll limitado, tumble, recover en suelo/aire | sin quedarse inmóvil, evitar combos de stun infinitos |
| H02 | gesto abrir mano, cierre, invocar doble brazo, escudo, pinza, martillo | cuerpo más erguido y majestuoso; constructos como mallas/VFX independientes, no otros 6 brazos de skeletal rig todo el tiempo |
| H03 | desaparición/desfase breve, reaparecer, contra-blink, golpe de salida | teleport validado por game logic, shaders y motion pose cortos, cámara fluye sin posiciones inválidas |
| Robot | idle/sensor, walk/fly, ataque, desequilibrio, módulo roto, recuperación | piezas del modelo/sockets, al perder propulsor cancela vuelo/ajusta IA |
| Kaiju | resoplar, walk pesado, anticipación golpe/cola, derribo, resistir, reacción al perder cuerno/placa/cola | masa sugerida con anticipación/postura/effect antes que cientos de huesos en Chaos |
| Ciudadano | idle/walk/run/flee/look, shelter, pedir ayuda, recuperación | sin FBIK/física completa salvo NPC interactivo cercano |

Primer héroe necesita aprox. **6–10 loops/poses de vuelo** y **15–25 clips humanos/combate cuidadosamente elegidos**, no 300 clips nuevos. El ajuste de torso, cuello, brazos, piernas y pelvis puede ser aditivo, gobernado por curvas de impulso/aceleración.

## 4. Transiciones naturales y control responsivo
| Situación | Comportamiento deseado |
|---|---|
| Ground -> Takeoff -> Hover | input de vuelo inicia inmediatamente movimiento autorizado; despegue visual acompaña, no bloquea 1s hasta que termine un clip |
| Hover -> Boost/Sonic | suavizar aceleración física y transición visual, FOV/audio/estela aumentan con parámetros reales |
| Sonic -> Brake | frenar con deceleración progresiva, torso se levanta y brazos se abren, nunca speed snap invisible |
| Sonic -> Grab | necesita alcance, precisión o asistencia limitada; no snap/warp al enemigo detrás de objeto |
| Carry -> Turn | objeto y volumen colisionan; pose ajusta masa y giro, no oscila resorte en alta velocidad |
| Punch -> Recover/Next combo | ventanas de cancelación claras; buffer de input, no 0.8 s de bloqueo sin feedback |
| Hit -> Air Recovery | blend reacción con movilidad recuperable y resistencia a stun encadenado |
| Blink -> Exit Attack | destination collision loaded and clear, pose desfasada se dibuja en destino seguro, no cámara de cortes locos |

**Parámetros para primer ensayo (NO valores finales)**: buffer de input 80–140 ms, blend inercial 80–180 ms en transiciones compatibles, giro de torso 5–15 grados proporcional al empuje lateral antes de limitar por calidad. Ataques ligeros tienen menor anticipación/recuperación que pesados. Audio/FX/camera con 10–30 ms tolerancia de desfase a probar por captura a 60fps. No trasladar valores a red sin medir.
**Hitstop**: probar 25–50 ms de compresión VISUAL en impacto pesado para jugador local; nunca Freeze World/SetGlobalTimeDilation para gameplay/Co-op. Efecto puede ser microdesaceleración/pose de ataque y shake, no tocar simulación. Ofrecer desactivar shake/hitstop.
**Contactos**: línea/shape trace ocurre en ventana definida por gameplay; se puede adaptar anim montage con notify, pero daño final solo una vez por AttackId/TargetId/phase. El efecto sigue la confirmación del hit, no dispara 15 eventos en un mismo frame de fragmentos.
**Camera anti-motion sickness**: cámara suave con anticipación visual (look-ahead) e inclinación del personaje desacoplada; horizon local estable, roll cámara pequeño o nulo por defecto, FOV clamp configurable, motion blur OFF al inicio, shake en impulso corto y amplitud ajustable; objeto sostenido no puede tapar retícula.

## 5. Epicidad diseñada, no una cinemática costosa por golpe
Se definen **cinco escalas** con VFX/SFX/physics variables por material y energía, cada cual con presupuesto propio:
1. **Contacto**: ligero sonido/material y reacción de torso/arma; sin destruir estructura.
2. **Golpe potente**: desplazamiento claro, polvo o chispas, estela y micro-reacción de cámara local.
3. **Impacto estructural**: ruptura de panel preparado; hueco real, 2–5 fragmentos significativos reutilizables, audio de concreto y nube breve.
4. **Acontecimiento de entorno**: un elemento grande/columna/robot pesado se desarma, colapso parcial controlado, alertas y peatones huyen.
5. **Momento raro de gran escala**: meteorito, invasión o kaiju grande, prefabs y streams preparados, sólo uno principal activo por región hasta medir; no simular toda ciudad.

**Regla visual**: la lectura de rival, cámara y trayectoria del objeto gana sobre espectacularidad. La nube de polvo se desvanece rápidamente si oculta una acción. Diseñar limitadores de efectos por cercanía (Niagara Effect Types) y colisiones físicas separadas de partículas.
**Tres momentos "WOW" para test humano**:
- embestir a rival -> atravesar dos fachadas preparadas -> recuperar vuelo sin corte;
- levantar un carro y lanzarlo desde arriba -> destruir muro -> agarrar un escombro de lo que se rompió;
- kaiju pierde un módulo/placa y cae -> el jugador usa la pieza mientras los ciudadanos escapan.
No prometer interacciones triples hasta que cada primitiva pase su gate.

## 6. Firma de animación por personaje
- **H01 (fuerza)**: peso hacia delante, cuerpos rectos con puños cerrados, aceleración que cambia la silueta, frenado agresivo, embestidas, agarres de cintura/pierna/hombro con IK. Golpes "pesados" por seguimiento del cuerpo y empuje, no largos time-freezes.
- **H02 (genio esmeralda)**: vuelo más erguido, brazos abiertos/calma antes de gestos precisos; manos espectrales aparecen a partir de gestos cortos; energía cristalina clara, no saturada en cada golpe; Martillo, Pinza y Escudo como implementaciones del mismo sistema de daño/agarre.
- **H03 (blink)**: anticipación mínima, silueta angular, pose de desaparece/reaparece, 2–3 cuadros de estela visual adaptables; al reaparecer conserva orientación/inercia permitida; teleports con validación y daño solo al golpe confirmado.
- **Robots**: interrupción de movimiento al perder pieza, respuesta mecánica distinta por armamento, humo/chispas discretos, fragmentos físicos seleccionados.
- **Kaijus**: anticipaciones grandes y lectura clara para esquiva, impactos más lentos de masa alta; no ragdoll de todo el rig a máxima resolución; cues de audio bajos y cámara dinámica sólo cerca, no juego de Quick Time Events.

## 7. Optimización por jerarquía de importancia
No optimizar "menos animaciones = más FPS"; medir coste CPU animación, Game Thread, Render Thread, GPU, Chaos y VRAM por separado.
**Prioridad de actualización visual**:
- Tier 0 jugador local, compañero en pelea y enemigo objetivo: AnimationBP/IK/reacción visual máxima dentro presupuesto, preferentemente cada frame render; hitboxes/collision always correct.
- Tier 1 rivales próximos y NPC interactivos: animación normal, IK sólo cuando realizan interacción real, costes escalados.
- Tier 2 peatones medianos: simplified AnimBP, retarget bakeado, Animation Sharing/Animation Budget Allocator cuando exista número que lo justifique, menores tasas.
- Tier 3 peatones/distantes: instancia/impostor/LOD visual, sin física individual ni IK, animaciones compartidas/culling.
**Anim optimizations**: preferir thread-safe functions/Fast Path sobre EventGraph Tick pesado; Rewind Debugger/Animation Insights en casos; Animation Budget Allocator limita SkeletalMesh ticking por significancia, NO sustituye movimiento gameplay; Animation Sharing ofrece poses comunes a multitudes del mismo esqueleto.
**Mallas modulares**: Leader Pose sencillo pero Render Thread caro; Copy Pose aún más caro; estudiar merge de mallas para NPC abundantes, con cuidado morph targets. No llenar NPC con diez componentes skinned si se necesita 100 de ellos. Para robots de partes desprendibles, preferir partes rígidas visibles con mallas estáticas adjuntas/sockets; no 20 AnimBlueprints por robot.
**Niagara**: Effect Types por sistema con scaling/per-instance culling/distance; impostores y luces cortas, pooling, siluetas claras. VFX de baja prioridad se culla, no colisión real de muro.
**Chaos**: prefracturar, fragmentos activados selectivamente, sueño/limpieza, máximo un gran evento de destrucción en V0. No dejar ragdoll 100 ciudadanos y 1kaiju activos simultáneamente.
**VRAM**: RTX 4070 8GB: textura escalable, LOD/HLOD, evitar Lumen hardware, materiales transparentes masivos y megatexturas, no cargar 4 planetas/city al mismo tiempo. 64GB RAM no aumentan VRAM. Linux Vulkan medido en ejecutable.
**Red**: hitstop local VFX nunca ralentiza estado replicado, autoridad de grab/damage/destruction/incident; no depender de reproducir Animation Montages exactamente a la misma fase para decidir golpes.

## 8. Presupuesto inicial de rendimiento — objetivos, no mediciones
- 60 FPS a 1080p Medium = 16.67ms por frame (CPU/GPU se solapan; los subpresupuestos no se suman linealmente).
- Meta de perfil de estudio **p95 <= 16.7ms** en recorrido sin grandes eventos; p99 <= 33.3ms en primer prototipo de destrucción, luego bajar hitches; temporalmente p95 alto = requiere arreglo antes de ampliar mapa. Valores iniciales debatibles tras hardware.
- CPU Gameplay/AI/Physics y animación deben medirse por separado, GPU/VRAM y render. Probar un perfil de 0/4/8/12 ciudadanos activos con 0/1/2 eventos de física, 30/60 FPS si hardware alcanza; no marcar 60 FPS alcanzados por build en editor.
- Reaccion en input: registrar tiempo entre pulsación y movimiento visible, en primer prototipo aspirar a respuesta instantánea dentro del siguiente frame para movimiento, mientras ataques/gestos siguen ventanitas de preparación. No superar por defecto cientos de milisegundos de bloqueo al agarrar/soltar.

## 9. QA de fluidez y epicidad (FEL-01 a FEL-10)
| ID | Test | Gate |
|---|---|---|
| FEL-01 | Diez minutos de vuelo, 4 modos, frenado y banking en circuito | sin control perdido, sin FOV mareante; usuario puntúa >=4/5 control |
| FEL-02 | 20 transiciones Ground/Takeoff/Hover/Cruise/Sonic/Brake/Recover | sin snap de pose notable o cámara saltando |
| FEL-03 | Golpe ligero, pesado, falla y cancel; buffer permite encadenar sin doble golpe | 50 ejecuciones, sin bloqueos arbitrarios |
| FEL-04 | 30 agarres carro ligero/grande, humanoide, robot y objeto largo, desde tierra/aire | manos cerca de sockets, carga respeta colisiones |
| FEL-05 | 20 knockbacks contra fachada, recobro en aire/caída | sin ragdoll eterno ni teletransporte al salir |
| FEL-06 | 50 impactos en muro: sonido y ruptura sincronizados y hueco real | pantalla legible; no falsa colisión |
| FEL-07 | Cuatro niveles VFX distintos y una invasión/meteorito rare | VFX no ocultan enemigo/retícula, max 1 gran evento |
| FEL-08 | H02 escudo/mano/martillo, H03 blink/reaparición | mismo sistema hit/damage; 50 destinos seguros |
| FEL-09 | Perfil Low/Medium 1080p, GPU + AnimInsights, con 0/4/8/12 NPC | p95/p99, memoria y costes medidos; priorizar player |
| FEL-10 | 2 jugadores LAN, velocidades altas, agarrar/carry/impacto | sin hitstop global ni estados divergentes; autoritario, cliente suavizado |

### Sesión de valoración creativa
Usuario puntúa 1–5: **respuesta del input, sensación de potencia, legibilidad de golpes, naturalidad del agarre, deseo de repetir**, además de "¿qué provocó rigidez/mareo?". Objetivo inicial >=4 en los tres pilares de control/potencia/deseo de repetir, ninguna molestia que impida jugar. Los resultados de IA no sustituyen valoración humana.

## 10. Roles/skills y orden de trabajo
Codex Gameplay implementa F01 AnimLayer básico, Codex Character/Animation (skill impact-character-animation) elige rig, clips, retarget/Blender y AnimBP, Codex QA mide y reproduce. **No empezar Motion Matching, Motion Warping, Control Rig de cuerpo entero o GASP completo si el set básico aún no se mueve bien.** Motion Warping/FBIK se activan **cuando un test evidencia alineación mala de golpe/agarre**.
- FEEL-00: documento + primeras curvas/criterios, sin build.
- FEEL-01 (tras F01): 1 rig, hover/cruise/sonic/brake, cámara, sonidos/sombra básicos; prueba humana.
- FEEL-02 (tras P01/D01): agarrar objetos distintos, golpes/contextuales, hit reaction/air recovery, IK solo para agarre.
- FEEL-03 (tras S01): FX hit layering, cascada de destrucción y escalado de Niagara; pruebas VFX/fps.
- FEEL-04 (cuando NPC >6-12): Animation Budget Allocator/Sharing y perfiles de crowd; solo después de medición.
- FEEL-05 (tras N0/N1): cámara/hitstop local y poses en red; no congelar server.
- FEEL-06 (tras K01): kaiju peso/desarme/cámara especial; no necesitar nuevos sistemas físicos.

## 11. Fuentes externas oficiales verificadas 2026-11-14
- Epic UE 5.8 Animation optimization / thread-safe / fast path: https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine
- Epic Anim blueprint blend/inertial: https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-blend-nodes-in-unreal-engine
- Epic transition limits: https://dev.epicgames.com/documentation/en-us/unreal-engine/transition-rules-in-unreal-engine
- Epic Full Body IK: https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-full-body-ik-in-unreal-engine
- Epic Motion Warping: https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-warping-in-unreal-engine
- Epic Physics based animation: https://dev.epicgames.com/documentation/unreal-engine/physics-driven-animation-in-unreal-engine
- Epic Animation Budget Allocator: https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-budget-allocator-in-unreal-engine
- Epic Animation Sharing: https://dev.epicgames.com/documentation/unreal-engine/animation-sharing-plugin-in-unreal-engine
- Epic modular character render costs: https://dev.epicgames.com/documentation/en-us/unreal-engine/working-with-modular-characters-in-unreal-engine
- Epic Niagara Effect Type/scalability: https://dev.epicgames.com/documentation/unreal-engine/scalability-and-best-practices-for-niagara
- Epic Animation Insights: https://dev.epicgames.com/documentation/unreal-engine/animation-insights-in-unreal-engine
- Epic UE performance profiling: https://dev.epicgames.com/documentation/en-us/unreal-engine/introduction-to-performance-profiling-and-configuration-in-unreal-engine
