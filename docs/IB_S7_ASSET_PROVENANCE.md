# Procedencia del arte de la versión jugable S7

Actualizado: 2026-11-16. Esta nota registra instalación, autoría ejecutada y pruebas con alcance explícito. **La autoría y el paquete final pasaron las pruebas DX12, el gate de contenido y el perfil confirmatorio; la evaluación humana se registra aparte.**

| Recurso | Adquisición e instalación confirmadas en esta entrega | Carpeta de contenido |
|---|---|---|
| Adventure Characters / Adventures Pack | Ya estaba en la biblioteca del usuario; instalado hoy mediante Epic Games Launcher | `Adventures_Pack/` |
| [SciFi Beast06 Bull](https://www.fab.com/listings/126120a3-996e-4dc3-94f1-6550f0adac03) | Ya estaba en la biblioteca del usuario; instalado hoy mediante Epic Games Launcher | `SciFi_Beast06/` |
| [Sci Fi Robot 13](https://www.fab.com/listings/2b68ee14-cec8-4347-a5fa-5ba870160467) | Agregado gratuitamente a la biblioteca e instalado hoy mediante Epic Games Launcher | `robot13/` |

Los tres paquetes se instalaron en el proyecto donante autorizado del autor y se copiaron a la integración privada conservando sus rutas de Content. La variable `IB_PLAY_DONOR_CONTENT` identifica esa ubicación local; no se publica una ruta personal.
Las carpetas locales de integración se comprobaron por nombres, tamaños y presencia de archivos; no se enviaron bytes, texturas, mallas ni imágenes de los paquetes NoAI al modelo.

Los enlaces de Bull y Robot 13 proceden del [catálogo auditado](FAB_CATALOGO_MAESTRO.csv). La comprobación de biblioteca e instalación de esta entrega actualiza los estados históricos «NO_VERIFICADO» de los documentos anteriores; no convierte sus especificaciones anunciadas en pruebas de ejecución.

La integración excluye mediante `.gitignore` estas carpetas y todos los derivados de `ImpactBeyond/Content/IB_Playable/`. Los recursos originales y derivados permanecen locales y **no se suben como archivos de assets a GitHub**. El repositorio recibe código, scripts e informes de metadatos. La entrega prevista es un juego privado cocinado que incluya las dependencias utilizadas, no los paquetes fuente completos.

La autoría integrada en `b341476` **pasó con exit 0 y cero dependencias ausentes**, según el integrador: run `s7-author-20261116T053232Z`. Usa el C++ compilado `6b4631d` y el script entonces vigente `742ff49`. La revisión final `730b39f`, integrada en `ead1d59`, pasó en `s7-author-20261116T060201Z`: verificó once materiales y cambió/guardó únicamente `agua`. [IBPlay_AuthorProduction.py](../Tools/IBPlay_AuthorProduction.py) selecciona el personaje masculino Adventure05, conserva sus materiales originales y reserva los colores de identidad para energía/accesorios. Prepara referencias a Robot 13/Bull, animaciones, piezas por subárbol de hueso y unidades S4 derivadas localmente. Los detalles procedurales de contenedor, roca, propulsión y plataforma blindada son geometría original del proyecto.

El script escribió `Saved/IB_Playable_Authoring/author-report.json` con referencias, nombres de huesos, recuentos, advertencias y dependencias. La receta actual verifica flags de instancing de ocho materiales del mundo y tres del árbol, propios/CC0, que enumera la [receta reproducible](IB_S25_SOURCE_REBUILD.md), sin modificar padres compartidos, texturas ni fuentes Fab. Los `.uasset` modificados se guardan localmente y no forman parte de esta exportación.

El paquete final `8b56a20` pasó cook en `s7-package-20261116T060244Z` (37,01 s), cuatro funcionales DX12 con cero recursos ausentes/materiales de reserva en `s7-tests-20261116T060433Z` y perfil confirmatorio en `s7-tests-20261116T060613Z`. El ejecutable C++ `fa75f7f` conserva el mismo SHA-256 probado. El último cambio de contenido fue el flag local de instancing del agua; sus bytes no se exportan.

El perfil final de 90,011 s usa salida 1920×1080, escala interna 83%, calidad alta/2 y límite 60 FPS: media 59,9253 FPS, p95 16,6848 ms, p99 16,7421 ms, máximo 93,0082 ms con un fotograma >50 ms. No garantiza rendimiento universal ni sustituye revisión humana. Steam Input está configurado; arranque visible pendiente de cerrar aviso Windows de QA, sin cambios de firewall. No se publican imágenes, frames, texturas ni assets. [Resultados y límites](IB_S7_CLOSURE_REVIEW.md).
