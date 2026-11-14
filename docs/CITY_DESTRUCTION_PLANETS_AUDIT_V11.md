# IMPACT: BEYOND — Auditoría de ciudades destructibles y planetas v1.1
14 nov 2026 | REVISIÓN DE CÓDIGO Y DOCUMENTACIÓN, NADA EJECUTADO EN LINUX. Un jugador offline. Cooperativo muy lejano. Equipo conocido: 64 GB RAM, RTX 4070 8 GB VRAM; CPU/distro/driver/SSD/Unreal local pendientes. Presupuesto RD$0.

## 1. Decisión práctica
Ciudad = kits Quaternius Downtown City Standard y Kenney CC0 (licencia precisa por ZIP a comprobar) + Unreal PCG. Fachadas preparadas para rotura, no toda la ciudad sometida a Chaos.
Soporte/derrumbes = evaluar C++ MIT de PlotBuilder 5.8 en UNA casa o fachada modular. Alternativa: paneles prefracturados Chaos nativo y colapso guiado por datos. No migrar todo un juego de construir casas.
Impacto físico = UE_Physics MIT 5.5 para estudiar escenas reproducibles, no plugin obligatorio. Escombros físicos grandes limitados y visuales Niagara cerca.
Espacio/planeta = Ephemera MIT 5.8, laboratorio aislado para coordenadas, movimiento y tránsito planeta-luna. Cosmic Architect MIT 5.7 para cuerpos celestes como plan B; no fusionar ambos motores de planetas.
Terreno editable = prefabs de cráter en V1; RealtimeTerrain 5.8 MIT en reserva por dependencias complejas.
Prioridad actual sigue siendo vuelo natural, caminar/correr, carro agarrable, muro perforable, robot, ciudad. El sistema solar no detiene esas entregas.

## 2. Repositorios inspeccionados: hechos frente a promesas
| Proyecto | Datos reales inspectados | Licencia / motor | Uso y límite |
|---|---|---|---|
| Atomium90/PlotBuilder https://github.com/Atomium90/PlotBuilder | README, Docs/TECHNICAL.md (documentación extensa), Source/PlotBuilder/Environment/StructuralSupportComponent.h/.cpp: sockets, anclajes, propagación de soporte, una recomputación como máximo por frame, cola de derrumbes; manipulación física y durabilidad también descritas | MIT en LICENSE; UE 5.8 declarado | PRIORIDAD 1 destrucción estructural. Solo módulos preparados, no 1000 rascacielos |
| Josemaik/UE_Physics https://github.com/Josemaik/UE_Physics | Árbol con BreakableTarget.cpp, PhysicsPickUpComponent y PhysicsProjectile; Physics.uproject EngineAssociation 5.5 | MIT | Laboratorio de paredes/Chaos. No ciudad completa |
| REPIRK/Modular-Mechanics-Toolkit-UE5 https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5 | PhysicsGrabberComponent.h y Build.cs auditados previamente | MIT, documentación UE5.7 Win64 | Probar agarre de carro por separado y barrido de su volumen; Linux sin verificar |
| Grzybojad/ProceduralCityGeneration https://github.com/Grzybojad/ProceduralCityGeneration | ProcGen.uproject 5.1, C++ de RoadGraphVoronoi, spline roads, building extrusions y parcelas | MIT | Solo reutilizar algoritmos de carreteras si PCG UE no basta. Proyecto completo antiguo |
| magnificus/Procedural-Cities https://github.com/magnificus/Procedural-Cities | README anuncia generación y algunos interiores; autor admite proof-of-concept sin optimizar, con vuelo sin colisiones | MIT, código viejo | Referencia visual, no base de ciudad |
| jason9075/city-pcg https://github.com/jason9075/city-pcg | Algoritmos JS deterministas, carreteras, bloques, parcelas y alturas | MIT | Reserva para exportar distribución urbana, no importación UE directa |
| AlexKorotkin/CityGenerator https://github.com/AlexKorotkin-creator/CityGenerator | City Sample+OpenStreetMap+Houdini22, documentación producida con Claude Code | MIT del integrador, Houdini Indie/commercial requerido; Apprentice no sirve | Fuera de presupuesto/requisitos; no base |
| per-simmons/unreal-agent-harness https://github.com/per-simmons/unreal-agent-harness | Guía IA+UE5.8+MCP, README/Getting Started Windows/macOS, City Sample ~90 GB | No licencia raíz verificada | Referencia de workflow IA, no copiar código sin licencia |
| eanticev/niagara-destruction-driver https://github.com/eanticev/niagara-destruction-driver | GPU visual debris from Chaos geometry collections; autor prueba UE5.5.4 | MIT | Solo escombros COSMÉTICOS, no reemplaza colisión ni piezas agarrables |
| 0xWerz/ephemera https://github.com/0xWerz/ephemera | Proyecto UE5.8, Source+Python/sh para generar Content, gravitación, planeta-luna; algunos plugins Win64 | MIT | PRIMERA referencia orbital, compilación Linux pendiente |
| narratech/Cosmic Architect https://github.com/narratech/TFG-Bravo-Perez-Rubio | plugin generación de sistemas celestes UE5.7, FastNoiseLite MIT | MIT | Segunda opción; no presume aterrizajes/destrucción reales |
| ape1121/Unreal-5-Procedural-Planets-and-Space-Physics https://github.com/ape1121/Unreal-5-Procedural-Planets-and-Space-Physics | Crescendo.uproject UE5.3; Source casi vacío, mayoría planetaria en UAssets que son punteros LFS ~130B | MIT | Baja utilidad práctica salvo recuperar UAssets completos con licencia |
| FugoShimizu/PlanetGenerator https://github.com/FugoShimizu/PlanetGenerator | UE5.8 con mucho código planeta/agua/clima; README Windows/PowerShell; no LICENSE raíz localizada (sí en terceros) | Sin permiso de copia identificado | REFERENCIA, no donante |
| TobyGame/PlanetCreatorPlugin https://github.com/TobyGame/PlanetCreatorPlugin | plugin UE5.5+, clases planetas/órbitas; no LICENSE raíz localizada | Sin permiso identificado | REFERENCIA, no donante |
| draquel/VoxelWorlds https://github.com/draquel/VoxelWorlds | UE5.8, README dice voxel sphere/orbit/edición y fuente C++; LICENSE raíz no encontrada | Sin permiso identificado | REFERENCIA; además aumenta riesgo técnico |
| maciektk88/ProceduralPlanetGeneration https://github.com/maciektk88/ProceduralPlanetGeneration | plugin de mundos esféricos; licencia GPL-3.0 | GPL-3.0 | NO base para mantener distribución simple, estudiar si se reconsidera |
| HawkDev15/VoxelWorld https://github.com/HawkDev15/VoxelWorld | UE5.7, chunk terrain Minecraft y destrucción de bloques; sin LICENSE raíz hallada | Sin permiso | REFERENCIA y estética poco útil para ciudad |
| MOChloe/ElementSandbox https://github.com/MOChloe/ElementSandbox | UE5.6, README sobre ECS/destrucción/meteoritos con presentación HISM/WPO, benchmarks no reproducidos; no LICENSE raíz hallada | Sin permiso | Referencia de rendimiento, no asumir que 120k escombros son 120k cuerpos Chaos |
| manndevani/RealtimeTerrain https://github.com/manndevani/RealtimeTerrain | UE5.8 MIT; proyecto activa CesiumForUnreal, GRT, CelestialVault, etc; Build.cs depende GRT | MIT, dependencias por validar | Reservar hasta necesidad real de terreno editable, no copiar suite grande |

Los números de estrellas/FPS de los autores NO son prueba local. Ninguno se probó en la máquina del usuario. La selección no garantiza interoperabilidad en UE/Linux.

## 3. Arquitectura de ciudad: una ciudad con densidad y destrucción selectiva
Un edificio de prueba tendrá (a) malla visual ligera, (b) paneles de fachada perforables con estados intacto/roto, (c) puntos estructurales preparados con grafo adaptado de PlotBuilder, (d) Chaos y escombros solo en zona activa, (e) piezas grandes desprendibles y agarrables, (f) reconstrucción de colisión al reset.
Los edificios lejanos serán proxies/HLOD. No todos los rascacielos tienen física total, pero el jugador debe encontrar repetidamente fachadas perforables y objetos arrojables en rutas de pelea. Si el grafo de PlotBuilder cuesta demasiado portar, usar un sistema de paneles Chaos sencillo sin grafo.
Impacto de carro -> estructura reporta punto/material/impulso -> se actualiza estado/colisión -> se crean 1-3 piezas interactivas -> otros fragmentos son visuales -> evento de incidente/alarma si corresponde.

## 4. Arquitectura de planetas compactos
Planeta origen con ciudad detallada + horizonte esférico; cuerpo proxy desde atmósfera/órbita, región local con colisiones. Sol visible nunca aterrizable, Luna y dos planetas con UNA región visitable cada uno. BodyId, RegionId, posición global y espacio local para físicas. Distancia de viaje comprimida; sin simular cuatro terrenos detallados simultáneos.
Ephemera se revisará de forma aislada. Inspeccionar Content generado, scripts y licencias antes de intentar build Linux. Si falla, elegir esfera proxy y regiones pequeñas con código propio; Cosmic Architect segunda opción.
No instalar Win64-only plugins ni configurar dos motores planetarios juntos. Evaluar material de planeta, gravedad, transición desde suelo, visibilidad del sol y volver a misma calle.

## 5. Copyright y private use
Uso privado/no comercial NO da permiso universal para copiar código, modelos, animaciones, mods o música. Licencia MIT/CC0 cubre reutilización según términos, reteniendo avisos pertinentes. Compartir un ejecutable con amigos puede ser redistribución; revisar Fab/Epic/QAL y terceros. No subir UAssets cerrados a GitHub público; separar descargas legítimas y fuentes propias. Las demos y scripts de Roblox/GTA/Marvel sin licencia sirven como referencia, no como pack libre. GitHub sin LICENSE no es necesariamente dominio público.
No descargar ejecutables desconocidos ni ejecutar scripts de terceros sin inspección previa.

## 6. Experimentos decididos (aún NO ejecutados)
- AUD-D01: crear en Linux nivel de 1 edificio de 2–3 pisos con 6–12 módulos preparados, extraer soporte PlotBuilder MIT y comparar contra Chaos sin soporte. Impacto de carro, 50 roturas, 20 resets, agujero transitable, limitación física 1080p y VRAM/fps p95/p99.
- AUD-C01: 2–4 manzanas PCG con Quaternius/CC0 y Kenney, recorrer caminando/corriendo/volando, colisiones y streaming. Grzybojad sólo si faltan calles coherentes. Medir CPU/GPU y comprobar identidades de assets.
- AUD-W01: Ephemera como sandbox aparte; inspeccionar scripts Python/sh, licencia/asset, compilar UE Linux y probar vuelo planeta->luna->retorno. Si falla, opción proxy y Cosmic Architect.
- AUD-P01: REPIRK PhysicsGrabber versus Physics Handle/CMC nativos, 30 ciclos de agarre/throw y objeto sujeto colisionando contra muro sólido.
Orden de ejecución real sigue L00 Linux + F01 vuelo + C01/P01/D01; laboratorio orbital NO bloquea la ciudad. No crear multijugador, N0/N1 ni backend.

## 7. Regla de aprobación
Por donante: licencia y edición verificadas; componentes concretos inspeccionados; compilación UE Linux real; mapa de test ejecutado; rendimiento con RTX4070 8GB/CPU real; QA y valoración de diversión. Resultado PROBAR / RESERVA / REFERENCIA / DESCARTAR / ACEPTADO. Un README no equivale a ACEPTADO.
