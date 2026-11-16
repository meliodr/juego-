# Procedencia del arte de la versión jugable S7

Actualizado: 2026-11-16. Esta nota registra instalación y preparación local; **no acredita todavía compatibilidad ni funcionamiento en UE 5.8 o en el juego empaquetado**.

| Recurso | Adquisición e instalación confirmadas en esta entrega | Carpeta de contenido |
|---|---|---|
| Adventure Characters / Adventures Pack | Ya estaba en la biblioteca del usuario; instalado hoy mediante Epic Games Launcher | `Adventures_Pack/` |
| [SciFi Beast06 Bull](https://www.fab.com/listings/126120a3-996e-4dc3-94f1-6550f0adac03) | Ya estaba en la biblioteca del usuario; instalado hoy mediante Epic Games Launcher | `SciFi_Beast06/` |
| [Sci Fi Robot 13](https://www.fab.com/listings/2b68ee14-cec8-4347-a5fa-5ba870160467) | Agregado gratuitamente a la biblioteca e instalado hoy mediante Epic Games Launcher | `robot13/` |

Los tres paquetes se instalaron en el proyecto donante autorizado del autor y se copiaron a la integración privada conservando sus rutas de Content. La variable `IB_PLAY_DONOR_CONTENT` identifica esa ubicación local; no se publica una ruta personal.
Las carpetas locales de integración se comprobaron por nombres, tamaños y presencia de archivos; no se enviaron bytes, texturas, mallas ni imágenes de los paquetes NoAI al modelo.

Los enlaces de Bull y Robot 13 proceden del [catálogo auditado](FAB_CATALOGO_MAESTRO.csv). La comprobación de biblioteca e instalación de esta entrega actualiza los estados históricos «NO_VERIFICADO» de los documentos anteriores; no convierte sus especificaciones anunciadas en pruebas de ejecución.

La integración excluye mediante `.gitignore` estas carpetas y todos los derivados de `ImpactBeyond/Content/IB_Playable/`. Los recursos originales y derivados permanecen locales y **no se suben como archivos de assets a GitHub**. El repositorio recibe código, scripts e informes de metadatos. La entrega prevista es un juego privado cocinado que incluya las dependencias utilizadas, no los paquetes fuente completos.

La autoría está **pendiente de ejecución y validación**. [IBPlay_AuthorProduction.py](../Tools/IBPlay_AuthorProduction.py) selecciona el personaje masculino Adventure05, conserva sus materiales originales y reserva los colores de identidad para energía/accesorios. Prepara referencias a Robot 13/Bull, animaciones, piezas por subárbol de hueso y unidades S4 derivadas localmente. Los detalles procedurales de contenedor, roca, propulsión y plataforma blindada son geometría original del proyecto.

Al ejecutarse, el script escribirá `Saved/IB_Playable_Authoring/author-report.json` con las referencias, nombres de huesos, recuentos, advertencias y paquetes/archivos exactos de dependencia. Las comprobaciones de esqueleto, materiales, colisiones, piezas desprendibles, animaciones, PIE y paquete Windows siguen pendientes; no deben darse por aprobadas solo por tener los archivos instalados.
