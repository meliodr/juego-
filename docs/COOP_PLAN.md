# DOCUMENTO POSPUESTO — no aplicable al desarrollo actual (v0.9)

El usuario decidió tratar la versión de dos jugadores como **una posibilidad muy lejana**. Este archivo se conserva exclusivamente para futura investigación. Sus indicaciones de pruebas N0/N1 tempranas, autoridad del servidor, despliegue LAN, replicación y roadmap han quedado **reemplazadas** por docs/FLIGHT_REUSE_AUDIT_V09.md. No crear ni probar estas capacidades ahora. El proyecto actual es offline para un solo jugador. Mantener separación modular útil también para guardado/QA; no prometer compatibilidad futura automática.

# IMPACT: BEYOND — Cooperativo de dos jugadores

Fecha: 2026-11-14. Estado: PROPUESTA TÉCNICA; no implementado ni ejecutado. El usuario quiere poder jugar con dos jugadores en una entrega posterior. La elección LAN/online antes que pantalla dividida es una propuesta de implementación, no una preferencia confirmada del usuario.

Extiende GAMEPLAY_SPEC.md y SANDBOX_DESIGN.md. La primera entrega pública puede seguir siendo individual, pero ya no se debe consolidar una arquitectura que asuma un único jugador. Las exclusiones antiguas de multijugador se interpretan como exclusión de la primera entrega, no como permiso para ignorar la futura red. Linux nativo y compras RD$0 siguen vigentes.

## 1. Alcance decidido para planificar

Diseñar primero cooperativo PvE para dos equipos, cámara individual y un jugador anfitrión. Validar en LAN antes de conexión por Internet. Ambos pueden elegir VECTOR o PRISMA, incluyendo personajes repetidos. Modo individual sin conexión sigue disponible. No campaña cooperativa, PvP competitivo, pantalla dividida ni cuatro jugadores como condición de esta entrega.

El anfitrión ejecutaría un listen server: juega y mantiene el estado autoritativo. Unreal documenta este modelo y también su coste extra y ventaja de latencia para el anfitrión [S1]. No exige contratar un servidor dedicado para la primera prueba LAN. Esto no resuelve automáticamente descubrimiento por Internet, NAT, invitaciones ni relés: esa integración se evaluará aparte, con condiciones y presupuesto comprobados. No abrir puertos ni modificar red del usuario durante planificación.

Sin migración de anfitrión inicial: si sale, finalizar la sesión con mensaje claro; guardar el estado que corresponda sin prometer continuidad automática. Un cliente que se desconecta no deja objetos en propiedad indefinida.

Primera zona cooperativa: un distrito acotado y anunciado en la selección de modo. No una cuerda invisible que teletransporte al compañero. La exploración planetaria individual puede avanzar separadamente; que un jugador esté en órbita y otro en superficie exige otra prueba de carga, coordenadas y presupuesto.

## 2. Qué preparar ahora y qué aplazar

**Ahora:** separar reglas de juego de cámara/VFX; identidad de jugador por posesión, no GetPlayerController(0); autoridad de daño/agarre; estado persistente de estructuras; datos de movimiento compatibles con predicción; pequeña prueba con dos conexiones tras vuelo y tras carro/pared.

**Después:** interfaz de salas, invitaciones online, reconexión amigable, tutorial cooperativo y transporte de aliados. No completar toda la infraestructura de sesiones antes de comprobar que volar es divertido.

No presentar 'poner Replicates=true' como implementación del cooperativo. Epic distingue replicación de estado, interacciones y presentación; parte del trabajo se define explícitamente en gameplay [S1].

## 3. Responsabilidades propuestas

| Sistema | Autoridad | Presentación/predicción local |
|---|---|---|
| Movimiento del héroe | Valida y reconstruye entradas de vuelo | Respuesta inmediata y suavizado; correcciones cuando proceda |
| Golpe/daño | Decide impacto válido, salud, desequilibrio y deduplicación | Animación y anticipación visual; no decide salud definitiva |
| Agarre | Valida distancia, elegibilidad y reserva exclusiva | Pose/objetivo visual; cancela si la solicitud se rechaza |
| Carro lanzado | Estado inicial, colisiones significativas y daño | Interpolación/predicción a evaluar, sin inventar resultados de daño |
| Estructura | Estado de paneles, colisión jugable y versión | Mismo hueco jugable; polvo y fragmentos cosméticos locales |
| Enemigos | IA, objetivo, ataques y estados | Animación, interpolación y señales de ataque |
| Construcciones | Autoriza tipo, coste, duración y propietario | Aparición anticipada controlada y VFX |
| Reinicio | Anfitrión coordina y confirma estado | Interfaz local y aplicación del nuevo estado |

Los nombres siguientes son contratos propuestos, no clases ya existentes ni obligación de crear un framework grande.

### Movimiento

Conservar ACharacter/CharacterMovementComponent como primera base. Epic documenta movimiento replicado, predicción y datos de movimientos guardados; una mecánica personalizada necesita integrarse en ese flujo [S2].

Registrar intención de impulso, frenado y régimen dentro de movimientos guardados/datos de red cuando afecten a la simulación. No mover el héroe mediante SetActorLocation cada Tick esperando que la replicación lo arregle. Probar cambios de modo, colisiones y transporte a velocidad máxima admitida en el prototipo.

Agarre de un combatiente necesita un estado explícito 'transportado', con autoridad única sobre su desplazamiento. No dejar CMC, ragdoll, adjunción y un resorte físico intentando moverlo simultáneamente. Recuperación/soltado restaura el régimen anterior válido. No prometer sincronización de toda la pose ragdoll como requisito del primer combate: priorizar raíz y estado jugable coherentes.

### Agarre y transferencia

Estado mínimo: ObjectId, HolderId, modo libre/sujeto/lanzado, secuencia de acción y revisión. Solicitudes desde el Pawn/Controller poseído o su componente replicado, no RPC de servidor desde cualquier objeto sin dueño.

La autoridad valida objetivo, alcance, estado, límite y obstrucciones. Dos solicitudes sobre el mismo carro no producen dos dueños: aceptar una de forma ordenada y comunicar rechazo de la otra. Diferenciar dueño de gameplay de ownership de red; no transferir autoridad al cliente simplemente porque lo sostiene.

Relevo de carro: al liberarse puede pasar a lanzado; otra acción validada lo intercepta y cambia a sujeto. No copiar el actor ni crear un segundo carro para simular la recepción. Desconexión, muerte, destrucción, cambio de personaje o reinicio liberan el agarre y limpian colisiones de forma segura.

### Destrucción compartida

Separar tres niveles:

1. Estado jugable persistente: panel intacto/dañado/ausente, apoyos, revisión e identificador estable de estructura.
2. Fragmentos grandes interactivos: pocos actores identificados con transformaciones y estado de agarre/daño.
3. Polvo, chispas y trozos pequeños: locales, temporales, sin daño ni colisión que afecte al gameplay.

Si un cliente ve un agujero, la colisión jugable debe corresponder al mismo estado aprobado en el servidor. No dar por suficiente enviar un evento de explosión o usar la misma semilla de fractura: eso no demuestra una simulación Chaos idéntica en dos máquinas.

Propuesta inicial: pared modular con máscara/estado de secciones y colisión reproducible; Chaos presenta la rotura y promueve solo trozos elegidos. Los booleanos arbitrarios de una malla no se convierten en requisito del cooperativo. R03 u otro plugin solo se acepta después de demostrar red y reconstrucción de estado en Linux, no por su demo individual.

Los cambios relevantes se conservan en un estado/snapshot versionado. Multicast puede acompañar la presentación, pero no es la única fuente: el que entra tarde o vuelve a una zona necesita reconstruir los daños actuales sin haber recibido cada evento pasado. Resolver y ordenar actualización de geometría/colisión antes de habilitar paso e interacción; una nube de polvo no justifica una pared fantasma.

### Física

La documentación de Epic advierte que velocidad y latencia dificultan la réplica de interacciones físicas [S3]. Por eso no se aprueba 'cada ordenador simula todo y coincidiremos'.

Empezar con pocas cargas importantes y transporte controlado. Comparar la réplica/interpolación apropiada para objetos libres, bajo la versión fijada del motor; no activar resimulación de todo el mapa. La selección de un modo depende de pruebas y de compatibilidad entre objetos/character controller, no de escoger el sistema más nuevo.

Un objeto lanzado tiene velocidad de juego limitada; comprobar barridos y colisiones con piezas delgadas. Las aparentes mejoras de impacto no autorizan duplicar daño por corrección de posición. Cada acción/impacto tiene identidad para deduplicar.

### Relevancia y carga para dos jugadores

El servidor debe conservar contenido de simulación alrededor de ambos jugadores y del recorrido de objetos importantes. Cada cliente necesita la geometría y los actores que pueden afectarlo, incluyendo un carro que viene desde lejos. Epic documenta relevancia por conexión [S4]; no confundirla con que las celdas y colisiones estén ya cargadas.

No usar un único origen global tomado de 'jugador 0' ni descargar una zona porque el anfitrión se alejó si el compañero sigue allí. Mantener estados fuera de celdas efímeras. Medir coste con ambos juntos y separados; el límite regional inicial se declara en el modo de juego, no se oculta tras teletransportes.

## 4. Reglas de experiencia cooperativa

Daño amigo desactivado por defecto. Un modo caos es optativo y compartido, no un requisito para combinaciones. El agarre de aliados requiere aceptación temporal individual y ofrece soltarse inmediatamente; ninguna configuración del anfitrión elimina ese escape.

No obligar a usar héroes diferentes. Enemigos y objetivos permiten fuerza física, lanzamiento o construcciones; la cooperación acelera/varía la solución pero no sustituye todas las soluciones individuales.

Escalar por composición y reparto de atención, no duplicar todas las barras de salud. Límite por jugador de ataques de alta amenaza, avisos y alternancia de objetivos. No añadir un compañero IA para sustituir al segundo jugador en solitario.

La cámara, HUD, controles, sonido de indicación y accesibilidad pertenecen a cada jugador. Una sacudida no se aplica al compañero por usar una cámara global. Hit-stop y cámara lenta: no pausar o dilatar todo el mundo en cada golpe; presentación local compatible, sin cambiar la simulación solo en un cliente.

Menú/pérdida de foco: pausar en individual; online liberar entradas pero la sesión sigue. Comunicarlo. Reinicio de zona pedido por anfitrión con aviso y confirmación del compañero; cancelar agarres, construcciones y ataques antes de restaurar. Primera versión puede permitir entrada solo desde sala/estado seguro; no anunciar entrada libre en mitad de combate antes de resolver snapshot.

Respawn en punto despejado cerca de la acción. Transporte/rescate de aliado se añade después del agarre de combatientes, no antes. Sin economía ni progreso competitivo compartido que obligue a servicios externos.

## 5. Fases y puertas técnicas

| Fase | Momento | Prueba | No equivale a |
|---|---|---|---|
| N0 | Tras base Linux y vuelo inicial | Dos conexiones, personajes visibles, movimiento/impulso/freno independientes | Cooperativo completo |
| N1 | Durante carro + pared | Dueño exclusivo, lanzamiento, estado de panel y hueco coincidentes | Destrucción universal |
| N2 | Durante rival y PRISMA | Daño/IA, intercepción y combinaciones; mismos héroes posibles | Lobby o juego online sin fricción |
| N3 | Entrega cooperativa local | Paquetes Linux en dos equipos, sala mínima, entrada/salida segura, restablecer zona | Prueba de Internet con NAT |
| N4 | Tras N3 | Sesión por Internet bajo configuración documentada y límites reales | Servidores gratis permanentes o compatibilidad con cualquier red |
| N5 | Ampliación | Dos jugadores separados y tránsito regional/orbital | Simulación de todo el planeta |

No posponer N0/N1 hasta terminar el arte. Tampoco bloquear la primera entrega individual por construir N4. Si un donante no supera una prueba pequeña razonable de red, registrar el problema y preferir el componente mínimo; no rehacer un juego entero por conservarlo.

## 6. Matriz de aceptación propuesta

Pruebas pendientes. Ejecutar host y cliente, después invertir quién aloja. PIE facilita pruebas, pero no reemplaza dos paquetes Linux independientes [S5]. Medir hardware, resolución, driver, uso de CPU/GPU/memoria, tráfico y RTT real.

Perfiles sugeridos: local limpio; RTT medido alrededor de 50, 100 y 150 ms; pérdida de paquetes 0%, 1% y 3%; una prueba degradada adicional. Son condiciones de evaluación, no promesa de jugabilidad perfecta a todas las latencias. Registrar parámetros concretos: el retraso aplicado por dirección no es necesariamente el RTT final. Unreal dispone de emulación de latencia/pérdida [S6].

| ID | Escenario | Resultado exigido |
|---|---|---|
| N-T01 | Dos jugadores aceleran, frenan, cambian modo y chocan | Ambos conservan control; medir correcciones, no ocultarlas |
| N-T02 | 30 intentos simultáneos de agarrar el mismo carro | Un solo portador; ningún duplicado ni estado bloqueado |
| N-T03 | 30 lanzamientos e intercepciones | Transferencia coherente, sin daño duplicado |
| N-T04 | Un jugador abre 20 huecos; el otro los atraviesa | Misma colisión jugable y revisión en ambas máquinas |
| N-T05 | Cliente entra en estado seguro tras destruir y vuelve a zona descargada | Reconstruye daños; no aparecen paredes intactas fantasma |
| N-T06 | Cliente sale sosteniendo objeto; se destruye un objeto sujeto | Liberación o limpieza definida; sin propietario inexistente |
| N-T07 | Dos jugadores derriban/agarran al mismo rival | Estados y daño consistentes; recuperación posible |
| N-T08 | PRISMA intercepta carro de VECTOR; pareja de héroes iguales | Sin exclusividad artificial de clase ni poderes compartidos por error |
| N-T09 | Menú, foco, cámara y control reasignado de un cliente | No pausa toda la sesión ni modifica entrada/cámara ajena |
| N-T10 | 20 reinicios y cancelaciones durante actividad | Conteo estable después de limpiar; estados restaurados |
| N-T11 | Jugadores separados dentro de región permitida | Objetos y colisiones necesarios siguen activos para ambos |
| N-T12 | Host cierra o se pierde conexión | Mensaje y salida segura; no falsa promesa de migración |
| N-T13 | Latencia/pérdida sobre pruebas N-T02/03/04 | No duplicaciones ni estado permanente divergente tras estabilizar red; medir percepción |

No exigir coincidencia de cada partícula. Sí exigir el mismo carro, portador, daño, enemigo y hueco. Rendimiento y suavidad se juzgan con medidas y prueba humana, no solo ausencia de crash.

## 7. Fuentes primarias consultadas

Consultadas el 2026-11-14. Las páginas sin versión pueden cambiar de selector automáticamente; verificar APIs y opciones contra la versión del motor que realmente se instale. No se cambia a una Preview por leer la documentación actual.

- S1 — Epic, Networking Overview: https://dev.epicgames.com/documentation/unreal-engine/networking-overview-for-unreal-engine
- S2 — Epic, Networked Movement in the Character Movement Component: https://dev.epicgames.com/documentation/unreal-engine/understanding-networked-movement-in-the-character-movement-component-for-unreal-engine
- S3 — Epic, Networked Physics Overview: https://dev.epicgames.com/documentation/unreal-engine/networked-physics-overview
- S4 — Epic, Actor Relevancy: https://dev.epicgames.com/documentation/unreal-engine/actor-relevancy-in-unreal-engine
- S5 — Epic, Multiplayer Programming Quick Start: https://dev.epicgames.com/documentation/en-us/unreal-engine/multiplayer-programming-quick-start-for-unreal-engine
- S6 — Epic, Using Network Emulation: https://dev.epicgames.com/documentation/unreal-engine/using-network-emulation-in-unreal-engine

Estas fuentes respaldan capacidades y límites del motor. La selección de héroes, estados, materiales y fases es una propuesta propia, no una capacidad automática de Unreal ni resultado de pruebas realizadas.
