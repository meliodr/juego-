# IMPACT: BEYOND — Flight donor audit & scope v0.9
Fecha: 2026-11-14. Estado: INVESTIGACIÓN; ningún módulo ha sido descargado/compilado/ejecutado en el Linux del usuario. GPU declarada RTX 4070 8 GB VRAM, RAM 64 GB. CPU, driver, distro, Unreal no comprobados.
**Decisión del usuario: el cooperativo es MUY LEJANO, fuera de la ruta crítica, sin pruebas de red tempranas.** Este documento prevalece sobre planes anteriores con N0/N1 tempranos o matchmaking.

## 1. Auditoría profunda ManOfSteel
Fuente https://github.com/ProdiG66/ManOfSteel, LICENSE raíz MIT (Copyright 2024 Jan Enri Arquero), proyecto UE5.3.
Árbol remoto inspeccionado: 681 entradas, 61 archivos .cpp/.h, 140 rutas bajo Content/Animations/Superman/Flight. Los UAssets revisados contienen únicamente punteros Git LFS de ~130 bytes: NO son las animaciones. Ejemplo Content/Animations/Superman/Flight/FastMove/A_Flight_FastMove_A.uasset remite a 290911 bytes lógicos. El README admite que su cuota LFS expiró y que los clones traen binarios incompletos. Fork Aaryan-Umar inspeccionado mostraba punteros similares; NO equivale a copia restaurada.
GIFs de documentación SÍ son completos y se pueden estudiar como referencia visual: Flight.gif 23.7 MB, Superhero_Landing.gif 9.1 MB, Strike.gif 4.3 MB, Beatdown.gif 5.4 MB.
- https://github.com/ProdiG66/ManOfSteel/blob/main/Documentation/Flight.gif
- https://github.com/ProdiG66/ManOfSteel/blob/main/Documentation/Superhero_Landing.gif
- https://github.com/ProdiG66/ManOfSteel/blob/main/Documentation/Strike.gif

### Código real estudiado
- Source/ManOfSteel/Private/Components/Flight.cpp y Public/Components/Flight.h: vuelo CharacterMovement MOVE_Flying, hover/sprint max speeds + acceleration + braking; leaning a partir de diferencia de rotaciones de velocidad suavizada; SpringArm camera lag, anim montages, Niagara trail/wave/sonic boom y superficie/polvo.
- Source/ManOfSteel/Private/Actors/Superman.cpp: inputs cámara-relativos, avance automático sprint, control ascent/descent, llamadas Flight.Update y FX/Afterimage/Smear.
- Source/ManOfSteel/Private/Components/FlightCombat.cpp: golpes, MotionWarping, montages, targetlock y recovery. No puede portarse trivialmente sin Blueprints/animations/dependencias.
- Parámetros de muestra: SprintFlySpeed=6000 UU/s y HoverFlySpeed=1500 UU/s. En Unreal por defecto son 60m/s (216km/h) y 15m/s (54km/h). El efecto cinematográfico "supersónico" deriva en gran medida de poses/FOV/sonido/FX; no es Mach 1 en ese preset.
- Defectos para nuestro proyecto: ascenso/descenso llama SetActorLocation sin sweep (puede atravesar paredes); GetFirstPlayerController asume un solo player; múltiples dependencias Stats/TargetLock/HeatVision/Niagara; división de diferencia angular por delta debe protegerse ante delta ~0; version 5.3 -> UE estable actual debe probarse.
- MIT del código no garantiza derechos de todas las mallas/animaciones externas alojadas fuera de GitHub. Un Google Drive alterno mantenido por tercero en README es candidato para AUDITAR propiedad/licencia, no permiso automático para copiar personaje Superman ni third-party FBX.

**Decisión:** opción A es integrar SOLO ideas/funciones necesarias del C++ MIT de Flight y lean/camera en nuestro Third Person con CMC y verificar Linux. Reconstruir AnimationBP/poses mediante Quaternius Standard CC0, Rokoko superhero gratis (solo clips descargados/verificados), Blender/Control Rig para poses faltantes. No instalar ManOfSteel completo ni importar Superman.

## 2. Otros proyectos similares inspeccionados
| Candidato | Evidencia encontrada | Licencia/límite | Uso |
|---|---|---|---|
| https://github.com/ProdiG66/FastestManAlive | UE5.3, 33 archivos fuente .cpp/.h; SpeedForce.cpp, combat, Niagara/Afterimage/Smear; UAsset LFS pointers | MIT en raíz; identidades/FX/arte de Flash fuera; no migrar proyecto entero | Reserva para estelas, rapidez, sensación de impulso |
| https://github.com/kn-404/UE5_Superhero-Flight-System | descripción detallada vuelo camera-relative + 8-direction blend, CMC nativo; GitHub tree solo README, proyecto en Google Drive externo | No LICENSE verificada ni proyecto integrado en GitHub | Referencia/fallback de grafo recreado con API nativa, no copiar archivos |
| https://github.com/ShikenNuggets/SecretIdentity | repo archivado 2024, UE5.4 y dependencias CitySample muy grandes; demo vuelo, combate y destrucción; README reconoce assets no incluidos | Sin licencia explícita identificada; incompatibilidades/arte CitySample | Referencia de sensación/ciudad/alerta; NO donante |
| https://github.com/Otoni24/Platformer | demo UE5 C++ con dash, knockback, get-up, animation montage/notify y código de jefe | LICENSE MIT propio; Content tiene POLYGON/Synty y otros assets terceros, no asumir cubiertos | Reserva para principios de knockback y recuperación; auditar assets por separado |
| https://github.com/KishorePaul0205/Superhero-Mechanics---VFX-Driven-Powers-in-Unreal-Engine-5 | README de Blueprint VFX/hero abilities, repositorio diminuto | no proyecto substancial inspeccionado | Referencia, no "juego descargable" |
| https://tjatomica.itch.io/superman-ue5 | demo de vuelo en ciudad basada en City Sample | ejecutable Windows gratuito, no fuente ni permiso de extraer asset | Referencia visual únicamente |
| https://www.youtube.com/watch?v=mLld-VfBFnQ | LocoDev tutorial vuelo dinámica Blueprint UE5 | referencia educativa | Fallback para implementación propia de cámara y blend |
| https://www.unrealengine.com/tech-blog/download-the-latest-game-animation-sample-project-now-updated-for-ue-5-8 | GASP 5.8, locomoción, derribos y ragdoll | Fab gratuito, etiqueta "Allows usage with AI: No" requiere revisión de condiciones | Laboratorio aislado, no integrarlo todo |

Los packs comerciales GASP Superhero Flight y Superhero Flight Animations de Fab/Gumroad quedan fuera por RD$0. No descargas piratas.

## 3. Criterios de elección
A. Probar código C++ MIT ManOfSteel recortado: FlightProfile, Lean, Camera & FX events (sin dependencias basura) en proyecto limpio Linux UE estable; no copiar World/HeatVision/Character/IP.
B. Si A requiere portar varios sistemas, volver a CMC MOVE_Flying + BlendSpace 8-direcciones basado en concepto kn-404, con animaciones libres y lógica propia; conservar idea del lean.
C. Solo si mirror ManOfSteel se comprueba legal y técnicamente, considerar clips concretos retargeted en sandbox separado. Copias de Superman/asset third-party no.
La rapidez real se decide por pruebas streaming/colisión (incluso objeto sujeto), no por el número "Sonic" en una animación. Hover/Boost/Brake/Lean/Banking deben sentirse bien a 30/60fps. Registrar footage comparativo con GIFs del autor sin copiar su apariencia.

## 4. Nuevos gates
FLT-AUD: fuente/licencia/LFS/mirror/animaciones exactas anotados; parte C++ sí estudiada, binarios siguen sin validar.
FLT-BASE: adaptación mínima y build/package Linux; prueba hover+boost+brake+lean.
FLT-ANIM: 1 rig con blend hover/cruise/sonic/brake/land/recovery, usando clips CC0/permitidos; FX Niagara propio; test video.
FLT-QA: 10 minutos de vuelo, 20 transiciones, colisión bajo boost, FPS p95/p99/Vulkan/VRAM, valoración de usuario >=4/5; sin pruebas sintéticas inventadas.
FLT-GRAB: volar con carro, sweeps para volumen sujetado y frenadas, hueco transitable.
**Stop**: si portar ManOfSteel implica reconstruir 200 Blueprints sin LFS, NO hacerlo, adoptar el principio técnico en sistema pequeño.

## 5. El cooperativo sale del plan de entregas próximo
- Implementar SOLO un jugador offline ahora y las siguientes entregas. No montar servidor LAN, lobby, sesiones, EOS, P2P, network replication ni probar N0/N1 al principio. No trabajar roles Codex networking.
- Conservar solo buenos límites de componentes y IDs estables de objeto/parte/incidente, independientes de cámara, porque sirven también para persistencia local y QA.
- Reabrir decisión de 2 jugadores únicamente tras sandbox individual muy divertido/estable, tres personajes, ciudad/karma/eventos y exploración individual suficiente, y cuando usuario lo solicite. No prometer que costará poco adaptar física/destrucción después.
- COOP_PLAN.md queda investigación a futuro; las referencias N0/N1/N02 anteriores están DEFERRED, no prioridad. Ninguna prueba temprana de red bloquea vuelo.

## 6. Próximo trabajo aprobado para PLANEAR, no ejecutar sin Linux/permiso
L00 auditar Linux real -> FLT-AUD completado de licencias -> FLT-BASE -> FLT-ANIM -> primer distrito -> carro -> pared -> robot -> sandbox -> H02/H03 -> kaiju -> planeta. Congelar expansión de búsqueda cuando un vuelo reproducible cumpla gates. Nada descargado/importado/compilado en esta conversación.
