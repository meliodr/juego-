# IMPACT: BEYOND — Realismo con recursos selectivos, sin descargar City Sample entero

**2026-11-15 · Estado: recomendación de integración, NO descarga ni importación ejecutada.** El usuario ha autorizado elegir/obtener materiales gratuitos útiles, pero reclamar un producto Fab, aceptar nuevos términos, instalar plugins, modificar Windows y fusionar main continúan requiriendo autorización específica. No sustituir el proyecto real por otro juego de ejemplo.

## Evidencia técnica

Sprint 2 (PR #19, commit 1d01db54b63250ed0aacb0704a5ac16290324e32) usa cubos para carretera, esferas para colinas, conos/cilindros para árboles y un plano para océano en `Tools/Author-Sprint02World.py`, más coches del Kit Cidade Low Poly. Eso explica gran parte de la discrepancia visual. Requisitos medibles: 5 capturas de comparación con referencias Epic a cámara/luz/FOV similares, modelos de coche de calidad, vegetación PBR, calle/fachadas y agua creíbles, fps p95/p99 con audio/DX12 visible.

## Paquetes oficiales Epic verificados como Free en su ficha pública — importación no ejecutada

1. **[City Sample Buildings](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5):** >2,000 módulos, moderno/clásico, esquinas, muros, entradas, UE-Only. Elegir inicialmente **dos estilos + entradas + esquinas + cornisas** en distrito piloto. Evitar importar >2,000 piezas a ciegas. La ficha pública todavía muestra Add to My Library: NO se ha verificado propiedad para esta ficha separada.
2. **[City Sample Vehicles](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f):** 13 vehículos, incluidos sedán/SUV/deportivo/taxi/camión/bus. Ensayar **tres**: turismo, SUV y camión/furgoneta, con ruedas/colisión y material realistas. El H01 sigue usando física/agarre del juego; NO agregar sistemas de conducción o masa de tráfico si no se necesitan. UE-Only, gratis en ficha, reclamación y bytes locales sin comprobar.
3. **[City Sample Crowds](https://www.fab.com/listings/903037e9-e1ac-4f41-96e8-1683c6fa7ad4):** humanos/ropa; más adelante dos policías y civiles. No asumir uniformes/armas o IA incluidos. Modelo/licencia/rig reales a validar. No descargar cientos de MetaHumans para probar dos NPC.
4. **[Electric Dreams Environment](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306):** referencia de bosque/vegetación PCG realista UE; seleccionar mallas/materiales/árboles particulares solo si formato, licencia, RAM y VRAM lo permiten; no descargar/portar un entorno completo para tres árboles.

## City Sample PCG 5.8 como catálogo de IDEAS/ejemplos

**[City Sample PCG 5.8 de Epic](https://dev.epicgames.com/documentation/unreal-engine/city-sample-pcg-for-unreal-engine?lang=en-US)** reimagina la ciudad exclusivamente con PCG in-engine: curvas, relieve, bosques y gramáticas de edificios. Incluye grafos de ejemplos `City_Circular_1km_2_arteries` y pequeñas pruebas de construcciones, además de la demo completa con 18 grafos interdependientes. **La demo completa está pensada como aprendizaje, no como componente de producción listo**: no tiene tráfico/pedestres AI configurados como el original; con World Partition, streaming está desactivado por defecto, PCG no particionado y colisiones de edificios requieren configuración. En nuestra prueba se evalúa **un subgrafo pequeño** de calles/lotes/fachadas y su coste, no activar los 18 a la vez.

**Plan de adquisición más ligero:** primero verificar qué assets ya están en UE/Launcher/cache local; segundo pedir al usuario autorizar reclamación Fab de los dos packs separados Buildings/Vehicles si no están reclamados; tercero consultar tamaño anunciado/real de descarga y SSD libre; cuarto elegir paquetes/subconjuntos por necesidad; quinto importar en laboratorio aislado local; sexto medir contra anterior. No tomar ningún URL de ficha como enlace directo a ZIP: Fab entrega formatos UE vía Launcher/gestor y puede requerir acceso autenticado. El hecho de que City Sample completo esté reclamado en inventario NO implica que los tres paquetes separados estén reclamados.

**Fallback con CC0 ligero:** Poly Haven (https://polyhaven.com/license), ambientCG (https://ambientcg.com), mallas urbanas CC0 ya incluidas y Quaternius si su estilo se puede adaptar; priorizar asfalto y materiales de suelo PBR, texturas de 1K–2K y árbol realista de menor coste en vez de conos y esferas. No usar activos de Steam Workshop ni juegos comerciales salvo licencia expresamente compatible con redistribución en nuestro juego.

**Entrega de ensayo mínimo (S2.5):** una calle con 4 edificios, 3 autos y 1 parque/costa, cinco capturas y prueba con 1080p DX12/audio/60fps en RTX 4070 Laptop 8GB. Si no supera apariencia, no replicar el kit a 8 ciudades. Scripts para autoría reproducible con datos, no miles de archivos manuales en repo. UE-Only/Fab no se publica como asset standalone en GitHub público. No enviar bytes NoAI a IA generativa.

## Cronología para revisión nocturna programada

Las tareas programadas analizarán el estado real de Github en fases: +3h auditar/refinar cuatro laboratorios; +5h preparar o lanzar hasta ocho frentes hacia S7 solo si existe de verdad un ejecutor Codex conectado; +6h revisar y ensayar integración segura tras comprobar pruebas. **El acceso actual de este chat NO permite controlar directamente Codex Windows ni descargar assets UE a ese equipo**, de modo que no se afirma que esas acciones ya hayan ocurrido o que una tarea futura las pueda realizar sin conexión. Si el entorno sigue sin acción Codex, cada tarea publicará encargos, bloqueos y una ruta segura de ejecución, nunca éxitos falsos.

Relacionado: [plan de paralelismo](PLAN_PARALELO_S25_S3_S35_S4_20261115.md), [prompt S2.5](PROMPT_CODEX_S25_REALISMO.md), [roadmap](ROADMAP_SPRINTS_02_A_08.md).
