# IMPACT: BEYOND — Auditoría Fab, lote 05 (24/11/2026)

## Resumen y cobertura

Se leyeron README, AGENTS, plan activo, selección y catálogo; lotes 01/02/04. La base vigente era 91 registros únicos (52 originales + ampliaciones), no 52. Se añadieron **14 fichas con indicador público Free**, sin duplicar UUID. Total **105**: 12 SELECCIONAR, 19 PROBAR, 52 RESERVAR, 22 DESCARTAR. Las nuevas son 5 PROBAR y 9 RESERVAR. Dos ejemplos Epic ya eran conocidos por el proyecto, pero no estaban en el CSV. No es un censo exhaustivo de Fab; se hizo búsqueda dirigida y comprobación pública de fichas. Las fechas de rastreo de algunas fichas varían; el indicador Free es evidencia pública, no garantía de precio permanente ni de acceso de la cuenta. No se revisó individualmente el precio de las 91 entradas anteriores.

## Candidatos y decisiones

| Recurso (ficha pública) | Decisión | Evidencia y alcance |
|---|---|---|
| [City Street Props](https://www.fab.com/listings/717d655e-923f-4c03-bc0b-3509fd1b0ec8) | PROBAR P0 | Dekogon, 305 mallas, Nanite/Lumen, arte urbano realista de Matrix Awakens. Probar 10–20 objetos para calles y malecón vs City Props Sample; NoAI. |
| [Quantum Modular Character Free Sample](https://www.fab.com/listings/8e200050-3158-4762-b297-f785b5b1533d) | PROBAR P1 | Una muestra de personaje masculino modular táctico, skeleton UE5, compatible con MetaHuman alto/masculino, FBX. No incluye los 1000 módulos de pago. Candidato a policía/seguridad, no civil genérico. |
| [Monster (Khornes)](https://www.fab.com/listings/6fc1668f-c5d9-4146-8da1-ee3d7f958026) | PROBAR P1 | Autor declara UE4.26–5.8, 24962 tris, rig UE4, textura 4K, un personaje; no confirma clips de combate ni piezas funcionales. Comparar contra kaiju actual. |
| [Game Animation Sample](https://www.fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016) | PROBAR P1 | Epic, 500+ clips y ejemplo Motion Matching terrestre. Ya conocido en investigación, nuevo en CSV. NoAI. Seleccionar 6–10 clips como máximo; no reemplazar controlador de vuelo. |
| [Sparks Embers](https://www.fab.com/listings/48e20eb7-0812-4670-b206-44d3b5aa8a01) | PROBAR P1 | Niagara parametrizable, verificar variante UE y probar un efecto contra Niagara Examples. |
| [Lower Sector Building Kit](https://www.fab.com/listings/709924e0-3128-4d23-9d36-fe35991d03c0) | RESERVAR | Kit sci-fi, bot y vehículo generador, autor declara CC0; verificar edición/licencia. No sustituye ciudad realista. |
| [Casual Characters Pack FREE](https://www.fab.com/listings/41894633-7d07-4279-8303-d462f5da91db) | RESERVAR | NPC casuales rigged Mixamo, estilizados, solo proxies si se necesitan. |
| [Automotive Substrate Materials](https://www.fab.com/listings/1272587c-1431-4878-9d78-2a6b84ebe839) | RESERVAR | Epic, 280+ materiales, 150 configuraciones, 47 plantillas. NoAI. No cambiar render global sin ensayo de shaders/VRAM. |
| [ASC Teuthisan](https://www.fab.com/listings/932b69b8-cc42-4586-be9b-d327d8195736) | RESERVAR | Criatura de cortometraje con Control Rig; referencia técnica, NoAI, identidad propia obligatoria. |
| [Content Examples](https://www.fab.com/listings/4d251261-d98c-48e2-baee-8f4e47c67091) | RESERVAR | Epic, UE-only, NoAI, ejemplos de animación, físicas, materiales, audio y Blueprints. Ya conocido, nuevo UUID en CSV; no migrar proyecto completo. |
| [Free Niagara Particles](https://www.fab.com/listings/183732bc-c2fb-465c-9453-f70a1ce7ba2c) | RESERVAR | Partículas y parámetros expuestos; CC BY 4.0 anunciado, conservar atribución. Redundancia con Niagara Examples. |
| [GeoForgeRuntime](https://www.fab.com/listings/ac179c02-f8bf-45b0-802f-87a098ec2ab4) | RESERVAR | Plugin C++: autor declara UE5.7/5.8, cube-sphere, gravedad radial, colisión local, terreno editable y ProceduralMeshComponent. Generated with AI=Yes. No sustituir planeta actual; solo estudiar futuro laboratorio si hay necesidad y licencia/dependencias verificadas. |
| [Damage Vocalizations (Male)](https://www.fab.com/listings/e61741f5-822a-47ff-8fe4-f1a7f1b236f0) | RESERVAR | Reacciones humanas al daño; valoración baja, atribución preferida VoiceBosch. NoAI. |
| [Deadly Creatures Sound Effects Pack 1](https://www.fab.com/listings/9d4c3cc6-1720-447a-981c-ba1349cf2810?lang=en) | RESERVAR | Rugidos para kaijus, baja valoración, portada generativa declarada; no asumir origen del audio. NoAI. |

Solo Monster y GeoForgeRuntime de este lote tienen rango UE5.8 explícito en la descripción consultada. Los otros son candidatos nativos UE/formatos importables, no compatibilidad 5.8 certificada. No se descargó nada.

## Comparaciones y exclusiones

- Ciudad: City Street Props es un candidato más amplio que City Props Free Sample; conservar ambos y ensayar subconjunto. City Sample Buildings sigue como donante de fachadas. City Boulevard Creator (https://www.fab.com/listings/2f47f57e-0d10-4c5c-9408-235f796554f0) y Street Modular Set (https://www.fab.com/listings/3e3cb8db-57f4-46b4-b756-25c58ce98d02) no mostraron Free verificable; no añadir a CSV gratuito.
- Destrucción: Chaos existente permanece principal. Content Examples es referencia, Item Destruction FX (ya catalogado) es solo visual. Urban Cinematic Destruction SFX (https://www.fab.com/listings/e6a3a313-cafa-4091-a1ff-1642690669fa) muestra compra.
- Vuelo: Game Animation Sample no es vuelo superheroico. Fantasy Wings (https://www.fab.com/listings/e4a43c42-bece-4ae4-b604-7c80ba453ad0) tiene demo gratis, no paquete confirmado gratuito.
- Robots: Scout Bot, Sci Fi Robot 13 y robot 3DLAKOUR ya están en CSV; no duplicar. Cute Robot 4/5 y Worker Robot no mostraron Free verificable.
- Vehículos: no se verificó un nuevo pack completo gratuito de carros. City Sample Vehicles y Vehicle Variety Pack 1/2 siguen candidatos; Modular Driveable Vehicle Pack Vol.1 (https://www.fab.com/listings/c5dcff3c-bdae-45f1-9158-224d6dae653c) anuncia muestra de camión gratis, no pack completo gratis.
- Planetas: GeoForgeRuntime aporta referencia técnica nueva, pero voxel y plugin C++ cambian el alcance. [FREE Planet Project](https://www.fab.com/listings/fc591129-a147-4684-acbb-38a990e30c3a) sigue sin indicador Free verificable en ficha; no aprobado.
- Paragon: Rampage, Grux y Minions muestran Free en fichas públicas, pero se omiten del catálogo operativo por dirección de identidad propia y redundancia con criaturas/robots; no se prohíben por el mero hecho de ser Paragon. Enlaces: https://www.fab.com/listings/0807cf74-08fd-4a33-8c8d-f33c9439fb1f , https://www.fab.com/listings/8c4bac2c-f7f7-4632-a644-47f4e104f5d8 , https://www.fab.com/listings/039ea035-9360-4e76-ad06-5d3a92da6f65 .

## Precios y promociones

[Fab Limited-Time Free](https://www.fab.com/limited-time-free), consultado 24/11: las mismas tres promociones se mantienen hasta **01/12/2026, 9:59 a. m. ET**: Absorption FX, Post-Apocalyptic Factory y Advanced Companion Drone. No hay nuevas promociones en esta consulta; conservar los estados anteriores. Las del 03–17/11 están vencidas; titularidad desconocida. City Sample y City Sample Vehicles muestran Allows AI=Yes y Generated with AI=No en las fichas abiertas el 24/11 (un resultado indexado antiguo de Vehicles aún mostraba No); prevalece la ficha actual, no cambiar CSV. City Sample Buildings sigue con NoAI en la ficha indexada consultada. No se verificaron todos los precios antiguos; no afirmar que los 105 están gratis hoy simultáneamente.

## Propiedad y límites

El navegador conectado devolvió error de falta de contexto del navegador de origen; no fue posible consultar Mi biblioteca privada ni editor local. Conservar todos los estados de propiedad e instalación existentes; nuevos registros NO_VERIFICADO, prueba UE5.8 NO_EJECUTADO. No comprar, reclamar, descargar, instalar, entrenar IA ni enviar bytes NoAI a generadores. Solo metadatos y enlaces en GitHub. Uso privado/offline, RD$0. La build S1–S7 sigue siendo la base según plan, no modificada por esta auditoría.

## Próximo ensayo recomendado para Codex

Priorizar City Street Props frente a City Props Sample en una sola manzana/malecón: 10–20 mallas, instancias repetidas, colisiones, materiales, VRAM y frame time antes/después. Mantener DZ Sample Trees, Water System nativo y edificios ya seleccionados. Ensayos opcionales independientes: un policía, un kaiju, 6–10 clips locomoción y una chispa. GeoForgeRuntime solo reserva; ningún plugin externo por defecto. Reutilizar lo máximo, crear lo mínimo.
