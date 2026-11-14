# IMPACT: BEYOND

Sandbox 3D de vuelo superhumano, exploración, agarres y destrucción. Investigación y planificación de reutilización gratuita para Unreal Engine.

## Objetivo

Conseguir primero esta secuencia jugable: despegar, acelerar entre edificios, agarrar un carro, lanzarlo contra una pared, atravesar el hueco y continuar la pelea. La visión posterior incluye dos personajes originales —fuerza física y construcciones de energía— y un planeta visualmente continuo con regiones detalladas y salida al espacio.

**Presupuesto de compras de assets y plugins: RD$0.** No implica hardware gratuito ni consumo ilimitado de servicios de IA. Prioridad: un sandbox divertido y pequeño antes de ampliar el mundo.

## Estado real — 14 de noviembre de 2026

Este repositorio se encontró vacío y se inicializa con documentación. No contiene todavía un proyecto Unreal, un ejecutable ni paquetes de terceros. No se han compilado o ejecutado los candidatos de esta investigación en Unreal.

Se catalogan **28 candidatos y 3 índices de descubrimiento**, con distintos niveles de evidencia: páginas de autor, licencias, estructura del repositorio y, para componentes seleccionados, código inspeccionado. Un candidato no equivale a una dependencia aprobada o probada.

## Documentos

- [Catálogo de proyectos y recursos](docs/REUSE_CATALOG.md): fuentes, licencias observadas, utilidad, obstáculos y decisión por candidato.
- [Plan de experimentos e integración](docs/REUSE_PLAN.md): qué probar, qué conservar, qué descartar y criterios verificables.
- [Reglas para agentes de programación](AGENTS.md): reutilización responsable, presupuesto y evidencias requeridas.

## Selección de trabajo

| Necesidad | Primera opción de evaluación | Límite importante |
|---|---|---|
| Agarrar y lanzar objetos | Extraer el componente PhysicsGrabber de REPIRK/Modular-Mechanics-Toolkit-UE5 | El repositorio inspeccionado contiene un módulo C++; no un juego completo listo para importar. |
| Destrucción jugable | Chaos nativo; comparar una pared con Realtime Destructible Mesh | El plugin figura gratis, pero licencia de adquisición, versión y rendimiento deben validarse antes de integrarlo. |
| Personajes y animación | Paquetes Standard gratuitos de Quaternius; maniquí provisional como respaldo | Los paquetes completos/Source y todas las animaciones anunciadas no son necesariamente gratuitos. |
| Separación corporal estilizada | Thyke/Dismemberment, después de estabilizar agarres y combate | Requiere preparar mallas, huesos y piezas; no funciona mágicamente con cualquier modelo. |
| Exploración planetaria | Estudiar 0xWerz/ephemera en un experimento separado | Es un sandbox ambiental, no una base de combate superheroico. |

Las fuentes y sus condiciones están enlazadas en el catálogo. Estas son decisiones de evaluación, no confirmaciones de compatibilidad.

## Principios

1. Reutilizar un componente pequeño antes que fusionar varios juegos completos.
2. Una sola implementación principal por sistema: movimiento, agarres y destrucción jugable.
3. Conservar licencias y avisos al incorporar código. Revisar por separado los recursos artísticos.
4. No copiar repositorios sin licencia ni redistribuir paquetes de Fab en este repositorio público sin permiso comprobado.
5. No sustituir pruebas por afirmaciones de README, capturas promocionales o porcentajes inventados.

La primera entrega técnica pendiente es un proyecto base que compile, se empaquete y permita probar vuelo + objeto agarrable + pared rompible.
