# IMPACT: BEYOND — Diseño ampliado de los grandes sprints 3, 4 y 5

**Decisión del usuario: 2026-11-15. Estado: PLAN FUTURO; NO IMPLEMENTADO NI PROBADO.** No iniciar este trabajo ni modificar la build del Sprint 1 o 2 ahora. Esta ficha desarrolla la [hoja de ruta principal](ROADMAP_SPRINTS_02_A_08.md), complementa el [Sprint 2](SPRINT02_FEEL_WORLD.md) y **prevalece para el alcance futuro de Sprints 3/4/5** por decisión más reciente. Mantener S6 H02+H03 juntos, S8 primer trabajo espacial, UE 5.8.4 Windows/DX12, 60 FPS objetivo real, RD$0, single player offline.

## Principios

- **Personaje, planeta y ciudad se sienten jugables antes que verse enormes**. En cada sprint una build única, un integrador, hasta cuatro auxiliares, worktrees separados y cero escritura simultánea de un mismo `.umap`/`.uasset`. El último checkpoint Win64 DX12 con Steam Input se conserva.
- Preferir sistemas y arte **existentes** (Unreal 5.8, código del juego, Fab legítimo, CC0) antes de crear. Elegir 1 candidato + 1 respaldo por funcionalidad, con licencia real, versión/descarga y prueba de consumo de memoria. No reclamar Fab/aceptar licencia, instalar plugins o cambiar sistema sin autorización; el permiso previo para descargar recursos gratis NO reemplaza la confirmación requerida para reclamar Fab.
- No tratar NullRHI como aceptación audiovisual; separar compilado, automatización de física, DX12 renderizado, sesión real con mando/audio y evaluación humana.
- No construir todo el planeta/ciudades como instancias físicas siempre activas. Variar niveles de detalle, precarga según velocidad, streaming, persistencia por región y físicas solo en burbuja cercana. Si la esfera/topología se bloquea, reportar PARCIAL, no simular un bucle falsamente pegando los bordes del mapa.

## Sprint 3 — PLANET ALIVE: ciudades espectaculares y movilidad/combatibilidad

**Experiencia de cierre:** viajar sin límite artificial perceptible entre **cuatro ciudades reconocibles** del mismo planeta, consultar mapa y minimapa, circunnavegar, regresar al mismo distrito y conservar daños/interacciones. Se continúa utilizando H01, código de vuelo y físicas de S1/S2; no estrenar H02/H03 ahora.

### Ciudades y vida visual
1. **Central:** rascacielos, fachadas con ventanas de interior aparente, plazas, azoteas; la identidad principal.
2. **Portuaria:** contenedores, grúas, muelles, caminos industriales, objetos pesados agarrables.
3. **Costera:** playa, zona comercial baja, malecón, parques, mar y paseos.
4. **Montaña:** carreteras en desnivel, puentes, casas, laderas y túneles/viaductos.

Reutilizar Poly Haven, Kit Cidade, materiales HISM, coches existentes, packs CC0/legalmente gratis concretos, variaciones de color, arquitectura modular, vegetación y agua de S2. Cada ciudad requiere núcleo con suelo/colisión y al menos **una interacción local diferenciada**; nunca presentar skyline sin colisión como barrio jugable. Reutilizar el mismo kit en las cuatro con identidades por iluminación, bioma, silueta y accesorios. Repetición consciente > decenas de packs incompatibles.

### Mapa y minimapa
- **Mapa global de planeta** con marcador de H01, latitud/longitud o coordenada BodyId/RegionId, cuatro ciudades, rumbo, lugares descubiertos, zoom y marcador de destino. Los meridianos y polos no deben producir saltos de iconos; un solo estado global.
- **Minimapa local UMG** en marco tangente de la esfera, calle/objeto importante/rumbo y altura; usar datos precalculados, mallas/topografía simplificada o texturas generadas offline **antes** que capturas 3D `SceneCapture2D` cada frame. Cuando el héroe pase de ciudad a travesía, cambiar el zoom y el origen con continuidad.

### Movimiento y acciones de H01
- Refinar animaciones de tierra/aire, sprint, supersalto, impulso, despegue, aterrizaje, giros y frenada; sincronizar audio, IK y cámara. Regresión de 5 modos de vuelo S2 y colisiones a alta velocidad.
- **Puñetazo básico en Sprint 3**: golpe rápido y pesado en suelo/aire; animación con notifies, barrido/trace físico, hit react, sonido y empuje con un único sistema de impactos. Permitirá romper al menos objetivos preparados y golpear coches; todavía no combos/IA compleja del S4.
- **Agarre universal por categorías**: coche, contenedor/caja, poste, roca, barril, resto de pieza preparada. Usar el mismo `Grab/Carry/Throw`, distintos puntos de agarre/volumen/masa/animaciones; no crear `CarGrabSystem` diferente por asset. El actor debe conservar colisión durante transporte, soltarse correctamente si se bloquea, y persistir por RegionId.
- Variedad visible de carros y objetos por región, con sueño físico cuando están inactivos y activación cercana.

### Ensayo planetario
Reutilizar LWC, orientación radial y marco local, gravedad de la esfera, streaming por dirección de vuelo y 3–4 núcleos locales. La vuelta completa es **continua en coordenadas/recorrido**, con proxies visuales y celdas detalladas únicamente al acercarse. Duración objetivo de ensayo 8–12 minutos a velocidad de travesía, calibrada con radio y FPS medidos; no prometer todavía un planeta con cada metro de suelo fracturable.

**Gates S3:** 4 ciudades visitables; 1 circunnavegación verificada por coordenadas y regreso a punto de partida con persistencia, más 2 repeticiones; mapa/minimapa navegables sin saltos; 30 golpes mixtos; 20 agarres de tipos NO coche; vuelo y control visual probados 15min; 1080p/60 FPS aspirado con p95/p99/VRAM registrados.

## Sprint 4 — UNLEASHED POWER: destrucción, enemigos, vuelo rasante y poderes

**Objetivo:** todo debe reaccionar razonablemente a los impactos en la zona jugable; la meta a largo plazo es un planeta ampliamente destructible. **No afirmar destrucción arbitraria total en cada triángulo del planeta durante S4**: es incompatible con 60 FPS y recursos RD$0 sin demostrarlo. Introducir una arquitectura de respuesta a daño reutilizable que pueda expandirse en sprints sucesivos.

### Tres capas + apariencia de destrucción universal

| Capa | Elementos | Método | Colisión y persistencia |
|---|---|---|---|
| D1 — interactivos completos | coches, farolas/postes, cajas, contenedores, rocas, escombros grandes, piezas enemigas | Chaos cuerpos físicos limitados / sockets / estado; física dormida lejos | física real, daño local y RegionId persistente |
| D2 — edificios preparados | cristales, puertas, columnas, fachadas, paneles, techos parciales y pisos de algunas estructuras | Chaos Geometry Collections/fractura por niveles, Physics Fields o estados de panel; soporte estructural simple/PlotBuilder MIT solo si aporta | huecos realmente transitables, pocos fragmentos con colisión, reconstruir tras reinicio |
| D3 — suelo/terreno local | asfalto, aceras, tierra y roca golpeados por puño/impacto/láser | parches de cráter **prefracturados o intercambio de malla** con profundidad limitada; deformación visual/material para alrededores | cráter jugable con colisión ajustada de forma coherente, sin túneles infinitos ni actualizar todo Landscape/cube-sphere arbitrariamente |
| D4 — mundo lejano | edificios/HLOD, montañas, costa, visuales distante | proxies de daño/hollín/chispa cuando procede; activar detalle solo al entrar a burbuja | no simular fragmentos físicos ni daño lejano invisible |

Cráter inicial: tamaño/profundidad editables, ensayo local 0.2–1.2m de profundidad y radio 1–4m en zonas habilitadas; no depender de decals únicamente cuando el hoyo deba ser transitado/afectar colisión. Limitar simultáneos y consolidar estado al alejarse (semilla, celdas, niveles de daño). Nunca permitir que el impacto rompa suelo y deje caer al héroe fuera del mapa.

**Reutilización prioritaria:** Chaos Fracture, Geometry Collection, Fields (`FS_MasterField`), malla/colisión actual, Niagara, materiales/descal para escombros/hollín. [Epic: Chaos Destruction](https://dev.epicgames.com/documentation/en-us/unreal-engine/destruction-overview), [Fields](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-physics-fields-in-unreal-engine). Ensayar 1 edificio y 1 parche suelo antes de propagar. Alternativa a la fractura arbitraria: piezas en estados con mallas de agujero reales. Ningún mundo completo con físico activo.

### Mecánicas de poder y combate (no mezclar identidades futuras de héroes)

1. **Puños H01**: ampliar golpe rápido/pesado de S3 con combo corto 2–3 golpes, uppercut, puño cargado y golpes en vuelo; retarget de clips reales/AnimBP, AnimNotifies sincronizados con Damage/Impact, knockback, reacción de superficies y cámara. Añadir embestida/air dash que usen velocidad actual.
2. **Rayos láser**: prototipo de visión térmica de H01 o ataque focalizado temporal para probar Beam+Hit; origen de ojos/manos elegido conscientemente, aim cámara, line trace/hitbox autoridad, daño controlado por tiempo y material, VFX Niagara solo representación. Evitar múltiples rayos golpeando al mismo actor en el mismo tick. No copiar identidad visual de otras franquicias.
3. **Telequinesis experimental**: agarrar un objeto cercano sin contacto mediante el componente de agarre existente con modo de portador a distancia/Physics Handle, masa/alcance/line-of-sight limitados, rotar, atraer, empujar/lanzar. Usar H01 en **modo creativo/prototipo** si hace falta probar la mecánica, sin convertirlo en la habilidad distintiva de H01; en S6 H02 podrá refinarla visualmente como manifestación de mano/energía esmeralda. Objetos y colisiones siguen reales.
4. **Defensa/evasión**: bloqueo corto, esquiva aérea y recuperación de impacto; sin invulnerabilidad continua ni cancelaciones que eliminen decisiones.
5. **Poderes contra el mundo**: golpe de choque/onda expansiva mejorada, arraigar/postes, lanzar enemigo/objeto contra estructura; reutilizar daños y tipos de superficie con un único disparador de respuesta.

### Enemigos

**Primera entrega S4:** robot terrestre modular con 2 piezas funcionales y dron aéreo con 2–3 ataques telegrafiados; rival superhumano de vuelo simple como tercera amenaza condicionada a QA. Reusar rigs/animaciones Fab/CC0 autorizados y StateTree/Behavior Tree/NavMesh cuando ya estén disponibles; IA limitada a región activa y distancia. Dañar una parte elimina la capacidad asociada (p.ej. propulsor, arma, pinza); piezas arrancadas pasan a objeto agarrable. Mantener fuera kaiju (S7).

### FLY-LOW: volar a centímetros/metros del suelo sin correr por accidente

**Bug/deficiencia a corregir:** en el S1 actual, en `ImpactBeyondCharacter::Tick` hay transición a `MOVE_Walking` al descender cuando se detecta suelo a <=4cm; `NotifyHit` en modo rápido también puede forzar caminar por contacto de caída. **Proximidad/colisión no equivalen a intención de aterrizar.** Separar el estado de vuelo del estado de suelo y la acción explícita de aterrizaje:
- H01 podrá mantener `MOVE_Flying` o modo custom con **hover bajo** a 0.5–3m sobre suelo, adaptar altura suavemente a pendientes y aceras, colisión de cápsula activa y apuntado sin teleport.
- Si el usuario desea aterrizar, usar F/R3 para salida intencional, descenso sostenido con interacción separada o un gesto de aterrizaje definido; confirmar que no rompe R2/L2 de ascenso/descenso ni el impacto V/arriba. Cuando roza el piso sin intención, debe bloquear deslizar/sostener altura segura, no entrar en locomoción de correr ni foot animation.
- Evitar que el frame de boost/hover atraviese suelo o paredes; QA de barrido a 30/60fps, pendientes/azoteas, objetos llevados y cámaras.
- Poder golpear/craterizar suelo mientras se vuela bajo: impacto real en patch D3, visual y audio sincronizados sin convertir el héroe a Walking.

### Cámara / audio / VFX gratis

Cámara: perfil de vuelo bajo cercano al piso, orientación de mira independientemente del cuerpo, seguimiento elástico, distancia/fov según velocidad, near miss, impactos con micro-shake amortiguado y accesibilidad; sin forced camera y cero cinematics largas en combate. Reusar SpringArm, Camera Shakes nativos, interpolación y VFX con budget; audio por superficies y velocidad.

**Fab precio público Free confirmado por ficha oficial a 2026-11-15 (NO poseído/descargado/comprobado en UE5.8.4):**
- [Niagara Examples Pack — Epic](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600): trazas, impactos, explosiones, estelas, humo, lightning y ejemplos optimizados Niagara Data Channels; candidato principal general, etiqueta NoAI.
- [Free Magic Niagara — Lord Enot](https://www.fab.com/listings/d0fe50c4-6ebe-40d5-b78a-56960832f49e): golpes/olas/trazos, color y escala ajustables, etiqueta NoAI.
- [FX Variety Pack — Kakky](https://www.fab.com/listings/53531e17-369f-4bba-b493-8588f0dec07b): fire/thunder/water/storm, colección antigua: verificar sistema de partículas/versión y rendimiento antes de importar.
- [Magic Energy VFX Materials Lite — GLab](https://www.fab.com/listings/5d240930-5cdf-4eed-b788-66483790e5fd): superficies Wave/Flow/Distortion, gratis en ficha; materiales, NO Niagara; NoAI, GeneratedWithAI indicado como Yes.
- [Free Modular Magic SFX — Le Binh Son](https://www.fab.com/listings/f0422e67-33d3-4c6e-b7a1-1059193960de): WAV de beams/impacts/charges y 22 MetaSound graphs; el autor declara licencia CC BY 4.0: **conservar atribución real** aunque el texto promocional diga que puede omitirse. Evitar dependencia de su plugin comercial; usar WAV con licencia si está permitido.

**Pendientes, NO clasificarlos como Free garantizados para usuario:** [Stylized Laser Beam VFX — Free](https://www.fab.com/listings/e63c140a-4ef9-448a-8e58-133ae1487534) (descripción/free en algunos resultados, pero ficha actual en otra vista tiene selector Buy now/importe opaco); [Shockwave Distortion](https://www.fab.com/listings/db0d837f-3c94-4a67-86a8-5653ebddce12) (autor declara free pero selector Buy now opaco). Verificar precio RD$0 y licencia en la sesión antes de reclamar/usar; opción principal para láser es **Niagara beam nativo** + efecto existente. NoAI: no subir bytes/arte de packs a herramientas generativas. No instalar plugins nuevos sin consentimiento.

**Prueba S4:** 30 golpes+20 combos/uppercuts, 20 lanzamientos telequinéticos, 20 rayos contra materiales distintos, 50 roturas de panel (pared/vidrio/techo), 20 cráteres con colisión real y reset, 20 combates robot/dron y 20 pasadas en hover a 0.5–3m sin auto-walking. Perfil de 60fps con máximo de piezas físicas activas controlado, vista/sonido/cámara real por DualSense.

## Sprint 5 — LIVING WORLD: día/noche y mundo que reacciona

**Decisión de ciclo inicial (propuesta de diseño verificable):** **1 día completo = 36 minutos reales**. Modelo in-game: 24h; aproximación orientativa: **21 minutos de claridad (incluyendo amanecer/atardecer)** y **15 minutos de noche** en región inicial/latitud base. En un planeta esférico, la hora solar local varía según longitud: mantener **UN reloj global de planeta** (rotación/ángulo), no cuatro horarios incoherentes por ciudad. La proporción día/noche cambiará según latitud/estación cuando se agregue, sin tener que simular estaciones para S5. Ajustes de juego alternativos **18/36/72 minutos**, pausa del reloj y saltar a amanecer/mediodía/atardecer/noche en modo creativo. Guardar hora del planeta al cambiar de región y al recargar la sesión; no reiniciar al entrar en otra ciudad.

**Tecnología**:
- Reusar Sky Atmosphere, Directional Light (Sol y opcional Luna), Sky Light, Fog y nubes actuales; actualizar orientación y materiales de forma parametrizada, con clock global. [Epic Sky Atmosphere](https://dev.epicgames.com/documentation/en-us/unreal-engine/sky-atmosphere-component-in-unreal-engine).
- Evaluar Day Sequence UE5.8 **experimental** solo si usuario autoriza habilitarlo y demuestra menos trabajo/mejor control que actor pequeño + curvas nativas; [Epic Day Sequence](https://dev.epicgames.com/documentation/unreal-engine/day-sequence-time-of-day-plugin-for-unreal-engine?application_version=5.8). Tiene `Time Per Cycle` y evaluación presupuestada. No instalar plugins de terceros ni reemplazar iluminación del nivel sin checkpoint/rollback.
- SkyLight con **Real Time Capture/time slicing** cuando la GPU lo tolere, cubemap simplificado y capture menos frecuente si no. [Epic Sky Light](https://dev.epicgames.com/documentation/unreal-engine/sky-lights-in-unreal-engine). Control de cascadas de sombras, farolas/ventanas nocturnas con materiales emisivos, pocas luces dinámicas cercanas y culling; no dar una luz por cada habitación.
- En cuatro ciudades: ventanas cambian de iluminación, farolas por zonas, señales, ambiente de tráfico/audio nocturno, brisa/aves/insectos opcional, sonidos ambientales y algunos ciudadanos con rutinas simples. Las ciudades conservan la misma posición real del sol para el mismo instante global.
- Incidentes y vida: 3–6 civiles en región cargada inicial, huida/refugio ante impactos, alerta/karma/amenaza distintos y 1 evento activo importante con límites. No multitudes masivas. Zona remota duerme sus actores; persistencia de impacto/objetos.

**QA de tiempo:** 3 ciclos acelerados para tests deterministas + **una vuelta completa de 36min en DX12 con audio**, capturas al amanecer/mediodía/atardecer/medianoche, métricas 60fps/día/noche, regreso entre ciudades en horas solares distintas, actualización de luces y sonidos, reinicio/guardado, sombras/reflejos sin tirones. Evitar afirmar clima completo si solo hay luz.

## Cadencia de trabajo y anti regresiones

- **Sprint 3**: mundo/ciudades + punches S3 + categorias de agarre + navegación/mapas + planeta circular. Validar con mapa minimapa y 3 vueltas.
- **Sprint 4**: poder/destrucción/enemigos/hover rasante y cámara/VFX. Validar colisión, IA y física con render/audio.
- **Sprint 5**: global time of day + iluminación/ambiente + pocos civiles/eventos. Medir día/noche, persistencia y 60fps.
- **Sprint 6**: H02/H03 con modelos EXISTENTES, poderes independientes (H02 conservará su identidad de energía esmeralda al asumir telequinesis refinada). **Sprint 8**: espacio, no adelantarlo.
- Un solo integrador de gameplay y mundo canónico, hasta 4 agentes aislados (assets, físicas/IA, animación/VFX, QA). Sin merges a `main` sin autorización, cero cambios en el sprint 1/2 actualmente en marcha.

**Estado de esta ficha: decisión de roadmap, ningún recurso Fab reclamado ni descargado, ningún código de S3/S4/S5 escrito/probado.** Los valores de cráter/FPS/duración del día son criterios iniciales de ensayo, no resultados reales.
