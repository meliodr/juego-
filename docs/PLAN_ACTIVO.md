# IMPACT: BEYOND — Plan activo de producción

**Principio rector: «Reutilizar lo más que se pueda y crear lo mínimo que se pueda».** Cada ticket debe indicar el recurso existente elegido (motor, Fab, código o asset autorizado), el mínimo trabajo propio necesario y por qué no puede resolverse enteramente reutilizando. Reutilizar no exime de verificar permisos, compatibilidad, estabilidad y calidad. Ver [AGENTS.md](../AGENTS.md).

## Fab — reutilización por lotes (2026-11-15)

El [lote 01 de auditoría Fab](FAB_AUDITORIA_LOTE01_2026-11-15.md) identifica 25 fichas que muestran 'Free', 3 ofertas con fecha límite y 4 precios no confirmados. Es información pública: no presume biblioteca del usuario, ni descarga, instalación o prueba UE5.8. Antes de buscar una mecánica desde cero, mirar la lista, comprobar la ficha/NoAI/licencia y seleccionar UN candidato. Mantener W00/W01 y locomoción como primeros gates; no importar cientos de assets por anticipación.

## Plano urbano de aceptación H01 — decisión 2026-11-15

La especificación técnica de la primera manzana y la matriz de pruebas están en [IB_C00_MANZANA_PRUEBA.md](IB_C00_MANZANA_PRUEBA.md): recorrido en L con tramo de aceleración, freno, carro con colisión activa durante el agarre, fachada con hueco transitable y reset determinista. Es **diseño previo, no implementación ni prueba superada**. Se mantiene una única tarea operativa activa: IB-W00; después IB-W01 y la cola original. Geometría primitiva Unreal antes de acudir a Fab.

## Selección de plugins por fase, 2026-11-15

[PLUGIN_SELECTION_V14.md](PLUGIN_SELECTION_V14.md) gobierna qué plugins del motor y externos pueden evaluarse y cuándo. Enhanced Input desde el inicio; animación (IK/Control Rig), Niagara, Chaos/Fracture y PCG conforme aparezca cada necesidad. PCGEx 5.8 MIT y Realtime Destructible Mesh Free/Fab solo experimentos aislados, no dependencias instaladas. RMC Core MIT en reserva; MCP editor experimental y opcional. No alterar arranque IB-W00/IB-W01, presupuesto RD$0 o restricción de un jugador offline. Código y prueba local aún inexistentes.


## Actualización de plataforma 2026-11-15 — prioritaria

El usuario decidió **Windows como plataforma principal**, con Unreal Engine 5.8 instalándose en Epic Games Launcher según su captura. Fab UE Plugin y Quixel Bridge todavía estaban en cola en ese momento. **Ninguna instalación completada, primer arranque, compilación o juego han sido verificados.** Referencia operativa: [WINDOWS_INICIO_V12.md](WINDOWS_INICIO_V12.md). La nueva cola comienza en **IB-W00 -> IB-W01**, no IB-L00; las descripciones anteriores de build Linux/Vulkan quedan suspendidas hasta eventual port. Render inicial DX12 y juego Windows. Se mantiene objetivo un jugador offline, compra de assets RD$0 y cooperativo muy lejano. 


Actualización: 2026-11-14. Estado: **PLAN DE TRABAJO; NO IMPLEMENTACIÓN VALIDADA**.

Este es el único plan de entregas vigente. Sustituye el orden y los requisitos de cierre de los planes versionados anteriores, que se conservan como investigación. `AGENTS.md` establece reglas; `TAREAS_ACTIVAS.md` establece la cola. No hay que leer todas las versiones para comenzar una tarea.

## 1. Evidencia disponible y decisiones que no cambian

En la revisión del árbol de `main`, commit `b060bc7cc6fe828405463e02bf7df1a637f10330`, se encontraron documentación y siete skills, pero no un proyecto `.uproject`, código de juego ni un ejecutable. No se ha inspeccionado aquí el Windows del usuario ni se ha ejecutado Unreal. Las anteriores auditorías de donantes se consideran antecedentes, no certificaciones de compatibilidad.

Decisiones del usuario: sandbox de superhumanos, sin campaña obligatoria; Windows como plataforma principal; caminar y correr además de volar; velocidad, libertad, agarres, combate y destrucción como pilares; compras de assets/plugins RD$0. Equipo reportado: 64 GB RAM y RTX 4070 con 8 GB VRAM. CPU, modelo preciso de GPU, versión de Windows, driver/DirectX 12, almacenamiento, Visual Studio/MSVC/SDK y Unreal instalado se verificarán sin volver a pedir RAM/GPU.

Conservar H01 de fuerza, H02 genio cósmico masculino esmeralda del concepto aceptado y H03 de teletransporte. Son identificadores, no nombres artísticos. No recuperar VECTOR/PRISMA ni al enemigo constructor genérico. Robots y kaijus pierden piezas con consecuencias; presentación sin gore explícito. Ciudadanos, alerta, karma y eventos son parte del sandbox ampliado.

La visión espacial permanece: planeta principal pequeño y visualmente continuo, Luna y otros dos planetas con regiones visitables; Sol visible, no destino. **Cooperativo muy lejano: no LAN, replicación, pruebas N0/N1, backend ni sesiones en ninguna entrega actual.**

Lo que sigue son decisiones de implementación propuestas para ejecutar esa visión. Las cantidades y umbrales son criterios iniciales de ensayo, no capacidades medidas.

## 2. Primer resultado: un recorrido completo, no una colección de sistemas

> Caminar por una calle, correr, saltar, despegar, acelerar, frenar, agarrar un carro, volar con él, lanzarlo contra una fachada, atravesar el hueco, golpear a un rival y recuperar el vuelo. Reiniciar y repetir.

La primera entrega integrada tendrá un héroe provisional con identidad de fuerza, una manzana ampliable, un carro, paredes preparadas, un robot con pocas piezas desprendibles, un rival aéreo sencillo y controles de reinicio/generación de objetos. Primero se prueban los elementos individualmente; no se exige completar todo esto en el primer commit.

**No bloquearán esa entrega:** arte final, los otros dos héroes, ciudadanos, karma, eventos aleatorios, kaiju, población masiva, mapa enorme, interiores de cada edificio, órbita ni multijugador. No se cancelan: se incorporan después de comprobar que el núcleo merece ampliarse.

## 3. Orden único de entregas

| Entrega | Resultado observable | Condición para avanzar |
|---|---|---|
| E0 — Base reproducible | Proyecto mínimo Unreal que compila, se empaqueta y arranca en Windows fuera del editor | Versión/toolchain fijados, input/audio/colisión básicos y logs reales |
| E1 — Movimiento agradable | Un mismo personaje camina, corre, esprinta, salta, vuela, frena y aterriza; cámara y poses legibles | 30 ciclos suelo-aire-suelo; recorrido de 10 minutos; prueba humana de control |
| E2 — Bucle físico | Carro, golpe, lanzamiento, muro perforable y escombro importante reutilizable | 30 ciclos de agarre, 50 impactos y 20 reinicios; hueco realmente transitable |
| E3 — Sandbox urbano | Expandir de una a 2–4 manzanas; robot modular, rival aéreo, generador y reinicio; audio/efectos de impacto | Sesión jugable de 15 minutos, sin fallos críticos de control y con rendimiento registrado |
| E4 — Personalidades y variedad | H02 y H03, cada uno por separado; un kaiju mediano preparado tras estabilizar el desarme robótico | H02 usa el mismo carro/pared; H03 supera destinos válidos e inválidos; kaiju cambia ataques al perder piezas |
| E5 — Ciudad que reacciona | Pocos ciudadanos, incidentes, alerta/karma y después robo, meteorito y miniinvasión | Causa atribuida correctamente, huida/rescate, límites de eventos y modos desactivables |
| E6 — Exploración espacial | Ascenso y regreso a una región; después Luna y un planeta adicional cada vez | 10 viajes de retorno coherentes; cada destino anunciado como visitable tiene colisión e interacción real |

La investigación de animaciones puede acompañar E1, y la revisión documental espacial puede continuar fuera de la rama jugable. **No se integran planetas antes de estabilizar E3, ni se abren trabajos paralelos que compitan por el mismo editor.** E4/E5 son una propuesta de prioridad tras E3; solo se reordenan mediante una decisión registrada, no por iniciativa de cada agente.

## 4. Qué reutilizar, qué adaptar y qué no importar

Las fichas previas documentan candidatos; antes de incorporarlos se debe revisar la revisión/edición real, licencia de código y arte, archivos completos y dependencias. Un archivo LFS pequeño es un puntero, no la prueba de que el objeto esté disponible: hay que verificar su resolución. El aviso de cuota de ManOfSteel no se considera resuelto por existir un fork.

| Necesidad | Primera evaluación | Adaptación propia mínima | Alternativa y condición de descarte |
|---|---|---|---|
| Proyecto y movimiento terrestre | Third Person de Unreal; variante Combat si está disponible en la versión elegida | Caminar/sprint y curvas del único controlador | Plantilla básica sin cambiar de motor para obtener una variante |
| Vuelo | C++ de ManOfSteel: vuelo, inclinación, cámara y eventos visuales | Desacoplar dependencias, corregir movimiento sin barrido, ajustar al personaje propio | CMC nativo + poses permitidas si exige reconstruir gran parte del proyecto donante |
| Animaciones | Maniquí/rig compatible y clips concretos de Quaternius Standard; evaluar Rokoko | Retarget, 6–10 poses/transiciones aéreas que falten e IK de agarre | Blender/Control Rig; nunca declarar disponibles clips aún no descargados |
| Ciudad y objetos | Un kit urbano de Quaternius y/o Kenney, según licencia/edición comprobadas | Un bloque manual de referencia y luego distribución con PCG | Proxies sencillos mientras se valida; no City Sample completo ni Houdini de pago |
| Agarre | Componente mínimo de REPIRK, si es separable y portable | Transporte controlado y prueba del volumen de la carga | Physics Handle/interacción nativa mínima; no toolkit completo |
| Paredes y derrumbes | Chaos preparado; ensayo acotado del soporte de PlotBuilder | Adaptador de daño y estado de secciones; colapso limitado | Paneles con estados si el grafo de soporte exige importar inventario, tienda y construcción completos |
| Robots/kaiju | Un modelo autorizado animado de cada familia | Partes separadas, puntos de unión, cambio funcional y colisiones | Robot rígido modular y criatura provisional; nada de corte universal |
| Espacio | Ephemera en laboratorio separado; Cosmic Architect como alternativa | Conexión de una región urbana con representación orbital y persistencia | Esfera visible + regiones locales; no combinar generadores completos |

**Experimento de reutilización cerrado:** inventario → licencia → dependencias → mapa mínimo → compilación/empaquetado Windows → prueba. Si falta permiso, bytes o portabilidad, documentar el bloqueo y pasar a la alternativa. Después de un intento mínimo y una corrección dirigida, una migración que crece exige decisión explícita; no encadenar arreglos sin límite.

Las auditorías detalladas son [vuelo](FLIGHT_REUSE_AUDIT_V09.md), [ciudades/destrucción/planetas](CITY_DESTRUCTION_PLANETS_AUDIT_V11.md) y [animaciones](ANIMATION_CHARACTER_PIPELINE_V07.md). No convertir sus afirmaciones en dependencias aceptadas sin prueba local.

## 5. Arquitectura pequeña, con responsabilidades claras

Empezar con un proyecto y un módulo principal de gameplay, no varios motores o una docena de plugins propios. Carpetas y componentes separan responsabilidades; se extraerán módulos solo si aparece una necesidad comprobada.

| Área | Responsable del estado | Qué no debe hacer |
|---|---|---|
| Movimiento | Personaje/CMC y componente de vuelo: trayectoria, velocidad, colisión, caminar/volar | Dejar que un efecto visual o un montage largo determine todo el control |
| Animación/cámara | AnimationBP, poses, mezclas y cámara local | Crear daño o corregir físicamente teletransportes ocultos |
| Interacción | Objeto libre/sujeto/lanzado, un portador, soltar y restaurar colisiones | Mantener física libre y transporte controlado peleándose a la vez |
| Combate/desarme | Ataque e impacto identificados, daño, aturdimiento, pieza disponible/ausente | Duplicar daño por callbacks o seguir atacando con un brazo desprendido |
| Destrucción | Estado de panel/soporte y su colisión | Sustituir un hueco jugable por una nube de polvo |
| Mundo | Población de celdas, estado por región y, después, transformaciones planeta-región | Simular simultáneamente todos los edificios, ciudadanos y planetas |
| Incidentes/respuesta | Causa del daño/rescate, alerta, karma y director de eventos | Culpar al jugador por un ataque enemigo o puntuar cada fragmento |
| Presentación | Audio, partículas y mensajes a partir de resultados | Ser fuente de verdad para reglas |

IDs mínimos de objeto, ataque, parte, región e incidente sirven para guardar, reiniciar y depurar offline. **No se construye arquitectura de servidor por un cooperativo hipotético.**

Parámetros de aceleración, frenada, fuerza, resistencia, alerta y efectos van en datos editables. C++ para componentes y reglas que requieren pruebas; Blueprints para composición y ajuste. No introducir GAS, Motion Matching completo, MassCrowd o un editor de construcción hasta que resuelvan una necesidad demostrada.

## 6. Animación, peso y naturalidad

Un esqueleto humanoide compatible y un único personaje para tierra/aire. Estados base: reposo, caminar, correr, sprint, salto/caída, despegue, estacionario, crucero/impulso, freno, aterrizaje y recuperación. El torso, la inclinación y los brazos aportan variaciones aditivas; IK solo cuando hay contacto que ajustar. No pasar del vídeo de ManOfSteel a prometer sus animaciones como descargadas.

H01 transmite fuerza e impulso; H02 conserva el genio masculino esmeralda y gestos de manifestación; H03 utiliza poses cortas y destinos de teletransporte válidos. Los diseños aceptados no se sustituyen por héroes genéricos. La presentación de referencia en 2D aún requiere modelo, rig, materiales y prueba de deformación.

En E1 se revisará el vuelo sin desenfoque y con efectos mínimos. En E2 se añadirá audio/material, reacción, polvo breve y cámara configurable. Si la secuencia solo resulta atractiva al ocultarla con partículas, todavía no está aprobada.

## 7. Ciudad, karma y eventos sin ampliar el primer prototipo

E5 comenzará con 3–6 ciudadanos en un área cargada: caminar, percibir peligro, huir y llegar a un refugio. Representaciones lejanas más baratas se evalúan solo después de medir. No convertir multitudes experimentales en requisito de E1.

Distinguir tres datos: amenaza del mundo, alerta local 0–5 y reputación del jugador por región. Los valores -100..100 y demás tablas previas son propuestas de balance, no hechos del motor. El daño enemigo no baja automáticamente el karma del héroe; rescates de peligro autogenerado no pueden producir recompensas infinitas. Creativo permite desactivar consecuencias.

Un director de eventos administra requisitos, región cargada, presupuesto, aviso, fase activa, resultado y limpieza. Primero robo simple, después meteorito con zona preparada y luego invasión que reutiliza los mismos robots. Un gran evento simultáneo como límite inicial de ensayo. Kaiju y eventos orbitales solo cuando existan esos participantes. Sin historia ni misiones obligatorias.

## 8. Espacio: conservar la visión sin reconstruir el juego

La ciudad es la primera región detallada del planeta principal. La representación lejana utiliza una esfera y detalle geográfico simplificado; el sistema carga la región cercana para interacción. Primero probar ascenso/órbita/regreso; después la Luna, y los otros dos planetas uno por uno. Sol visual solamente.

No cerrar radio planetario, máximo de velocidad ni distancia espacial antes de pruebas de horizonte, precisión y carga. Se permite comprimir distancias de viaje. Un globo visto en el cielo no se declara exploración implementada; una pantalla sin colisión no se declara superficie jugable. Los daños importantes de la ciudad deben reaparecer al volver a su región.

## 9. Roles y herramientas, sin inventar agentes activos

| Rol | Responsabilidad y entrega |
|---|---|
| Usuario / dirección creativa | Prioridad y aprobación de sensación; prueba corta de control/potencia/deseo de repetir. No debe montar manualmente todos los sistemas |
| ChatGPT / planificación y revisión | Mantener este plan/cola coherentes, comparar alternativas y revisar evidencia; no simular ejecución local |
| Robert / posible ejecutor local | Recibir el primer encargo solo cuando se le asigne; verificar acceso a Linux/repo/Unreal. No se presume conexión a este chat ni tarea ya enviada |
| Codex / implementación | Integración mínima en el entorno autorizado, compilación, pruebas y reporte. Puede desempeñar roles secuenciales; no se anuncian cuatro agentes corriendo |
| QA / revisión separada | Reproducir aceptación, medir, registrar fallos y comparar con versión anterior. Si el mismo agente implementa y prueba, indicarlo: no fingir independencia |

Herramientas seleccionadas para evaluar en entorno: Unreal 5.8 Windows/DX12 y toolchain MSVC/SDK correspondiente, C++/Blueprints, Git/GitHub, Blender para edición necesaria, componentes nativos de movimiento/Chaos/entrada/animación, PCG en expansión urbana y herramientas de perfiles/pruebas de Unreal. El acceso concreto a Codex/editor y sus límites se verifican; no se contrata API ni hay IA obligatoria durante gameplay.

MCP es una vía opcional de automatización del editor si la instalación lo soporta. Si falla, usar código y scripts de editor documentados. No detener el prototipo intentando automatizar el 100% del editor ni exponer sus puertos.

Siete skills existentes, sin crear más por acumular: `impact-linux-gate`, `impact-license-audit`, `impact-gameplay-gate`, `impact-character-animation`, `impact-qa-review`, `impact-civic-systems`, `impact-world-events`. Se lee únicamente la pertinente y el documento de su subsistema. Las instrucciones históricas de red no se ejecutan.

## 10. Flujo de una tarea y límites de autonomía

Cola → ficha con resultado/prueba → rama aislada → auditoría de donante → cambio mínimo → build y prueba → revisión humana si cambia sensación → integración y build de referencia.

Una tarea activa de implementación y, como máximo, una revisión independiente que no escriba los mismos archivos. Un solo responsable de integración. No editar simultáneamente el mismo Blueprint/mapa, no force-push, no reset destructivo ni mezclar otros repositorios. La planificación actual no autoriza instalaciones, compras, migración de motor o cambios de sistema.

Antes de cada tarea: propósito, archivos permitidos, dependencias, pruebas, plan de reversión y entorno disponible. Al terminar: commit, versión motor/compilador, escenario y pasos, resultados observados, logs, captura cuando sea visual, métricas pertinentes, límites pendientes. Un vídeo del autor no es evidencia de nuestra build.

Estados: **PLANIFICADO → EN CURSO → IMPLEMENTADO SIN VALIDAR → COMPILADO → EJECUTADO → ACEPTADO**. BLOQUEADO incluye causa y próximo intento. No inventar porcentajes o fechas de entrega. No abrir una nueva investigación cuando la opción actual ya pasa la prueba.

## 11. Rendimiento y prioridades de ajuste

Objetivo inicial, pendiente de medir: ejecutable Windows a 1080p y calidad ajustable, aspirando a 60 FPS. El equipo reportado no garantiza ese rendimiento; verificar CPU, GPU exacta, driver, temperatura/potencia y almacenamiento. La RAM de 64 GB no sustituye los 8 GB de VRAM.

Registrar frame time medio, p95/p99, mayor tirón, CPU/GPU, memoria y conteos de cuerpos/actores. Separar primera ejecución y ruta repetida; con semilla, cámara y escenario reproducibles. Comparar además con la build base vacía antes de culpar un recurso.

Optimizar primero sombras/efectos superpuestos/texturas/fragmentos innecesarios/población lejana. Nunca eliminar colisión cercana o degradar respuesta del jugador silenciosamente para aumentar FPS. Un bloque con sobrecoste medido vuelve a ajuste; no se aumenta la ciudad sobre un problema sin resolver.

Criterios iniciales de repetición: 30 ciclos de suelo-vuelo; 30 agarres por tipo esencial; 50 impactos en paneles; 20 reinicios; 10 retornos de órbita cuando corresponda. Son muestras de regresión, no demostración de ausencia universal de bugs. La prueba humana aspirará a 4/5 en control, claridad y deseo de repetir, sin mareo que impida jugar; registrar el resultado real.

## 12. Riesgos y alternativas ya decididas

| Riesgo | Respuesta antes de ampliar |
|---|---|
| ManOfSteel no entrega assets o requiere demasiadas dependencias | Extraer solo lógica permitida; CMC y clips autorizados de respaldo |
| PlotBuilder obliga a portar un juego de construcción | Paneles/Chaos; dejar el grafo estructural en laboratorio |
| Agarres vibran o atraviesan paredes | Estado de transporte controlado y barrido de carga; una fuente de movimiento |
| Arte gratis no es modular | Preparar pocas piezas en Blender; no prometer corte arbitrario |
| GPU/memoria o CPU no alcanzan la escena | Reducir densidad/FX/física no útil y medir; no simular cuatro planetas |
| Agente declara éxito sin pruebas | Clasificar IMPLEMENTADO SIN VALIDAR o BLOQUEADO, mantener última build válida |
| Licencia, origen o descarga no claros | No importar ese archivo; usar alternativa disponible y conservar procedencia |
| Documentos contradictorios | Este plan y la cola mandan para alcance; documentos previos son consulta por tema |

## 13. Primer encargo y límite de este turno

Primer trabajo operativo previsto: **IB-W00 — verificar Unreal/Windows/toolchain, seguido de IB-W01 para crear y empaquetar la base**. Seguir después la cola de `TAREAS_ACTIVAS.md`. CPU/driver/almacenamiento/instalación se obtienen mediante inspección autorizada, sin repetir preguntas ya respondidas.

Con este turno se consolidan documentos y tareas. No se instala Unreal, no se clonan todos los donantes, no se ejecuta código de terceros y no se afirma que Robert recibió instrucciones. Acceso al equipo y ejecución siguen pendientes.

## Fuentes y trazabilidad

- Decisiones del usuario y auditorías reabiertas: [vuelo](FLIGHT_REUSE_AUDIT_V09.md), [movilidad terrestre](GROUND_FLIGHT_CREATORS_V10.md), [ciudad y planetas](CITY_DESTRUCTION_PLANETS_AUDIT_V11.md). Esos documentos distinguen inspección y ejecución pendiente.
- Reglas de concepto: [personajes](COMBAT_ROSTER_V03.md), [ciudadanos](CIVILIANS_ALERTS_KARMA_V06.md), [eventos](REFERENCES_DYNAMIC_EVENTS_V07.md), [sensación](ANIMATION_FEEL_OPTIMIZATION_V08.md). Consultar solo el tema necesario; prevalecen las decisiones actuales del usuario.
- Epic: [Linux Development Quickstart](https://dev.epicgames.com/documentation/en-us/unreal-engine/linux-development-quickstart-for-unreal-engine), [Movement Components](https://dev.epicgames.com/documentation/unreal-engine/movement-components-in-unreal-engine?lang=en-US), [Geometry Collections](https://dev.epicgames.com/documentation/en-us/unreal-engine/geometry-collections-user-guide). Se consultó documentación pública el 2026-11-14; fijar documentación al motor realmente elegido en IB-L00, no copiar su selector como garantía de compatibilidad.
- Estado previo de referencia: commit `b060bc7cc6fe828405463e02bf7df1a637f10330`. Git conserva el historial de README/AGENTS para comparar lo consolidado sin borrar investigación.
