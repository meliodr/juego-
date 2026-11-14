# IMPACT: BEYOND — Plan maestro ejecutable v0.5
Fecha: 2026-11-14. Estado: **PLAN / INVESTIGACIÓN, NO PROYECTO COMPILADO**. Plataforma: Linux nativo, Unreal Engine versión estable que pase auditoría local. Objetivo de compras de assets/plugins: RD$0.

## 1. Contrato de diseño
Un sandbox 3D **sin historia obligatoria** que sea divertido por vuelo, exploración, agarres, lanzamientos, combate y destrucción con consecuencias. Sensación urbana vertical de gran ciudad con identidad original, no reutilización de contenido de GTA/Spider-Man. Tres personajes originales y distintos:
- **H01 Fuerza:** vuelo, aceleración, golpes, embestida, agarre de carros y personas preparadas, arrancar elementos anclados, lanzarlos y reutilizar escombros.
- **H02 Genio cósmico esmeralda:** personaje masculino del concepto ya aceptado; formas/manos de energía, escudo, puño, martillo, pinza, manifestaciones mayores después. Sin fruta del diablo, ni anillo/logo/traje de Linterna Verde; evitar nombre PRISMA.
- **H03 Desplazador espacial:** estética original grafito/índigo y fracturas azul-violeta; Blink seguro, contra-blink, golpe de reaparición, marca de retorno luego. No calcar Nightcrawler. Debe poder explorar mundo/espacio con movilidad general y tener nicho de combate propio.
Nombres finales de héroes pendientes; nombres VECTOR/PRISMA rechazados.
Enemigos: robots modulares desmontables y lanzables, kaijus con partes preparadas desprendibles (sin gore explícito), rivales superhumanos rápidos. No enemigo constructor genérico. Co-op futuro para dos personas, PvE inicialmente; no bloquear el modo local.

## 2. Lo que garantiza cada entrega
**No confundir visión final con vertical slice.**
- **Prototipo A — físico divertido:** cámara de tercera persona + vuelo + un carro que pueda ser transportado a velocidad + muro perforable con apertura de colisión + robot sencillo desmontable; mapa gris o una manzana. Linux ejecutable.
- **Sandbox B — una ciudad para jugar:** 4–8 manzanas densas (objetivo inicial, no superficie medida), avenidas largas, azoteas, 3 tipos de objeto lanzable, robot y rival superhumano, spawner/reinicio; destructibilidad distribuida pero selectiva. H01 jugable.
- **Variedad C:** H02, H03, robot pesado, kaiju prototipo de pocos módulos; control de opciones y modos libres. Evitar rig/modelado final antes de validación.
- **Mundo D:** ciudad y periferia ampliadas; planeta pequeño visualmente esférico, transición superficie/atmósfera/órbita y regreso; Sol visual, Luna y dos planetas con UNA región interactiva pequeña por destino.
- **Co-op E:** 2 jugadores en la misma región, anfitrión/cliente Linux, mismo daño, partes, carros y huecos; entrada LAN antes de conexión Internet. El multiplayer espacial con jugadores separados por planetas es posterior a E.

## 3. Arquitectura mínima que no impide crecer
Módulos: Core (estado y pruebas); Flight (CMC primero, Enhanced Input, cámara); Interaction (objetos e interfaz común); Damage/Destruction (autoridad de estado, Chaos/paneles); Combat (golpes, lanzamientos, ragdoll, IA); Heroes (configuración por Data Assets para H01/H02/H03); Enemies (partes modulares por PartId y cambio funcional); World (instancias urbanas, streaming, regiones planetarias); Presentation (Niagara/audio/UI, sin ser fuente de verdad); Persistence (estado por región, reinicio); Multiplayer (autoridad y replicación de estados importantes).
Blueprints para ensamblado, mallas, materiales y ajuste; C++ para estados, movimiento personalizado, red, IDs e interacción. No reconstruir CharacterMovement ni crear GAS completo hasta que exista necesidad demostrada.
En cada acción separar **intención -> validación -> cambio de estado -> efectos visibles**. Daño, desarme, agarre y aperturas no dependen de animación específica ni de un sistema gráfico. Un objetivo solo tiene dueño de agarre; una parte solo se desprende una vez.
Mantener PlayerController/cámara/HUD por jugador para futuro co-op. No depender globalmente de PlayerController(0). El servidor será autoridad de daños, teleport, agarres, huecos, spawns. Partículas no jugables pueden calcularse solo localmente.

## 4. Decisión urbana: ciudad densa, no gigantesca vacía
- Reutilizar **Quaternius Downtown City MegaKit Standard gratuito** como primera prueba de formas y edificios; contiene solo parte del catálogo completo, no shaders/Unreal project Source de pago.
- Complementos CC0 Kenney Roads/Car Kit/Industrial y PCG nativo. Mantener estilo gráfico coherente. PCGEx MIT (rama 5.x estable acorde al motor) es reserva, no dependencia automática.
- Generador procedural determina trazado de avenidas, parcelas, lotes y alturas; colocar manualmente puntos de interés/combate. PCG no crea interiores creíbles por sí solo.
- Instancing/HLOD para ciudad estática y LOD simplificado lejano; actores físicos solo en zonas de acción. Colocar fachadas destruibles en varios edificios, no todo el volumen de cada torre.
- Destrucción en capas: VFX -> paneles con hueco transitable -> objetos móviles/agarrables -> derrumbe parcial controlado. Chaos usa fracturas/clusters preparados. Usar campos de sueño/eliminación para retirar fragmentos irrelevantes. **NO** prometer corte universal arbitrario de edificios.
- Los robots/kaijus usan un rig base más piezas accesorias por sockets/uniones, con estados intacto/dañado/desprendido y consecuencias sobre IA. Partes desprendidas se vuelven objetos físicos elegibles; no cortar cualquier malla orgánica en tiempo real.

## 5. Vuelo extremo: límites medibles
La velocidad es una fantasía de control, no un número de marketing. Movimiento: hover/precisión, boost/combate, travesía. Sin combustible. Frenar es explícito y progresivo. Cámara ajusta FOV/anticipación sin blur obligatorio.
A 1000 km/h se recorren aprox. 278 m/s, 4.6 m por frame a 60 FPS; a 3000 km/h ~13.9 m por frame. Necesarios barridos de colisión (incluida carga sujetada), detección anticipada, límites de aceleración, paso de simulación adecuado y precarga. **No** asumir CCD resuelve todas las combinaciones físico-fragmento-red; probar placas delgadas/esquinas.
Precarga World Partition por posición cercana y fuente predictiva adelante, con HLOD de seguridad. Si la celda detallada no está lista, reducir capacidad de interacción/cargar fallback; jamás atravesar suelo colisionable inexistente. Bajo Linux Vulkan registrar hitch p95/p99, memoria, GPU/CPU, shaders, física, streaming.

### Colisión y enemigos que vuelan: dos límites confirmados
Unreal documenta que durante un movimiento barrido de UMovementComponent se comprueba principalmente el UpdatedComponent; los componentes adjuntos pueden desplazarse sin colisionar independientemente. Por tanto, agarrar un carro requiere barridos/shape tests propios para la carga y resolución de golpes. Física substepping puede aumentar estabilidad a costa de CPU y callbacks duplicables: no habilitarla globalmente sin medición. Fuentes: https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UMovementComponent?lang=en-US y https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-sub-stepping-in-unreal-engine.
NavMesh por defecto describe superficies caminables, no un volumen 3D de vuelo libre. Primera IA aérea: dirección/intención, detección de edificios, steering y EQS para escoger puntos de evasión; no resolver desde el inicio rutas óptimas 3D para miles de agentes. Encontrado tutorial nativo de UE 5.6–5.8 publicado en noviembre de 2026: https://dev.epicgames.com/community/learning/tutorials/jOZpv/unreal-engine-ue5-flying-ai-tutorial-real-time-obstacle-avoidance-target-tracking. Es una guía, no plugin licenciado/importable ni prueba de nuestra IA.

## 6. Planeta pequeño y sistema solar: solución híbrida
No utilizar un mapa plano World Partition como planeta esférico universal. Mantener referencia del cuerpo celeste (BodyId, RegionId), posición global de alta precisión (coordenadas relativas al centro), y origen local próximo para físicas y render.
**Tres escalas**: entorno interactivo local con streaming; terreno/horizonte distante en teselas/LODs; esfera proxy desde órbita con atmósfera/iluminación. Transiciones visuales/streaming para sensación de continuidad, no continuidad geométrica literal universal.
Empezar con prueba ciudad -> ascenso -> planeta visto desde órbita -> regreso al edificio de salida. Posteriormente nave espacial no obligatoria: viaje por vuelo de travesía comprimido con puntos de interés y aproximación asistida opcional.
Destinos: planeta principal (ciudad + periferia real), Luna (cráter/base/rocas), planeta rojo (cañón/base), planeta azul/tormentoso (plataformas/torres/robótica aérea). Sol como fuente/luz/fondo, **NO destino**. Todos los cuerpos pueden mostrarse desde el principio como proxies visuales; solo marcar visitable cuando aterrizaje+colisión+acciones estén probados.
No simular gravedad real, órbitas científicas, clima global, océanos enteros ni partículas en todos los planetas. Compresión de distancias deliberada. Un planeta pequeño requiere probar curvatura, horizonte y escala de edificios: no fijar radio definitivo antes de test.
Ephemera MIT en UE5.8 = laboratorio separado, no importar proyecto entero. Cosmic Architect MIT 5.7 = alternativa si aporta más. OpenWorldGenerator MIT y VoxelTerrain = reserva de mayor riesgo de integración. Priorizar Unreal nativo.

## 7. Tercer héroe (teleport) sin trampas
Primera implementación con teleport nativo + trazas y shape sweeps. Elige destino libre con colisión cargada; chequea cápsula y objeto transportado; impide atravesar sólidos si el poder no permite esa ruta; maneja destino no válido con señal no intrusiva. No telefrag automático. Ventanas de combo y gasto solo en desafíos, no en vuelo.
Pruebas: 50 destinos borde/cornisa/techo/suelo/aire, en dirección y en vehículo sujetado; cero interpenetración ilegítima. En co-op validar en servidor y replicar cambio de ubicación a clientes, sin duplicar golpes.

## 8. Cooperativo con presupuesto cero
Listen-server LAN primero; no servidor de pago, relay, lobby ni NAT traversal en el primer hito. Probar dos instancias en Linux en circuito de vuelo sencillo y luego con carro y panel. Replicar estado estructural y de partes, no física fractal idéntica para cada cliente. El servidor valida el único portador del carro; eventos de impacto con IDs para evitar duplicar daño; retirar propiedad al desconectar.
Internet público, dos planetas simultáneos, splitscreen, PvP y migración de anfitrión quedan fuera hasta pasar pruebas de latencia. Fuego amigo desactivado; agarre de aliado con consentimiento y cancelación.

## 9. Herramientas y costo real
Unreal Linux precompilado o fuente oficial, toolchain correspondiente a versión; Vulkan. Git + Git LFS SOLO para propios/assets redistribuibles dentro de cuota, Blender opcional para dividir partes rígidas, Quaternius/Kenney gratuitos, Chaos/PCG/Enhanced Input nativos, Codex CLI + Unreal MCP si está disponible.
Epic UE5.8 requisitos orientativos: 32 GB RAM, GPU dedicada ~8 GB VRAM o más; clang 20.1.8 para 5.7–5.8 según página de requisitos versionada (no confundir con quickstart que puede mostrar clang 18). Si máquina queda corta, bajar escala, iluminación, número de fragmentos y calidad antes de instalar más plugins. Ningún hardware del usuario inspeccionado todavía.
Unreal MCP 5.8 es **experimental**: algunas API faltan, herramientas incompletas, cambios frecuentes, llamadas al game thread serializadas. Habilitar ModelContextProtocol + AllToolsets local; generar config para Codex desde el editor, inspeccionar que funcione. No exponer puerto de MCP a redes externas.
Las pruebas de compilación no prueban VFX, fun ni cámara; se debe ejecutar paquete con GPU. Unreal Automation Tests/Functional Testing/Gauntlet sirven para regresiones; sesión humana decide diversión.

## 10. Orden de tareas y puertas (sin fechas prometidas)
| Gate | Qué construir/usaremos | Prueba mínima y decisión |
|---|---|---|
| L00 | Auditar Linux y Unreal, crear base C++ y mapa gris | paquete Linux abre fuera del editor; informe GPU/VRAM/driver/toolchain |
| F01 | Flight CMC + cámara + control | 10 min circuito, colisiones finas, frenada sin teleports, 30/60 FPS medidos si hardware alcanza |
| C01 | Quaternius Standard + Kenney + PCG nativo, 4–8 manzanas | rutas legibles, azoteas, HLOD/carga sin colisión ausente |
| P01 | Physics Grab nativo o REPIRK mínimo | 30 agarrar/soltar/lanzar de carro/bloque/rival, sin cuerpo atrapado ni objeto atravesando pared |
| D01 | Chaos paneles, huecos, escombros elegibles | 50 impactos; hueco transitable, reset reproducible, perfiles VRAM/CPU |
| E01 | Robot modular y rival aéreo | pérdidas de brazo/propulsor afectan capacidades; persecución y evasión jugables |
| S01 | sandbox divertido con spawner/experimentos | 15 min sesión humana, cambiar objetos y rivales, control/velocidad/calidad >= 4/5 subjetivo |
| H02 | genio esmeralda | mano/escudo/puño/martillo/pinza comparten físicas/dano |
| H03 | desplazador espacial | 50 blinks destinos libres + combate contextual y reconocimiento de alcance |
| K01 | kaiju mediano | 3 puntos de desprendimiento con consecuencias jugables y rendimiento medido |
| N01 | dos instancias cliente/anfitrión Linux con objeto y muro | sin doble agarre ni divergir huecos, simular latencia/pérdida |
| W01 | esfera/atmósfera/órbita/llegada retorno | 10 viajes ida-vuelta sin cambiar incorrectamente origen/estado |
| W02 | Luna + 2 planetas (1 área cada uno) | ascender/acercarse/aterrizar/explorar/volver, superficies no meras imágenes |
| N02 | cooperativo regional completo | pelea conjunta, reinicio, desconexión, spawns y estado sincronizados |

**Regla de parada:** un bloque no se amplía hasta que pase su gate; si un recurso exige fusionar un framework grande o no compila Linux, se descarta y se usa alternativa nativa. Sin porcentajes ficticios; estado de cada bloque DOCUMENTADO/IMPLEMENTADO/COMPILADO/EJECUTADO/ACEPTADO/BLOQUEADO.

## 11. Riesgos / respuesta
| Riesgo | Señal temprana | Respuesta |
|---|---|---|
| Rendimiento Vulkan/VRAM | editor o juego usa VRAM excesiva / tirones | perfil Low/Medium, sombras/efectos limitados, menos fragmentos/actores |
| Ciudad artificial | vuelo bonito pero sin decisiones | mejorar rutas, alturas y cosas interactivas antes de ampliar kilómetros |
| Destrucción insuficiente | solo polvo, sin huecos ni cambios | mantener piezas físicamente aprovechables y pasar D01 |
| Destrucción demasiado cara | spikes en ruptura | prefractura/clusters, límite de eventos, sleep/remove, piezas visuales |
| AI genera código incoherente | API inexistente, referencias rotas, Blueprints dañados | cambios pequeños, compilar, revisar logs, hacer rollback |
| Licencia mezclada | asset sin términos claros | no importar, conservar evidencia de página/archivo y alternativa CC0 |
| Planeta irrealizable | fallas de precisión/streaming | separar laboratorio orbital, proxy + región local, sin planeta físico completo |
| Red se descontrola | carros duplicados, agujeros diferentes | autoridad de servidor desde P01/D01, pruebas N01 |
| Contenido infinito | muchos enemigos/modelos sin juego | spawner + dos enemigos y diversión primero, luego variación |

## 12. Primeros tres tickets listos para Codex
- **IB-L00**: inspeccionar Linux real, resolver versión estable/SDK/driver, proyecto Third Person C++ mínimo, paquete ejecutable y logs; sin instalar dependencias de pago ni inventar hardware. Necesita entorno Linux accesible.
- **IB-F01**: implementar vuelo por CMC con input/cámara, mapa de circuito, pruebas de aceleración/freno/colisión/latencia de entrada; no tocar destrucción ni planetas.
- **IB-P01**: inspectar REPIRK commit/licencia y PhysicsGrabber; probar importación aislada Linux y escenario carro/bloque; si falla, solución mínima Physics Handle con transporte asistido. Aceptación con 30 ciclos.
Evitar tres grandes cambios simultáneos. Conservar un build jugable previo. Rama/PR por gate cuando se implemente.

## 13. Fuentes primarias (consultadas 2026-11-14)
UE Linux requirements: https://dev.epicgames.com/documentation/unreal-engine/linux-development-requirements-for-unreal-engine?lang=en-US
Unreal MCP 5.8: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8
World Partition: https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine
Chaos Destruction: https://dev.epicgames.com/documentation/en-us/unreal-engine/destruction-overview
Chaos performance: https://dev.epicgames.com/community/learning/tutorials/KWeX/unreal-engine-a-tech-artist-s-playbook-for-chaos-performance
Unreal automation testing: https://dev.epicgames.com/documentation/unreal-engine/run-automation-tests-in-unreal-engine
Networking: https://dev.epicgames.com/documentation/unreal-engine/testing-and-debugging-networked-games-in-unreal-engine
Official UE license: https://www.unrealengine.com/license
AI/asset tool sources: ver FREE_RESOURCE_AUDIT_V05.md.

## 14. Regla editorial
No prometer un sistema solar entero en una sola fase. No indicar juego creado, plugin Linux compatible o hito aprobado sin pruebas reproducibles. Una demo ajena no es un proyecto reutilizable por licencia. Reutilizar primero, crear solo el pegamento faltante, medir y repetir.
