# IMPACT: BEYOND

Sandbox 3D de superhumanos para **Windows como plataforma principal y un jugador offline**: caminar, correr, volar, combatir, agarrar y lanzar objetos, y destruir partes preparadas del entorno. Presupuesto de compras de assets/plugins: **RD$0**. Reutilización primero.

## Estado actual — 16 de noviembre de 2026

**La integración técnica S1–S7 está publicada en [PR33](https://github.com/meliodr/juego-/pull/33), pero el usuario rechazó su presentación con 2/10. No está aceptada como juego.** La prioridad es [corregir animación, cámara, sensación de velocidad, efectos y escenario](docs/IB_S7_PRESENTATION_REWORK.md), conservando las mecánicas y velocidades. S8 permanece aplazado.

El repositorio contiene el [proyecto Unreal](ImpactBeyond/ImpactBeyond.uproject), fuentes, recetas y pruebas. La entrega anterior, `s7-package-20261116T060244Z`, aprobó build, pruebas funcionales y perfil DX12/audio y arrancó desde Steam; sus [resultados y límites](docs/IB_S7_CLOSURE_REVIEW.md) son evidencia histórica de esa revisión. No certifican la candidata de presentación en curso ni sustituyen la valoración del usuario.

La copia pública no suministra los packs Fab ni sus derivados privados. [Reproducir la partida](docs/IB_S7_SOURCE_OVERVIEW.md) requiere los recursos autorizados y generar el mapa local; compilar C++ por sí solo no produce una entrega completa.

## Empezar aquí

- **[Plan activo de producción](docs/PLAN_ACTIVO.md)**: alcance vigente, contratos y criterios de cierre.
- **[Cola activa de tareas](docs/TAREAS_ACTIVAS.md)**: correcciones y siguiente condición verificable.
- **[Corrección de presentación](docs/IB_S7_PRESENTATION_REWORK.md)**: causas identificadas, reutilización y aceptación pendiente.
- **[Fuentes y reproducción](docs/IB_S7_SOURCE_OVERVIEW.md)**: componentes, dependencias locales, pruebas y arranque.
- **[Instrucciones para agentes](AGENTS.md)**: reglas y uso selectivo de las skills de `.agents/skills/`.

El plan y la cola vigentes sustituyen el orden de los documentos históricos. Las instrucciones de instalación inicial de [Windows/Unreal](docs/WINDOWS_INICIO_V12.md) son antecedentes; no ordenan rehacer W00/W01. Linux y cooperativo no forman parte de la entrega actual.

## Fab: selección y biblioteca — 15/11/2026

**[Selección activa de Fab](docs/FAB_SELECCION_ACTIVA.md)** · **[Catálogo CSV para Codex, 52 fichas públicas](docs/FAB_CATALOGO_MAESTRO.csv)** · [Lote 01](docs/FAB_AUDITORIA_LOTE01_2026-11-15.md) · [Lote 02](docs/FAB_AUDITORIA_LOTE02_2026-11-15.md).

La criba del catálogo del 15/11 registró 8 SELECCIONAR, 5 PROBAR, 27 RESERVAR y 12 DESCARTAR. Es una selección de fichas públicas, no un inventario actual de **Mi biblioteca**. La instalación y el uso local confirmados de Adventure Characters, Robot 13 y Bull están en [procedencia del arte S7](docs/IB_S7_ASSET_PROVENANCE.md); no generalizar esa evidencia a otros paquetes. No entrenar IA ni proporcionar bytes NoAI al modelo. La revisión periódica prevista desde el 18/11 no demuestra revisiones futuras, compras ni descargas.


> **Regla de desarrollo: reutilizar lo más que se pueda y crear lo mínimo que se pueda.** Primero buscar y verificar herramientas, modelos, animaciones y código existentes; configurar y adaptar antes de desarrollar desde cero. Crear solo lo indispensable para una experiencia estable y divertida. Detalles obligatorios en [AGENTS.md](AGENTS.md).

## Auditoría Fab — primer lote 2026-11-15

**[Catálogo de Fab: 25 fichas gratuitas revisadas, 3 promociones temporales y 4 pendientes](docs/FAB_AUDITORIA_LOTE01_2026-11-15.md)**. Conserva la primera selección y sus precios/fechas históricos, sujetos a licencia y versión. Consultarlo por una necesidad concreta; una ficha no acredita adquisición, instalación ni funcionamiento.

## Plugins seleccionados — v1.4

**[Ver selección y pruebas de plugins para UE5.8](docs/PLUGIN_SELECTION_V14.md)**. El documento propone herramientas incluidas y evaluaciones por fase; no obliga a instalarlas todas. PCGEx, Realtime Destructible Mesh y Realtime Mesh Component Core son candidatos de investigación, no nuevas dependencias autorizadas por esta corrección. No añadir plugins ni bibliotecas por anticipación.


## Bucle que estamos corrigiendo

Caminar → correr → saltar → despegar → acelerar/frenar → agarrar un carro → transportarlo → lanzarlo contra una fachada → atravesar el hueco → pelear y recuperar el vuelo → reiniciar y repetir.

La integración técnica incluye tres héroes, cuatro regiones urbanas, objetivos preparados, civiles, robot/kaiju y rueda de poderes. La corrección actual busca que movimiento, presentación y entorno acompañen ese bucle; no añade campaña, población masiva ni sistema solar.

## Visión que conservamos

Tres héroes originales: H01 de fuerza; H02 genio cósmico masculino esmeralda según concepto aceptado; H03 teletransporte. Son IDs, no nombres definitivos. No usar los nombres de héroes rechazados ni recuperar el enemigo constructor genérico.

Robots de varias escalas, kaijus desarmables y rivales superhumanos; daño por piezas con consecuencias, sin gore explícito. S1–S7 ya integra civiles, un incidente opcional y consecuencias diferenciadas; la variedad futura de rescates, robos, meteoritos e invasiones sigue sujeta al plan.

La visión futura conserva ciudad vertical, salida a órbita, Luna y dos planetas con regiones visitables; no se anuncian entregados en S7. Sol solo visual. **Cooperativo muy lejano, completamente fuera de las entregas actuales: sin LAN ni pruebas N0/N1.**

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
