# Draft: S3.5 — perfiles de agarre, golpes y contrato aislado

Destino previsto: `meliodr/juego-`.
Head local: `prototype/ib-s35-interactions-lab`.
Base solicitada: `prototype/ib-sprint02`.
**No existe PR remoto creado por este encargo. Publicación bloqueada por revisión automática.**

## Descripción preparada

El circuito actual selecciona vehículos por tags y vincula directamente el IK al circuito. Este laboratorio añade perfiles Car/Container/Pole/Rock/LoosePart, propiedad exclusiva, transporte mediante caja barrida, soltar/cancelar sin lanzamiento, impulsos por masa y golpes rápido/pesado con un mensaje deduplicado por AttackId/TargetId.

Los cambios quedan únicamente en IB_S35, Tools/IBS35_* y docs/IB_S35_*. No modifica Character principal, Circuit, GripMesh, Build.cs, uproject, Config, mapas de producción ni carpetas de otros sprints. Incluye un derivado H01, notify/evento de contacto, contrato de referencia para S4 y generador de un laboratorio con nueve objetos, blanco, dos muros y carril de vuelo. El adaptador del IK existente está descrito sin aplicar.

Validado: ejecutable C++17/MSVC 14.52.37025 con /W4 /WX; 2.595 aserciones del header de reglas que usa el runtime. Gate de rutas y sintaxis Python aprobado. Ninguna compra, instalación o asset binario nuevo.

Pendiente: UHT/build Unreal; ejecución de las tres suites Automation; generación efectiva del .umap/DataAssets; contacto de manos/poses e IK; colisiones reales y velocidades extremas; DX12 1080p/perfil y mando físico. No se obtuvo turno exclusivo coordinado de editor/build. Mantener draft y no integrar hasta esos gates.

Ver `docs/IB_S35_CONTRACT.md`, `docs/IB_S35_ADAPTATION.md` y `docs/IB_S35_EVIDENCE.md`.

## Bloqueo de publicación

Acción exacta rechazada antes de ejecutar: `git push -u origin prototype/ib-s35-interactions-lab` al remoto `https://github.com/meliodr/juego-.git`.

Motivo devuelto por la revisión automática:

> Pushing the branch publishes nonpublic project code to an external GitHub repository and creates an external commitment; the available user text does not explicitly authorize that payload and destination.

Se consultó user_message_read después del rechazo. Los mensajes visibles autorizan iniciar los cuatro encargos y usar el equipo, pero no aportaron una autorización nueva específica de esta publicación. No se reintentó por CLI ni por API, y no se creó un PR sin rama remota. Hace falta aprobación explícita para publicar estos archivos de S3.5 en ese repositorio y abrir el draft contra la base indicada.
