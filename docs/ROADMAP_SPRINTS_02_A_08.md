# IMPACT: BEYOND — Macro-roadmap de los sprints 2 a 8

**Decisión de dirección — 2026-11-15.** Estado: PLANIFICADO / NO IMPLEMENTADO. **Sprint 1 sigue en ejecución**, y este documento NO inicia ni sustituye su rama, el editor ni la cola de tareas del presente. El usuario dirige el orden futuro: sprint 3 incluye varias ciudades, mapa y minimapa y planeta circunnavegable; sprint 4 amplía destrucción, enemigos y mecánicas; **sprint 6 debe entregar H02 y H03 juntos**; el trabajo espacial empieza en sprint 8.

**Fuentes:** `AGENTS.md`, `docs/PLAN_ACTIVO.md`, `docs/CITY_SOLAR_TELEPORT_V04.md`, `docs/CITY_DESTRUCTION_PLANETS_AUDIT_V11.md`, `docs/COMBAT_ROSTER_V03.md`, `docs/FAB_SELECCION_ACTIVA.md`, y propuesta [Sprint 6 con modelos existentes](SPRINT06_H02_H03_REUTILIZACION.md). Las referencias Linux en documentos históricos no sustituyen Windows/UE5.8.4/DX12 actual. La cronología E4/E5/E6 del plan previo se reordena por esta **nueva decisión explícita**, pero solo se hará cambio al plan operativo cuando el integrador haya cerrado sprint 1 y documente dependencias.

## Principios que no se negocian

- **Reutilizar > configurar > adaptar > crear lo imprescindible.** Priorizar nativo UE y arte ya integrado, Fab poseído y candidatos explícitamente gratuitos, Quaternius/Kenney CC0; no acumular plugins, código ni packs.
- **RD$0 en adquisiciones y plugins.** No descargar/reclamar Fab, instalar plugins, ejecutar scripts ajenos, modificar sistema o borrar archivos personales sin autorización. Licencias, variantes Personal/Professional y uso con IA deben verificarse por producto. No enviar bytes de activos NoAI a modelos generativos ni publicar packs Fab en repositorio público como redistribución suelta.
- Una **rama de integración canónica**, una sola build jugable y un propietario por archivo `.umap/.uasset`; hasta 5 agentes (un integrador más auditores/artistas/QA aislados). Un agente maneja el editor y el cook a la vez. PRs encadenados existentes no equivalen a integración en `main`; cero force-push/merge automático.
- En cada sprint: plan técnico corto → trabajo paralelo aislado → integración → compilación/automatización → render DX12/Steam Input → sesión humana de juego → corregir y congelar build. Las métricas son las observadas, no predicciones de FPS.
- Cada sprint reutiliza la infraestructura anterior y entrega una **experiencia jugable completa**, no solo nuevos ficheros. Meta configurable 1080p/60 FPS con GPU de 8 GB; perfilar p95/p99, VRAM, cargas, físicas.

## Sprint 1 — MVP urbano (EN CURSO; no intervenir)

H01: ciudad compacta, supersprint y supersalto, vuelo/boost/frenado/takeoff, coger/transportar/lanzar carro con potencia configurable y colisión activa, panel con hueco, reinicio, aterrizaje de impacto, cámara/HUD, control DualSense Steam Input. Se conservan las PR/ramas activas del integrador y la última build validada; NO asumir que esta lista ya está completamente aceptada. Sin planeta/segunda ciudad/héroes nuevos aún.

## Sprint 2 — Periferia y contratos de planeta

**Sprint 2 tiene ficha operativa ampliada:** [SPRINT02_FEEL_WORLD.md](SPRINT02_FEEL_WORLD.md). Prioriza el nuevo control Shift/L3 para turbo aéreo, botón separado de Travesía, cinco siluetas de vuelo, velocidad máxima de ensayo mayor, cámara/viento/impacto, más coches reutilizados, costa/naturaleza/agua y objetivo real de 1080p/60 FPS; conserva periféria y contratos del planeta. Valores de velocidad son metas de prueba, no resultados. No iniciar mientras Sprint 1 esté en ejecución.



Llevar la ciudad a periferia (carreteras, costa, montañas y terreno de fondo) con **regiones** y terreno simplificado. Preparar `BodyId`, `RegionId`, coordenadas globales de doble precisión / marco tangente local (arriba, gravedad y cámara), persistencia de objetos y muros por región y precarga predictiva delante del vuelo. Probar en **laboratorio aislado** una superficie curva con CharacterMovement y colisión, antes de trasladar la ciudad a un planeta. Reutilizar SkyAtmosphere, streaming, HLOD y código auditado (Ephemera solo si permiso, bytes y compatibilidad UE5.8.4 verdaderamente verificados). Si la tecnología de esfera bloquea, mantener MVP urbano jugable y laboratorio separado; no falsear un planeta con teleport de extremos.

**Aceptación:** salir de la ciudad, cruzar al menos una región no urbana con colisión, regresar sin pérdida de estado; horizonte lejano coherente y logs de carga/VRAM/errores.

## Sprint 3 — Planeta circunnavegable + varias ciudades + mapa/minimapa

**Actualización 2026-11-15:** [Ficha detallada S3/S4/S5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md). S3 conserva cuatro ciudades y circunnavegación, pero ahora exige también **puños básicos en tierra/aire, agarre de postes/contenedores/rocas/cajas además de vehículos**, más variedad de coches, refinamiento de cámara/animaciones, ambientes planetarios y mapa/minimapa. El combate avanzado y enemigos son S4, no S3.



**Objetivo:** vuelo continuo alrededor de un planeta ficticio pequeño, volver a la misma ciudad sin cambio visible instantáneo de borde; el tiempo objetivo de vuelta se decide por radio, curvatura, control y streaming **medidos**, propuesta de ensayo 8–12 min a velocidad de travesía cuando sea viable. No cargar la esfera ni todas las ciudades con física detallada simultáneamente.

**Cuatro ciudades propuestas reutilizando un único kit y variaciones:**
1. Central: rascacielos/avenidas/azoteas.
2. Portuaria: muelles/contenedores/objetos pesados.
3. Costera: edificios bajos/paseo marítimo/vuelo sobre agua.
4. Montaña: puentes, desniveles, acantilados.
Cada ciudad tiene **núcleo visitable con colisión**, una acción local y silueta reconocible. Reusar el mismo kit de fachadas/ventanas/portales y variaciones de materiales/altura; no importar cuatro kits grandes. Territorios intermedios visuales ligeros y caminos/zonas de aterrizaje reales donde se anuncien jugables.

**Mapa global:** proyección de coordenadas de planeta (longitud/latitud o formato elegido), marcadores de ciudades, héroe, orientación, regiones descubiertas y forma sencilla de señalar destino. **Minimapa local:** proyectar en marco tangente al planeta, giro con cámara opcional, altura, brújula y minimapa de zona cargada. Evitar que el minimapa salte en el antimeridiano o cerca de polos; test de latitud y cambio de ciudad. Reutilizar UMG, material de mapa o captura offline/bake si resulta más barata que SceneCapture2D frecuente en 8GB VRAM.

**Aceptación:** mínimo tres vueltas de prueba, ida y vuelta entre cuatro núcleos de ciudad, una circunnavegación sin bordes, marcadores correctos y minimapa útil en tierra/aire, cargas sin fallos de colisión ni actores duplicados; vuelve a ciudad inicial con muro/carro persistente.

## Sprint 4 — Destrucción y enemigos

**Actualización 2026-11-15:** [Ficha detallada S3/S4/S5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md). S4 expande puñetazos a combos, uppercut y golpes aéreos; agrega **láseres y telequinesis de prototipo**, enemigos terrestres/aéreos, reacciones por material, cámara/VFX/sonido y **vuelo rasante sin cambio involuntario a correr**. Incorporar cráteres en superficies preparadas **con colisión física real**. Destrucción amplia por capas y streaming, **no afirmar que cada triángulo del planeta es arbitrariamente fracturable a 60 FPS**. H02 conserva en S6 la identidad de sus construcciones esmeralda.



Ampliar a varias fachadas preparadas con fractura Chaos o paneles configurados, pero NO destrucción arbitraria de toda estructura. Medir fragmentos, colisiones de huecos y memoria; elegir un sistema principal. Robot pequeño modular primero, rival aéreo después: daño, agarre/lanzamiento, señales de ataque, evasión y recuperación. **Desarme funcional**: quitar propulsor/arma cambia el comportamiento; un único PartId por pieza; piezas importantes reutilizables. Reusar Physics/Chaos, Niagara para cosmético, robot riggueado legal y animaciones; preferir `Free Stylized Sci-Fi Scout Bot` / `Sci Fi Robot 13` tras verificación, sin asumir acceso Fab. Añadir golpe pesado / embestida H01 si la física básica lo permite.

**Aceptación:** 50 impactos y 20 resets de paredes, 20 desarmes funcionales, 20 secuencias con rival, sin paredes fantasma, ataques de miembros ausentes ni softlocks.

## Sprint 5 — Ciudad reactiva

**Actualización 2026-11-15:** [Ficha detallada S3/S4/S5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md). S5 añade un **ciclo de día y noche** con **36 minutos reales por día completo** (propuesta inicial, configurable 18/36/72), un único reloj solar planetario consistente entre ciudades, amanecer/atardecer, luces y ventanas nocturnas, sonido ambiental y pequeños eventos/ciudadanos. Preferir componentes de iluminación existentes y medir el coste de SkyLight/time slicing antes de habilitar plugins experimentales.



Pequeño número de ciudadanos y eventos: huir/refugio, incidente de robo, alerta local diferenciada de reputación y amenaza, atribución causal correcta, modo creativo para desactivar sanciones. Reusar pocos rigs/civil, navegación/StateTree si ahora aporta, y las mismas zonas/destrucción de sprint 4; no MassCrowd masivo. Un evento activo importante como límite inicial. Verificar rendimiento durante vuelo rápido.

## Sprint 6 — DOS HÉROES NUEVOS EN EL MISMO SPRINT: H02 + H03

**Ambos jugables en la build final del sprint, no solo prototipos de arte.** Modelos **preexistentes**: priorizar Fab ya reclamado y ficha gratuita verificada, esqueletos/ropa/animaciones existentes; NO esculpir/modelar cuerpos, pelo o trajes completos desde cero. Consultar [ficha de reutilización y selección por héroe](SPRINT06_H02_H03_REUTILIZACION.md).

H02 genio cósmico masculino esmeralda: aspecto propio sin anillo ni identidad de Linterna Verde, escudo, mano/pinza para agarre, puño/martillo con daño compartido y movimiento individual. H03 masculino ágil índigo/negro con teletransporte corto, contra-blink y ataque de reposicionamiento; verificar destino por cápsula/colisión/región cargada. Ambos usan el motor de daño/agarre, mundo, cámara, inventario de acciones y físicas sin duplicarlo ni obligar GAS antes de comparar. Cambiar personaje desde menú/selector claro.

**Aceptación:** 30 acciones de cada tipo H02 en el mismo carro/robot/muro; 50 blinks H03 incluyendo destinos válidos/inválidos, carga de región y ataques sin telefrag; ambos operan individualmente en todas las ciudades con las mismas reglas, cero assets no autorizados.

## Sprint 7 — Grandes amenazas y combinación de poderes

Robot pesado y **un primer kaiju mediano preparado**, no diez criaturas. Reusar el `SciFi Beast06 Bull` ya reclamado como posible referencia de rig/animación solo tras licencia/bytes/prueba. Tres héroes usando las mismas reglas de impacto y piezas, situaciones de rescate/eventos urbanos, miniinvasión pequeña; límites de IA y física por distancia. Kaiju pierde funciones según piezas, no gore explícito ni corte universal. QA de cargas y materiales.

## Sprint 8 — COMIENZO del espacio

Primero ciudad → altura orbital → regreso a **la misma región y estados**. Después laboratorio Luna y viaje reversible cuando realmente exista superficie colisionable jugable. Sol visible no visitable; dos planetas más quedan en sprints siguientes. Reusar Sky Atmosphere, marcos de coordenadas planetarios y demostraciones Ephemera/Cosmic Architect solo si derechos/versiones/dependencias y rendimiento se prueban. No simular varios planetas con todas sus físicas simultáneamente.

## Qué será medido en cada etapa

- Correctitud real: actor/posición/región, colisión, carga sostenida, daño y reinicio.
- Rendimiento: 1080p DX12, frame time p95/p99, GPU/CPU/VRAM, tiempo de streaming y picos de física; comparar centro urbano y naturaleza.
- Experiencia: grabación 10–15 minutos y 3 notas humanas (control, sensación de poder, deseo de repetir), objetivo orientativo ≥4/5.
- Gestión: rama/checkpoint reversible, logs vinculados a commit, licencia y origen por recurso, comparativa principal/respaldo y un solo responsable de integración.

**Ninguna de estas etapas futuras está implementada o aceptada por este documento. El integrador terminará y validará primero Sprint 1.**
