# IMPACT: BEYOND — Animaciones, personajes y fabricación reutilizable v0.7
Fecha 2026-11-14. Estado: DOCUMENTADO, NO IMPLEMENTADO, NO PROBADO en Linux. Hardware declarado: 64 GB RAM, RTX 4070 8 GB VRAM. Presupuesto de compras RD$0. Usar H01/H02/H03 internos, no nombres anteriores rechazados. Mantener personaje esmeralda masculino de imagen aprobada.

## 1. Decisión operativa
Primero reutilizar **un solo rig humanoide** compartido entre tres personajes jugables y personajes civiles compatibles; después integrar **clips gratuitos** retargeteados; por último crear con Blender/Unreal Control Rig unas pocas poses y transiciones de vuelo específicas. La animación visual sigue el FlightComponent y NO mueve por sí sola la posición del héroe a velocidades supersónicas. Mantener AnimationBP independiente de física/daño/red. Ejecutar en Unreal para Linux, no asumir que un FBX se importa a la primera.

## 2. Biblioteca exacta a auditar
| ID | Recurso y URL oficial | Qué aporta | Condición y decisión |
|---|---|---|---|
| AN01 | Quaternius Universal Base Characters https://quaternius.com/packs/universalbasecharacters.html | humanoides de proporciones superheroicas, regulares y jóvenes, rig compatible | Standard gratuito CC0; verificar que modelo masculino deseado esté en ZIP |
| AN02 | Quaternius Universal Animation Library Standard https://quaternius.itch.io/universal-animation-library | locomoción, correr, salto, golpes, bloqueo, empujar, caer, levantarse | Standard gratis CC0; Pro/Source fuera |
| AN03 | Rokoko 15 free superhero mocap https://www.rokoko.com/es/resources/rokoko-mocap-15-free-superhero-animations | 15 FBX, Mixamo rig, 30 FPS, capturas cuerpo completo y dedos | gratis para proyectos comerciales. No se ha inspeccionado si incluye poses de vuelo; evaluar después de AN02 |
| AN04 | Epic Game Animation Sample 5.8 https://www.unrealengine.com/tech-blog/download-the-latest-game-animation-sample-project-now-updated-for-ue-5-8 | 500+ animaciones, ragdoll, Física Control, levantarse y shoves/tackles/takedowns con dos personajes | Fab Free, en proyecto separado; ficha tiene Allows usage with AI: No: revisar términos antes de transferir archivos a flujo Codex |
| AN05 | Adobe Mixamo https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html | Auto rig y clips de humanoides bípedos | gratis con Adobe ID, uso en videojuegos comerciales; no criaturitas con alas/colas/brazos extra; no redistribuir biblioteca de origen |
| AN06 | Mesh2Motion https://mesh2motion.org/ | auto-rig y más de 170 animaciones, esqueletos para Kaiju, dragón, araña, humano, pájaro y otros | aplicación MIT, modelos/animaciones del autor CC0, export FBX/GLB; prioridad kaiju |
| AN07 | MakeHuman/MPFB https://www.makehumancommunity.org/about/license.html | humanos editables y variantes civiles | assets base CC0; licencia de extras comunitarios por separado; salida FBX/Blender |
| AN08 | Blender + Rigify https://docs.blender.org/manual/en/latest/addons/rigify/index.html | edición y rigging de mallas y poses, Python automatizable | gratuito; Rigify no resuelve por sí solo skin weights ni poses |
| AN09 | KinemIQ https://kinemiq.app/free-game-animations/ | 35 clips básicos CC0 descarga, biblioteca 460 clips con CC0/CC BY | reserva; cambios de plan beta en noviembre 2026, no dependencia permanente |
| AN10 | Quaternius Animated Mech https://quaternius.com/packs/animatedmech.html | 4 mechas con sus clips | CC0, reserva robots pesados |
| AN11 | Quaternius Animated Monster https://quaternius.com/packs/animatedmonster.html | 4 monstruos animados | CC0, kaiju prototipo |
| AN12 | Quaternius SciFi Essentials Standard https://opengameart.org/content/sci-fi-essentials-kit | 37 modelos Standard anunciados, incluye familia de robot/props dentro del set general | CC0, comprobar robot específico y rig en ZIP antes de prometerlo |

Quaternius: verificar licencia de cada edición real; páginas antiguas indican CC0, nueva QAL agosto 2026 puede cubrir otros archivos. No subir ediciones Source/Pro ni archivos de Fab con distribución restringida al repo público. Rokoko puede requerir formulario. No se ha bajado ninguna biblioteca.

## 3. Fabricación de H01/H02/H03
- **H01 fuerza:** Superhero Male u otro base CC0; traje grafito/placas marfil/acento coral original, rig común. Reutilizar correr/saltar/puño/caída; crear o adaptar hover, despegue, cruise, sonic, freno, agarrar objetos de varios tamaños, lanzamiento y choque. Pose de brazos/torso ajustada por velocidad y masa; manos IK a sockets del carro.
- **H02 genio esmeralda masculino:** mismo humanoide adaptado a diseño aceptado: cabello negro rizado corto, traje negro/grafito con plata y oro, energía esmeralda geométrica. Sin fruta ni símbolos/traje Linterna Verde. Gestos cortos de conjuro (mano abierta/cierre/doble brazo) más vuelo común; escudo, puño, martillo, pinza y manos múltiples son mallas/curvas/VFX reutilizables fuera del rig del héroe. Forma de genio colosal se prepara para fase posterior con mesh separado.
- **H03 desplazador espacial:** base regular/slim compatible, traje índigo oscuro con fracturas azul-violeta; sin cola, orejas ni uniforme Nightcrawler. Reutiliza vuelo/correr/golpe/esquiva; crear 3 poses de desfase, salida y contra-blink. Teleport real validado con colisiones/streaming, no root motion que atraviesa muros.
- **Ciudadanos:** iniciar 3–6 variantes de rig compartido con prendas, materiales, pelo y colores diferentes; idle/walk/run/flee/sit/fall/get-up, 1 animación de pedir ayuda. Distantes no todos SkeletalMesh completos. MakeHuman es reserva para más caras si no choca con estilo.
- **Robot:** de Sci-Fi Standard si modelo adecuado; si no, kitbash en Blender mediante primitivos y piezas rígidas. Módulos de cabeza, brazo, arma/propulsor separados en sockets con identificador y efectos de IA al perder módulo. Evitar corte universal.
- **Mecha:** adaptar uno de los cuatro mechas, separar arma y placa usando geometría adicional. Los clips son del rig particular, NO se retargetean automáticamente desde humanoides.
- **Kaiju:** importar un monstruo CC0 y animar como criatura pesada; alterar morfología y textura en Blender. Preparar cuerno, placa y segmento de cola desprendibles sin gore. Mesh2Motion para rig/animación si clips del modelo son insuficientes. No dar por finalizado solo por escalar modelo.

## 4. Ciclo de animación de vuelo: basado en proyectos existentes
Referencias públicas con permisos cerrados (NO copiar código/assets):
- Minecraft Viltrumite Flight: estados NONE/HOVER/CRUISE/SONIC, rotación de cuerpo, cámara, sonido y speed lock. Fuente https://modrinth.com/mod/viltrumite-flight
- Minecraft Invincible Incorporated: roadmap/changelog con pose drivers, shake, FOV, reentrada, shockwave al aterrizar y edición de keyframes. Fuente https://invincibleinc.net/changelogs/
- Roblox Project Power: boost, lock-on, Breaker/Clamp tras golpes/agarres, recuperación; fuente https://www.roblox.com/games/103072644129928/Project-Power
- Roblox Viltrumite Time: fly, grab, vehicle pickup/throw, dash/lock-on. Fuente https://www.roblox.com/games/122170231721791/Invincible-Viltrumite-Time
Flujo propuesto Grounded -> Takeoff -> Hover -> Cruise -> Sonic -> Brake -> Hover/Land; en cualquier punto HitReaction -> Recovery -> Hover. Additive banking/roll y cambios de brazo/torso según aceleración; sin animar 360 clips para 360 direcciones. Establecer orientación del personaje/cámara con velocidad + input, no con la animación como fuente de verdad.
En primer prototipo bastan unas 6–10 poses/loops de vuelo bien escogidas y variaciones aditivas, además de 15–25 clips terrestres/combatidos reutilizados.
No obligar motion blur; priorizar FOV suavizado, parallax y señales de velocidad, shockwave moderado, sonido de ruptura del aire, pose dinámica y control de cámara.

## 5. Flujo automático Blender/Codex
1. Documentar licencia del modelo/clip EXACTO y guardar versión, hash/ZIP, formatos. Descargar a área aislada autorizada, no todo el catálogo.
2. Importar un base humanoide a Blender Linux o Unreal; fijar 1 rig y coordenadas/escala. Aplicar materiales simples.
3. Codex puede generar scripts de Blender Python para armadura modular (placas con Mirror/Bevel/Solidify), asignación a huesos/sockets, generación de variaciones de materiales y export FBX/GLB. Revisar visualmente weights, clipping y LOD; un script no fabrica automáticamente arte final.
4. Retarget solo clips necesarios con Unreal IK Retargeter; AnimationBP/BlendSpace para movimiento; Control Rig/IK para manos y postura de carga.
5. Si falta un clip, editar pose/short keyframe en Blender/Control Rig o usar una captura gratuita concreta. Film-to-mocap con servicio de terceros solamente bajo licencia/costo verificados, sin asumir créditos ilimitados.
6. Probar 30 agarres, poses con vehículo de diversos tamaños, 20 cambios hover/cruise/sonic y recuperación aérea. Medir FPS/VRAM y compilar paquete Linux. No marcar aceptado por ver un trailer.

## 6. Primera tarea ANIM-01
R01: Quaternius base Standard + UAL Standard, verificar descarga y licencias/rig real. R02: seleccionar 1–2 clips Rokoko si aportan acción necesaria. R03: generar poses de vuelo insuficientes en Blender/Control Rig y probar AnimatedBP sobre un solo héroe. R04: probar caída/levantada de GASP solo en laboratorio autorizado y revisar términos de AI/Fab antes de integrarlo.
Aceptación: en paquete Linux hay ground idle/run/jump, hover/cruise/sonic/brake/landing/recovery continuos; sin snaps grandes ni manos atravesando un carro sujeto; usuario valora sensación de velocidad y control >=4/5 (propuesta).
