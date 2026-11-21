# IMPACT: BEYOND — Auditoría Fab, lote 04

**Fecha: 2026-11-21.** Se revisó el estado del repositorio: README, AGENTS, plan activo, selección Fab, catálogo y lotes 01/02/03. El catálogo anterior tenía 82 fichas. Se agregaron **9 URLs nuevas y únicas**, para un total de **91 registros**. Búsqueda pública dirigida por categorías; no es un rastreo exhaustivo de todas las páginas y resultados paginados de Fab.

## Estado consolidado

| Decisión | Antes (82) | Ahora (91) |
|---|---:|---:|
| SELECCIONAR | 12 | 12 |
| PROBAR | 10 | 14 |
| RESERVAR | 40 | 43 |
| DESCARTAR | 20 | 22 |

Todos los 91 registros tienen `mi_biblioteca=NO_VERIFICADO`, `instalado_ue=NO_VERIFICADO`, `prueba_ue58_windows=NO_EJECUTADO`. El indicador `GRATIS_EN_FICHA` describe solo el precio público visible al revisar, no titularidad.

## Nueve fichas nuevas con precio gratuito visible

| Recurso | Datos comprobados en la ficha | Compatibilidad / cautela | Decisión |
|---|---|---|---|
| [Free Animation Pack](https://www.fab.com/listings/8de31c5d-93bc-4bd4-9606-ca789ce91b99) | 24 animaciones mocap, 120 FPS, Manny/Quinn, Root Motion e In-Place, .uasset nativo; Free; Allows AI=Yes; Generated with AI=No | UE5; no prueba específica UE5.8. Comparar con Motifect, sin importar ambos por defecto | PROBAR |
| [Advanced Locomotion System Animations for UE5 Mannequin](https://www.fab.com/listings/d366bdfa-a665-424e-be88-0c9629582489) | FBX de animaciones extraídas del Advanced Locomotion Sample Project; Free; Allows AI=No; Generated with AI=No | La ficha indica que las animaciones son propiedad original de Epic y el vendedor no está afiliado. Verificar procedencia/licencia antes de usar; seguir su guía y no seleccionar Add to Project | RESERVAR |
| [Item Destruction FX](https://www.fab.com/listings/b23c6751-60f1-40b2-8ee7-5e4dc7f9c9e8) | VFX estilizados para romper, shatter y desintegrar objetos; Free; Allows AI=No; Generated with AI=No | Formato UE, sin versión exacta. Es VFX, no fractura de geometría ni sustituto de Chaos | PROBAR |
| [Vehicle Variety Pack Volume 2](https://www.fab.com/listings/591e3b3f-9d49-4cd2-8e28-d471c1a10cab) | 4 vehículos manejables con interiores: sedán, box truck, SUV, campervan; aprox. 4 slots de material promedio; Free; NoAI | Proyecto Unreal, versión exacta no indicada. Comparar con Vehicle Variety Pack v1 y City Sample Vehicles; elegir un solo donante | PROBAR |
| [Free Modular Magic SFX](https://www.fab.com/listings/f0422e67-33d3-4c6e-b7a1-1059193960de) | 290 efectos procedurales y 22 gráficos MetaSounds; Free; UE5.6+; CC BY 4.0 declarado; Allows AI=Yes; el autor declara que no usa audio generado por IA | Confirmar si los gráficos requieren el plugin Procedural SFX & Music MetaSounds. La ficha dice CC BY 4.0 pero también que no requiere atribución: conservar atribución y aclarar la contradicción antes de usar | PROBAR |
| [Promo Sound FX - Free](https://www.fab.com/listings/875e3bdb-8e39-444a-94c1-12c4cf1f2ac9) | 50 sonidos Foley, 24-bit/44.1 kHz; Free; NoAI; no generado con IA | Foley de objetos, tela, agua y vidrio; utilidad secundaria. UE5.8 no explicitado | RESERVAR |
| [M5 VFX Vol2. Fire and Flames (Niagara)](https://www.fab.com/listings/c5b0270a-a295-4644-a4be-42cb1e56a197) | Fuego, llamas y explosiones Niagara; Free; UE5+; NoAI; no generado con IA | UE5.8 es candidato según el rango declarado, no probado. Los archivos antiguos de Apex building destruction fueron excluidos en UE5: no es un sistema de fractura | RESERVAR |
| [Free Sample – Wooden Garage](https://www.fab.com/listings/6d700135-f9a4-41c4-9d3a-9221d56fcebc) | Garaje rural exterior; UE5.8 explícito, FBX/GLB, 4 LOD y colisión UCX; Free; Generated with AI=Yes | Un único edificio rural, sin interior y puertas cerradas; no aporta al estilo urbano superheroico actual | DESCARTAR |
| [Free Sample Dark Fantasy Gothic Environment Kitbash](https://www.fab.com/listings/338af1ae-80da-4d94-b3ef-38368e287678) | Muestra gratuita de un edificio gótico; Unreal Engine; NoAI; no generado con IA | No encaja con la dirección urbana moderna/realista | DESCARTAR |

## Duplicados y categorías sin nuevo Free confirmado

- **Robot:** [Robot de 3DLAKOUR](https://www.fab.com/listings/0f99e8f4-276a-44ce-b0c8-5b51d10e87f2) ya estaba en el catálogo. La ficha actual muestra Free, rigged, 6,410 vértices, una malla, sin animaciones, Windows/Mac, Allows AI=No y Generated with AI=No. No duplicar; no es un robot desmontable listo.
- **Ciudad:** [City Sample Buildings](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5) ya estaba catalogado; Free, 2,000+ módulos, UE-only y NoAI.
- **Vehículos:** [City Sample Vehicles](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f) ya estaba catalogado; Free, 13 vehículos, UE-only. La ficha actual muestra Allows AI=Yes y Generated with AI=No; el CSV anterior decía No, por lo que se corrigió la metadata sin cambiar su decisión RESERVAR.
- **Kaiju/NPC:** no se confirmó ninguna ficha nueva Free y apropiada para UE5.8 en esta búsqueda. Verity, UnderGround Monster, Goofy NPC y otros mostraron selector de licencia o precio no verificable; no se agregan como gratuitos.
- **Vuelo:** [Superhuman Flying Locomotion](https://www.fab.com/listings/c61d0203-47f3-4e2d-8dde-654c80bff1c6) anuncia 51 animaciones, pero la ficha muestra selector de licencia/Buy now, no Free. [Flight Locomotion System](https://www.fab.com/listings/c41fd776-6ddd-4dee-989e-99ef1a550956) no mostró precio Free verificable. No añadir.
- **Planetas:** [FREE Planet Project](https://www.fab.com/listings/fc591129-a147-4684-acbb-38a990e30c3a) tiene “FREE” en el título y describe UE5.3/5.4, pero la ficha abierta muestra selector de licencia y Buy now; no es Free confirmado y además declara versiones anteriores. No añadir.
- **Espacio:** [Spaceship Starter Kit](https://www.fab.com/listings/1a69489e-8c1f-4ab4-9082-ad3a16572ef0) dice “free taste” en la descripción, pero la ficha muestra selector/Buy now. No añadir.
- **Audio:** [SUB BOOM – 52 Sci-Fi Sound Effects](https://www.fab.com/listings/5a4a3bd2-ceb0-43a5-bffb-4b148d2b15b2) incluye 52 WAV/Sound Cues y declara UE5.0+, pero muestra selector de licencia y Buy now. No Free confirmado.

## Cambios de metadata revalidados

1. [City Sample](https://www.fab.com/listings/4898e707-7855-404b-af0e-a505ee690e68) sigue Free. La ficha actual muestra Allows AI=Yes y Generated with AI=No. La descripción incorpora para UE5.8 primitives de edificios y ejemplos PCG/CitySamplePCG con integración LLM vía Unreal MCP. El proyecto no usará LLM para generar ciudades ni entrenará IA; PCG manual podría estudiarse. El proyecto completo se mantiene DESCARTAR por escala/alcance.
2. [City Sample Vehicles](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f) sigue Free y la etiqueta actual es Allows AI=Yes, Generated with AI=No. Corregido de No a Sí en el CSV. UE5.8 no probado.
3. [City Sample Buildings](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5) sigue Free y NoAI; 2,000+ módulos. Mantener RESERVAR y extraer solo si se autoriza una manzana pequeña.

## Promociones y fechas

La página oficial [Fab Limited-Time Free](https://www.fab.com/limited-time-free), consultada el **21/11/2026**, sigue mostrando tres ofertas hasta el **1/12/2026 a las 9:59 a. m. ET**: [Absorption FX](https://www.fab.com/listings/81550176-a90e-4e07-8817-c8d1a1d3884b), [Advanced Companion Drone System](https://www.fab.com/listings/205d550d-0a5f-4d5c-9f3e-8f6254b9dc4f) y [Post-Apocalyptic Factory](https://www.fab.com/listings/c0226253-11f0-425d-8b2c-df9c42c73f49). Se mantienen decisiones anteriores: seleccionar Absorption FX para ensayo focalizado, probar solo piezas de la fábrica y descartar el dron por alcance. No se reclamó nada desde esta auditoría. Las promociones del 3–17/11 del lote 03 han caducado; no se sabe si el usuario las reclamó a tiempo.

## Falsos positivos de precio

- [FREE Planet Project](https://www.fab.com/listings/fc591129-a147-4684-acbb-38a990e30c3a): el título dice Free, pero ficha actual tiene selector/Buy now.
- [Spaceship Starter Kit](https://www.fab.com/listings/1a69489e-8c1f-4ab4-9082-ad3a16572ef0): “free taste” no equivale a precio Free; selector/Buy now.
- [SUB BOOM](https://www.fab.com/listings/5a4a3bd2-ceb0-43a5-bffb-4b148d2b15b2): selector/Buy now.
- [Superhuman Flying Locomotion](https://www.fab.com/listings/c61d0203-47f3-4e2d-8dde-654c80bff1c6): 51 animaciones, pero selector/Buy now.
- [Flight Locomotion System](https://www.fab.com/listings/c41fd776-6ddd-4dee-989e-99ef1a550956): no se confirmó precio cero.
- [UnderGround Monster](https://www.fab.com/listings/31b97270-25d2-4212-9887-522252ecd9e4), [Verity](https://www.fab.com/listings/48de8c9e-d36a-400a-b57b-54ea4deea546) y [Goofy NPC](https://www.fab.com/listings/82c821ea-d86a-498d-882f-3d4af67d40d3): precio Free no confirmado.
- [Urban Cinematic Destruction SFX Pack](https://www.fab.com/listings/e6a3a313-cafa-4091-a1ff-1642690669fa): Buy now y Generated with AI=Yes; no incorporar.

## Recomendación respetando S1–S7

El [plan activo](PLAN_ACTIVO.md) y [cierre S7](IB_S7_CLOSURE_REVIEW.md) confirman que S1–S7 está integrado y validado. No reconstruir sistemas por el catálogo. La prioridad de mejora visual sigue siendo una sola manzana/malecón: DZ Sample Trees, City Props Pack, una fachada modular y Water System nativo. Los nuevos candidatos son comparaciones opcionales. Si se autoriza prueba: Free Animation Pack vs Motifect en mapa aislado; Vehicle Variety v1 vs v2; Chaos Demo + un solo VFX; audio WAV primero, sin plugins nuevos. Preservar la build S7 y medir VRAM/memoria.

## IA y licencias

Epic Content License §17 define el contenido NoAI y prohíbe usarlo en datasets de IA generativa, desarrollar modelos con él o introducirlo como entrada a programas generativos. Esta regla sigue aplicando aunque el juego sea privado. El usuario no entrenará ningún modelo; assets NoAI fuera de prompts, uploads, análisis generativos y datasets. “Allows usage with AI” y “Generated with AI” son campos distintos: permitir IA no obliga a usarla ni indica que el asset fue generado con ella. Epic anunció cambios de EULA el 29/10/2026 para que desarrolladores puedan optar por entrenar con sus proyectos; este proyecto opta por no entrenar. El anuncio aclara que las etiquetas NoAI de Fab no cambian.

Fuentes: [Epic Content License](https://www.unrealengine.com/eula/content) · [Epic, 29/10/2026: UE devs can now choose to train AI Models on their projects](https://www.unrealengine.com/ue-devs-can-now-choose-to-train-ai-models-on-their-projects).

## Alcance y limitaciones

Esta es una búsqueda pública dirigida por las categorías solicitadas, no un rastreo exhaustivo de todas las páginas/resultados paginados de Fab. No se abrió Mi biblioteca privada. Los precios pueden cambiar y algunas promociones se muestran con precio ofuscado en visitante. “UE5+”, “UE5.6+” y “UE5.8” provienen de las declaraciones de cada ficha; no son prueba real en el editor. Ningún producto fue comprado, reclamado, descargado, instalado o probado en esta ejecución.

**Regla permanente: reutilizar lo máximo y crear lo mínimo, sin poner en riesgo la build S7 validada.**
