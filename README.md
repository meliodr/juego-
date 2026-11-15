# IMPACT: BEYOND

> **Regla de desarrollo: reutilizar lo más que se pueda y crear lo mínimo que se pueda.** Primero buscar y verificar herramientas, modelos, animaciones y código existentes; configurar y adaptar antes de desarrollar desde cero. Crear solo lo indispensable para una experiencia estable y divertida. Detalles obligatorios en [AGENTS.md](AGENTS.md).

## Auditoría Fab — primer lote 2026-11-15

**[Catálogo de Fab: 25 fichas gratuitas revisadas, 3 promociones temporales y 4 pendientes](docs/FAB_AUDITORIA_LOTE01_2026-11-15.md)**. Primera selección: Scout Bot modular, animaciones de caída/planeo, Niagara Examples, efectos de energía, muestra de Chaos y ciudad/vehículos de Epic, sujetos a licencia/versión. Las promociones de Fab anuncian fin el **17 de noviembre a las 9:59 a. m. ET**. NO hemos agregado activos a la cuenta, descargado paquetes ni inspeccionado Mi biblioteca. Se mantiene la regla **reutilizar al máximo y crear lo mínimo**, sin retrasar W00/W01.

## Plugins seleccionados — v1.4

**[Ver selección y pruebas de plugins para UE5.8](docs/PLUGIN_SELECTION_V14.md)**. Se usarán herramientas incluidas (Enhanced Input, IK Rig/Control Rig, Niagara, Chaos/Fracture y PCG), activadas por fase. Unreal MCP + AllToolsets será un experimento local de automatización cuando editor y agente funcionen. Externos elegidos para pruebas, no instalaciones actuales: PCGEx MIT rama 5.8, Realtime Destructible Mesh Free (Fab Standard) y Realtime Mesh Component Core MIT como reserva. **Ninguno se ha instalado ni probado en el equipo del usuario.** Primera tarea sigue IB-W00/W01: Windows/Unreal 5.8.


Sandbox 3D de superhumanos para **Windows como plataforma principal y un jugador offline**. Caminar, correr, volar, combatir, agarrar y lanzar objetos/personajes, y destruir partes del entorno. Presupuesto de compras de assets/plugins: **RD$0**. Reutilización primero.

## Empezar aquí

**Actualización de plataforma (15 de noviembre de 2026):** [Inicio en Windows con Unreal Engine 5.8](docs/WINDOWS_INICIO_V12.md). La instalación del motor aparece en curso en la captura del usuario; Fab/Quixel Bridge siguen en cola. Ningún ejecutable o proyecto Unreal se ha validado todavía. Linux es posible destino futuro, no requisito de inicio.


- **[Plan activo de producción](docs/PLAN_ACTIVO.md)**: alcance, arquitectura, selección de donantes, entregas, roles, riesgos y criterios de prueba.
- **[Cola activa de tareas](docs/TAREAS_ACTIVAS.md)**: primer encargo IB-L00 y fichas ordenadas hasta el sandbox jugable.
- **[Instrucciones para agentes](AGENTS.md)**: reglas actuales y uso selectivo de las siete skills de `.agents/skills/`.

Estos documentos sustituyen el orden de trabajo de los planes versionados anteriores. La investigación previa permanece en `docs/`, pero no hay que leer todas sus versiones ni ejecutar sus antiguas tareas de networking.

## Estado real — 14 de noviembre de 2026

El repositorio contiene planificación, auditorías y procedimientos; **todavía no contiene un proyecto Unreal jugable ni evidencia de compilación/ejecución en el equipo del usuario**. La revisión del árbol previa a esta consolidación fue sobre el commit `b060bc7cc6fe828405463e02bf7df1a637f10330`.

Hardware reportado: **64 GB RAM y RTX 4070 con 8 GB VRAM**. CPU, driver, DirectX 12, Visual Studio/MSVC/Windows SDK, almacenamiento y motor/toolchain pendientes de verificación local. No hay acceso a esa máquina confirmado en esta documentación.

**Próximo trabajo operativo: IB-W00 — confirmar instalación y herramientas de Unreal Engine 5.8 en Windows; después IB-W01 para crear un proyecto base.** Es una tarea preparada, no ejecutada ni enviada a Robert automáticamente. La planificación no autoriza instalaciones o cambios de sistema por sí sola.

## Primera experiencia que construiremos

Caminar → correr → saltar → despegar → acelerar/frenar → agarrar un carro → transportarlo → lanzarlo contra una fachada → atravesar el hueco → pelear y recuperar el vuelo → reiniciar y repetir.

Primero un héroe provisional, una manzana, un carro, paredes preparadas, un robot modular y un rival volador. No requiere terminar una campaña, personajes finales, población masiva o el sistema solar.

## Visión que conservamos

Tres héroes originales: H01 de fuerza; H02 genio cósmico masculino esmeralda según concepto aceptado; H03 teletransporte. Son IDs, no nombres definitivos. No usar los nombres de héroes rechazados ni recuperar el enemigo constructor genérico.

Robots de varias escalas, kaijus desarmables y rivales superhumanos; daño por piezas con consecuencias, sin gore explícito. Posteriormente: ciudadanos, amenaza/alerta/karma diferenciados, rescates, robo, meteorito e invasiones opcionales.

Ciudad vertical destructible por regiones en un planeta pequeño, salida a órbita, Luna y dos planetas con regiones visitables. Sol solo visual. **Cooperativo muy lejano, completamente fuera de las entregas actuales: sin LAN ni pruebas N0/N1.**

## Selección de evaluación, no dependencias ya aceptadas

| Sistema | Primera evaluación | Alternativa |
|---|---|---|
| Base y suelo | Plantilla Third Person/Combat disponible en el Unreal elegido | Plantilla simple nativa |
| Vuelo | Subconjunto de código de ManOfSteel y animaciones realmente disponibles/permitidas | CMC y pocas poses autorizadas |
| Ciudad/vehículos | Un kit Quaternius/Kenney con licencia concreta comprobada | Proxies mientras se valida |
| Agarre | Componente mínimo de REPIRK | Interacción nativa asistida |
| Rotura/soporte | Chaos y comparación acotada con PlotBuilder | Paneles/colapso preparado |
| Espacio | Ephemera aislado, Cosmic Architect como alternativa | Esfera visible y regiones locales |

Cada donante exige verificar revisión, archivos completos, permisos, compilación y prueba en Windows. Nada se considera funcional por un README, una demo ajena o la presencia de una licencia. No importar frameworks completos por acumulación.

## Investigación por tema

[Vuelo y LFS](docs/FLIGHT_REUSE_AUDIT_V09.md) · [Caminar/correr y creadores](docs/GROUND_FLIGHT_CREATORS_V10.md) · [Ciudad, derrumbes y planetas](docs/CITY_DESTRUCTION_PLANETS_AUDIT_V11.md) · [Modelos y animación](docs/ANIMATION_CHARACTER_PIPELINE_V07.md) · [Sensación y optimización](docs/ANIMATION_FEEL_OPTIMIZATION_V08.md) · [Personajes y enemigos](docs/COMBAT_ROSTER_V03.md) · [Ciudadanos/alertas/karma](docs/CIVILIANS_ALERTS_KARMA_V06.md) · [Eventos](docs/REFERENCES_DYNAMIC_EVENTS_V07.md).

Los documentos temáticos conservan hipótesis y afirmaciones de sus fuentes en las fechas de auditoría; confirmar la versión y licencia exactas antes de incorporar archivos. El plan actual gobierna prioridades y aceptación. El historial de Git conserva las versiones anteriores de README/AGENTS.
