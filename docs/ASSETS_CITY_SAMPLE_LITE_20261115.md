# IMPACT: BEYOND — Selección ligera de activos en vez de City Sample completo

**Revisión 2026-11-15. ESTADO: selección documental, no descargado ni integrado.** Usuario autorizó descargar recursos gratuitos realmente necesarios, siempre conforme a licencia y sin compras; las nuevas reclamaciones Fab/aceptaciones de términos, instalaciones de plugins o cambios de sistema requieren pasos de autorización que no se pueden eludir. En el chat NO hay conexión de ejecución local a Epic Launcher/Unreal/Codex. No declarar que se importaron assets por registrar URLs.

## Principio de calidad visual

Sprint 2 falló en arte porque el generador `Tools/Author-Sprint02World.py` utilizó primitivas visibles (Cube carretera, Sphere montañas, Cone + Cylinder árboles, Plane agua) y coches Kit Cidade Low Poly; esa geometría sigue sirviendo para pruebas geométricas aisladas, pero NO para el entorno final. Objetivo visual: 1/3–1/2 del **realismo percibido** de City Sample en núcleos críticos, no 1/3 o 1/2 de polígonos/VRAM. Validar con cinco vistas comparables y gameplay con audio/Steam Input.

## Orden de adopción mínimo y decisión de corte

| Prioridad | Recurso oficial | Qué extraer si está legalmente en disco | Estado |
|---|---|---|---|
| 0, ya presente | Poly Haven Urban Apartments Facade CC0 y materiales existentes `/Game/IB_Donors/UrbanFacades` | módulos de fachadas, texturas, ventanas interiores, mejorar roughness/normal/luces/relieve sin descargar nada | Ya contenido en branch S2; render final S2.5 aún pendiente |
| 1, condicionado | [City Sample Buildings](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5) — Fab **Free**, UE-Only, etiqueta NoAI | **una familia de 3–6 módulos** (esquina, entrada, muro, cornisa/techo), dos edificios completos como prueba de calidad. No importar 2000 módulos. | Precio gratis público verificado; Mi Biblioteca/bytes/size/instalación y compatibilidad efectiva no confirmados |
| 2, condicionado | [City Sample Vehicles](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f) — Fab **Free**, UE-Only, NoAI | **dos o tres modelos** (sedán/SUV/camión) y solo meshes/texturas necesarias; conservar nuestro PhysicsGrab/colisión, no importar IA o controlador de tráfico | Gratis público; no confirmada propiedad/bytes/instalación |
| 3, condicionado | [Game Animation Sample](https://www.fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016) — Fab **Free**, etiqueta NoAI | 2–5 animaciones específicas de correr, frenar, saltar, aterrizar, giro y reacción; retarget a Manny sin importar Motion Matching completo si no aporta | Gratis público; sample grande, no descargar sin necesidad medible |
| Reserva | [Electric Dreams Environment](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306) — Fab **Free**, UE-Only, NoAI | estudiar PCG/selva, como referencia; no descargar proyecto entero para conseguir 3 árboles | Gratis público; pesado y de versión antigua, no prioridad |
| 1 alternativa ligera | [Poly Haven](https://polyhaven.com/license), [ambientCG](https://ambientcg.com) — CC0 por licencia de biblioteca (verificar el archivo individual) | un árbol/roca PBR/HDRI y 2–4 materiales de suelo, asfalto, metal y hormigón para un bloque de demostración | Selección y descarga aún no realizada |
| Tecnológica, no paquete obligatorio | [City Sample PCG 5.8](https://dev.epicgames.com/documentation/unreal-engine/city-sample-pcg-for-unreal-engine?lang=en-US) | usar conceptos/PCG Primitives y subgrafos aislados de calles, parcelas y edificios si ya accesibles; **no cargar los 18 grafos** | Ref/documentación oficial, plugin CitySamplePCG dentro de sample pesado; disponibilidad separada no verificada |

**Advertencia de distribución:** los paquetes Fab Buildings/Vehicles se listan como assets **independientes**, pero eso NO asegura que la plataforma permita elegir un solo módulo al descargar. Si solo ofrece descargar el paquete completo y su tamaño es excesivo, **no** afirmar que descargamos individualmente las piezas: elegir alternativa ligera existente/PBR CC0 o esperar nueva autorización para coste de disco. Ni el pack ni sus fuentes sueltas deben publicarse como redistribución en repo público; aplicar licencia Unreal/Fab. No pasar meshes/texturas NoAI como entrada a herramientas de IA generativa.

## Orden de implementación sobre Windows y decisiones

1. Comprobar disco libre **actual**, versión del motor/Launcher y presencia local exacta de paquetes. Último valor ~115GB es histórico; no reservar recursos con ese número sin verificar.
2. Si materiales de facetas existentes logran calidad, no descargar nada. Si no, elegir un recurso **principal** con objetivo observable y registrar exacto tamaño/edición/archivo.
3. Previsualizar en mapa **IB_S25** con comparación A/B y licencia, no sobrescribir IB_C00. Ejecutar una sola instancia de Unreal Editor/cook a la vez.
4. Permitir como prueba inicial **solo 2 fachadas + 2 coches + 1 zona verde**. Si cumple visual/perf, ampliar a 1 distrito piloto. No importar la ciudad completa ni el sistema de tráfico/crowds de City Sample por inercia.
5. Meta 1080p 60FPS RTX 4070 laptop 8GB con audio y render visible; medir p95/p99, VRAM y hitches y comparar el mismo encuadre con la build anterior. Reducir luces, decals, texturas/LOD antes que colisión/velocidad.
6. Si hay opciones de descarga CC0 directa con archivos separados, usar sólo lo imprescindible y guardar URL+autor+licencia+hash; si Fab pide reclamación y aceptación de licencia nueva que no está completada, registrar bloqueado y no asumir permisos.

## Qué puede y qué no puede hacer esta respuesta

**Puede:** comprobar fuentes públicas Free y documentar el plan/URLs en GitHub; programar revisiones GitHub con automaciones. **No puede desde ChatGPT:** instalar/fetch en el Epic Launcher de Windows, seleccionar modelos en un chat Codex inexistente, garantizar tamaño descarga, activar Unreal/MCP ni probar una build. La configuración de modelos de Codex sólo se puede aplicar cuando la sesión ejecutora existe y confirma disponibilidad.

**Estado al escribir:** No se descargaron ni importaron nuevos activos de Fab/Poly Haven en Windows desde este chat.
