# Encargo para ChatGPT Work — Auditoría integral de Mi biblioteca y Fab

Fecha de preparación: 2026-11-15. **ESTADO: PREPARADO PARA WORK, NO EJECUTADO.** Este archivo es una instrucción para una nueva sesión Work y no implica que esa sesión haya sido creada ni que se haya accedido a la biblioteca privada.

## Objetivo

Revisar exhaustivamente la cuenta Fab del usuario y los recursos gratuitos compatibles con **Unreal Engine 5.8 / Windows**, para **IMPACT: BEYOND**, un videojuego 3D superheroico **privado, individual, no comercial**, sin entrenar ningún modelo de IA ni comprar assets/plugins. Principio: **reutilizar lo más posible y crear lo mínimo necesario**, con calidad y licencias comprobadas.

El usuario tiene la pestaña Fab abierta en otro chat. Work puede necesitar abrir Fab con su propio navegador o solicitar autenticación; no afirmar que ya está conectado.

## Instrucciones de navegación

1. Accede a https://www.fab.com/ con el navegador de Work. Comprueba si la cuenta está autenticada; si necesitas que el usuario intervenga para iniciar sesión o resolver un desafío de seguridad, pídeselo desde el navegador. **Nunca solicitar contraseñas en el chat ni eludir verificaciones**.
2. Entra en **Mi biblioteca / My Library**. Recorre todas las páginas y filtros disponibles. No inspecciones únicamente una portada o los primeros 20 resultados. Documenta páginas vistas, límites, contador total si existe y si el conjunto es completo o parcial.
3. Para cada producto **reclamado** registra: nombre, URL/UUID, autor, categoría, fecha de reclamo si visible, licencia/edición, estado comprado/reclamado, si es descargable, formato, versión de Unreal compatible, etiqueta NoAI, tamaño estimado y uso posible. Diferencia claramente RECLAMADO de INSTALADO: la biblioteca de Fab no prueba que el archivo esté en el proyecto Unreal del usuario.
4. Después busca en el **catálogo público de Fab**, aplicando filtros Free/Gratis, tipo Unreal Engine, versión 5.8 cuando exista y categorías: ciudad y edificios, calles, vehículos/objetos físicos, animaciones de caminar/correr/vuelo/combate, humanoides/civiles, robots desmontables/mechas, kaijus/monstruos, VFX Niagara, sonidos de impacto, teleportación, energía/constructos, Chaos/destrucción, terrenos/planetas/espacio. Recorre paginación de categorías y consultas relevantes, y explica la cobertura: **no afirmar 'todos los productos' si Fab tiene resultados inaccesibles o incontables**.
5. Separa **Gratis permanente**, **gratis por tiempo limitado**, **Desde Gratis** (varias licencias), **gratuito solo con condiciones** y **precio no verificable**. Antes de marcar un producto como Free, verifica que la ficha/variante realmente tenga precio cero.
6. Detecta duplicados, bundles, versiones antiguas, recursos pesados, características no incluidas (ej. animaciones mostradas en vídeo pero ausentes en paquete), restricciones por licencia de Fab/Unreal, NoAI, formatos y compatibilidad del motor. **No entrenar IA, no insertar IA generativa en runtime ni enviar recursos marcados NoAI como entradas a generadores**.

## Fuente de verdad y consolidación en GitHub

Conecta el repositorio `meliodr/juego-`. Antes de editar, lee:
- `AGENTS.md`
- `docs/PLAN_ACTIVO.md`
- `docs/FAB_SELECCION_ACTIVA.md`
- `docs/FAB_CATALOGO_MAESTRO.csv`
- `docs/FAB_AUDITORIA_LOTE01_2026-11-15.md`
- `docs/FAB_AUDITORIA_LOTE02_2026-11-15.md`
- `docs/PLUGIN_SELECTION_V14.md`

**No sobrescribir silenciosamente** observaciones o decisiones previas. El CSV inicial recoge 52 fichas gratuitas públicas; su columna `mi_biblioteca` está como `NO_VERIFICADO`. Reconciliar cada UUID con la biblioteca inspeccionada y actualizarlo a RECLAMADO/NO_ENCONTRADO_EN_REVISION_COMPLETA (solo si el inventario es exhaustivo) o conservar NO_VERIFICADO si no hay acceso, paginación completa o prueba. Registrar método y fecha de verificación. No concluir que algo está instalado en Unreal por aparecer en Fab.

Actualizar preferiblemente mediante una rama y PR para revisión:
- `docs/FAB_CATALOGO_MAESTRO.csv` (preservar datos previos, corregir/enriquecer mediante evidencia)
- `docs/FAB_SELECCION_ACTIVA.md` (lista priorizada para Codex)
- `docs/FAB_INVENTARIO_PERSONAL.md` (propiedad verificada y elementos aptos para el proyecto)
- `docs/FAB_AUDITORIA_LOTE03_<fecha>.md` o registro de novedades (URL de prueba y decisiones)

No subir a GitHub paquetes binarios Fab, datos privados, identificadores de cuenta, capturas con información personal o credenciales. El juego es solo para el usuario, pero conservar restricciones de las licencias. No usar archivos de otras franquicias sin derechos claros.

## Selección con puntuaciones y fases

Puntuar (0–10) cada candidato según utilidad directa para la visión de IMPACT, complejidad de adaptación, compatibilidad UE5.8, coste de VRAM (GPU de 8GB), calidad de rig/animaciones, destrucción y permisos. Clasificar:
- **SELECCIONAR** (mejor candidato por necesidad actual)
- **PROBAR** (requiere comparativa o prueba técnica)
- **RESERVAR** (útil para planetas, kaijus u otra fase futura)
- **DESCARTAR** (duplicado, inadecuado, caro, incompatible o falta de permiso)

Dar prioridad al primer juego jugable: **un héroe camina, corre, salta, vuela, agarra un carro, lo lanza contra una fachada perforable y combate**. No bloquear ese ciclo con un catálogo interminable. No integrar packs completos sin prueba pequeña. El cooperativo está muy lejos.

## Límites de acciones

**No comprar, reclamar, instalar, descargar, activar plugins, aceptar términos en nombre del usuario ni modificar el proyecto Unreal automáticamente.** Es una auditoría de catálogo y selección. No modificar contraseñas, seguridad, visibilidad del repo o configuración del equipo.

## Entrega

1. Cantidad total de elementos reclamados vistos y porcentaje/cobertura real (o imposibilidad de saberlo).
2. Tabla con RECLAMADOS útiles, incluyendo los ya instalados solo si se pudo verificar en el entorno.
3. Tabla amplia de recursos gratuitos públicamente comprobados con enlaces, prioridades y diferencias respecto al CSV original.
4. **Top 20** ordenado para el juego, con utilidad, coste de integración y fase.
5. Descartes razonados, duplicados y candidatos que solo parecían gratuitos.
6. Promociones con fecha límite sin reclamar automáticamente.
7. Enlace a archivos/PR/commit efectivamente actualizados en GitHub.
8. Bloqueos de acceso o licencias, sin afirmaciones inventadas.

Este encargo podrá continuar en varias sesiones si el volumen es grande. En cada etapa dejar inventario parcial y punto exacto para reanudar, sin afirmar revisión exhaustiva hasta terminar la paginación y filtros relevantes.
