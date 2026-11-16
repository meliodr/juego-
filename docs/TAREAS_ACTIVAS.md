# IMPACT: BEYOND — Cola activa

Actualización: 16 de noviembre de 2026. Rige [PLAN_ACTIVO.md](PLAN_ACTIVO.md). **S7 permanece en integración/validación; esta cola no certifica una build entregada.** Las fichas W00/W01/GND/F01 y laboratorios se conservan en Git y sus informes; no deben reiniciarse por estados históricos.

| Orden | Trabajo | Condición de salida |
|---|---|---|
| 1 | Congelar candidata local S1–S7 y cerrar correcciones | Commit exacto, árbol limpio y fallos conocidos registrados |
| 2 | Compilar, automatizar Unreal, generar arte y empaquetar con un único propietario | Evidencia por gate del mismo commit; correcciones y regresiones pertinentes |
| 3 | Exportar fuentes públicas desde la candidata congelada | Lista exacta/procedencia; ningún Content binario nuevo, historial privado ni ruta personal |
| 4 | Ejecutar CI nativa/estática y revisar el PR | Suites requeridas presentes/aprobadas; requisitos privados y límites documentados |
| 5 | Verificar paquete DX12/audio, controles y rendimiento | Ruta continua, acciones y persistencia; input simulado separado de mando físico |
| 6 | Entregar versión identificada y reconciliar PRs | Cierre preciso de labs cubiertos/sustituidos, investigación preservada y valoración humana |
| 7 | Planificar S8 sobre resultados observados | Pausa completa y comienzo del espacio conservados |

Preparación pública y validación local pueden avanzar en paralelo. Copiar Source requiere el SHA final confirmado; el árbol mutable no es una entrega. Un auxiliar no abre otro editor/cook.

Formato de entrega: qué cambió/por qué, reutilización/procedencia, commit, pruebas ejecutadas, limitaciones y siguiente decisión. IMPLEMENTADO, COMPILADO, EJECUTADO y ACEPTADO describen evidencias distintas.
