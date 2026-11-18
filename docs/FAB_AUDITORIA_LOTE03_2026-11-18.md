# IMPACT: BEYOND — Auditoría Fab lote 03 (18 de noviembre de 2026)

**Estado: auditoría de fichas públicas y promociones, NO lectura de Mi biblioteca ni integración UE.** Revisión elaborada en 2026-11-18 a partir de resultados de Fab y sus páginas públicas. Se añadieron **30 fichas** distintas (de ellas **27 con rótulo público Free/Gratis** y **3 promociones vigentes anunciadas por la portada oficial de Fab**), sin duplicar las 52 anteriores: ahora hay **82 entradas** en [FAB_CATALOGO_MAESTRO.csv](FAB_CATALOGO_MAESTRO.csv).

**Limitaciones de la evidencia:** un resultado indexado/crawleado puede no reflejar el precio de la cuenta hoy; se reabrieron directamente algunas fichas (incluida DZ Sample Trees, City Props y la página oficial de ofertas), pero NO se realizó una revalidación de cada ficha bajo sesión del usuario. Las fichas promocionales pueden mostrar "Buy now"/precio ofuscado en modo visitante aunque el portal de ofertas las anuncie gratis; se marcaron PROMOCION_HASTA_2026-12-01, **no GRATIS_EN_FICHA**. "Compatible con UE5.8" significa candidato potencial por tener formato UE o FBX; **no se certificó en el editor ni se prometió instalarlo**. El hardware objetivo tiene RTX 4070 Laptop 8GB VRAM, por lo que polígonos, Nanite, memoria y materiales requieren ensayo real.

## Hallazgo más urgente: nueva promoción oficial

La página [Fab Limited-Time Free](https://www.fab.com/limited-time-free), consultada el **18-11-2026**, muestra tres recursos **hasta el 1 de diciembre de 2026 a las 9:59 a.m. ET**:

1. [Absorption FX](https://www.fab.com/listings/81550176-a90e-4e07-8817-c8d1a1d3884b): **SELECCIONAR** para una prueba focalizada del poder de absorción/energía H02; 10 efectos Niagara con SFX y parámetros, NoAI. No implica que la capacidad de juego ya esté resuelta.
2. [Post-Apocalyptic Factory Overgrown Factory Facility & Vehicles](https://www.fab.com/listings/c0226253-11f0-425d-8b2c-df9c42c73f49): **PROBAR**, no adoptar el mapa completo. La ficha detalla 130 mallas, grúa, montacargas, camión, 60+ props, muros y 4 instancias de nivel. Valor en una zona industrial y objetos lanzables; estética abandonada no sirve para toda la ciudad.
3. [Advanced Companion Drone System | AI & Multiplayer](https://www.fab.com/listings/205d550d-0a5f-4d5c-9f3e-8f6254b9dc4f): **DESCARTAR ahora** por exceso de complejidad, replicación y compañeros no planificados. “AI” de la ficha se refiere a comportamiento de NPC en Unreal, **no** a entrenar un modelo, pero no necesitamos este sistema.

Las tres ofertas anteriores del 3 al 17 de noviembre (SciFi Beast06 Bull, 3D Platformer Game Bundle y Underground Tunnel) **han terminado según la página oficial** y sus fichas vuelven a mostrar selector de licencia/compra. **No sabemos si el usuario las reclamó a tiempo**, pues no tuvimos acceso a su biblioteca privada. Si están reclamadas, los derechos adquiridos deben comprobarse en la biblioteca; no solicitaremos nueva compra.

## Reorientación de prioridades según el plan activo real

La versión actual de [PLAN_ACTIVO.md](PLAN_ACTIVO.md) declara **S1–S7 integrado y pruebas funcionales/paquete DX12 completadas** en 2026-11-16; [revisión de cierre S7](IB_S7_CLOSURE_REVIEW.md). Ya NO corresponde presentar W00/W01 como trabajo no hecho. Esta auditoría **no ha vuelto a ejecutar el juego**. En la siguiente revisión artística conviene priorizar realismo de una manzana, arbolado adecuado, malecón, puertos, mobiliario urbano y una escena de prueba sin degradar el perfil previo (~60 FPS informado por la revisión S7, no medido en esta corrida).

Las alternativas de bajo coste más interesantes:
- **[DZ Sample Trees](https://www.fab.com/listings/7c95ac5f-246e-42fb-9d2a-4807db87f69d)**: muestra gratuita UE con cinco árboles Nanite, **cocotero y palmera**, colisiones y control de viento; publicada para UE 5.1+, pero probar específicamente 5.8. Un candidato por especie, medido en móvil de cámara y vuelo, no poblar 10.000 instancias.
- **[City Props Pack | Free Sample](https://www.fab.com/listings/59bc06c7-a293-4282-8b10-cde8fd0d4616)**: banco, aparcabicicletas, parada de autobús y palés en FBX. Permite agregar vida urbana/malecón sin crear objetos desde cero; de momento cuatro, **NO** los 20 objetos que trae el pack pagado.
- **[Modern City Environment](https://www.fab.com/listings/6e72d76b-9cf9-4391-aa38-69772d666ac4)**: escenario urbano PBR con aceras, pasos y park; es **FBX/Blender**, no un proyecto de UE preparado; costo real de colisiones, iluminación y materiales por verificar.
- **[temperate Vegetation: Foliage Collection](https://www.fab.com/listings/6a5ae8db-d80f-4b23-b276-87da390cfe56)** y **[optimized Grass Library](https://www.fab.com/listings/8b68642e-35f4-438e-82b4-799fc2228303)**: evitar instalarlos juntos hasta una escena comparativa de rendimiento/densidad.
- **[Procedural Building Generator Free](https://www.fab.com/listings/783058bc-1afc-472a-96b1-289ad13c6aa9)**: gratuito público pero versión 5.8 no demostrada; comparar primero con PCG nativo y módulos reales.
- **[Starfield FREE](https://www.fab.com/listings/349d203e-aac2-40bc-8c92-7a3f6f89cf31)**: 3 Blueprints + cielos 8K; fondo orbital lejano; no geometría de planetas aterrizables.
- **[Wooden Boat 1 JFG](https://www.fab.com/listings/2716fe7a-ce60-4007-b4d2-4d599f3131b8)**: bote de un mesh sin rig/animaciones; un toque costero, **no resuelve oleaje, fauna o tráfico marítimo**.
- **[Modern Outdoor Wooden Bench Pack](https://www.fab.com/listings/259193d3-db2f-48dc-88b9-d339801d1c95)**: respaldo para variedad de bancos si los cuatro props de City Props no bastan.

**Problema sin resolver con paquetes:** el mar “vacío” requiere **Water System de UE (agua y oleaje), espuma/estelas, audio ambiental y fauna/embarcaciones con lógica**; ningún paquete de bancos o árbol llena el mar por sí solo. Antes de añadir un plugin externo, usar los sistemas nativos del motor. Los Megaplants gratuitos de Quixel (Ginkgo, Elder, Bilberry) se reservan porque su PVE/Nanite Foliage está marcado experimental.

## Nuevos candidatos (solo enlaces públicos)

| Recurso | Precio público | Decisión | Notas |
|---|---|---|---|
| [DZ Sample Trees](https://www.fab.com/listings/7c95ac5f-246e-42fb-9d2a-4807db87f69d) | Free en ficha pública | **SELECCIONAR** P0 | Unreal Engine; 5 árboles Nanite incluyendo cocotero, palmera y alcornoque; viento, estaciones y colisiones según autor; anuncia UE5.1+; 5.8 y VRAM sin probar. |
| [City Props Pack | Free Sample Edition](https://www.fab.com/listings/59bc06c7-a293-4282-8b10-cde8fd0d4616) | Free en ficha pública | **SELECCIONAR** P0 | FBX/OBJ/Blend; 4 objetos: banco, aparcabicicletas, parada y palés; texturas 1-2K; útil en malecón; ninguna animación ni LOD incluidos. |
| [Starfield FREE](https://www.fab.com/listings/349d203e-aac2-40bc-8c92-7a3f6f89cf31) | Free en ficha pública | **SELECCIONAR** P2 | Unreal Engine; 3 Blueprints de estrellas/nebulosas, 8K mapas y fondos; shader/Virtual Camera indicado; SOLO cielo orbital, no planetas aterrizables. |
| [Absorption FX](https://www.fab.com/listings/81550176-a90e-4e07-8817-c8d1a1d3884b) | Promo hasta 1 dic | **SELECCIONAR** P1 | PROMO oficial Fab hasta 2026-12-01 09:59 ET; 10 FX Niagara de absorción con SFX; buen candidato para H02 energía; ficha no muestra 0 USD sin sesión. |
| [Modern City Environment](https://www.fab.com/listings/6e72d76b-9cf9-4391-aa38-69772d666ac4) | Free en ficha pública | **PROBAR** P1 | FBX/Blender; calles, cruces, mobiliario, park, fachadas urbanas PBR; requiere importar y resolver colisión/materiales; no proyecto UE. |
| [[Free] Procedural Building Generator](https://www.fab.com/listings/783058bc-1afc-472a-96b1-289ad13c6aa9) | Free en ficha pública | **PROBAR** P1 | Unreal Engine; generador modular/spline; plugin/Blueprint y UE5.8 sin ensayo; contrastar con PCG nativo y City Sample Buildings. |
| [Post-Apocalyptic Factory Overgrown Factory Facility & Vehicles](https://www.fab.com/listings/c0226253-11f0-425d-8b2c-df9c42c73f49) | Promo hasta 1 dic | **PROBAR** P1 | PROMO oficial hasta 2026-12-01 09:59 ET; 130 mallas, grúa/camión/montacargas/props y muros modulares; ambiente abandonado, extraer solo estructuras/props par... |
| [temperate Vegetation: Foliage Collection](https://www.fab.com/listings/6a5ae8db-d80f-4b23-b276-87da390cfe56) | Free en ficha pública | **PROBAR** P1 | Unreal Engine; 277 agrupaciones de césped/flores/plantas, viento y reacción shader; probar densidad VRAM/fillrate en manzana. |
| [temperate Vegetation: optimized Grass Library](https://www.fab.com/listings/8b68642e-35f4-438e-82b4-799fc2228303) | Free en ficha pública | **PROBAR** P1 | Unreal Engine; 12 hierbas, 110 grupos y material de landscape; redundante parcialmente con Foliage Collection; elegir uno. |
| [Modern Outdoor Wooden Bench Pack](https://www.fab.com/listings/259193d3-db2f-48dc-88b9-d339801d1c95) | Free en ficha pública | **RESERVAR** P2 | FBX/OBJ/Blend/GLTF; varios bancos realistas PBR para malecón; redundante con City Props, comparar cuando haya diseño final. |
| [Planet 385CY](https://www.fab.com/listings/8532716c-2b50-4aa3-b4cb-8c9c5547ea1a) | Free en ficha pública | **RESERVAR** P2 | Unreal Engine; diorama alienígena con Lumen/Nanite descrito como pesado; referencia de iluminación, NO planeta explorable. |
| [Warehouse Building](https://www.fab.com/listings/1876bb93-5a5f-486e-9ba9-82f299b6b05e) | Free en ficha pública | **RESERVAR** P2 | FBX/GLB; nave industrial básica, no destructible ni modular probado; redundante con otras fábricas. |
| [Wooden Boat 1 JFG [Free Download]](https://www.fab.com/listings/2716fe7a-ce60-4007-b4d2-4d599f3131b8) | Free en ficha pública | **RESERVAR** P2 | FBX/GLB; bote de madera 996 triángulos, 6 materiales y 30 texturas; un mesh sin rig ni animación; solo utilería, no fauna marina. |
| [Gallions Reach Dock Structure](https://www.fab.com/listings/6479ecfb-5056-404c-9a4c-b1f0921c6818) | Free en ficha pública | **RESERVAR** P2 | OBJ fotogrametría de muelle concreto; escala/colisión/texturas y derechos de reutilización deben comprobarse en edición real. |
| [New Crane Stairs Mooring Feature](https://www.fab.com/listings/e6df1e36-b03e-48d4-9b5b-d0632d49027f) | Free en ficha pública | **RESERVAR** P2 | OBJ escaneo fotogramétrico de amarre marítimo; ambiente detallado, pero objeto aislado y sin lógica de juego. |
| [tropical Vegetation: Banana Plants](https://www.fab.com/listings/9dfde0b8-807c-4390-96ae-afa6a95aa573) | Free en ficha pública | **RESERVAR** P2 | Unreal Engine; 14 plantas tropicales interactivas; más útil para regiones de isla que para malecón urbano. |
| [Temperate Vegetation: Fern Collection](https://www.fab.com/listings/b778bd8f-524c-42b6-b60c-4caac59029c1) | Free en ficha pública | **RESERVAR** P2 | Unreal Engine; 8 helechos fotoscaneados con viento; útil bordes de senderos, no prioridad sobre árboles urbanos. |
| [Megaplants: Ginkgo](https://www.fab.com/listings/53f033c3-00ec-425b-b44e-65c04fdbd2b0) | Free en ficha pública | **RESERVAR** P3 | Unreal Engine; árbol procedural fotorealista; necesita Procedural Vegetation Editor + Nanite Foliage experimentales; no integrar a build estable todavía. |
| [Megaplants: Elder](https://www.fab.com/listings/fe7a58b3-19fe-4249-b496-2214b32650b3) | Free en ficha pública | **RESERVAR** P3 | Unreal Engine; planta fotorealista, Nanite Foliage/PVE experimentales; replicar tres especies sin validar no aporta calidad. |
| [Megaplants: Bilberry](https://www.fab.com/listings/5ed2fbb3-9da1-4718-847e-5b02dd2beedc) | Free en ficha pública | **RESERVAR** P3 | Unreal Engine; arbusto/quixel, exige PVE/Nanite Foliage experimentales; usar solo sandbox si DZ falla. |
| [Pine Tree Pack (Imposter)](https://www.fab.com/listings/1be6afff-f5db-4a8b-9fa3-ba554b9fe3f6) | Free en ficha pública | **RESERVAR** P2 | Unreal Engine; 15 billboards de pinos como impostores de distancia; no sirven para árboles muy cercanos al héroe. |
| [Doomsday Pickup Truck - Drivable Post-Apocalyptic Vehicle](https://www.fab.com/listings/d89201ed-5de8-437c-9b91-3af2d913421a) | Free en ficha pública | **RESERVAR** P2 | UE+FBX+Unity; camioneta manejable postapocalíptica; probar como vehículo levantable solo si estilo ciudad lo admite. |
| [Assetsville Town](https://www.fab.com/listings/fd558d8c-bd7e-461f-8449-a7cc9c277078) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine; ~800 modelos y arquitectura modular; estilo low-poly choca con objetivo actual de realismo urbano. |
| [Paragon: Crunch](https://www.fab.com/listings/c23ee3a7-4a73-4a83-9061-30b682d269f8) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine; robot y Animation Blueprint de Paragon, identidad y marca de franquicia ajena; si se usa, solo mecánica/estudio autorizado, sin identidad. |
| [Paragon: Gadget](https://www.fab.com/listings/52a621fd-28bb-4898-a2e7-93229a40e3f4) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine; personaje/FX/Animation Blueprint de Paragon, no encaja con identidades originales. |
| [Advanced Companion Drone System | AI & Multiplayer](https://www.fab.com/listings/205d550d-0a5f-4d5c-9f3e-8f6254b9dc4f) | Promo hasta 1 dic | **DESCARTAR** P3 | PROMO oficial hasta 2026-12-01 09:59 ET; AI = NPC lógica/behavior de Unreal y replication, NO entrenamiento; no necesitamos acompañante permanente ni multiju... |
| [Cozy Nature - Foliage Pack](https://www.fab.com/listings/d6f25a58-a0ed-4491-be48-e764cb26cef6) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine; árboles, hierba y rocas low-poly cartoon; no encaja con dirección realista de ciudad/malecón. |
| [HighPoly Tree Model](https://www.fab.com/listings/2f1f43fe-fd59-42ff-967d-c8eb1a0785e7) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine/FBX; aprox 500.000 triángulos por árbol; coste visual no justificado sin calidad observada en juego. |
| [KV Beech Tree 01 - Cinematic Animated](https://www.fab.com/listings/fb155516-1b56-47b9-82ae-bba8343fad9b) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine; 1.64 M tris highpoly, texturas 8K y Niagara; cine, excesivo para fondo urbano VRAM 8GB. |
| [Super Tree Model](https://www.fab.com/listings/1c3a1de2-7222-489d-82c7-cbd187565cdf) | Free en ficha pública | **DESCARTAR** P3 | Unreal Engine/FBX; un árbol de ~400 tris, útil móviles, puede romper realismo en tomas cercanas; alternativa lejana solo. |

## Criba global tras lote 03
**82 entradas:** 12 SELECCIONAR, 10 PROBAR, 40 RESERVAR y 20 DESCARTAR. Categorías sin nueva oferta gratuita con utilidad fuerte y evidencia suficiente: animaciones de combate/volar, kaiju modular y NPC urbanos originales. Se mantienen los candidatos de lotes 01/02.

## Exclusiones, pruebas de precio y permisos

| Fuente encontrada | Veredicto |
|---|---|
| [Wooden Pier Collection](https://www.fab.com/listings/d23bd509-54ef-489c-9499-a84f61c14a63) | 20 muelles modulares PBR; **NO gratuito verificado** (selector de licencia y Buy now); queda fuera del catálogo gratis |
| [Wrecked City](https://www.fab.com/listings/55924754-690d-4e61-9ffe-80291d96bfc9) | Compatible **5.6/5.7/5.8 según autor**, pero selector Buy now, **no Free**. No importar ni comprar |
| [Ocean Waves](https://www.fab.com/listings/bfb5c9f8-4e57-4cbe-8273-c88540965412) | No confirmar como gratis, selector de licencia/compra; evaluar Water nativo primero |
| [Low Poly Harbour Free - Container Port Kit Sampler](https://www.fab.com/listings/42ded03c-7f24-478d-8bca-118867bb05c0) | Búsqueda indexada afirma Free, pero enlace directo falló **404** en revisión. No contarlo confirmado ni reclamar |
| [Flying Drone Character](https://www.fab.com/listings/6d5cb4e7-a8df-41d3-a01a-c17ffd162e89) | Free pero **paquete Unity**, no formato Unreal o FBX acreditado; descartado de este lote |
| [Old West VOL 7 - Foliage](https://www.fab.com/listings/78a6b75f-4687-4ba6-ab4b-d863d0af9491) | Free, **UEFN/referenced asset**; no confundir contenido UEFN con paquete clásico UE5 |

**Propiedad y pruebas:** la lectura de pestañas del navegador conectadas falló por contexto de navegador; no se pudo acceder a la sección privada **My Library**. Dejar **mi_biblioteca=NO_VERIFICADO, instalado_ue=NO_VERIFICADO, prueba_ue58_windows=NO_EJECUTADO** para todas las entradas nuevas. Se conservan estados de las 52 antiguas; ni siquiera la entrega S7 prueba integración de estos productos concretos. Ninguna compra, reclamo, descarga, instalación o entrenamiento IA.

La licencia Fab Standard permite uso en juegos privados según términos, pero **no** republicar assets como paquetes sueltos. Los elementos con **Allows usage with AI: No (NoAI)** no se deben enviar como bytes/texturas/audio/mesh a modelos generativos. “AI & Multiplayer” en el nombre de un plugin no implica que entrenemos un modelo. Fuente: https://dev.epicgames.com/documentation/fab/purchasing-and-downloading-assets-in-fab y https://www.fab.com/eula.

## Siguiente revisión
Comprobar primero si se puede acceder a **My Library** mediante Work autenticado; si no, no afirmar propiedad. Revisar cambios de precio el 1 de diciembre y mantener solo un candidato principal por necesidad. Registrar diff/fecha y URL/commit de GitHub, sin borrar decisiones antiguas.
