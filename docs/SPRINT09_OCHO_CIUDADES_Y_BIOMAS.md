# IMPACT: BEYOND — SPRINT 9: 8 ciudades mayores, biomas y mundo escalable

**2026-11-15 · Estado: PLAN FUTURO, NO IMPLEMENTADO.** Este documento sustituye la "reserva de integración" del sprint 9 tras decisión expresa del usuario. S3 mantiene 4 ciudades + planeta + mapa/minimapa; S4 combate/destrucción; S5 día/noche; S6 H02+H03; S7 amenazas; S8 espacio inicial y menú pausa; **S9 escala el mundo a ocho ciudades físicas y más grandes con naturaleza diversa**; S10 identidad y nombres artísticos. No tocar el trabajo activo de Codex, no descargar/instalar nada ni fusionar main.

## 1. Referencias: qué extraer exactamente

| Juego | Principio que sí aporta | Implementación en IMPACT | No copiar |
|---|---|---|---|
| Megaton Rainfall https://store.steampowered.com/app/430210/Megaton_Rainfall/ | planeta, vuelo supersónico, ciudades semiprocedimentales | coordenadas planetarias + representación orbital lejana + precarga predictiva de ciudades al aproximarse | arte/código propio del juego |
| Marvel's Spider-Man 2: https://www.gdcvault.com/play/1034212/Applied-Mesh-Analysis-Automating-Distant y https://gdcvault.com/play/1034803/Technical-Artist-Summit-Procedural-Tool | generación procedural de mundo + LOD distante automatizado | fábrica de barrios que prepara manzanas y fachadas repetibles; HLOD/impostores por distrito | herramientas propietarias de Insomniac |
| Prototype 2 https://store.steampowered.com/app/115320/Prototype_2/ | distritos con distintas amenazas e interacción con vehículos | perfiles de ciudad/zona con facción, ambiente, peligros y piezas agarrables; reglas de daño compartido | identidades y poderes biológicos |
| Red Faction Guerrilla https://www.redfactionwiki.com/wiki/Geo-Mod_2.0 | destrucción basada en estructuras preparadas | colisiones reales de partes y soportes con Chaos cerca del jugador y diffs persistentes | física estructural de todo el planeta a la vez |
| No Man's Sky https://store.steampowered.com/app/275850/No_Mans_Sky/ | viaje sin corte y generación procedural reproducible | regiones con semilla + mapas de bioma y terreno/vegetación por distancias | universo ilimitado |
| Just Cause 3 https://store.steampowered.com/app/225540/Just_Cause_3/ | islas grandes, verticalidad, cadenas físicas emergentes | puertos, puentes, torres y parques industriales con miniinteracciones de objetos/poderes | repetir bases idénticas en cada ciudad |
| Teardown https://store.steampowered.com/app/1167630/Teardown/ | abrir atajos mediante destrucción | pared/carretera/puente roto altera paso y estado de ciudad | convertir el planeta completo a vóxeles activos |
| UNDEFEATED https://store.steampowered.com/app/1116960/UNDEFEATED/ | patrullaje opcional con incidentes | alertas de región que pueden ignorarse y dos o tres rutas de rescate/lucha por ciudad | misiones obligatorias |
| Hulk Ultimate Destruction | vehículos convertidos en herramientas | objetos con perfiles Weaponizable/Carryable/Throw, sin sistema de agarre exclusivo por modelo | animaciones y assets propietarios |
| Crackdown 3 | exploración vertical urbana | torres, tejados, rampas de supersalto y landmarks que premian movilidad | coleccionables obligatorios |

## 2. Ocho ciudades, todas mayores que los núcleos S3

Plan cuantitativo de ensayo: huella urbana total 38–64 km², NO 38–64 km² construidos a mano; el centro con arte rico es una fracción y el resto se genera mediante parcelas, calles modulares, variaciones y colisiones/objetos simplificados según distancia.

| Tipo | Número | Huella urbana por ciudad | Centro curado/alto detalle |
|---|---:|---:|---:|
| Metrópolis Central y Portuaria | 2 | 8–12 km² | 1–2.5 km² |
| Ciudad Costera y de Montaña | 2 | 5–8 km² | 0.8–1.8 km² |
| Regionales nuevas | 4 | 3–6 km² | 0.5–1.2 km² |

Núcleos S3 que se expanden: CITY_CENTRAL (rascacielos), CITY_PORT (industria/puerto), CITY_COAST (playa/malecón), CITY_MOUNTAIN (montaña/puentes). Cuatro nuevas: CITY_FOREST (valle boscoso y campus), CITY_DESERT (altiplano árido y energía solar), CITY_WETLAND (lago y canales) y CITY_HIGHLAND (altiplano frío o volcánico/observatorio). Nombres literarios y símbolos serán aprobados en S10; IDs son estables.

Cada ciudad debe ser FÍSICAMENTE VISITABLE, no solo skyline: superficie colisionable, varias calles, plazas/azoteas, mínimo un landmark y dos interacciones/escenarios particulares. No pedir ocho kits únicos; usar el mismo kit urbano existente con CityRecipe y variantes de materiales, tipologías, altura, parcelas y props, y recursos exteriores Fab/CC0 legítimos. No importar City Sample completo si basta un subconjunto.

## 3. Planeta y áreas verdes (escenario inicial, no números fijados)

El planeta de ejemplo anterior de R=25 km se vería demasiado compacto para ocho ciudades y grandes distancias. **Ensayar radio 60–90 km** y seleccionar según curvatura, horizonte y streaming. En radio 70 km: circunferencia ~439.8 km, superficie total ~61,575 km². A 400m/s, vuelta ecuatorial ~18.3 min; a 800m/s ~9.2 min (a velocidad fija y sin paradas). El sprint 9 no impone velocidad de 800m/s a ras del suelo: a 60FPS son 13.3m/frame; requiere barridos y precarga, con control de cámara. Sprint 3 deberá validar cómo escala el planeta antes de aceptar ese radio.

**Distribución propuesta de la superficie (suma 100% incluyendo océano)**:
- 40% océanos/lagos grandes.
- 18% bosque templado/tropical.
- 16% praderas/sabanas.
- 10% montañas/altiplanos/acantilados.
- 7% desiertos/tierra seca.
- 3% deltas/pantanos/manglares.
- 5.9% agricultura, cinturones verdes/suburbios.
- 0.1% núcleos urbanos y polígonos de ciudad.

Las zonas verdes/campos/humedales suman 42.9% de la superficie planetaria (~71.5% de tierra firme cuando 40% es agua). En radio 70km, 0.1% representa ~61.6km² de ciudad, consistente con la propuesta urbana. Son máscaras geográficas generables, **no millones de árboles físicos**.

**Escalas jugables de naturaleza:** cinturones verdes de 2–5km junto a núcleos; corredores naturales entre ciudades de 10–40km cuando el mapa lo permita; 1–2 áreas señaladas por bioma con detalle de 0.5–2km², monumento natural y acción física. Bosques de coníferas o selva, pradera/sabana, desierto, acantilados alpinos, manglar/humedal, playas/costas, agricultura y zonas de lago/altiplano. Cambiar relieve/densidad/audio/materiales y distribución de props más que crear decenas de meshes únicos. Mezclar regiones de transición; no cortar bosque a desierto con una línea.

## 4. Arquitectura que admite la ciudad novena sin reescribir el motor

- **WorldManifest** de datos: PlanetId, BodyId, RegionId, CityId, geolocalización global doble precisión, marco tangente/gravedad local, semilla, biome, referencia a celdas/recursos, estado y destinos.
- **CityRecipe**: red vial con splines; lotes; tipologías, alturas, paletas; landmarks autorales; densidad de autos/ciudadanos; área dañable y puntos de interés; colisión cercana. Elegir seed y datos para crear barrio 9 en sandbox, no lógica C++ nueva por ciudad.
- **BiomeRecipe**: humedad, temperatura, altitud y pendiente con máscaras precalculadas para tipos de flora/roca/suelo y densidad; seed determinista y referencias de recursos importados legalmente.
- **RegionState**: guardar solo cambios del jugador (muros/carreteras rotos, cráteres preparados, objetos desplazados y eventos), no cada árbol o follaje; rehidratar sin duplicaciones. Evento remoto pasa a estado resumido, no mantener IA/física activa lejos.
- **ObjectInteractionProfile**: componentes existentes de agarre/daño/colisión, definidos por categoría de objeto, no un componente nuevo por asset.

**World Partition no es una solución esférica planetaria por sí solo**: usarlo en áreas locales/celdas/ciudades, coordinado por gestor global de planeta y coordenadas locales. Probar doble precisión LWC, cambios de origen/marcos tangentes y gravedad radial sin asumir que Landscape XY se vuelve esférico simplemente escalándolo. No hacer ocho mapas monolíticos ni editar el mismo .umap desde varios agentes.

**PCG jerárquico**: macrotopografía/máscara a 8–32km, tiles de entorno 0.5–2km, celdas urbanas 128–256m, vegetación/detalles 32–128m como ensayos ajustables. Generar en Editor/cook de ser posible para evitar picos de runtime al viajar a 400–800m/s; dispersión procedural bajo reglas/semillas para contenido repetible. One File Per Actor/Data Layers para autoría y HLOD para los skylines, no HLOD automático asumido para Landscape/agua.

**Costes y visibilidad:**
- CERCA: ciudad interactiva, colisión, peatones/eventos, coches físicos necesarios, vegetación cercana.
- MEDIO: mallas urbanas HLOD, coches dormidos, hierba en instancias, montañas/terreno simplificado.
- LEJOS: contornos de ciudad, impostores/cubiertas de vegetación, sin simulación de miles de rocas.
- MUY ALTO/ÓRBITA: planeta proxy y máscaras/clima visuales, no ciudades físicas completas.
Usar ISM/HISM/Static Mesh Foliage para miles de repeticiones; Actores individuales solo en vegetación dañable seleccionada. Compilar/prefabricar HLOD y colisiones para velocidad. Nanite puede beneficiar roca/fachadas, pero hojas densas y geometría agregada requieren benchmark por overdraw (documentación Epic). Water nativo solo si está autorizado/habilitado y aporta, alternativa plano con material. LOD de Landscape/Water necesita su estrategia propia.

**Precarga:** distancia frente al héroe = velocidad × latencia de carga medida + frenada + margen de seguridad; considerar giros/brusca pérdida de altura. Vuelos rápidos a gran altura requieren proxies con anticipación; ningún jugador atraviesa colisión invisible por una ciudad todavía no cargada. Si el servidor de cargas no alcanza velocidad, reportar explícitamente degradación y diseñar transición segura, no degradar física o FPS en silencio.

## 5. Cadencia del gran sprint 9

1. **S9.0:** auditar el estado real de S3–S8, confirmar radio, seeds, celdas, últimas builds y presupuesto de memoria/SSD; rama canónica y checkpoint.
2. **S9.1:** construir/validar fábrica CityRecipe y agrandar 1 ciudad existente; comparar coste de HLOD, estabilidad de parcelas y guardado; iterar sin romper landmarks.
3. **S9.2:** ampliar las otras tres ciudades y crear las cuatro nuevas; 1 landmark + 2 interacciones por ciudad, 3 o más familias arquitectónicas con los mismos kits; revisar colisiones y diferencias.
4. **S9.3:** construir corredores de costa, bosques/pradera, montaña, humedales y región árida, con puntos de interés y audio/vegetación variados; no generar todas las mallas en memoria.
5. **S9.4:** streaming/persistencia, cambio H01/H02/H03, mapa, día/noche, eventos y destrucción en las ocho, prefetch en vuelo y vista orbital.
6. **S9.5:** QA y build Win64 DX12 Steam Input, pruebas de 8 entradas/salidas, 3 vueltas completas, guardado de daños/reentrada, FPS/VRAM con audio/controles reales.

Cinco roles: A1 integración/planeta/cámara/build; A2 CityRecipe y módulos urbanos; A3 BiomeRecipe/naturaleza/agua; A4 QA/perfil/continuidad; A5 assets/licencias/deduplicación. Solo A1 controla el editor y mapa de integración; los demás trabajan en laboratorio y entregables aislados.

## 6. Criterios de aceptación del S9

- **Ocho ciudades visitables**, cuatro ampliadas y cuatro nuevas, con rutas y edificios/props reales cerca, no ocho fondos; huella de 38–64 km² como objetivo de producción por ajustar según mediciones.
- **Mínimo seis familias de bioma perceptibles** con rutas y transiciones, elementos de paisaje colisionables donde se permite jugar.
- **Mapa/minimapa planetario conservado**, IDs estables, regreso a una zona con daño para verificar persistencia.
- **Tres circunnavegaciones** y ocho llegadas con streaming y colisión fiables; zonas bajas no atraviesan edificios al aparecer.
- **1080p/60 FPS** objetivo con RTX 4070 Laptop/8GB, GPU/CPU/VRAM/hitches medidos; p95 <=16.67ms y p99 <=20ms como umbrales de ensayo aspiracionales; audio y DualSense encendidos, sin teleports ocultos en ensayo continuo.
- Un procedimiento demostrado para añadir la **ciudad 9 como CityRecipe de prueba en mapa aislado sin modificar el controlador planetario ni H01**. No incorporarla al mundo todavía. Es la prueba de que es escalable.

## 7. Fuentes técnicas Unreal 5.8 / material de investigación (no dependencias externas)

- World Partition: https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine
- HLOD: https://dev.epicgames.com/documentation/unreal-engine/world-partition---hierarchical-level-of-detail-in-unreal-engine
- PCG modos jerárquicos: https://dev.epicgames.com/documentation/unreal-engine/using-pcg-generation-modes-in-unreal-engine?lang=en-US
- PCG + WP: https://dev.epicgames.com/documentation/unreal-engine/using-pcg-with-world-partition-in-unreal-engine
- Foliage instancing: https://dev.epicgames.com/documentation/unreal-engine/foliage-mode-in-unreal-engine
- LWC: https://dev.epicgames.com/documentation/en-us/unreal-engine/large-world-coordinates-in-unreal-engine-5
- Landscape technical: https://dev.epicgames.com/documentation/unreal-engine/landscape-technical-guide-in-unreal-engine
- Nanite aggregate foliage: https://dev.epicgames.com/documentation/unreal-engine/working-with-naniteenabled-content?lang=en-US

**Estado:** solo documento de planificación. No se modificó gameplay ni el proyecto Unreal. Sin descarga, compra, reclamo Fab, plugin instalado ni merge main.
