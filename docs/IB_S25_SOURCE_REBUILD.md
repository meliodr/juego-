# Fuentes S2.5 conservadas para la partida

Esta selección procede del [PR25](https://github.com/meliodr/juego-/pull/25), commit `70187086042669e8ec178b3b268c57730da8492f`. Conserva recetas y metadatos útiles para reconstruir dependencias, no declara el laboratorio completo integrado ni aceptado. No incluye renders, OBJ generados, texturas descargadas ni paquetes Unreal nuevos.

## Qué aporta cada fuente

| Fuente en Tools | Función |
|---|---|
| IBS25_recipe.py + IBS25_author.py | Layout base, geometría original regenerable, materiales de asfalto/acera/marcas e importación en el laboratorio aislado |
| IBS25_pbr.py + IBS25_sources/manifest.json | Seis mapas oficiales CC0 de Asphalt 02 y Concrete Pavement; URL, tamaño y hash por fuente |
| IBS25_stage.py | Importación local de texturas y árbol, LOD y ejecución de la receta base |
| IBS25_soil.py | Tres mapas de Brown Mud Leaves 01 y material de tierra |
| IBS25_refine_recipe.py + IBS25_refine.py | Detalle original R2 y materiales de tejado, metal, piedra y suelo utilizados por la partida |
| IBS25_lookdev.py | Correcciones históricas de componentes, cámara y LOD de la escena base |
| IBS25_validate.py + IBS25_refine_validate.py | Repetición de autoría y comparación numérica en Unreal |
| IBS25_test.py + IBS25_metrics.py + IBS25_refine_test.py | Pruebas de recetas, geometría, fuentes declaradas y lectura de métricas; no certifican apariencia |

Fachadas BayWide/BayDouble/Entry/Cornice: proceden de la base pública, con `Tools/Apply-RealisticFacades.py` y entrada local `IB_POLY_FACADE`. Ese script trabaja sobre C00; no ejecutarlo sobre la partida final para reconstruir indiscriminadamente un nivel. Las ciudades e hitos actuales usan sus propias recetas S3/Playable. Los OBJ del lab no son una dependencia del código de sus ciudades.

## Provisión local y reproducción

Utilizar un checkout aislado y turno exclusivo de Unreal. Conservar la escena original; no copiar el mapa R2 antes de reconstruir la base: el autor rechaza sobrescribir su marcador refinado. La cadena histórica necesita fachadas, coche y materiales existentes de la base S2; no basta un clone sin las dependencias autorizadas.

1. Ejecutar `python -B Tools/IBS25_recipe.py --output Tools/IBS25_generated` para generar geometría original. El directorio está ignorado.
2. Revisar los recibos y obtener únicamente las fuentes CC0 declaradas. `IBS25_pbr.py` descarga seis texturas y `IBS25_soil.py` tres; ejecutarlos es una acción explícita del autor, nunca un paso de CI. No descargan Fab ni instalan plugins.
3. Proveer los 11 archivos del [recibo del árbol](IB_S25_TREE_SOURCES.json) bajo `Tools/IBS25_sources/tree/`, conservando rutas relativas. El downloader PBR no obtiene el árbol. Verificar tamaño/hash antes de importarlo. [Recibo del suelo](IB_S25_R2_SOIL_SOURCES.json).
4. En Unreal, con los donantes legales presentes, ejecutar `IBS25_stage.py`, después `IBS25_lookdev.py`. El preflight debe pasar; no sustituir silenciosamente dependencias ausentes por arte de otra procedencia.
5. Ejecutar `IBS25_refine_recipe.py`; después, en el laboratorio base y fuera de PIE, `IBS25_refine.py`. Validar repetición con `IBS25_validate.py` en la fase base y `IBS25_refine_validate.py` tras R2. No aplicar el validador base al mapa R2.
6. El integrador provee a la partida solo los assets requeridos y valida referencias/cook. `IBPlay_AuthorProduction.py` genera derivados privados en `IB_Playable`; su exportación pública es únicamente código y metadatos sanitizados. La receta de instancing siguiente forma parte de esa ejecución.

Los scripts conservan los guards originales de propietario, hash y mapa. Generan salidas locales ignoradas; los recibos no conceden una licencia nueva ni sustituyen la licencia de shaders, plantillas o recursos privados dependientes. No publicar packs o derivados privados. No suministrar bytes NoAI a herramientas generativas.

## Flags de los once materiales del mundo para instancias

`Tools/IBPlay_AuthorProduction.py`, snapshot `730b39f`, contiene `material_usage_preflight()` y `prepare_world_instancing_materials()`. El modo `IB_PLAY_API_ONLY=1` comprueba la API sin modificar materiales. La ejecución completa aplica `InstancedStaticMeshes` solo a esta lista explícita de materiales propios/CC0: los ocho de `IBS3World::ProductionMaterials` y los tres del árbol:

- `/Game/IB_S25/Materials/M_Asphalt`
- `/Game/IB_S25/Materials/M_Markings`
- `/Game/IB_S25/Materials/M_Paving`
- `/Game/IB_S25/Materials/M_R2_Soil`
- `/Game/IB_S25/Materials/M_R2v3_Roof`
- `/Game/IB_S25/Materials/M_R2v3_Metal`
- `/Game/IB_Donors/UrbanFacades/ImpactApartments/Materials/modular_urban_apartments_facade_plaster`
- `/Game/IB_Donors/CityKit/hotel/hotel/Materials/agua`
- `/Game/IB_S25/Vegetation/S25_IslandTree/Materials/island_tree_01`
- `/Game/IB_S25/Vegetation/S25_IslandTree/Materials/island_tree_01_branches`
- `/Game/IB_S25/Vegetation/S25_IslandTree/Materials/island_tree_01_leaves`

Se validan todas las rutas y tipos antes de escribir; una redirección fuera de la lista falla. Las instancias reciben un override local de uso: no se cambia su padre compartido de Interchange ni materiales, texturas o PNG de Fab. Cada cambio se verifica, guarda y registra en `Saved/IB_Playable_Authoring/author-report.json`, apartado `world_instancing`. Los `.uasset` modificados permanecen locales; la rama pública incorpora la receta que permite reproducir los flags y no esos binarios. La ejecución `s7-author-20261116T060201Z` verificó los once y guardó solo `agua`; los otros diez ya tenían el uso activado. No modifica fuentes Fab ni amplía permisos por carpeta. Comprobar el código de salida externo, render y cook: un reporte generado no prueba por sí solo que el proceso haya terminado correctamente.

## Pruebas de esta preparación

`IBS25_test.py`: 10 pruebas aprobadas y 1 omitida por ausencia deliberada de texturas locales. `IBS25_refine_test.py`: 5 aprobadas. El test de recibos sigue activo aunque falten bytes; la omisión no certifica integridad del asset. No se ejecutó autoría Unreal, cook, render, audio ni benchmark desde esta preparación pública.
