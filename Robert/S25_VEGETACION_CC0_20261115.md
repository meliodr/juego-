# S2.5 · Una malla vegetal, no otro catálogo

Corte 2026-11-15 12:14 UTC. Uso personal no comercial de Jarry; RD$0. Se contrastó PR #19 (1d01db54), PR #17 y ASSETS_CITY_SAMPLE_LITE_20261115: la alternativa vegetal ligera aún no identificaba una malla concreta. No se modifica la implementación ni la selección de edificios/coches.

## Candidato único: Shrub 03 de Poly Haven
Fuente oficial: https://polyhaven.com/a/shrub_03
Licencia: https://polyhaven.com/license

Ficha consultada: gratuito; autor Rico Cilliers; CC0. Arbusto de 1,4 m de ancho, 17K triángulos publicados; formatos FBX/glTF/Blend/USD; opciones 1K/2K/4K y etiqueta LODs. La página muestra 15,67 MB en su configuración recuperada: no es tamaño comprobado de un ZIP descargado ni garantía para todos los formatos. Dispone de alpha, normal DX y mapas PBR.

CC0 permite uso personal, modificación y redistribución sin atribución obligatoria. La licencia de assets consultada no impone NoAI; no extender esto a logos, texto o renders promocionales del sitio. No se copian assets en este PR.

## Decisión
CANDIDATO PARA ENSAYO, no instalado, importado ni probado. Resuelve vegetación baja del pequeño parque S2.5; **no sustituye un árbol ni resuelve toda la costa**. No recomendar despliegue masivo. La ficha no certifica UE5.8/Windows ni suministra prueba de material Unreal correcto.

Siguiente paso del integrador cuando corresponda: comprobar que no exista ya en disco; seleccionar FBX y texturas 1K–2K; registrar archivo, tamaño real y hash; importar una unidad en IB_S25 sin tocar IB_C00. Validar escala, normal DX, alpha/material masked/two-sided, LOD/culling y coste de overdraw. Comparar A/B con cámara/luz iguales, luego un grupo pequeño con GPU/VRAM medidos. No prometer 60 FPS por el número de triángulos. Si no mejora el aspecto, no integrar.

Control de descarte acotado: https://polyhaven.com/a/tree_small_02 es gratuito CC0 pero su ficha declara 5M tris; no se elige como alternativa ligera ni se descarga antes de revisar geometría/LOD. Ninguna otra colección solicitada.
