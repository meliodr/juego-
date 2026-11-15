# IMPACT: BEYOND — Selección activa de Fab para Codex

Actualizado: 2026-11-15. **Fuente operativa** para elegir qué recursos merece la pena probar. Juego individual y privado en UE5.8 Windows, compras RD$0 y sin entrenar ningún modelo de IA. La prioridad sigue siendo el proyecto jugable: caminar/correr/volar → agarrar vehículo → perforar fachada → combate. El cooperativo está aplazado.

**Estado real:** catálogo público con **52 productos que mostraron Gratis/Free** en las fichas al revisar. No se comprobó si estaban reclamados por el usuario: **0 elementos con propiedad confirmada, 52 NO_VERIFICADO**. Tampoco hay prueba de instalación, render/compilación ni compatibilidad real de cada asset con 5.8. El inventario de titularidad no está vacío: está PENDIENTE de consulta; no equivale a «no poseído».

## Resumen de la criba inicial

| Decisión | Fichas | Significado |
|---|---:|---|
| SELECCIONAR | 8 | Primer candidato para el hito correspondiente; **no se importa automáticamente** |
| PROBAR | 5 | Alternativa/ensayo A/B si la función no se resuelve con el principal |
| RESERVAR | 27 | Guardar enlace; no descargar ahora |
| DESCARTAR | 12 | No importar: redundancia, estilo, procedencia o coste de adaptación |

Todos los ítems individuales viven en **[FAB_CATALOGO_MAESTRO.csv](FAB_CATALOGO_MAESTRO.csv)**, válido para lectura por Codex; incluye categoría, enlace Fab, indicador de precio, etiqueta NoAI, decisión, prioridad y campos diferenciados Mi biblioteca/Instalado/Prueba. Los detalles públicos y caveats están en [lote 01](FAB_AUDITORIA_LOTE01_2026-11-15.md) y [lote 02](FAB_AUDITORIA_LOTE02_2026-11-15.md).

## Los 8 seleccionados para primeros experimentos

| Recurso | Cuándo usarlo | Por qué se selecciona |
|---|---|---|
| [Free Animation Set: Zipline Skydiving Glide Dash Falling](https://www.fab.com/listings/f036035a-9edf-407a-99d3-4ef97924ddac) | E1 — vuelo y caída | 21 clips in-place; 7 skydiving, 3 glide, 3 dash; retarget y vuelo propio |
| [Free Stylized Sci-Fi Scout Bot](https://www.fab.com/listings/9c57ad32-3d3e-45ed-91e8-b32fc3af2ac0) | E3 — primer robot desmontable | Modelo low-poly con piezas separadas, candidato prioritario para desarme; no prometer clips incluidos |
| [Chaos Destruction Demo](https://www.fab.com/listings/3fd0a610-852d-4246-a554-ce3c68489a98) | E2 — primer muro de Chaos | Ejemplos de Geometry Collection, fracturas, clusters y campos; usar laboratorio no importar proyecto entero |
| [Niagara Examples Pack](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600) | E2/E3 — efectos y vuelo | Epic: estelas, impactos, explosión, humo, teleports, Niagara Data Channels; referencia de optimización |
| [METALLIC – 50 Impact Sound Effects](https://www.fab.com/listings/dd15984f-4267-4bd5-81f0-ca106c3044ed) | E2/E3 — impactos metálicos | 50 WAV y 50 cues para golpes metálicos; el autor los presenta royalty-free |
| [Motifect Locomotion Motion Pack](https://www.fab.com/listings/021d0450-017e-4262-8769-03953338dacc) | E1 — caminar/correr/esprintar | 45 animaciones humanoides en FBX, incluidas caminar/correr/sprint/salto/dodge. Retarget necesario; autor declara UE5.0+. |
| [Port Yard Props - Free Sample (12 PBR Props)](https://www.fab.com/listings/ae3b6b05-245d-4eec-a80f-b378a5e88fb0) | E2/E3 — objetos lanzables | 12 props de puerto con UCX/FBX/GLB, 2092 tris en total reportados; no probado dentro de Unreal por autor. |
| [free scifi containers](https://www.fab.com/listings/8e506a48-a6e9-4249-833f-9f4d86b08fe2) | E2/E3 — puerto y objetos grandes | Contenedores sci-fi FBX, proxies para puerto y lanzamiento. Colisión y material a preparar. |

**Condición antes de importar:** revisar licencia de esa ficha en la sesión del usuario, descarga realmente accesible/formatos, compatibilidad UE5.8; importar un subconjunto en mapa aislado y comprobar memoria/colisiones. **No instalar los ocho como lote**. Para el primer movimiento usare primero Third Person de Epic, no Motifect por obligación.

## Alternativas que solo probaremos si se justifican

| Recurso | Test condicionado |
|---|---|
| [Cartoon City Free - Low Poly](https://www.fab.com/listings/e41364e1-c811-40db-85f5-84441192591d) | Edificios/calles/coches lowpoly en FBX/GLB/Unreal, pero dirección artística debe unificarse |
| [Sci Fi Robot 13](https://www.fab.com/listings/2b68ee14-cec8-4347-a5fa-5ba870160467) | 15 animaciones de disparos, movimiento, sprint y reacciones; no garantiza partes desmontables |
| [Realtime Destructible Mesh](https://www.fab.com/listings/46f4e8fc-c95c-4e1c-be61-17439bcbbd1b) | Booleanos y huecos reales según autor; licencia Fab Standard, comparar vs Chaos en sandbox, UE5.8 aún sin prueba local |
| [Procedural Asteroid Field Tool](https://www.fab.com/listings/7682cc66-36ad-4b70-a6c3-f37dcd04f266) | Blueprint ligero para campos de asteroides en editor con variaciones/volumen/espaciado |
| [Vehicle Variety Pack](https://www.fab.com/listings/dc1ada50-2523-44b1-b0e2-a72d14076fb4) | 5 vehículos manejables de 4 materiales aproximadamente; probar UNA unidad como objeto levantable. No importar interiores/blueprints hasta necesidad. |

## Descartados de esta etapa (12)

No son malas obras en general. **No aportan valor suficiente a este sandbox ahora**, o su cadena de procedencia necesita cautela. Sin descargas ni imports.

- **[City Sample completo](https://www.fab.com/listings/4898e707-7855-404b-af0e-a505ee690e68)** — UE5.8 CitySamplePCG con ejemplos MCP; proyecto grande, preferir packs independientes
- **[Paragon: Howitzer](https://www.fab.com/listings/7397dd8d-2e47-4e68-9e9d-a10c6fa0a9c7)** — Mecha/animaciones/FX gratis para UE; identidad Paragon no debe usarse en nuestro juego
- **[Free Killer Doll Character](https://www.fab.com/listings/3a5f7cb6-ed57-4d11-9200-fbd61aba9c7c)** — Quinn rig y 14 partes estáticas; NO incluye animaciones pese a vídeo de promo
- **[Robotz Modular Cartoon Robot](https://www.fab.com/listings/ed13427d-c7a8-4676-ad09-557a784b2b21)** — Blender editable y modular, pero sus 5 animaciones son gestos sociales, no combate; empaquetado nativo UE no anunciado.
- **[Some Robot](https://www.fab.com/listings/1f6c8e1f-00b6-486d-bc36-3c6f5c07ff6c)** — Autor indica que modeló basándose en imagen de autor desconocido; procedencia artística no clara.
- **[Demonic Spider Free Animated Monster](https://www.fab.com/listings/6cd4cfee-3247-49e6-af10-9fadb0c7f0e4)** — Criatura de terror generada con IA; estilo ajeno a primera ciudad y no necesario para sandbox.
- **[Halloween Pumpkin](https://www.fab.com/listings/4ba3165d-66cd-4c92-ab44-52abeb08a987)** — 22 animaciones; estética Halloween no necesaria para robots/kaiju.
- **[PSX Horror Monsters Free Sample](https://www.fab.com/listings/6580ee34-14aa-4e67-8f79-6b7aad2221a1)** — 2 modelos PS1 rigged con 6 clips; estética incompatible con ciudad semiestilizada.
- **[UE TCA Russel/Admiral Rylak](https://www.fab.com/listings/f3450190-7d37-4620-9674-aef98d6c5c3a)** — Rig/outfits/groom, pero redundante con otros humanoides; no usar IP del personaje.
- **[Low Poly City Buildings Pack](https://www.fab.com/listings/38e0e92c-bae5-42b9-a8a8-6ae4f60cc291)** — Solo Blender indicado, rating bajo, alternativas más completas en Quaternius/Kenney/Cartoon City.
- **[Shipping Container PBR UNITY 6 SAMPLE](https://www.fab.com/listings/c81d98a3-8efc-41d9-8b59-f77422a0c3ca)** — Proyecto Unity-only, no FBX anunciado; mejor elegir FBX directo que importar prefabs de Unity.
- **[PS1 Car Free](https://www.fab.com/listings/d196cffd-297b-4889-8578-20d75003f8db)** — Carro PS1 lowpoly; estilo inadecuado frente a otros paquetes.

## Reservas (27)

Consultar el CSV, **no importar ahora**. Contiene City Sample Buildings/Crowds/Vehicles, héroes de referencia TCA, más robots/monstruos, efectos de energía, ciudades futuristas, CosmoKit y sonidos. Algunos muestran NoAI, otros generados con IA; la decisión se basará en licencia, estilo y rendimiento. No se copiarán identidades de superhéroes de otros autores como personajes originales.

## Fuentes con precio NO confirmado o uso restringido: NO comprar

| Ficha | Problema |
|---|---|
| [Freeflow Combat](https://www.fab.com/listings/a51dbdb2-eb68-4d92-a5c7-f2509e6e162e) | **«Freeflow» no significa gratuito**: página muestra selector de licencia y Buy now, precio desconocido → DESCARTAR presupuesto RD$0 |
| [Modular City Streets Builder (136)](https://www.fab.com/listings/cf8f6692-36e0-4305-9589-cbd567652b1b) | Muy interesante en FBX/GLB pero no se confirmó **Free**: selector de licencia y comprar → PENDIENTE/DESCARTAR hasta cambio |
| [Low Poly City Locations Pack](https://www.fab.com/listings/5c944115-e098-4329-855a-bd373efbf42b) | 133 modelos listados, pero no precio gratis verificable → PENDIENTE |
| [New York City Streets Environment](https://www.fab.com/listings/09484226-f51b-4a63-a4fc-eb7c88a15e76) | 228 meshes, muestra opción de compra y precio ofuscado; **la demo gratis no implica pack gratis** → DESCARTAR por ahora |
| [Energy weapon VFX Pack](https://www.fab.com/listings/1d3c8d00-473b-4eb8-b0d4-471337285c20) | Selector de licencia/Buy now; no Gratis verificado → DESCARTAR |
| [Shockwave Distortion](https://www.fab.com/listings/db0d837f-3c94-4a67-86a8-5653ebddce12) | Descripción dice free, pero precio en ficha no confirmado → PENDIENTE |
| [Dynamic Falling & Rolling Animation Pack](https://www.fab.com/listings/68aca290-3b17-4acc-93f7-719d980f17f9) | Aparece «From Free» en buscador pero selector de licencia incierto → PENDIENTE |
| [Free Animation Library](https://www.fab.com/listings/481ef75b-892b-424f-a213-f1cc058c9c19) | Nombre no prueba precio; revisar en cuenta → PENDIENTE |
| [FREE Planet Project](https://www.fab.com/listings/fc591129-a147-4684-acbb-38a990e30c3a) | Nombre no prueba precio, compatibilidad UE5.8 no probada → PENDIENTE |

## Promociones de corta duración (NO reclamadas automáticamente)

Fab anunció [ofertas hasta el 17 de noviembre de 2026, 9:59 a.m. ET](https://www.fab.com/limited-time-free):

1. [SciFi Beast06 Bull](https://www.fab.com/listings/126120a3-996e-4dc3-94f1-6550f0adac03): candidato kaiju (29 animaciones anunciadas), NoAI. Interesante **para reclamar manualmente si todavía marca Gratis**, no para entrenar IA.
2. [3D Platformer Game Bundle](https://www.fab.com/listings/1898b906-d481-4a05-8515-f2b4d3b28378): homing attack, boost y rieles, posible referencia de targeting. Verificar Free y términos antes de obtener.
3. [Underground Tunnel](https://www.fab.com/listings/6c9e3f18-0b5e-4240-9aea-eecb515175c1): prioridad baja para ciudad, adaptar de UE4.

**La cuenta del usuario NO fue inspeccionada: no se ha reclamado ninguno de estos productos desde ChatGPT.** La automatización de revisión con fecha de 18 de noviembre no evitará que ofertas anteriores caduquen; adquisición es decisión y acción del usuario.

## Lo que hará Codex al iniciar una tarea de recursos

- Consultar este archivo y CSV antes de modelar/crear una mecánica de cero. Afirmar solo el estado real: público gratis, reclamado NO_VERIFICADO, instalado NO_VERIFICADO, probado NO_EJECUTADO.
- Buscar primero en Mi biblioteca si acceso real disponible. Si no, NO supondrá que el usuario no posee el paquete.
- Para cada selección registrar UUID Fab, licencia exacta y fecha de descarga, versión del motor compatible, formatos disponibles y archivo real importado. No exportar materiales Fab al repo público como paquetes sueltos.
- No entrenaremos ninguna IA. [Epic Content License §17](https://www.unrealengine.com/eula/content) impide entregar contenido NoAI como entrada a programas de IA generativa, además de datasets/entrenamiento. Codex puede trabajar con los nombres/metadata/nuestro código sin subir archivos NoAI al modelo. Tener etiqueta «Generado con IA» no implica que tengamos que entrenar un modelo.
- **Regla innegociable:** reutilizar lo máximo posible y crear solo el pegamento técnico imprescindible, sin sacrificar compatibilidad, control, claridad o estabilidad.
- Probar UN donante a la vez con logs/capturas y evaluación de rendimiento en Windows UE5.8, 8GB VRAM; no instalar media biblioteca simultáneamente.

## Mantenimiento

La tarea periódica «Auditoría Fab de IMPACT» quedó activada **cada 3 días a ~09:00 America/Santo_Domingo** y primero se ejecutará 2026-11-18. Al ejecutarse intentará investigar cambios y actualizar estos recursos en GitHub; si la conexión o propiedad no puede verificarse, lo indicará en el reporte. No se garantiza acceso futuro ni se puede declarar por anticipado ninguna actualización. Una revisión posterior debe preservar el historial de candidatos, explicar altas/bajas y marcar el precio/propiedad como comprobados solo con evidencia actual.
