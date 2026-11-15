# Draft: S3.5 — perfiles de agarre, golpes y contrato aislado

Destino previsto: `meliodr/juego-`.
Head publicado: `prototype/ib-s35-interactions-lab`.
Base solicitada: `prototype/ib-sprint02`.
**Publicado: [PR #26](https://github.com/meliodr/juego-/pull/26), abierto en draft contra prototype/ib-sprint02, sin merge.** Publicación autorizada expresamente el 2026-11-15; la publicación no acredita compilación Unreal.

## Descripción preparada

El circuito actual selecciona vehículos por tags y vincula directamente el IK al circuito. Este laboratorio añade perfiles Car/Container/Pole/Rock/LoosePart, propiedad exclusiva, transporte mediante caja barrida, soltar/cancelar sin lanzamiento, impulsos por masa y golpes rápido/pesado con un mensaje deduplicado por AttackId/TargetId.

Los cambios quedan únicamente en IB_S35, Tools/IBS35_* y docs/IB_S35_*. No modifica Character principal, Circuit, GripMesh, Build.cs, uproject, Config, mapas de producción ni carpetas de otros sprints. Incluye un derivado H01, notify/evento de contacto, contrato de referencia para S4 y generador de un laboratorio con nueve objetos, blanco, dos muros y carril de vuelo. El adaptador del IK existente está descrito sin aplicar.

Validado: ejecutable C++17/MSVC 14.52.37025 con /W4 /WX; 2.613 aserciones del header de reglas que usa el runtime y 280 aserciones de comparación con las políticas nativas S4 leídas sin modificarlas. Gate de rutas y sintaxis Python aprobado. Ninguna compra, instalación o asset binario nuevo.

La revisión S4 documenta incompatibilidades de enums, signo/semántica de daño, unidades de impulso, identidad/tiempo/dedup y agarre de piezas. S3.5 ahora conserva pieza/origen y daño solicitado, y rechaza enum/edad/salud inválidos antes del ledger. No se instaló un adaptador global ni se modificó S4; ver `IB_S35_S4_INTEGRATION_REVIEW.md`.

Pendiente: UHT/build Unreal; ejecución de las tres suites Automation; generación efectiva del .umap/DataAssets; contacto de manos/poses e IK; colisiones reales y velocidades extremas; DX12 1080p/perfil y mando físico. No se obtuvo turno exclusivo coordinado de editor/build. Mantener draft y no integrar hasta esos gates.

Ver `docs/IB_S35_CONTRACT.md`, `docs/IB_S35_ADAPTATION.md` y `docs/IB_S35_EVIDENCE.md`.

## Historial del bloqueo de publicación

Acción exacta rechazada antes de ejecutar: `git push -u origin prototype/ib-s35-interactions-lab` al remoto `https://github.com/meliodr/juego-.git`.

Motivo devuelto por la revisión automática:

> Pushing the branch publishes nonpublic project code to an external GitHub repository and creates an external commitment; the available user text does not explicitly authorize that payload and destination.

En aquella revisión se consultó user_message_read después del rechazo. Los mensajes visibles no aportaban autorización nueva específica; no se reintentó por otra vía ni se creó un PR sin rama remota.

Actualización 2026-11-15, 16:09 UTC: el usuario aprobó explícitamente subir las cuatro ramas al repositorio público meliodr/juego- y abrir PR en borrador sin fusionarlos. Se verificó la respuesta junto a la pregunta original. Antes de publicar S3.5 se comprobó que no existían rama remota ni PR duplicado, que los 30 archivos eran código/documentación propios sin assets binarios y que los cuatro commits locales no coincidían con los patrones de secretos revisados. Esta aprobación permite un nuevo intento de publicación; no autoriza saltar una nueva denegación ni hacer merge.

Resultado confirmado 16:14 UTC: push aceptado sin force; rama nueva publicada y PR #26 creado con draft=true y merged=false. La base remota continúa en `1d01db54b63250ed0aacb0704a5ac16290324e32`. Los gates técnicos pendientes de la descripción permanecen sin ejecutar.
