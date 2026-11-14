# IMPACT: BEYOND

Sandbox 3D de vuelo superhumano, exploración, agarres y destrucción. Desarrollo previsto en Unreal Engine para **Linux nativo**, mediante reutilización gratuita. Primera experiencia individual; objetivo posterior de **cooperativo para dos jugadores**, con pruebas técnicas de red tempranas.

## Objetivo

Conseguir primero esta secuencia jugable: despegar, acelerar entre edificios, agarrar un carro, lanzarlo contra una pared, atravesar el hueco y continuar la pelea. La visión incluye dos personajes originales —fuerza física y construcciones de energía— y un planeta visualmente continuo con regiones detalladas y salida al espacio.

**Presupuesto de compras de assets y plugins: RD$0.** No implica hardware gratuito ni consumo ilimitado de servicios de IA. Prioridad: un sandbox divertido y pequeño antes de ampliar el mundo. El multijugador futuro no sustituye el modo individual sin conexión.

## Estado real — 14 de noviembre de 2026

El repositorio contiene documentación de investigación y diseño. No contiene todavía un proyecto Unreal, un ejecutable ni paquetes de terceros. No se han compilado o ejecutado los candidatos en Unreal ni se ha inspeccionado el equipo Linux del usuario.

Se catalogan **28 candidatos y 3 índices de descubrimiento**, con distintos niveles de evidencia: páginas de autor, licencias, estructura del repositorio y, para componentes seleccionados, código inspeccionado. Un candidato no equivale a una dependencia aprobada o probada. La actualización Linux añade filtros de portabilidad; no certifica los candidatos. Las nuevas especificaciones de arte, enemigos y red son propuestas, no implementaciones.

## Documentos

- [Diseño del sandbox](docs/SANDBOX_DESIGN.md): identidad visual, VECTOR y PRISMA, enemigos, distrito, modos, interacciones y criterios de diversión. Nombres provisionales.
- [Plan cooperativo de dos jugadores](docs/COOP_PLAN.md): alcance LAN/online propuesto, preparación temprana, agarres exclusivos, destrucción compartida, fases y pruebas.
- [Especificación de jugabilidad](docs/GAMEPLAY_SPEC.md): vuelo, controles, agarres, combate, destrucción útil, exploración y pruebas iniciales.
- [Base Linux](docs/LINUX_BASELINE.md): editor/paquete nativos, Vulkan, auditoría de plugins, automatización y fuentes técnicas.
- [Catálogo de proyectos y recursos](docs/REUSE_CATALOG.md): fuentes, licencias observadas, utilidad, obstáculos y decisión por candidato.
- [Plan de experimentos e integración](docs/REUSE_PLAN.md): evaluación técnica de componentes.
- [Reglas para agentes de programación](AGENTS.md): reutilización responsable, presupuesto y evidencias requeridas.

Precedencia: LINUX_BASELINE para plataforma; GAMEPLAY_SPEC para controles y núcleo individual; SANDBOX_DESIGN para identidad, personajes/enemigos y alcance creativo; COOP_PLAN para la nueva intención multijugador, pausas online, autoridad y puertas de red. Las exclusiones antiguas de multijugador describen la primera entrega, no una arquitectura permanentemente individual. El catálogo no certifica compatibilidad.

## Jugabilidad y entregas

**Linux y vuelo -> carro + pared -> rival y sandbox -> forjador y variedad -> cooperativo regional -> expansión orbital.**

Las pruebas N0/N1 de dos conexiones, movimiento y agarre/destrucción empiezan durante las primeras fases; no esperar a terminar todo el contenido para investigar red. No hace falta un lobby online completo para entregar el primer sandbox individual.

Vuelo sin combustible, frenado explícito, un mismo sistema para agarrar carros/fragmentos/combatientes y destrucción que abra rutas reales. VECTOR, nombre provisional, representa fuerza e impulso. PRISMA crea escudo, puño, martillo y pinza sobre las mismas reglas. Ambos deben ser autosuficientes; permitir personajes repetidos en cooperativo.

Enemigos prioritarios: rival volador, dron Avispa y variante pesada Ancla. Interdictor y Bastión quedan para después. Escenario inicial por sectores: avenida, puerto y cantera; no tres mundos nuevos a construir simultáneamente. Sin campaña obligatoria, generación de objetos con IA en runtime ni compras.

La entrega debe funcionar desde un ejecutable Linux fuera del editor. Las pruebas de control y diversión se complementan con mediciones; ni compilar ni leer un README demuestra una mecánica jugable.

## Selección de trabajo

| Necesidad | Primera opción de evaluación | Límite importante |
|---|---|---|
| Agarrar y lanzar objetos | Extraer PhysicsGrabber de REPIRK/Modular-Mechanics-Toolkit-UE5 | Módulo C++ orientado originalmente a Win64; portabilidad y paquete Linux pendientes. Probar autoridad y dos solicitudes sobre un mismo objeto. Respaldo: interacción nativa mínima. |
| Destrucción jugable | Chaos nativo con módulos de colisión; R03 queda como comparación condicionada | Confirmar Linux, licencia/acceso, rendimiento y reconstrucción del estado en red. No asumir física determinista idéntica entre clientes. |
| Personajes y animación | Standard gratuito de Quaternius; maniquí provisional como respaldo | Confirmar archivos realmente incluidos, formatos, licencias y retargeting. |
| Separación corporal estilizada | Thyke/Dismemberment, después de estabilizar agarres y combate | Preparar mallas, huesos y piezas; compilar/probar en Linux. No bloquea el sandbox ni exige replicar partículas. |
| Exploración planetaria | Estudiar 0xWerz/ephemera en un laboratorio separado | Es un sandbox ambiental. Algunas integraciones están limitadas a Win64; núcleo Linux no probado. Multijugador orbital es otra validación. |

Las fuentes y sus condiciones están enlazadas en el catálogo, la base Linux y el plan cooperativo. Estas son decisiones de evaluación, no confirmaciones de compatibilidad.

## Principios

1. Reutilizar un componente pequeño antes que fusionar varios juegos completos.
2. Una sola implementación principal por sistema: movimiento, agarres y destrucción jugable.
3. Conservar licencias y avisos al incorporar código. Revisar por separado los recursos artísticos.
4. No copiar repositorios sin licencia ni redistribuir paquetes de Fab en este repositorio público sin permiso comprobado.
5. No sustituir pruebas por afirmaciones de README, capturas promocionales o porcentajes inventados.
6. Priorizar respuesta, claridad y estabilidad del vuelo sobre densidad visual o tamaño del mapa.
7. Separar estado jugable, simulación y presentación; preparar dos jugadores sin convertir el primer hito en infraestructura online completa.

La primera tarea técnica pendiente es inspeccionar el entorno real y conseguir una base Linux que compile, se empaquete y permita ensayar el circuito de vuelo. Después se conecta el bucle físico de carro y pared. El plan no representa trabajo ejecutado en el equipo del usuario.
