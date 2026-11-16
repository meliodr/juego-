# Procedencia del arte de la versión jugable S7

Actualizado: 2026-11-16. Esta nota registra instalación, autoría ejecutada y pruebas con alcance explícito. **El paquete final y su presentación todavía no están validados.**

| Recurso | Adquisición e instalación confirmadas en esta entrega | Carpeta de contenido |
|---|---|---|
| Adventure Characters / Adventures Pack | Ya estaba en la biblioteca del usuario; instalado hoy mediante Epic Games Launcher | `Adventures_Pack/` |
| [SciFi Beast06 Bull](https://www.fab.com/listings/126120a3-996e-4dc3-94f1-6550f0adac03) | Ya estaba en la biblioteca del usuario; instalado hoy mediante Epic Games Launcher | `SciFi_Beast06/` |
| [Sci Fi Robot 13](https://www.fab.com/listings/2b68ee14-cec8-4347-a5fa-5ba870160467) | Agregado gratuitamente a la biblioteca e instalado hoy mediante Epic Games Launcher | `robot13/` |

Los tres paquetes se instalaron en el proyecto donante autorizado del autor y se copiaron a la integración privada conservando sus rutas de Content. La variable `IB_PLAY_DONOR_CONTENT` identifica esa ubicación local; no se publica una ruta personal.
Las carpetas locales de integración se comprobaron por nombres, tamaños y presencia de archivos; no se enviaron bytes, texturas, mallas ni imágenes de los paquetes NoAI al modelo.

Los enlaces de Bull y Robot 13 proceden del [catálogo auditado](FAB_CATALOGO_MAESTRO.csv). La comprobación de biblioteca e instalación de esta entrega actualiza los estados históricos «NO_VERIFICADO» de los documentos anteriores; no convierte sus especificaciones anunciadas en pruebas de ejecución.

La integración excluye mediante `.gitignore` estas carpetas y todos los derivados de `ImpactBeyond/Content/IB_Playable/`. Los recursos originales y derivados permanecen locales y **no se suben como archivos de assets a GitHub**. El repositorio recibe código, scripts e informes de metadatos. La entrega prevista es un juego privado cocinado que incluya las dependencias utilizadas, no los paquetes fuente completos.

La autoría integrada en `b341476` **pasó con exit 0 y cero dependencias ausentes**, según el integrador: run `s7-author-20261116T053232Z`. Usa el C++ compilado `6b4631d` y el script exacto `742ff49`; la limpieza de wrappers resolvió el fallo anterior durante teardown de Python. [IBPlay_AuthorProduction.py](../Tools/IBPlay_AuthorProduction.py) selecciona el personaje masculino Adventure05, conserva sus materiales originales y reserva los colores de identidad para energía/accesorios. Prepara referencias a Robot 13/Bull, animaciones, piezas por subárbol de hueso y unidades S4 derivadas localmente. Los detalles procedurales de contenedor, roca, propulsión y plataforma blindada son geometría original del proyecto.

El script escribió `Saved/IB_Playable_Authoring/author-report.json` con referencias, nombres de huesos, recuentos, advertencias y dependencias. Aplicó flags de instancing a los siete materiales propios/CC0 S25 que enumera la [receta reproducible](IB_S25_SOURCE_REBUILD.md), sin modificar padres compartidos, texturas ni fuentes Fab. Esos siete `.uasset` se guardaron localmente y no forman parte de esta exportación.

`Playable.RealInput` pasó con DX12 offscreen sobre `761a2b5` en `s7-tests-20261116T052438Z`: eventos simulados de teclado/mando en la partida, no una evaluación de arte, rendimiento, audio o DualSense físico. Fortnite estaba activo durante esa ejecución. Las pruebas funcionales y la sesión sin render se detallan en [la revisión de cierre](IB_S7_CLOSURE_REVIEW.md). La inspección de materiales, animaciones y piezas a cámara de juego, junto con cook, arranque y recorrido del paquete exacto, permanece pendiente; ningún PASS de autoría la sustituye.
