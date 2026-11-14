---
name: impact-qa-review
description: Review an IMPACT BEYOND single-player task against its acceptance criteria, reproduce available Linux tests and report evidence without claiming unexecuted results.
---
# QA y revisión de entrega

Leer `AGENTS.md`, `docs/PLAN_ACTIVO.md` y la ficha en `docs/TAREAS_ACTIVAS.md`. Los documentos antiguos aportan casos adicionales cuando sean pertinentes, pero no son requisitos de red. **No ejecutar N0/N1, dos clientes, servidores, LAN o pruebas de latencia: cooperativo muy lejano.**

1. Identificar ticket, revisión exacta, cambio, permisos de recursos y resultado esperado. Separar lo inspeccionado de lo ejecutado. No aceptar la afirmación «terminado» del implementador como evidencia.
2. En entorno autorizado comprobar compilación, paquete y ejecución Linux cuando corresponda; registrar motor/toolchain, hardware, driver, resolución y calidad. No confundir contenedor del agente con equipo del usuario. Sin GPU/editor marcar el ámbito bloqueado.
3. Ejecutar la matriz del ticket: movimiento tierra-aire, colisión de carga, hueco transitable, desarme funcional, limpieza y reinicio. Las cantidades propuestas de repetición no prueban ausencia universal de errores.
4. Medir frame time, p95/p99, peor tirón, CPU/GPU/VRAM y actores/cuerpos cuando toque rendimiento; separar primera ejecución de recorrido repetido. No afirmar 60 FPS por un ajuste de límite o benchmark de terceros.
5. Para ciudadanos/eventos cuando se incorporen: comprobar atribución del incidente, alerta/karma separados, ausencia de recompensas duplicadas, huida y límites del director de eventos.
6. Pedir valoración del usuario cuando el cambio afecte control, claridad, cámara o potencia. No afirmar diversión a partir de logs. Si el mismo agente implementó y revisó, indicarlo y no simular QA independiente.
7. Emitir PASS/FAIL/BLOCKED por prueba, pasos para reproducir, evidencias, regresiones y recomendación CONTINUAR/CORREGIR/BLOQUEADO. Conservar la build anterior y no integrar fallos críticos ocultos.

No instalar drivers, cambiar el motor, borrar trabajo, exponer puertos o contratar servicios para superar un bloqueo sin autorización. La revisión no concede permisos nuevos ni inicia trabajo futuro automáticamente.
