# IMPACT: BEYOND — Movimiento completo y nuevos creadores v1.0
Fecha: 2026-11-14. ESTADO: AUDITORÍA DOCUMENTAL, sin proyecto Unreal compilado ni animaciones importadas en Linux. Objetivo **1 jugador offline**, compras de recursos RD$0, hardware declarado 64GB RAM / RTX4070 8GB VRAM. Cooperativo expresamente MUY LEJANO, no pruebas N0/N1.
Esta decisión complementa FLIGHT_REUSE_AUDIT_V09, ANIMATION_CHARACTER_PIPELINE_V07 y ANIMATION_FEEL_OPTIMIZATION_V08. Si una referencia contradice esta ruta de locomoción, priorizar este documento.

## 1. Requisito nuevo explícito: caminar y correr bien
Los 3 héroes deben **caminar, trotar, correr/esprintar, girar, saltar, caer, aterrizar, iniciar vuelo desde suelo o salto, mantenerse en hover, acelerar, frenar, descender/aterrizar y retornar a caminar** sin cambiar de pawn ni perder cámara o control. La movilidad terrestre no es decorativa; podrán pasear por ciudad, llevar objetos, esquivar, pelear y rescatar civiles desde tierra. Cada héroe usa mismo controlador locomotor y AnimatorBP base, pero postura, gestos y aceleración visual distintivos.

### Movimiento principal de un solo personaje
- Grounded: Idle -> Walk -> Jog/Run -> Sprint con velocidades por DataAsset, input WASD camera-relative, transiciones por velocidad real/desired gait y dirección. Opciones para caminar despacio y Shift sprint terrestre.
- Turns: start/stop/turn-in-place/pivots según necesidad; al principio 1–2 clips de giro, no integrar locomotion framework grande.
- Jump/Fall: salto físico desde suelo, caída controlada y landing soft/heavy.
- Takeoff: F desde suelo o aire; Capsule/CMC cambia Walking/Falling a Flying; animación corta arranca sin bloquear input.
- Hover: ascender/descender Espacio/Ctrl; WASD mueve respecto a cámara.
- Cruise/Sonic: Shift en el aire es boost, se preserva impulso/giro progresivo y lectura visual.
- Brake: X frena; animación responde desaceleración, no velocidad instantáneamente 0.
- Landing: al aproximarse al suelo y descender, transición a Walking con velocidad terrestre razonable. Nunca teletransportar debajo del suelo ni imponer larga escena.
- Hit -> Recovery: aterrizar o recuperar aire según estado; input y caída vuelven a funcionar.
Control: Shift sprint en suelo/boost en aire; F vuelo on/off; Espacio salto suelo/ascender aire; Ctrl agacharse opcional suelo/descender aire; X freno aire. No mapear una tecla a dos habilidades simultáneas ambiguas.
El movimiento gameplay reside en CharacterMovement/FlightComponent, animación en AnimationBP con BlendSpaces y blending/inertialization según caso. Reusar skeleton UE Manny o rig humanoide elegido, no portar ALS, Lyra y GASP completos juntos.

## 2. Creadores y proyectos adicionales auditados
| Creador / fuente | Qué está publicado | Licencia/acceso observado | Veredicto para IMPACT |
|---|---|---|---|
| **Epic Games — Third Person / Combat** https://dev.epicgames.com/documentation/en-us/unreal-engine/third-person-template-in-unreal-engine | Manny/Quinn; ground move/jump y mallas; Combat variante ataques, combos, enemigos e interfaz | Incluido en Unreal; documentación actualizada UE5.8 | **BASE ÚNICA**: Third Person Combat; añadir caminar (plantilla por defecto corre/salta), sprint y flight |
| **Epic Games — Game Animation Sample UE5.8** https://www.unrealengine.com/tech-blog/download-the-latest-game-animation-sample-project-now-updated-for-ue-5-8 | 500+ animaciones, motion matching, stop/start/pivots, ragdoll/get-up y shoves/tackles. Documentación permite migrar Animation Sequences a otro proyecto: https://dev.epicgames.com/documentation/en-us/unreal-engine/game-animation-sample-project-in-unreal-engine | Free en Fab; ficha "Allows usage with AI: No" debe verificarse antes de usar contenido con Codex o migrar a repositorio público | **LABORATORIO SEPARADO PRIORITARIO** para movimiento natural, no importar todo Motion Matching |
| **Jan Enri Arquero / ProdiG66 — ManOfSteel** https://github.com/ProdiG66/ManOfSteel | C++ MIT vuelo hover/boost/lean/landing/camera; proyecto UE5.3, 140 rutas de vuelo .uasset pero punteros LFS en GitHub | MIT para código propio; licencia de terceros animación por comprobar | **PRIMER DONANTE VUELO C++**, no modelo/personaje Superman ni assets LFS faltantes |
| **Haez** https://www.youtube.com/watch?v=YfB887DF9dM | Video tutorial UE5 Superhero Flight, inclina personaje, estados Fast Flight; descripción corrige nodos y señala uso de cámara en avance | video educativo, descargas enlazadas no auditadas, NO licencia automática de assets | **REFERENCIA PARA BLEND DE VUELO**, no importación directa |
| **LocoDev** https://www.youtube.com/watch?v=mLld-VfBFnQ | vídeo ~43min muestra cambios walking/flying 02:59, camera 03:55, hover 06–11, fast flight 19–28, additive 33:30 | tutorial gratuito, proyecto por Patreon no verificado como gratuitamente libre | **PRIMERA REFERENCIA PARA TRANSICIONES SUELO-AIRE**, implementar conceptos |
| **kn-404** https://github.com/kn-404/UE5_Superhero-Flight-System | README de 8-way flight blend, input camera-relative, CMC, walking/flying; repo SOLO README, proyecto en Google Drive externo | sin LICENSE explícita en GitHub | **REFERENCIA SIN COPIA**, fallback de diseño de controles |
| **Matt Aspland** https://www.youtube.com/watch?v=BuoeWNQOe0Y | tutorial 2025 de AnimationBP y BlendSpace para idle/walk/run/jump, enlaza Mixamo | video gratis; archivos proyecto Patreon no son necesariamente gratis ni bajo licencia abierta | **APOYO PARA GROUND BLEND MINIMALISTA** |
| **PanicPetal / ALS Community** https://github.com/PanicPetal/ALS-Community | C++ MIT, movimiento con start/stop/turn, soporte Linux declarado, plugin UE5.4. Autor recomienda sistemas modernos como Lyra | MIT en LICENSE del código, content puede venir de ALS original con condiciones específicas; migración 5.8 no probada | **RESERVA**, NO integrar locomotion entero antes de comparar Ground de plantilla y GASP |
| **VeryHotShark — RealisticThirdPersonCharacter** https://github.com/VeryHotShark/RealisticThirdPersonCharacter | walk/run/jump/crouch, start-stop/pivots, root-motion y FemaleAnimsetPro+Mixamo | NO LICENSE encontrada en archivos inspeccionados; pack FemaleAnimsetPro de tercero | **REFERENCIA VISUAL ÚNICAMENTE**; prohibida copia sin permiso |
| **Fontiegow — Project 100** https://github.com/Fontiegow/project-100-unreal-engine | GIFs/Blueprint walk/jump/sprint, de momento movimiento simple | README limita uso a personal/educativo, comercial requiere permiso; no licencia MIT vista | **REFERENCIA DIDÁCTICA**, no dependencia |
| **Gorka Games** https://www.youtube.com/watch?v=UP3zWCMPn3E | análisis 2026 de GASP 5.8 y locomoción/Physics Control e integración | tutorial gratis; referencia Epic oficial, no repositorio de assets propio | **APOYO DE INTEGRACIÓN DE GASP**, si se puede usar bajo términos |
| **Tyson Butler-Boschma** https://tjatomica.itch.io/superman-ue5 | demo gratis de vuelo en gran ciudad, sólo Windows, sin fuente Unreal | no copiar activos de CitySample ni personaje o demo | **BENCHMARK VISUAL**, no integrar |

## 3. Política de reutilización concreta
- Para **ground walking/running**: usar Third Person Combat UE y Manny como fuente base. El tercer-person template del motor ya tiene personaje corriendo y salto; agregar caminar, jog/sprint con MaxWalkSpeed + BlendSpace (o Chooser de GASP en laboratorio si la primera solución se siente rígida).
- Para **flight**: aislar ManOfSteel C++ MIT flight/lean/camera, adaptar a Unreal actual/Linux, sin dependencias Stats/HeatVision/TargetLock/Anime BPs rotos. Cuando falten poses usar Quaternius Universal Animation Library Standard CC0 y Rokoko, crear loops Hover/Cruise/Sonic/Brake en Blender/ControlRig.
- Para **ground natural premium**: estudiar GASP completo fuera del proyecto principal, migrar SOLO clips compatibles y permitidos tras revisar Fab/AI; no instalar Motion Matching completo si aumenta VRAM/CPU o colisiona con Flight AnimBP.
- Si tras 1 experimento el run/walk básico tiene mal pie/snap, evaluar OSS ALS Community solamente con licencia/code y riesgo de port; en ningún caso dos frameworks completos.
- **Ningún video crea permiso de copiar**: scripts, assets/animaciones de Roblox, Minecraft, Superman, Flash, Fallout o Spider-Man no entran al juego sin licencia expresa. No incluir clips del Patreon de otros si no son gratuitos y redistribuibles según términos.

## 4. Combinaciones de animación y locomoción que deben quedar bien
- Idle -> Walk -> Jog -> Sprint -> Brake/Stop/TurnInPlace, basado en speed+direction; movimiento instantáneo por Input, apariencia interpolada.
- Sprint -> Jump -> Falling -> F toggled => Takeoff -> Hover -> Boost -> Brake -> Land -> Run. El personaje no pierde inercia bruscamente ni interrumpe controles por un montage largo.
- Ground + carr y enemigo: postura cambia para cargar un objeto, pero caminar/correr debe seguir habilitado según peso/collision.
- Landing suave / aterrizaje fuerte (onda estética si suficiente velocidad), sin daño o Chaos si no corresponde.
- Transiciones de H01 más agresivas, H02 majestuosas, H03 ágiles; mismo skeleton base y CMC; material/accesorios distintos.

## 5. Aceptación técnica de locomoción (GND)
- **GND-01** 10 min caminando, trotando, corriendo/esprintando entre aceras, puertas, rampas, pasos y azoteas; sin pies patinando excesivamente ni cambios bruscos de cámara.
- **GND-02** 30 secuencias Idle->Walk->Run->Sprint->Stop->Turn; un solo input no provoca doble acción, velocidades coherentes a 30/60 FPS alcanzables.
- **GND-03** 30 secuencias Run->Jump->Flight->Cruise/Sonic->Brake->Land->Run, incluido salto desde techo y aterrizaje en superficie inclinada; no teleports, z-fighting, snapping o stuck movement.
- **GND-04** 20 transportes caminando/corriendo y 20 volando con carro/objeto, volumen agarrado no atraviesa sólidos y manos ajustan IK.
- **GND-05** probar impacto con knockdown/camera y recuperar caminar o vuelo sin bloquear teclas; medir p95/p99 y GPU/CPU/VRAM en paquete Linux Vulkan real.
- **GND-06** usuario puntúa fluidez/naturalidad/cámara/respuesta >=4/5 y comunica pies deslizándose, cancelaciones o mares. No puede considerarse PASS solo por compilar.

## 6. Impacto en el backlog y prioridades
Después de **L00 Linux**, ejecutar **FLT-AUD/FLT-BASE** y **GND-00 (ground walking/jog/sprint)** para dar una base real terrestre. Ejecutar **GND-01 (ground-to-flight)** junto a F01/FLT-ANIM y seguir solo después con ciudad/carro/pared.
Evitar duplicar trabajo: Integrador Unreal en una sola rama locomoción, auditor de assets en paralelo sin editar UAssets, QA independiente cuando exista build. No invertir en multiplayer, planeta o masas de NPC hasta que el mismo personaje se sienta bien caminando y volando.
No iniciar motor ni instalaciones en este chat. Estado sigue documental. Próxima evidencia: Linux packaged + GND-01/03, no screenshots de terceros.

## 7. Notas de origen temporal
- Epic GASP 5.8 update: August 12, 2026; fuente oficial:
https://www.unrealengine.com/tech-blog/download-the-latest-game-animation-sample-project-now-updated-for-ue-5-8
- LocoDev tutorial Aug 2023; Haez Feb 2024; Matt Aspland March 2025; fuentes YouTube arriba.
- ALS Community confirma Linux pero autor advierte técnicas de animación antiguas; no asumir compatibilidad UE5.8.
- Mira Youtube o descargas externas solo cuando aporten evidencia concreta; no recolección sin fin.
