# Preparación de la publicación S7

**Estado: fuentes públicas preparadas; exportación final de Source y entrega jugable pendientes.** Base pública `6c2d903c708553e7c840d27a96e5eb86810cb8a7`. La rama nace directamente de esa base; no hereda el historial de all-staging ni sus commits locales de assets.

La lista exacta de fuentes S2.5 ya recuperadas y de rutas candidatas está en [IB_S7_PUBLIC_EXPORT.json](IB_S7_PUBLIC_EXPORT.json). Su inventario preliminar corresponde a `4f95872`; no identifica la futura build final. Los campos de commit final, exportación y evidencia se completan únicamente cuando el integrador entregue el SHA congelado.

## Exportación final

1. Verificar que la base pública continúa siendo la esperada. Si main avanzó, reconciliar conservando sus cambios. No sustituir el documento `Robert/FACHADAS_CC0_20261116.md` ni aplicar su eliminación accidental desde la rama local antigua.
2. Recibir el SHA final del integrador y leer archivos por ese commit, no copiar un árbol mutable. Exportar solo cambios permitidos de Source, herramientas seleccionadas, configuración textual necesaria y `.uproject`. Incorporar pruebas nuevas posteriores al inventario preliminar. No mergear/cherry-pickear la rama privada ni sus commits de arte.
3. Preservar las recetas S2.5 de esta rama y la fixture actualizada `Tools/IBS35_CompareS4.cpp`; no sobrescribirlas con versiones ausentes o históricas del staging. Fusionar `.gitignore` manteniendo exclusiones privadas. Conservar el plan/cola consolidados de esta preparación.
4. Revisar cada documento adicional. Recibos históricos con rutas personales, logs completos y capturas no se copian por defecto. Reescribir requisitos locales como rutas relativas o variables explícitas, en particular el fallback personal de `IB_PLAY_DONOR_CONTENT`.
5. Prohibido añadir binarios de Content, packs Fab, derivados `IB_Playable`, Binaries, Intermediate, Saved, DDC, archivos fuente descargados o payloads de portabilidad. No confiar únicamente en `.gitignore`: Git puede seguir archivos ya añadidos. El gate inspecciona todos los commits introducidos, incluso un asset borrado después.
6. Actualizar `final_source_commit` y `final_export_paths` con la lista real y hashes relevantes; verificar diff, procedencia y árbol limpio. La lista preliminar no es autorización para importar cualquier archivo nuevo.

El repositorio conserva los assets que ya estaban en la base pública. Esta operación no vuelve a añadirlos ni amplía su distribución. El autor debe proveer localmente las dependencias licenciadas y regenerar los outputs del juego; un clone de fuentes no promete una build visual autosuficiente.

## Comprobaciones antes de publicar

El workflow existente `Lab audit tests` conserva sus pruebas de gates. `Integrated source checks` añade política de publicación, sintaxis Python sin importar Unreal, recetas S2.5, geometría S3 y 13 suites C++17 con MSVC en Windows. Falta cualquier suite requerida implica fallo, nunca PASS vacío. Un repo que solo contiene esta preparación aún no puede pasar la etapa nativa: Source final no se ha exportado.

`IBCI_Native.py` admite `--root` y salida fuera del checkout. No ejecuta UBT/editor, no instala herramientas y comprueba hashes antes/después para detectar cambios durante las pruebas. Los wrappers históricos `IBS35_CheckScope.py`, `IBS4_Check.py` e `IBS6H_Check.py` conservan gates de laboratorios S2; no se usan como CI de integración porque rechazan legítimamente cambios compartidos.

La comparación S35/S4 se conserva actualizada: rechaza replay con timestamp refrescado e IDs retirados tras reutilizar slots; mantiene pruebas de signo, unidades, categorías, capacidad e inmunidad. Los tests `ImpactContractNative` y `IBS4_ContractConsumerTests` cubren además el contrato canónico actual. No confundir esa comparación con ejecución de UObject/CMC.

Preparación comprobada localmente: 12 suites nativas del staging con snapshot estable y 282 assertions de la comparación actualizada, compiladas con MSVC; pruebas de recetas documentadas en [reconstrucción S2.5](IB_S25_SOURCE_REBUILD.md). La ejecución de las 13 juntas en la exportación final y en GitHub sigue pendiente. No se ha ejecutado Unreal desde este frente.

El integrador añade evidencia del SHA final para Editor/Game, automatización real de input, cook/paquete, DX12/audio, controles, persistencia y rendimiento. Registrar aparte las comprobaciones que requieren DualSense físico y aceptación humana. No afirmar S7 completado porque el CI nativo esté verde.

## Reconciliación de PR

Tras integrar y validar el PR limpio, #26/#27/#28 pueden cerrarse como cubiertos por la integración. #25 puede cerrarse como laboratorio sustituido por la partida S7, preservando la rama y sin afirmar que todo su contenido fue importado. #7/#8 son prototipos anteriores; #21 tiene sus scripts/tests incorporados mediante PR30. PR17/22/23 aportan decisiones consolidadas en el plan activo; sus prompts históricos siguen en sus ramas. Investigación/inventarios #1/#5/#6/#20 no se etiquetan integrados cuando sus archivos continúan ausentes.

Esta preparación no publica comentarios, pushes, merges ni cierres. La publicación y evidencia final quedan a cargo del integrador dentro de la autorización vigente del usuario.
