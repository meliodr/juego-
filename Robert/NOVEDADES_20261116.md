# Recursos gratuitos: revisión del 16 de noviembre de 2026

Comparado con Robert/RECURSOS_GRATUITOS.md, docs/REUSE_CATALOG.md y docs/FAB_CATALOGO_MAESTRO.csv. Solo investigación documental; no adquisición, descarga, instalación, importación, escucha ni prueba UE5.8. No modifica la selección activa o las correcciones en curso.

## Megascans: actualización oficial relevante para el entorno realista
[Quixel anunció el 5 de noviembre de 2026](https://quixel.com/news/update-megascans-are-going-free-again) que su biblioteca completa vuelve a ser gratuita en Fab. El mismo anuncio informa nuevos derechos de IA para Quixel; no extrapolarlos a contenido de otros autores ni convertirlos en autorización de entrenar modelos. IMPACT mantiene su regla de no entrenamiento.

**Decisión:** consultar una superficie o pieza concreta cuando el integrador identifique una carencia, no descargar la biblioteca. La noticia confirma gratuidad de la fuente, no propiedad, instalación ni compatibilidad de cada archivo. Antes de incorporar: comprobar ficha/edición, licencia efectiva de adquisición, formatos, tamaño y colisión. El [resumen Fab Standard](https://www.fab.com/eula) permite uso privado/comercial y adaptación dentro de proyectos, pero no redistribución del asset suelto. Conservar el acuerdo completo aplicable, no tratar este resumen como sustituto. No subir paquetes a GitHub.

## Reserva condicionada: Free Modular Magic SFX
[Ficha del autor Le Binh Son en Fab](https://www.fab.com/listings/f0422e67-33d3-4c6e-b7a1-1059193960de): precio **Free** comprobado. Anuncia 290 efectos, incluidos whooshes, viento, rayos e impactos, y 22 grafos MetaSounds. WAVE disponibles según descripción; grafos para UE5.6+ declarados, UE5.8 real no probado. Posible utilidad: audio de vuelo/poderes, solo si falta un sonido en la biblioteca existente.

El autor declara CC BY 4.0 y dispensa atribución en el texto; conservar crédito, enlace y cambios de todas formas siguiendo [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Ficha: uso con IA sí, generado con IA no. No se ha comprobado el LICENSE del paquete. El anuncio menciona un plugin de síntesis: **no asumir que los grafos funcionan sin él ni que el plugin completo es gratuito**. Evaluar primero los WAV autónomos; si requieren compra o dependencia no gratuita, descartar esa ruta. No instalar el plugin.

## Estado
Ambas entradas: candidato documental, no dependencia aceptada. No se ha escuchado audio, medido rendimiento, inspeccionado bytes ni validado arte. Elegir como máximo el subconjunto necesario y verificarlo en sandbox con el responsable del proyecto antes de importar. No se inicia nueva búsqueda masiva de Fab.

## Contraste dirigido de asfalto: conservar la opción existente

Revisión documental adicional del 16 de noviembre, sin acceder a bytes del proyecto local ni ejecutar sus recetas.

- [Asphalt de Quixel](https://www.fab.com/listings/eb97ec51-1041-46a4-831d-78178efd2651): Free, texture set/glTF, área de escaneo 2 × 2 m; la ficha recuperada indica uso con IA **Yes**.
- [Asphalt Surface de Quixel](https://www.fab.com/listings/7ed0b883-55d3-456a-ba47-f2a2112d7c2b): Free, texture set/glTF, área 0,42 × 0,42 m; la ficha recuperada también indica uso con IA **Yes**. Por tanto, esta consulta no reproduce la diferencia de indicadores informada previamente. No inferir una fecha de cambio ni invalidar una captura anterior: guardar la licencia concreta al adquirir. Ninguno se descargó o probó en UE5.8.
- El proyecto ya tiene una ruta documentada para [Asphalt 02 de Poly Haven](https://polyhaven.com/a/asphalt_02), CC0 según [licencia oficial](https://polyhaven.com/license). `Tools/IBS25_pbr.py` selecciona tres mapas 2K y `Tools/IBS25_sources/manifest.json` registra sus tamaños y SHA256: basecolor, normal DirectX y rugosidad. `docs/IB_S25_SOURCE_REBUILD.md` registra `/Game/IB_S25/Materials/M_Asphalt` entre los materiales de producción. Esto acredita receta/procedencia y recibos publicados, no una nueva verificación de archivos locales o del render actual.

**Decisión concreta:** no sustituir ahora el asfalto por Quixel ni añadir ambas alternativas al catálogo activo. Ya existe un recurso CC0 con receta y recibos. Primero diagnosticar escala UV, repetición, normal, rugosidad e iluminación en la misma build que se evalúe; cambiar de fuente solo si una captura comparativa demuestra una carencia que el material existente no puede resolver. Si hiciera falta un ensayo Quixel, usar un único material aislado, comprobar licencia y tamaño y conservar el original. La distinta área de escaneo obliga a respetar escala física; mayor densidad declarada no demuestra mejor apariencia o rendimiento. Ningún archivo de otros trabajos fue modificado.
