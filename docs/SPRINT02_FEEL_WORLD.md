# IMPACT: BEYOND — Sprint 2: Velocidad, animación, vehículos, ambiente y periferia

**Decisión:** 2026-11-15 · **Estado:** PREPARADO, NO INICIADO/NO PROBADO · **No interrumpir sprint 1 (PR #18)** · Objetivo: Win64 DX12 UE 5.8.4, RTX 4070 Laptop 8GB, RD$0. **Siguiente sprint ≠ crear otro proyecto nuevo.** Fuente de realidad: comprobar HEAD y working tree de \`prototype/ib-urban-mvp\`/PR #18 cuando termine; referencia en esta preparación \`20e6a0081e33ab1abb7c8e7b67b55a4bc44a5443\`.

## Entrega jugable, no investigación indefinida

Una ciudad existente más viva y cuidada, con **12–20 coches visibles (mínimo 6 físicamente utilizables)**, salida por carretera hacia costa/naturaleza y una zona de vuelo abierta; H01 corre, hace supersalto, despega, acelera con **Shift/L3 también en vuelo**, distingue cinco regímenes con siluetas/animaciones inequívocas, entra a **modo Travesía con un botón separado**, alcanza objetivos de ensayo hasta **320 m/s y turbo hasta 400 m/s** solo cuando colisión, cámara, streaming y performance lo permitan; aterriza, agarra vehículos, los transporta, los lanza con potencia variable y provoca impactos con efectos y sonido convincentes. Vuelve a la ciudad sin perder estado y sin fallo de colisión.

**El sprint 3 sigue reservado a planeta circunnavegable, cuatro ciudades, mapa y minimapa.** En Sprint 2 se preparan solo las identidades de región, precarga predictiva y laboratorio esférico aislado; nada de implementar el planeta completo. Sprints 4/6/8 conservan destrucción/enemigos, H02+H03 con modelos existentes y primer espacio respectivamente.

## Lecciones verificadas de Sprint 1

- PR #18 ya tiene 28 fachadas vestidas, 72 ventanas mapeadas, 442 instancias HISM, ConceptCar, supersalto, impacto, IK de carro, 5 marchas \`9/18/44/72/105 m/s\`, pruebas repetidas y audio básico. **Reutilizarlo antes que importar un framework nuevo**.
- En \`ImpactBeyondCharacter.cpp\` el \`IsSprintHeld\` controla movimiento terrestre, pero el turbo aéreo no usa el mismo modificador; marchas manuales E/Q/R1/L1 dominan todo. \`PlayAnimation\` directo enlaza movimientos de vuelo, frenado y despegue con empalmes potencialmente visibles.
- \`UImpactLandingComponent\` emite anillo simple \`ULineBatchComponent\` y un thump sintetizado: física funcional ≠ golpe con calidad audiovisual.
- Bench de \`IB_MVP_URBANO.md\`: 612.1 s con 132.35 FPS medios, p95 8.58ms, p99 9.27ms a escala interna 83 %, con audio APAGADO, render fuera de pantalla, y teleports de testing. No acredita sensación con DualSense/monitor/audio ni streaming de periferia. 25 hitches >50ms, 24 próximos a reset.

## 1. Controles nuevos; Shift/L3 será siempre la intención de correr rápido

Usar Enhanced Input/Mapping Contexts de la plantilla y la misma fuente de verdad del CMC; adaptar mapeos, evitar bindings duplicados.

| Acción | Teclado | DualSense | Comportamiento |
|---|---|---|---|
| Sprint/turbo | **Shift sostenido** | **L3 sostenido** | En suelo, sprint; en vuelo, boost del nivel elegido; mantener al hacer supersalto→despegue conserva impulso |
| Marcha arriba/abajo | E/Q | R1/L1 | Cambia nivel **dentro del modo** activo, no teletransporta velocidad ni añade acciones contradictorias |
| Cambiar Cruce/Travesía | B | Cruceta derecha | Separado de turbo; dos velocidades máximas, HUD explícito; solo en vuelo |
| Freno | X | Cuadrado | Respuesta predecible en cualquier modo; nunca activar vuelo accidental |
| Entrar/salir vuelo | F | R3 | Conservar transición de despegue, superjump y recuperación |
| Resto de mecánicas | Sin cambios | Sin cambios | Agarre G/Círculo, T/Triángulo cargar/lanzar, V/↑ impacto, R/↓ reinicio, F2/← calidad |

**Valores iniciales a ensayar, no velocidades aún logradas**:
- Cruce 3 niveles: **12 / 35 / 90 m/s**. Shift/L3 entrega boost progresivo, caps de ensayo **20 / 55 / 130 m/s** mientras se mantiene.
- Travesía 2 niveles: **180 / 320 m/s**. Shift/L3 aumenta aceleración y permite objetivo temporal **hasta 400 m/s** en el nivel 2, solo si la colisión y precarga de región lo soportan. Cargar coche mantiene barrido del volumen y una política visible de seguridad, sin atravesar obstáculos.
- Conservar posibilidad de girar/ascender y frenada de emergencia a cualquier velocidad; cambio de modo interpola curva y velocidad, nunca reduce pos instantáneamente o \`SetActorLocation\` sin sweep. Colisión a 400m/s a 60fps implica >6m de avance por frame; se exige barrido desde posición previa de cápsula y carro y precarga anticipada. No declarar 'supersónico' sin umbral real verificado.
- Separar **sensación de velocidad** de números: referencia visual, FOV controlable, viento, close pass, cámara, sonido, estelas de calidad limitada y poses aerodinámicas.

## 2. Animación completa por mecánica, correctamente sincronizada

**Obligatorio:** una animación/pose/gesto apropiado para CADA acción de H01, no necesariamente un nuevo clip de cero por cada estado. Construir matriz de estados \`acción → animación existente/retarget → entrada → fase de contacto/hit/soltado → salida → transición/cámara/audio → test\`.

| Estados visibles | Regla |
|---|---|
| Idle, caminar, correr, sprint, stop/giro, salto, supersalto, aterrizar | Reusar Manny / secuencias de plantilla; foot contact, sin pisadas en aire |
| Takeoff quieto / desde carrera / con carga, hover, ascenso, descenso, aterrizaje | Anticipación, impulso, suspensión y recuperación coherentes |
| Crucero 1, 2, 3 | **Tres siluetas distintas**: control erguido → inclinación moderada → un brazo delante y perfil aerodinámico; no solo playrate o FOV |
| Travesía 1 y 2 | **Dos siluetas distintas**: brazos atrás y línea estirada → lanzamiento aerodinámico extremo, tensión de torso y piernas; postura no debe ocultar dirección o colapsar rig |
| Turbo ON/OFF, aceleración, frenada, bank en giro y near-miss | Pose aditiva/curvas + wind-up/aflojamiento perceptibles, sin clips nuevos innecesarios ni corte seco |
| Agarrar, levantar, mantener, transportar volando, cargar lanzamiento, lanzar, soltar, reiniciar | Carga físicamente alineada con manos y chasis mediante IK; AnimNotifies sincronizan agarre/impulso/soltar en fase correcta |
| Impact dive, golpe suelo, shockwave, recuperación | Pose de preparación, contacto en el suelo, audio/VFX/impulso en mismo evento y recuperación sin teletransporte |

**Motor de animación:** priorizar \`AnimBP\` del maniquí, State Machine, BlendSpaces, Blend by Enum/Int, Animation Slots, Layered Blend Per Bone e Inertialization (Epic UE5.8), Control Rig y TwoBone IK existentes. Preservar CMC como motor físico, NO hacer root-motion del vuelo sin barrido. Evaluar Pose/Distance/Motion Warping nativo en sandbox solo si demuestra mejora y se autoriza habilitar plugin.
**Activos:** revisar clips YA importados \`Content/IB_Flight/\`, maniquí, \`docs/FAB_SELECCION_ACTIVA.md\`: \`Free Animation Set: Zipline Skydiving Glide Dash Falling\` (21 clips gratis según ficha, NoAI), Motifect y Quaternius UAL Standard; si no descargado, obtener solo clips gratuitos autorizados y legales, probar retarget a Manny, no enviar bytes NoAI a modelos generativos. Usar método determinista de editor para modificar poses cuando sea necesario. No diseñar cuerpos/animaciones completas desde cero como primera solución.
**QA visual obligatorio:** frontal, lateral y 3/4 para las cinco marchas, turbo, freno, giro, takeoff, impacto y carry; cero clipping de carro/cuerpo o foot-skating grave; blend corto configurado, ninguna fase deja IK bloqueado.

## 3. Cámara, viento y retroalimentación de velocidad

- \`SpringArm\` actual con perfiles interpolados \`Ground, Sprint, Hover, Cruise1..3, Travel1..2, Boost, Carry, Landing/Impact\`. Cada perfil regula FOV, target length, offsets, lag/rotation lag, shake, velocidad de corrección. No activar/desactivar lag de forma abrupta al agarrar el coche. Mantener collision probe, cap de lag \`CameraLagMaxDistance\` y evaluar substepping para 30/60fps. Evitar nausea, FOV excesivo y shaking constante. Cámara siempre deja apuntar y mirar atrás; no secuestra el stick.
- **Viento/brisa audible:** 3 capas cruzadas por velocidad/altura/aceleración: hover breeze, cruise wind y travel roar; whoosh de turbo y near pass; un solo audio controller/owner, fades y pooling, sin duplicaciones tras reset. Reusar motor Audio/MetaSounds y muestras legítimas ya existentes; MetaSounds admite controlar viento proceduralmente por velocidad.
- **Viento visual:** líneas de velocidad discretas en los bordes de pantalla, estela y turbulencia local basada en velocidad real, paso estrecho entre edificios y presión al aterrizar; Niagara si ya está autorizado/habilitado, si no materiales o emisor mínimo. Evitar motion blur alto; control de intensidad/capacidades accesibilidad.
- **Impacto más potente:** un golpe físico consistente, shake único muy corto, polvo y pequeños escombros próximos con límites de overdraw/física, sonido por superficie, audio de shockwave y cámara que se recupera a tiempo. Reusar \`ImpactLandingComponent\`, clips y Niagara/Metasound existentes; no fabricar otro sistema de impacto.
- **Audio ON en pruebas humanas y de rendimiento.** Reutilizar sonidos gratis y con licencia compatible. Si nada sirve, MetaSound procedural breve es alternativa al PCM manual por cada evento.

## 4. Más coches y ciudad/entorno **ya hechos**

- Mantener \`ConceptCar\` del motor y \`Kit Cidade\` CC0 existentes. Candidatos **Quaternius Cars Pack CC0** (8 modelos, FBX/OBJ/Blend) https://quaternius.com/packs/cars.html ; Fab \`Vehicle Variety Pack\`/ City Sample Vehicles solo con versión, licencia y adquisición legítimas revisadas. **Meta: 12–20 coches visibles de >=3 tipos; al menos 6 interactuables** (agarrar/lanzar/impactar/soltar/reiniciar), distribuidos en ciudad/periferia; no 20 coches con física siempre despierta. Resto objetos dormidos/parked o cosméticos, control de colisión al activarlos. Ningún tráfico autónomo exigido.
- **Reutilizar entorno**: \`Poly Haven Modular Urban Apartments Facade\` CC0 y Kit Cidade existentes, prefab/inventario real de City Sample (no importar proyecto completo), Quaternius **Downtown City MegaKit Standard CC0** (fracción gratuita, NO Source de pago, estilo a evaluar) https://quaternius.com/packs/downtowncitymegakit.html; escoger un subconjunto y armonizar materiales. No recrear ciudad con 500 bloques.
- **Periferia sprint 2**: salida por carretera a costa y montaña/colinas con una región no urbana colisionable, skyline lejano, vegetación optimizada HISM y LOD. Usar \`Quaternius Simple Nature Pack CC0\` https://quaternius.com/packs/simplenature.html, terreno/heightmap legal prehecho o Landscape nativo, luces/sistemas atmosféricos de UE (\`Directional Light, SkyLight, SkyAtmosphere, VolumetricClouds\`). No diseñar de cero árboles, terreno/materiales/texturas que ya existan.
- **Agua**: UE5.8 tiene Water System **experimental** para océano/lago/río, que debe habilitarse por proyecto. Evaluar si está habilitado y su coste de shader/VRAM; no habilitar sin permiso anterior específico si aplica. Si falla/performance no alcanza, usar un plano de agua con material y normal maps permitidos, colisión/altura coherente; no simular fluidos volumétricos. No construir 4 ciudades ni planeta en S2.
- **Interiores falsos**: reutilizar \`M_MVP_WindowInteriors\` y 72 módulos como base, reducir frecuencia según distancia y coste. Sin habitaciones reales, capturas por ventana o luces individuales.
- La descarga de **assets realmente gratis y legalmente reutilizables** está permitida por decisión más reciente del usuario; verificar precio final RD$0, variante Standard, licencia, procedencia y tamaño antes de descargar. Evitar kits grandes innecesarios, malware, plugins instaladores; **si Fab requiere reclamación/aceptación de términos nuevos, solicitar autorización antes de esa acción**, no presumirla. No publicar paquetes de Fab sueltos en Git público ni enviar bytes NoAI a sistemas generativos. Registrar sólo recursos reales descargados/importados/compilados.

## 5. Preparación Sprint 3 (sin hacerlo todavía)

- Asignar IDs de región y estado de carro/pared/objetos por región, checkpoint y persistencia al salir/reingresar (sin compartir backend).
- Streaming/prefetch según velocidad y dirección con margen; para velocidades máximas no aparecer de golpe un muro o carretera. Considerar origen local y coordenadas globales LWC UE5.8; evitar \`UpVector\` fijo en nuevos contratos planetarios cuando no sea necesario.
- Laboratorio de esfera + gravedad local en mapa aislado para validar el enfoque con CMC sin destruir la ciudad actual. Probar Ephemera MIT solo si se audita código y dependencia real en Windows. La integración de planeta, cuatro ciudades y mapa/minimapa es exclusivamente del **Sprint 3**.
- Dejar interfaz mínima de orientación/región sin construir mapa global prematuramente.

## 6. Rendimiento: 60 FPS reproducibles en pantalla

**Meta de diseño:** 1080p DX12 **60 FPS estables** en RTX 4070 Laptop 8GB, laptop conectada a energía, calidad escalable; presupuesto 16.67ms por frame. Criterios de ensayo: **p95 <=16.7ms, p99 <=20ms**, sin hitches >50ms durante juego normal después de precarga; VSync/cap 60 cuando corresponda, comparar uncapped para margen; VRAM con holgura y máximos medidos, no inventar. Registrar por fase CPU Game/Render/GPU, streaming, material interiors, sombras, Niagara, agua, 12–20 coches y nuevas animaciones. \`Unreal Insights\`, \`stat unit/gpu/streaming\` y CSV existentes. Si falla, culling/HISM, HLOD/LOD/Nanite si aporta, texturas <=2K para prop estándar, materiales maestros, audio pooling, sombras/FX dinámicas limitadas, TSR opcional. **Nunca sacrificar la colisión de cápsula/carros ni ocultar que no se mantiene 60**.

**Separar cuatro pruebas**: (A) headless lógica de vuelo/sprint/carro/panel, (B) screenshot/render DX12 de poses/cámara/ventanas/agua, (C) circuito continuo grabado en pantalla con audio y DualSense SIN teleports durante >=10min, (D) soak de 15 min con velocidad máxima y región cargando/periferia, regresiones de UI y sesiones repetidas. Las medidas del Sprint1 no prueban Sprint2 ni se trasladan a nuevo HEAD.

## 7. Flujo de hasta cinco agentes y entrega única

A1 **Integrador**: exclusivo del personaje C++ compartido, input definitivo, mapa canónico y empaquetado. A2 **Animación/cámara**: selección de clips, rig, prototipos en sandbox, entrega poses revisables; no tocar A1. A3 **Mundo/assets**: candidatos de coches/edificios/naturaleza/agua/iluminación y sandbox aislado, registrar licencias y coste. A4 **Viento/audio/VFX**: MetaSounds, feedback, impacto, activos autorizados y opciones de intensidad en archivos separados. A5 **QA/perfil**: tests, reproducibilidad de 60 FPS, colisiones a 320–400m/s y evidencias sin tocar archivos A1. Un único editor/cook a la vez. Branch y worktrees por rol sin ediciones simultáneas de \`.umap/.uasset\`. Si no hay subagentes realmente disponibles, realizar en serie sin fingir concurrencia.

**Orden de integración del GRAN SPRINT**: primero QA/checkpoint S1 → control boost/travesía y colisión a velocidad → animaciones distinguidas de 5 niveles y cámara → viento/impacto/audio → 6+ coches físicos + ciudad vestida + costa/entorno y agua → contratos región/prefetch/laboratorio curva → build/perf/QA humano. QA corre continuamente sin reemplazar gate final. Producir **UNA build final Win64** y PR de sprint contra rama base validada; conservar última build jugable, no fusionar main sin autorización.

**Evidencia final**: commit y HEAD, 1080p/60 (p95/p99/VRAM) en sesión real audio ON con Steam Input, tabla de 5 siluetas con capturas, video Boost+Travesía a 320m/s y prueba de 400 si es segura, 12–20 coches visibles/6 interactuables, salida ciudad→costa→montaña/colina→regreso, sonido/viento e impacto, colisión y reset pasados, licencias por asset, blockers conocidos y petición humana puntual. Si cualquiera de los criterios falla, estado PARCIAL, no \`SPRINT 2 ACEPTADO\`.
