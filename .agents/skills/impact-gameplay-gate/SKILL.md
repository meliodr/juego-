---
name: impact-gameplay-gate
description: Implement and validate one small single-player IMPACT BEYOND mechanic in the authorized Linux environment, reusing approved components before creating replacements.
---
# Gameplay: una tarea, una prueba observable

Leer `AGENTS.md`, `docs/PLAN_ACTIVO.md` y la ficha de `docs/TAREAS_ACTIVAS.md`. Consultar únicamente la auditoría/skill de arte necesaria. Los planes históricos no reactivan red: **cooperativo, N0/N1, replicación y LAN están aplazados**.

1. Confirmar tarea, dependencia aprobada, acceso al entorno, cambios previos y rama. Especificar resultado, archivos permitidos, prueba y reversión. No modificar sistema, motor global u otros repositorios sin permiso.
2. Elegir un donante permitido y una alternativa nativa. Revisar licencia, edición, integridad de archivos/LFS y dependencias. Importar solo lo necesario; si implica reconstruir un framework entero, registrar bloqueo y volver a la alternativa.
3. Mantener un controlador de caminar/correr/saltar/volar. Separar locomoción, interacción, daño y presentación. Datos editables para tuning; no rehacer toda la arquitectura en un ticket.
4. Mantener IDs y propietario lógico por objeto para pruebas y guardado **offline**. No implementar autoridad de servidor, RPC ni predicción de red. Un impacto/parte no se aplica dos veces.
5. La carga transportada necesita comprobación de su volumen; la pared rota necesita colisión transitable; el teleport requiere destino válido/cargado; una pieza desprendida deshabilita su capacidad. Animación y VFX no son fuente de daño.
6. Compilar y ejecutar las pruebas de la ficha en Linux cuando haya entorno. Incluir regresión del bucle anterior y paquete fuera del editor para el cierre correspondiente. Registrar versiones, escenario, resultados y mediciones reales.
7. Solicitar valoración humana si cambia control/cámara/sensación. El mismo agente que implementa y prueba no debe describirse como revisor independiente. Proponer integrar solo tras aceptación; conservar versión válida anterior.

Estados honestos: PLANIFICADO, EN CURSO, IMPLEMENTADO SIN VALIDAR, COMPILADO, EJECUTADO, ACEPTADO o BLOQUEADO. Si falta editor/GPU/permisos, indicar la prueba pendiente y no declararla superada. Una captura del donante no valida nuestra implementación. Este procedimiento no activa herramientas ni permisos nuevos.
