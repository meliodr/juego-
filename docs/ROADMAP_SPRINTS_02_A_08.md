# IMPACT: BEYOND — Macro-roadmap de los sprints 2 a 15

**Decisiones de dirección — 2026-11-15. Estado: PLANIFICACIÓN FUTURA, NO IMPLEMENTACIÓN.** Se conserva este nombre de archivo para no romper referencias anteriores. La base jugable publicada inspeccionada es PR #18, `prototype/ib-urban-mvp`, commit `20e6a0081e33ab1abb7c8e7b67b55a4bc44a5443`; eso no certifica aceptación visual ni el estado actual del trabajo local de Codex. No interrumpir ni modificar el sprint activo.

El usuario dirige el orden futuro: sprint 3 incluye varias ciudades, mapa y minimapa y planeta circunnavegable; sprint 4 amplía destrucción, enemigos y mecánicas; **sprint 6 entrega H02 y H03 juntos**; **sprint 8 comienza el espacio y añade menú de pausa completo**; **sprint 10 trabaja identidad original/lore ambiental**; **sprint 15 considera héroes aliados NPC ocasionales**. El selector radial de poderes se propone para S7, sin reemplazar sus grandes amenazas. No es una campaña lineal: salvar, destruir, explorar y atender o ignorar eventos son elecciones válidas.

**Fuentes:** `AGENTS.md`, `docs/PLAN_ACTIVO.md`, `docs/CITY_SOLAR_TELEPORT_V04.md`, `docs/CITY_DESTRUCTION_PLANETS_AUDIT_V11.md`, `docs/COMBAT_ROSTER_V03.md`, `docs/FAB_SELECCION_ACTIVA.md`, [Sprint 6 con modelos existentes](SPRINT06_H02_H03_REUTILIZACION.md) y [dirección sandbox, auditoría de herramientas y S8/S10/S15](DIRECCION_SANDBOX_S08_S10_S15.md). Las referencias Linux en documentos históricos no sustituyen Windows/UE5.8.4/DX12 actual. La cronología E4/E5/E6 del plan previo se reordena por decisiones explícitas posteriores; el integrador reconciliará el plan operativo y sus dependencias al cambiar de sprint, sin borrar pruebas ni instrucciones vigentes.

## Principios que no se negocian

- **Reutilizar > configurar > adaptar > crear lo imprescindible.** Priorizar nativo UE y arte ya integrado, Fab poseído y candidatos explícitamente gratuitos, Quaternius/Kenney CC0; no acumular plugins, código ni packs.
- **RD$0 en adquisiciones y plugins.** El usuario permite obtener assets gratuitos útiles, verificando precio final, licencia, formato, tamaño y procedencia. Esto no autoriza nuevas reclamaciones/aceptaciones de términos Fab, instalaciones de plugins, cambios de sistema, ejecución de instaladores ajenos o borrado de archivos personales sin aprobación. No enviar bytes de activos NoAI a modelos generativos ni publicar packs Fab en repositorio público como redistribución suelta.
- Una **rama de integración canónica**, una sola build jugable y un propietario por archivo `.umap/.uasset`; hasta 5 agentes (un integrador más cuatro colaboradores aislados). Un agente maneja el editor y el cook a la vez. PRs encadenados no equivalen a integración en `main`; cero force-push/merge automático.
- En cada sprint: objetivo corto → trabajo paralelo con entregables y rutas exclusivas → integración → compilación/automatización → render DX12/Steam Input/audio → sesión humana → corregir y congelar build. Las métricas son observadas, no predicciones de FPS. Agentes de auditoría read-only no se cuentan como implementadores sin permiso explícito de escritura acotada.
- Cada sprint reutiliza infraestructura anterior y entrega una **experiencia jugable completa**, no solo nuevos ficheros. Meta configurable 1080p/60 FPS con GPU de 8 GB; perfilar p95/p99, VRAM, cargas y físicas.
- La sociedad reacciona sin obligar al heroísmo: distinguir amenaza, alerta local, reputación y relaciones. No bloquear vuelo/poderes por ignorar emergencias. Modo creativo permite configurar consecuencias y reiniciar.

## Sprint 1 — MVP urbano (base publicada; aceptación visual pendiente)

H01: ciudad compacta, supersprint y supersalto, vuelo/boost/frenado/takeoff, coger/transportar/lanzar carro con potencia configurable y colisión activa, panel con hueco, reinicio, aterrizaje de impacto, cámara/HUD, control DualSense Steam Input. Se conservan las ramas activas del integrador y la última build validada; NO asumir que toda esta lista está aceptada. PR #18 contiene entrega y mediciones, pero la calidad de control, pose y presentación sigue siendo un gate humano. Sin planeta/segunda ciudad/héroes nuevos en este sprint.

## Sprint 2 — Periferia y contratos de planeta

**Ficha operativa:** [SPRINT02_FEEL_WORLD.md](SPRINT02_FEEL_WORLD.md). Prioriza Shift/L3 para turbo aéreo, botón separado de Travesía, cinco siluetas de vuelo, velocidad máxima de ensayo mayor, cámara/viento/impacto, más coches reutilizados, costa/naturaleza/agua y objetivo real de 1080p/60 FPS; conserva periferia y contratos del planeta. Valores de velocidad son metas de prueba, no resultados. Verificar checkpoint y trabajo local antes de iniciar o modificar.

Llevar la ciudad a periferia (carreteras, costa, montañas y terreno de fondo) con **regiones** y terreno simplificado. Preparar `BodyId`, `RegionId`, coordenadas globales de doble precisión / marco tangente local (arriba, gravedad y cámara), persistencia de objetos y muros por región y precarga predictiva delante del vuelo. Probar en **laboratorio aislado** una superficie curva con CharacterMovement y colisión, antes de trasladar la ciudad a un planeta. Reutilizar SkyAtmosphere, streaming, HLOD y código auditado (Ephemera solo si permiso, bytes y compatibilidad UE5.8.4 verdaderamente verificados). Si la tecnología de esfera bloquea, mantener MVP urbano jugable y laboratorio separado; no falsear un planeta con teleport de extremos.

**Aceptación:** salir de la ciudad, cruzar al menos una región no urbana con colisión, regresar sin pérdida de estado; horizonte lejano coherente y logs de carga/VRAM/errores, además de la matriz de sensación y controles de la ficha S2.

## Sprint 3 — Planeta circunnavegable + varias ciudades + mapa/minimapa

[Detalle S3/S4/S5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md). S3 conserva cuatro ciudades y circunnavegación y exige **puños básicos en tierra/aire, agarre de postes/contenedores/rocas/cajas además de vehículos**, variedad de coches, refinamiento de cámara/animaciones, ambientes planetarios y mapa/minimapa. Combate avanzado y enemigos son S4.

**Objetivo:** vuelo continuo alrededor de un planeta ficticio pequeño, volver a la misma ciudad sin cambio visible instantáneo de borde; tiempo de vuelta por radio, curvatura, control y streaming **medidos**, propuesta de ensayo 8–12 min a velocidad de travesía cuando sea viable. No cargar la esfera ni todas las ciudades con física detallada simultáneamente.

**Cuatro ciudades propuestas reutilizando un único kit y variaciones:**
1. Central: rascacielos/avenidas/azoteas.
2. Portuaria: muelles/contenedores/objetos pesados.
3. Costera: edificios bajos/paseo marítimo/vuelo sobre agua.
4. Montaña: puentes, desniveles, acantilados.
Cada ciudad tiene **núcleo visitable con colisión**, una acción local y silueta reconocible. Reusar fachadas/ventanas/portales y variaciones de materiales/altura; no importar cuatro kits grandes. Territorios intermedios ligeros y caminos/zonas de aterrizaje reales donde se anuncien jugables. IDs estables desde S3; nombres e identidad final se consolidan en S10.

**Mapa global:** proyección de coordenadas de planeta, marcadores de ciudades, héroe, orientación, regiones descubiertas y destino. **Minimapa local:** marco tangente al planeta, giro con cámara opcional, altura, brújula y zona cargada. Sin saltos de antimeridiano o polos; test de latitud y cambio de ciudad. Reutilizar UMG/material de mapa o bake si cuesta menos que SceneCapture2D frecuente en 8GB VRAM.

**Aceptación:** tres vueltas de prueba, ida y vuelta entre cuatro núcleos, circunnavegación sin bordes, marcadores correctos, minimapa útil en tierra/aire, cargas sin fallos de colisión ni actores duplicados; ciudad inicial conserva muro/carro. Incorporar golpes y agarres de categorías no coche según ficha.

## Sprint 4 — Destrucción y enemigos

[Detalle S3/S4/S5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md). Expandir puños a combos, uppercut y golpes aéreos; **láseres y telequinesis de prototipo**, enemigos terrestres/aéreos, reacciones por material, cámara/VFX/sonido y **vuelo rasante sin cambio involuntario a correr**. Cráteres en superficies preparadas **con colisión real**. Destrucción amplia por capas y streaming; no declarar cada triángulo del planeta arbitrariamente fracturable a 60 FPS. H02 conserva en S6 la identidad de sus construcciones esmeralda.

Ampliar fachadas preparadas con Chaos o paneles configurados. Medir fragmentos, colisiones de huecos y memoria; elegir un sistema principal. Robot pequeño modular, dron y rival aéreo progresivamente: daño, agarre/lanzamiento, ataques anunciados, evasión y recuperación. **Desarme funcional:** quitar propulsor/arma cambia comportamiento; un único PartId por pieza. Reusar Physics/Chaos, Niagara cosmético, robot riggeado legal y animaciones; verificar Free Stylized Sci-Fi Scout Bot / Sci Fi Robot 13 antes de adoptar, sin asumir acceso Fab. Añadir golpe pesado/embestida si la física básica lo permite.

**Aceptación:** 50 impactos y 20 resets de paredes, 20 desarmes, 20 secuencias con rival, sin paredes fantasma, ataques de miembros ausentes ni softlocks; pruebas adicionales de láser, telequinesis, terreno y vuelo rasante de la ficha.

## Sprint 5 — Ciudad reactiva y día/noche

[Detalle S3/S4/S5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md). **36 minutos reales por día completo** como propuesta inicial configurable 18/36/72; reloj solar planetario común, horas locales coherentes, amanecer/atardecer, luces/ventanas y audio nocturno. Priorizar iluminación existente y medir SkyLight/time slicing antes de plugins experimentales.

Pocos ciudadanos y eventos: huida/refugio, robo, alerta local separada de reputación y amenaza, atribución causal correcta, modo creativo para desactivar consecuencias. Reusar pocos rigs, navegación/StateTree cuando aporte; no MassCrowd masivo. Un evento importante activo como límite inicial de ensayo, sin avisos obligatorios ni escalada infinita por ignorarlos. Verificar FPS durante vuelo rápido. La profundidad social se amplía en S10 y héroes aliados en S15.

## Sprint 6 — DOS HÉROES NUEVOS: H02 + H03

**Ambos jugables en la misma build**, no solo arte. Modelos **preexistentes**: Fab ya reclamado o gratis verificado, esqueletos/ropa/animaciones existentes; NO cuerpos/pelo/trajes completos desde cero. [Ficha de reutilización](SPRINT06_H02_H03_REUTILIZACION.md).

H02 genio cósmico masculino esmeralda: identidad propia sin icono de Linterna Verde, escudo, mano/pinza para agarre, puño/martillo con daño compartido y movimiento individual. H03 masculino ágil índigo/negro: teletransporte corto, contra-blink y ataque de reposicionamiento; destino seguro por cápsula/colisión/región cargada. Ambos usan daño/agarre, mundo, cámara, acciones y físicas sin duplicarlos ni forzar GAS sin comparación. Selector claro H01/H02/H03. Preparar IDs/datos de poderes para futura rueda sin convertirla en requisito previo del sprint.

**Aceptación:** 30 acciones de cada tipo H02 sobre los mismos carro/robot/muro; 50 blinks válidos/inválidos H03; ambos juegan solos en todas las ciudades con mismas reglas y assets autorizados.

## Sprint 7 — Grandes amenazas y combinación de poderes

Robot pesado y **un kaiju mediano preparado**, no diez criaturas. SciFi Beast06 Bull ya reclamado como candidato tras licencia/bytes/prueba. Tres héroes usando reglas compartidas de impacto/piezas; rescates y miniinvasión pequeña con límites IA/física. Perder piezas cambia capacidades, sin gore explícito ni corte universal.

**Propuesta adicional de producción:** primera rueda radial de poderes, estilo de interacción solicitado por el usuario, 6–8 espacios, selección con mando/ratón, vuelta rápida a poder anterior y tiempo lento opcional. No copiar UI de GTA V ni seleccionar y activar accidentalmente a la vez. [Diseño y reutilización](DIRECCION_SANDBOX_S08_S10_S15.md). Fecha S7 propuesta; no altera que menú de pausa completo se entregue en S8.

## Sprint 8 — COMIENZO del espacio + MENÚ DE PAUSA

Ciudad → altura orbital → regreso a **misma región y estados**. Después laboratorio Luna y viaje reversible cuando haya superficie jugable real. Sol visible no visitable; destinos adicionales en sprints siguientes. Reusar Sky Atmosphere, coordenadas planetarias y Ephemera/Cosmic Architect solo con derechos/dependencias/rendimiento probados. No simular varios planetas con todas las físicas.

**Decisión explícita:** menú de pausa completo con continuar, mapa, poderes, controles, opciones gráficas/audio/cámara/accesibilidad, guardado/carga seguro cuando esté validado, reinicio de zona con confirmación y salida. Pausa congela correctamente mundo/física/eventos/tiempo y al reanudar no dispara acciones mantenidas. Reutilizar UMG; evaluar CommonUI y UltimateUI MIT en sandbox con autorización, sin migrar todo por defecto. [Ficha S8](DIRECCION_SANDBOX_S08_S10_S15.md).

## Sprint 9 — Reserva de integración

Alcance no cerrado por el usuario. Propuesta: estabilizar viajes, guardados y menús antes de ampliar. No inventar entregables obligatorios ni nuevas localizaciones sin decisión posterior.

## Sprint 10 — Identidad propia, lore ambiental y sociedad

**Decisión explícita:** nombres originales de ciudades, edificios emblemáticos, estatuas, instituciones, lugares cotidianos e interacciones que sugieran historia y memoria. No modo historia ni campaña lineal. Lore mediante señalización, monumentos, radio/noticias y consecuencias de eventos; nombres finales requieren criterio creativo del usuario. Cada lugar debe tener función o reacción, no solo decoración.

Reutilizar modelos de edificios y personajes autorizados para landmarks y estatuas. Inspiraciones de Invincible, Young Justice y Dispatch se traducen en escala de amenazas, redes de héroes y respuesta a emergencias, no copias de nombres/trajes/argumentos. Mantener libertad de salvar, destruir o ignorar. [Ficha y fuentes](DIRECCION_SANDBOX_S08_S10_S15.md).

## Sprints 11–14 — Reserva, sin alcance aprobado

Consolidar sistemas e interacciones conforme a pruebas y decisiones futuras. No prometer cuatro nuevos sistemas grandes solo por rellenar números. Preparar para S15 datos de relaciones, disponibilidad y servicios/eventos compartidos cuando sean necesarios, sin introducir héroes NPC activos antes de tiempo.

## Sprint 15 — Otros héroes buenos, no jugables

**Decisión explícita:** héroes originales que interactúen ocasionalmente con mundo/jugador sin ser seleccionables. Propuesta inicial: defensor, rescatista y técnico; ayudan en emergencias, reaccionan al daño y se retiran. No compañeros permanentes ni salvadores automáticos de todo evento. Compartir acciones, IA/regiones y datos de relaciones; actores lejanos abstractos para preservar 60 FPS. Modelos/animaciones existentes de Fab o CC0 adaptados lo mínimo. [Ficha S15](DIRECCION_SANDBOX_S08_S10_S15.md).

## Qué será medido en cada etapa

- Correctitud: actor/posición/región, colisión, carga, daño y reinicio; en UI, pausa/cancelación/foco; en eventos, origen y resultados sin duplicados.
- Rendimiento: 1080p DX12, audio activo, frame time p95/p99, GPU/CPU/VRAM, streaming y picos de física. Comparar ciudad/naturaleza/día/noche y recorrido continuo, no solo teleports automatizados.
- Experiencia: sesión de 10–15 minutos y evaluación humana de control, poder y deseo de repetir, objetivo orientativo >=4/5 sin inventar puntuaciones.
- Gestión: checkpoint reversible, logs ligados al SHA, licencia/origen de cada recurso y una integración. Probar MCP real antes de afirmar su uso; configuración de agentes no equivale a agentes ejecutados.

**Este roadmap no acredita implementación ni aceptación de sprints futuros. El trabajo activo de Codex conserva sus archivos y entorno.**
