# PR draft preparado localmente — NO PUBLICADO

Destino previsto: `meliodr/juego-`, base `prototype/ib-sprint02`, head `prototype/ib-s4-response-lab`, `draft=true`. Commit inicial de implementación local: `c46d18bb6b67f95bd4fbeede85636356d12f0ec5`; revisión posterior descrita en `docs/IB_S4_STATIC_REVIEW.md`. El HEAD de la rama identifica la entrega local actual. No se ha creado una URL de PR.

## Título

S4 lab · Respuesta no letal, poderes y destrucción preparada — pendiente Unreal

## Cuerpo propuesto

Este laboratorio añade respuestas diferenciadas para que las balas humanas no resten salud ni interrumpan a H01 y los misiles produzcan empuje acotado con protección contra impactos encadenados. Añade policía activada por causa registrada, lanzador estático de prueba, cañón robótico separable, láser por material, telequinesis con barrido del objeto, estados de muro/cráter con colisión y Pawn independiente con intención explícita de aterrizaje.

Base fija `1d01db54b63250ed0aacb0704a5ac16290324e32` (PR #19). Cambios únicamente en `Source/ImpactBeyond/IB_S4`, `Tools/IBS4_*` y `docs/IB_S4_*`; no modifica personajes/circuitos/mapas compartidos, Build.cs, proyecto ni Config. No depende de S3.5. Sin nuevos plugins, assets de terceros o compras.

**Validación ejecutada:** política C++ compilada con MSVC `/W4 /WX`, 2.010 comprobaciones PASS en la revisión: 20 balas, 20 misiles espaciados y 20 rápidos, 20 muestras por cada material de láser, 20 layouts de cráter, 10 secuencias de alerta y regresiones de deduplicación, salud y geometría de restauración. Son pruebas nativas de política/geometría, no ciclos físicos de Unreal. Auditoría de rutas y parseo de tres scripts Python PASS. En la entrega inicial también pasaron el parseo PowerShell y un fixture sintético del resumidor de métricas; no son un benchmark real.

La revisión separa daño solicitado de pérdida real de HP, rechaza la resurrección de un AttackId caducado con fecha renovada, consulta el volumen que ocupa el suelo al restaurarse y conserva el torso como bloqueador del cañón desprendido. Los contratos S3.5 siguen sin adaptador global: sus enumeraciones, signo de daño, unidades físicas e IDs requieren reconciliación explícita.

**Pendiente / no fusionar:** UHT/compilación Unreal, generación del `.umap`, suites físicas preparadas, 20 ciclos reales de cada mecánica, Niagara/audio/render, Steam/DualSense, rendimiento DX12 1080p/p95/p99/VRAM. El turno pesado está reservado a S2.5. El tanque es un launcher de caja si no se configura malla autorizada, el robot es un fixture y Manny es provisional; no se afirma arte final ni fotorealismo. Faltan animaciones de advertencia/apuntado/disparo integradas y assets Niagara cargados. El guardado preserva estados estructurales, no poses exactas de cuerpos sueltos.

Reproducción y matriz: `docs/IB_S4_RESPONSE_LAB.md`. Resultado de revisión con hashes: `docs/IB_S4_REVIEW_NATIVE_RESULTS.json`; resultado inicial histórico: `docs/IB_S4_NATIVE_RESULTS.json` (1.902 comprobaciones). Inventario/licencias: `docs/IB_S4_ASSETS_LICENSES.md`. Mapa previsto: `/Game/IB_S4/Lvl_IB_S4_Response`, generador `Tools/IBS4_AuthorLab.py` (todavía no ejecutado).

## Bloqueo de publicación

La llamada `git push -u origin prototype/ib-s4-response-lab` fue **RECHAZADA antes de ejecución** por revisión automática. Razón comunicada: publicación externa a remoto GitHub considerado no verificado; no consideró acreditadas la autorización explícita del payload ni titularidad/privacidad del repositorio a partir del pedido de enviar prompts. El encargo delegado sí solicitaba PR draft, pero el control automático no aceptó esa evidencia como suficiente.

Se consultó el historial visible mediante `user_message_read`; no había nueva autorización relevante. Conforme a la instrucción posterior de coordinación, **no se reintentó ni se cambió de método**. No se creó rama/commit/PR por API para eludir el bloqueo. Hace falta autorización verificable para publicar esta rama y estos archivos antes de continuar esa acción. El trabajo local y las pruebas permanecen disponibles.
