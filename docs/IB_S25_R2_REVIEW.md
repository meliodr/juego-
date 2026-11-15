# S2.5 R2: una manzana y parque, QA pendiente

El usuario rechazó el realismo de la primera tanda. Esta revisión concentra el trabajo en la manzana suroeste y el parque existente; no amplía la ciudad. Es autoría y revisión visual por el mismo agente, no QA independiente ni prueba de jugabilidad.

## Cambios y límites

- Tres fachadas: piedra clara con bandas horizontales; azul con aletas verticales; arcilla con balcones y barandillas. Se reutilizan las mallas reales de entrada, ventanas y cornisa. Las plantas bajas incorporan marquesinas, pilares laterales y bandas de rótulo todavía sin identidad comercial.
- Cuatro pasos de cebra, líneas de detención, bordillos segmentados y rebajes. Bancos de lamas, bolardos perfilados y luminarias con cabezal. La geometría se autoró localmente; no se sustituyeron árboles ni edificios por primitivas visibles.
- Parapetos continuos con albardillas; equipos de cubierta con apoyos, lamas, rejillas de ventilador y conductos. Las cubiertas secundarias de las otras manzanas no se detallaron.
- Parque con tierra PBR, caminos conectados y alcorques con borde y rejilla. Se conservan los árboles existentes. La vegetación lejana sigue perdiendo densidad visual; no hay nueva especie ni afirmación de optimización.
- Suelo de apoyo al horizonte, sin edificios nuevos, colisión ni sombras propias. Es contexto provisional para evitar el vacío negro; no constituye un paisaje terminado.

## Diagnóstico antes de corregir

**Ventanas:** captura A/B a la misma cámara 03, sin modificar mallas ni sol. Al sustituir temporalmente `M_MVP_WindowInteriors` por el cristal donante desaparecieron las diagonales. Esto localiza el defecto en el tratamiento del material aplicado, no demuestra una causa concreta dentro de su proyección. Se restauró el cristal original en los componentes afectados. El material original queda intacto, y el interior mapping requiere revisión de UV por paño antes de reactivarlo.

**Fachadas nuevas:** otro A/B con la misma geometría mostró manchas negras con el material nuevo y una superficie normal al volver al material donante. La solución conserva el grafo Substrate glTF existente mediante instancias propias y cambia únicamente `BaseColorFactor`. No se atribuye el defecto a la iluminación ni se oculta cambiando exposición.

Las cámaras mantienen exactamente las poses publicadas en la tanda anterior, incluida esquina (-18,1,1.7), FOV65. Exposición EV14, sol pitch -32/yaw -35, 65.000 lux. SkyLight pasa de 1 a 1,25 para relleno indirecto moderado; ese cambio está declarado en metadatos. No hay retoque de las capturas.

## Recursos y reproducción

Nuevo recurso: [Brown Mud Leaves 01](https://polyhaven.com/a/brown_mud_leaves_01), Rob Tuytel, [CC0](https://polyhaven.com/license), tres archivos 1K (diff JPG, normal DX PNG, roughness JPG), 7.959.346 bytes. Aceptado para prueba aislada; empaquetado Linux sin verificar. [Recibo con hashes](IB_S25_R2_SOIL_SOURCES.json). Coste RD$0. Los bytes fuente y uassets permanecen locales; no se publica contenido Fab/UE-Only.

Con la base del laboratorio ya reconstruida y su mapa abierto en un turno autorizado:

Para una reproducción desde cero, usar otro checkout aislado de la base congelada y llevar allí los scripts `IBS25_*` y sus OBJ, **sin copiar todavía el mapa R2**. Seguir primero la preparación documentada del laboratorio base (donantes locales autorizados, fuentes CC0, `IBS25_stage.py`, `IBS25_lookdev.py`). Después aplicar el refinador. No intentar que el autor base sobrescriba el mapa R2 publicado: su rechazo es intencional. Este procedimiento requiere el árbol y las dependencias locales que no se redistribuyen; no se promete que un clone por sí solo renderice el mapa.

1. `python -B Tools/IBS25_soil.py` descarga únicamente esos tres mapas con límite y reserva SSD.
2. `python -B Tools/IBS25_refine_recipe.py` genera OBJ propios inmutables y versionados.
3. `python -B Tools/IBS25_refine_test.py` y `python -B Tools/IBS25_test.py` ejecutan las pruebas ligeras.
4. En Unreal, ejecutar `IBS25_refine.py`; después `IBS25_refine_validate.py` y `IBS25_review.py`.
5. No ejecutar el autor antiguo sobre el mapa R2: el marcador de revisión lo rechaza. La validación R2 repite únicamente el refinador. Los scripts de diagnóstico producen carpetas nuevas y rechazan sobrescrituras.

No se ejecutaron cook, paquete, benchmark A/B, recorrido jugable ni prueba de colisiones. No se han medido FPS, p95/p99 o VRAM. El estado de realismo continúa **NO ACEPTADO / pendiente de QA humano**.

## Resultado de esta pasada

[Las cinco vistas y la comparación R1/R2](IB_S25_VISUAL_REVIEW.md) fueron inspeccionadas después de capturarse. La tanda final es `visual_20261115T172250707258`; cinco PNG 1920×1080, con diez segundos y al menos 240 ticks de Slate por vista. El mínimo de ticks evita tomar una imagen inmediatamente después de recuperar el foco del Editor; no es una métrica de rendimiento. Se añadió una toma cenital de intersección con cámara temporal, eliminada tras la captura.

Pruebas: 5 regresiones R2 y 11 pruebas base PASS. Repetición real del refinador: 86 actores/1.438 instancias, mismo hash `228e868be291d90452bfd61bf137af1c8d4ff0af8ba5adfd2119c5a5e6087ff1`. [Validación](IB_S25_R2_VALIDATION.json). Los fallos intermedios de API/material y las tandas rechazadas se conservan localmente, sin presentarlos como aprobados.

El soporte de horizonte extiende solo el suelo de fondo, sin nuevas manzanas ni colisión. Su escala horizontal es 100; la textura compensa esa escala para mantener el tamaño físico del material. El paisaje lejano continúa plano y provisional. No se usó niebla para esconder bordes ni se cambiaron cámaras/exposición.

La sesión final PID6672 cerró con código0 a 17:28:50 UTC; no quedó proceso UE/build activo y el mutex quedó libre. El cierre de la sesión anterior PID25820 no se observó; su ausencia se comprobó antes de abrir la sesión final. Config se respaldó y restauró a HEAD después del cierre.
