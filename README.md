# IMPACT: BEYOND

Sandbox 3D de vuelo superhumano, exploración, agarres y destrucción. Desarrollo previsto en Unreal Engine para **Linux nativo**, mediante reutilización gratuita.

## Objetivo

Conseguir primero esta secuencia jugable: despegar, acelerar entre edificios, agarrar un carro, lanzarlo contra una pared, atravesar el hueco y continuar la pelea. La visión posterior incluye dos personajes originales —fuerza física y construcciones de energía— y un planeta visualmente continuo con regiones detalladas y salida al espacio.

**Presupuesto de compras de assets y plugins: RD$0.** No implica hardware gratuito ni consumo ilimitado de servicios de IA. Prioridad: un sandbox divertido y pequeño antes de ampliar el mundo.

## Estado real — 14 de noviembre de 2026

El repositorio contiene documentación de investigación y diseño. No contiene todavía un proyecto Unreal, un ejecutable ni paquetes de terceros. No se han compilado o ejecutado los candidatos en Unreal ni se ha inspeccionado el equipo Linux del usuario.

Se catalogan **28 candidatos y 3 índices de descubrimiento**, con distintos niveles de evidencia: páginas de autor, licencias, estructura del repositorio y, para componentes seleccionados, código inspeccionado. Un candidato no equivale a una dependencia aprobada o probada. La actualización Linux añade filtros de portabilidad; no certifica los candidatos.

## Documentos

- [Especificación de jugabilidad](docs/GAMEPLAY_SPEC.md): vuelo, controles, agarres, combate, destrucción útil, personajes, exploración y pruebas.
- [Base Linux](docs/LINUX_BASELINE.md): editor/paquete nativos, Vulkan, auditoría de plugins, automatización y fuentes técnicas.
- [Catálogo de proyectos y recursos](docs/REUSE_CATALOG.md): fuentes, licencias observadas, utilidad, obstáculos y decisión por candidato.
- [Plan de experimentos e integración](docs/REUSE_PLAN.md): evaluación técnica de componentes; para orden de entregas y plataforma prevalecen los dos documentos nuevos.
- [Reglas para agentes de programación](AGENTS.md): reutilización responsable, presupuesto y evidencias requeridas.

## Jugabilidad y entregas

**Linux y vuelo -> carro + pared -> rival y sandbox -> forjador -> expansión orbital.**

Vuelo sin combustible, frenado explícito, un mismo sistema para agarrar carros/fragmentos/combatientes y destrucción que abra rutas reales. La primera versión prioriza el personaje de fuerza; las cuatro construcciones del forjador compartirán daño e interacción. No se requiere IA en tiempo de ejecución ni generación arbitraria de modelos.

La entrega debe funcionar desde un ejecutable Linux fuera del editor. Las pruebas de control y diversión se complementan con mediciones; ni compilar ni leer un README demuestra una mecánica jugable.

## Selección de trabajo

| Necesidad | Primera opción de evaluación | Límite importante |
|---|---|---|
| Agarrar y lanzar objetos | Extraer PhysicsGrabber de REPIRK/Modular-Mechanics-Toolkit-UE5 | Módulo C++ orientado originalmente a Win64; portabilidad y paquete Linux pendientes. Respaldo: interacción nativa mínima. |
| Destrucción jugable | Chaos nativo; R03 Realtime Destructible Mesh queda como comparación condicionada | Confirmar Linux, licencia/acceso y rendimiento del plugin antes de integrarlo. |
| Personajes y animación | Paquetes Standard gratuitos de Quaternius; maniquí provisional como respaldo | Confirmar archivos realmente incluidos, formatos y retargeting. |
| Separación corporal estilizada | Thyke/Dismemberment, después de estabilizar agarres y combate | Requiere preparar mallas, huesos y piezas; compilar/probar en Linux. No bloquea el sandbox. |
| Exploración planetaria | Estudiar 0xWerz/ephemera en un laboratorio separado | Es un sandbox ambiental. Algunas integraciones del descriptor están limitadas a Win64; núcleo Linux no probado. |

Las fuentes y sus condiciones están enlazadas en el catálogo y en la base Linux. Estas son decisiones de evaluación, no confirmaciones de compatibilidad.

## Principios

1. Reutilizar un componente pequeño antes que fusionar varios juegos completos.
2. Una sola implementación principal por sistema: movimiento, agarres y destrucción jugable.
3. Conservar licencias y avisos al incorporar código. Revisar por separado los recursos artísticos.
4. No copiar repositorios sin licencia ni redistribuir paquetes de Fab en este repositorio público sin permiso comprobado.
5. No sustituir pruebas por afirmaciones de README, capturas promocionales o porcentajes inventados.
6. Priorizar respuesta, claridad y estabilidad del vuelo sobre densidad visual o tamaño del mapa.

La primera tarea técnica pendiente es inspeccionar el entorno real y conseguir una base Linux que compile, se empaquete y permita ensayar el circuito de vuelo. Después se conecta el bucle físico de carro y pared.
