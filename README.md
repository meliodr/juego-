# IMPACT: BEYOND

Sandbox 3D de vuelo superhumano, exploración, agarres y destrucción. Desarrollo previsto en Unreal Engine para **Linux nativo**, mediante reutilización gratuita. Primera experiencia individual; objetivo posterior de **cooperativo para dos jugadores**, con pruebas técnicas de red tempranas.

## Objetivo

Conseguir primero esta secuencia jugable: despegar, acelerar entre edificios, agarrar un carro, lanzarlo contra una pared, atravesar el hueco y continuar la pelea. La visión incluye **tres personajes originales** —combatiente de fuerza H01, genio cósmico esmeralda H02 y desplazador espacial H03—, una ciudad vertical densamente explorable y parcialmente destruible sobre un planeta pequeño, y un sistema solar compacto con Sol visible (no visitable), Luna y otros dos planetas visitables por regiones.

**Hardware declarado para planificar: 64 GB RAM, NVIDIA RTX 4070 de 8 GB VRAM; CPU, distro Linux, SSD y driver por verificar en L00.**

**Presupuesto de compras de assets y plugins: RD$0.** No implica hardware gratuito ni consumo ilimitado de servicios de IA. Prioridad: un sandbox divertido y pequeño antes de ampliar el mundo. El multijugador futuro no sustituye el modo individual sin conexión.

## Estado real — 14 de noviembre de 2026

**V0.7 sigue siendo planificación documentada: no hay proyecto Unreal compilado ni librerías importadas a Linux; no supone autorización automática para instalar software.** Las cinco skills y los nuevos documentos son el resultado actual. Sistema de alerta/civiles/karma y rendimiento están especificados con parámetros iniciales ajustables, no implementados.

El repositorio contiene documentación de investigación y diseño. No contiene todavía un proyecto Unreal, un ejecutable ni paquetes de terceros. No se han compilado o ejecutado los candidatos en Unreal ni se ha inspeccionado el equipo Linux del usuario.

El catálogo original contiene **28 candidatos y 3 índices**; la auditoría v0.5 añade nuevas opciones comprobadas documentalmente (robots animados, mechas, monstruos, PCG, etc.). No son sistemas instalados. Se trabaja con distintos niveles de evidencia: páginas de autor, licencias, estructura del repositorio y, para componentes seleccionados, código inspeccionado. Un candidato no equivale a una dependencia aprobada o probada. La actualización Linux añade filtros de portabilidad; no certifica los candidatos. Las especificaciones de arte, personajes, movimientos, enemigos y red son propuestas, no implementaciones.

## Documentos

**Nuevos recursos de diseño (v0.7):** bibliotecas de animación reutilizables, fabricación 3D con Blender y sucesos aleatorios documentados desde referencias Roblox/Minecraft.

- [Pipeline de animación y personajes v0.7](docs/ANIMATION_CHARACTER_PIPELINE_V07.md): bases humanas, robots/kaijus, Rokoko/Quaternius/GASP/Mesh2Motion/MakeHuman, rigging y QA.
- [Referencias y eventos dinámicos v0.7](docs/REFERENCES_DYNAMIC_EVENTS_V07.md): análisis Project Power/Viltrumite Flight/Fisk, meteorito, robo, invasión, robots y director por incidentes.
- [Skills Codex de animación y eventos](.agents/skills/): nuevos impact-character-animation e impact-world-events; procedimientos, no agentes ejecutados.


**Última dirección operativa (v0.6):** diseño de civiles/alertas/karma, contrato de arquitectura/roles y backlog que condiciona el inicio a L00 Linux.

- [Contrato de producción y arquitectura v0.6](docs/PRODUCTION_CONTRACT_V06.md): requisitos FR/NFR, RACI, subsistemas, riesgos, perfiles, rutas críticas, DoR/DoD, herramientas.
- [Ciudadanos, alertas y karma v0.6](docs/CIVILIANS_ALERTS_KARMA_V06.md): testigos, tres tiers de ciudadanía, incidentes, 0–5 de alerta, karma −100..100, modo Creativo/Ciudad Viva/Desafío, co-op.
- [Backlog de ejecución v0.6](docs/EXECUTION_BACKLOG_V06.md): propietarios de tareas y gates, tickets IB-L00/IB-F01/IB-U00, aceptación y checklist previo al comienzo.
- [Cinco skills de Codex](.agents/skills/): auditoría de licencias, gate Linux, implementación jugable, sistemas cívicos, QA. Son instrucciones, no tareas ya ejecutadas.


- [PLAN MAESTRO v0.5](docs/MASTER_PLAN_V05.md): fuente principal para alcance, fases, límites del motor y de la IA, arquitectura, gates y decisiones de descarte.
- [Auditoría de recursos gratuitos v0.5](docs/FREE_RESOURCE_AUDIT_V05.md): nuevas opciones concretas para robots, mechas, kaijus, ciudad/espacio, licencias actualizadas y pruebas previas a importar.
- [Playbook Codex/Linux/QA v0.5](docs/CODEX_QA_PLAYBOOK_V05.md): qué puede automatizarse, qué no, MCP experimental, compilación nativa, test y evidencias.


- [Ciudad destructible, sistema solar y teletransporte v0.4](docs/CITY_SOLAR_TELEPORT_V04.md): actualización vigente de ciudades densas, cuatro cuerpos jugables/visibles, tercer héroe, recursos gratuitos concretos, orden de ensamblaje y pruebas. Prevalece para alcance de ciudad/espacio/teletransporte.

- [Personajes, enemigos y repertorios v0.3](docs/COMBAT_ROSTER_V03.md): dirección vigente de los dos héroes, ataques contextuales, diez conceptos de enemigos, desarme funcional y alcance escalonado. Prevalece sobre nombres/diseños anteriores.
- [Diseño inicial del sandbox](docs/SANDBOX_DESIGN.md): distrito, modos, interacciones y criterios de diversión. Su identidad y catálogo de personajes han sido sustituidos por COMBAT_ROSTER_V03.
- [Plan cooperativo de dos jugadores](docs/COOP_PLAN.md): alcance LAN/online propuesto, preparación temprana, agarres exclusivos, destrucción compartida, fases y pruebas. Los nombres antiguos de héroes son referencias históricas; aplicar los identificadores actuales H01/H02.
- [Especificación de jugabilidad](docs/GAMEPLAY_SPEC.md): vuelo, controles, agarres, combate, destrucción útil, exploración y pruebas iniciales.
- [Base Linux](docs/LINUX_BASELINE.md): editor/paquete nativos, Vulkan, auditoría de plugins, automatización y fuentes técnicas.
- [Catálogo de proyectos y recursos](docs/REUSE_CATALOG.md): fuentes, licencias observadas, utilidad, obstáculos y decisión por candidato.
- [Plan de experimentos e integración](docs/REUSE_PLAN.md): evaluación técnica de componentes.
- [Reglas para agentes de programación](AGENTS.md): reutilización responsable, presupuesto y evidencias requeridas.

Precedencia: ANIMATION_CHARACTER_PIPELINE_V07 para bibliotecas, creación de personajes y animaciones; REFERENCES_DYNAMIC_EVENTS_V07 para referencias y eventos aleatorios; PRODUCTION_CONTRACT_V06 para roles/arquitectura/requisitos, EXECUTION_BACKLOG_V06 para gate concreto, CIVILIANS_ALERTS_KARMA_V06 para ciudad/alerta/karma; MASTER_PLAN_V05 para fases históricas/puertas y riesgo; FREE_RESOURCE_AUDIT_V05 para selección reciente de recursos/licencias; CODEX_QA_PLAYBOOK_V05 para prácticas de agente; LINUX_BASELINE para plataforma; GAMEPLAY_SPEC para controles y núcleo individual; COMBAT_ROSTER_V03 para H01/H02, enemigos y movimientos anteriores; CITY_SOLAR_TELEPORT_V04 para ciudad, planetas y nuevo H03; SANDBOX_DESIGN para modos no sustituidos; COOP_PLAN para pausas online, autoridad y puertas de red. Las exclusiones antiguas de multijugador describen la primera entrega, no una arquitectura permanentemente individual. El catálogo no certifica compatibilidad.

## Personajes y enemigos

H01: combatiente superhumano de fuerza, nombre pendiente. Conservar la dirección visual de traje grafito con placas claras y acentos rojos; vuelo, embestida, agarre, arrancado, lanzamiento y armas improvisadas.

H02: genio cósmico masculino, nombre pendiente. Conservar el concepto aceptado de traje oscuro con plata/oro y energía esmeralda; manos espectrales, agarre, escudo, martillo y manifestaciones avanzadas. Sin fruta del diablo ni traje/iconos de una franquicia. No sustituirlo por la constructora genérica de la primera propuesta.

H03: desplazador espacial, nombre pendiente; estética índigo/azul-violeta original, teletransporte de corto alcance, contra-blink, golpe de reaparición y marcador de retorno posterior. Puede explorar y volar; no copiar aspecto de Nightcrawler. Su teleport exige destino libre y zona cargada.

Los dos nombres geométricos propuestos anteriormente fueron rechazados y no deben reutilizarse. Se retira el arquetipo de enemigo constructor/energético genérico, no los poderes de H02.

Tres familias de enemigos: robots de diferentes escalas, kaijus y superhumanos. Robots/kaijus pierden piezas que modifican ataques, movilidad y objetos disponibles. Diez conceptos documentados no equivalen a diez enemigos encargados para la primera entrega.

## Jugabilidad y entregas

**Linux y vuelo -> distrito urbano denso -> carro + pared + robot modular -> rival y sandbox -> genio cósmico/kaiju -> teletransporte seguro H03 -> cooperativo regional -> órbita/Luna/dos planetas.**

Las pruebas N0/N1 de dos conexiones, movimiento y agarre/destrucción empiezan durante las primeras fases; no esperar a terminar todo el contenido para investigar red. No hace falta un lobby online completo para entregar el primer sandbox individual.

Vuelo sin combustible, frenado explícito, interacciones compartidas y destrucción que abra rutas reales. Los tres héroes deben ser autosuficientes y poder repetirse en cooperativo (una vez implementados). Las manifestaciones colosales, grandes jefes y vuelo planetario no bloquean el primer bucle divertido.

Primer robot propuesto: Carcelero con pocas piezas desprendibles. Malek, nombre provisional, representa el primer duelo aéreo; Rhazkar, también provisional, es el primer laboratorio kaiju con cuerno, placa y cola preparados. Las variantes se incorporan solo tras validar el bucle. Escenario inicial por sectores: avenida, puerto y cantera; no tres mundos nuevos simultáneos.

La entrega debe funcionar desde un ejecutable Linux fuera del editor. Las pruebas de control y diversión se complementan con mediciones; ni compilar ni leer un README demuestra una mecánica jugable.

## Selección de trabajo

| Necesidad | Primera opción de evaluación | Límite importante |
|---|---|---|
| Agarrar y lanzar objetos | Extraer PhysicsGrabber de REPIRK/Modular-Mechanics-Toolkit-UE5 | Módulo C++ orientado originalmente a Win64; portabilidad y paquete Linux pendientes. Probar autoridad y dos solicitudes sobre un mismo objeto. Respaldo: interacción nativa mínima. |
| Destrucción jugable | Chaos nativo con módulos de colisión; R03 queda como comparación condicionada | Confirmar Linux, licencia/acceso, rendimiento y reconstrucción del estado en red. No asumir física determinista idéntica entre clientes. |
| Personajes y animación | Standard gratuito de Quaternius; maniquí provisional como respaldo | Confirmar archivos realmente incluidos, formatos, licencias y retargeting. |
| Separación de piezas | Partes rígidas preparadas primero; Thyke/Dismemberment como candidato posterior | Preparar mallas, huesos y piezas; compilar/probar Linux. El desarme del primer robot no requiere corte arbitrario ni replicar partículas. |
| Ciudad densa | Quaternius Downtown City MegaKit Standard (solo parte gratuita CC0), PCG nativo, Kenney Roads/Industrial/Car Kit | No asumir que el Source de pago, shaders ni colisiones especializadas estén incluidos; primer distrito, no GTA/NY. |
| Teletransporte | Teleport/TeleportTo + comprobación de colisión de Unreal | H03 requiere comprobación de destino y precarga; pruebas Linux/red futuras. |
| Espacio y destinos | Ephemera (MIT, referencia), Cosmic Architect (MIT, alternativo), Quaternius Ultimate Space y Kenney Space Kits CC0 | Las superficies jugables se implementan por regiones, Sol solo visual, compatibilidad Linux pendiente. |
| Exploración planetaria | Estudiar 0xWerz/ephemera en un laboratorio separado | Es un sandbox ambiental. Algunas integraciones están limitadas a Win64; núcleo Linux no probado. Multijugador orbital es otra validación. |

Las fuentes y sus condiciones están enlazadas en el catálogo, la base Linux y el plan cooperativo. Estas son decisiones de evaluación, no confirmaciones de compatibilidad.

## Principios

1. Reutilizar un componente pequeño antes que fusionar varios juegos completos.
2. Una sola implementación principal por sistema: movimiento, agarres y destrucción jugable.
3. Conservar licencias y avisos al incorporar código. Revisar por separado recursos artísticos.
4. No copiar repositorios sin licencia ni redistribuir paquetes de Fab en este repositorio público sin permiso comprobado.
5. No sustituir pruebas por afirmaciones de README, capturas promocionales o porcentajes inventados.
6. Priorizar respuesta, claridad y estabilidad del vuelo sobre densidad visual o tamaño del mapa.
7. Separar estado jugable, simulación y presentación; preparar dos jugadores sin convertir el primer hito en infraestructura online completa.
8. Una pieza separada debe cambiar capacidades y no duplicarse; no atribuir daño a un brazo/arma que ya no existe.

La primera tarea técnica pendiente es inspeccionar el entorno real y conseguir una base Linux que compile, se empaquete y permita ensayar el circuito de vuelo. Después se conecta el bucle físico. El plan no representa trabajo ejecutado en el equipo del usuario.
