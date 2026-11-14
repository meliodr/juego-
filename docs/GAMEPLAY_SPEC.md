# IMPACT: BEYOND — Especificación de jugabilidad v1

Fecha: 2026-11-14. Estado: DISEÑO PROPUESTO; no implementado ni probado. Plataforma inicial: Linux nativo, tercera persona, un jugador. Compras de recursos/plugins: RD$0.

Esta especificación convierte la visión del usuario en pruebas jugables. Complementa REUSE_CATALOG.md y REUSE_PLAN.md. Para el orden de entregas y la plataforma prevalecen este documento y LINUX_BASELINE.md. No cambia el objetivo posterior de planeta visualmente continuo, regiones detalladas/destruibles y salida al espacio.

## 1. Promesa y bucle central

Ser un superhumano es divertido incluso sin misiones: despegar, acelerar, elegir una ruta, agarrar un objeto o combatiente, convertir el impulso en un impacto y seguir volando por el espacio que acaba de abrirse.

Bucle: descubrir algo visible -> acercarse volando -> elegir cómo interactuar -> cambiar el escenario -> aprovechar la nueva ruta o el nuevo objeto -> continuar o reiniciar inmediatamente.

No hacer primero un mundo enorme ni una colección de plugins. La prueba central es una secuencia continua: calle -> vuelo -> carro -> lanzamiento -> pared perforada -> persecución -> enemigo agarrado -> impacto -> recuperación aérea.

La velocidad, la lectura visual, el control y la respuesta al impacto tienen prioridad sobre población, tamaño y realismo gráfico. Ningún recurso se integra solo por tener muchas funciones.

## 2. Vuelo: libertad sin perder el control

Un controlador compartido para ambos personajes. Tres comportamientos conectados, no tres minijuegos:

- Precisión: vuelo estacionario, desplazamiento lateral, ascenso/descenso y colocación de agarres. Mantener posición es posible; sin combustible.
- Impulso: acelerar manteniendo un botón, conservar inercia y ampliar progresivamente el radio de giro. Frenar es una acción explícita, no una teletransportación a velocidad cero.
- Travesía: extensión del impulso para grandes distancias, desbloqueada técnicamente después de verificar carga y colisiones. La salida al espacio no obliga a manejar una nave.

Control asistido por defecto: horizonte estable respecto al suelo local, inclinación visual del cuerpo y giro 3D dirigido con cámara. No imponer un simulador de aviación de seis ejes. Las acrobacias son una ampliación, no requisito de la primera entrega.

Mantener apuntado solicita precisión: si se desea reducir velocidad, debe ser una desaceleración progresiva y explicable. No frenar bruscamente al entrar en un radio invisible de combate. Tampoco permitir un giro instantáneo de 180 grados a máxima velocidad.

En el primer sandbox no hay combustible ni una barra de energía que impida explorar. El reto consiste en frenar, orientar y encadenar acciones. No fijar una velocidad supersónica como garantizada antes de ensayar objetos delgados y curvas de carga. Valores de velocidad, aceleración y frenado estarán en un Data Asset.

Choque esperado: atravesar una pieza rompible si el impacto supera su resistencia; detenerse o desviarse con reacción breve frente a una pieza resistente. Nunca atravesar una pared intacta por un fallo de colisión.

Recuperación aérea: después de recibir un golpe fuerte, permitir recuperar control y volar antes de caer al suelo. Evitar cadenas de ragdoll sin oportunidad de actuar.

## 3. Controles iniciales propuestos

Todos reasignables. Acciones independientes de teclas físicas; teclado/ratón primero y mando después de estabilizar el bucle. El mapeo de mando no es una dependencia de la primera entrega.

| Entrada inicial | Acción |
|---|---|
| WASD + ratón | Movimiento y orientación; suelo relativo a cámara y vuelo con intención 3D |
| F | Activar/desactivar vuelo; desactivarlo en el aire inicia caída recuperable, no teletransporte |
| Espacio / Ctrl | Ascender / descender durante vuelo; Espacio salta en tierra |
| Shift mantenido | Acelerar; soltar vuelve al régimen de vuelo normal progresivamente |
| X mantenido | Frenado fuerte controlado, también mientras se transporta un objeto |
| Clic derecho mantenido | Apuntar/enfocar el objetivo; asistencia de precisión configurable |
| E | Agarrar lo señalado; con algo sujeto, soltarlo sin ataque |
| Clic izquierdo | Sin objeto: golpe o construcción seleccionada; con objeto: lanzamiento |
| Clic izquierdo mantenido | Cargar golpe o lanzamiento; soltar ejecuta la acción |
| Q | Evasión corta; en estado de derribo aéreo, recuperación cuando se habilite |
| 1–4, forjador | Seleccionar construcción; clic izquierdo la ejecuta si no transporta objeto |
| Esc | Pausa, controles y reinicio de zona |

Reglas de prioridad: menú > recuperación/estado incapacitante > transporte > construcción > golpe. Un único clic no puede lanzar el carro y golpear al enemigo a la vez. Al soltar E no se lanza por accidente. Cualquier acción inválida debe indicarse discretamente, sin abrir ventanas.

Los diálogos de sistema o pérdida de foco deben pausar el modo local y liberar entradas retenidas, sin provocar aceleración infinita al volver.

## 4. Agarrar: el mundo como inventario inmediato

Un mismo lenguaje de interacción para vehículos, props, fragmentos grandes y combatientes. No inventario tradicional ni menús para equipar una viga.

Cada objeto declara categoría, resistencia, peso de juego, puntos de agarre, colisión y estado de anclaje. Un indicador pequeño distingue: agarrable, necesita arrancarse, demasiado grande en esta versión. No prometer que todo el mapa se levanta.

Estados: libre -> arrancándose (solo anclados) -> transportado -> lanzado -> impacto/reposo/destruido. La destrucción de un objeto transportado cancela el agarre de forma segura.

Objetos libres: agarrar directamente. Postes o piezas ancladas: breve acción de arrancado, con señal de resistencia; puede cancelarse. No implementar simulación estructural universal para liberar una farola.

Transporte: permite volar, frenar y girar. El tamaño/peso puede limitar maniobra y aceleración mediante curvas, pero no debe convertir el juego en una gestión de carga. Una pose asistida estable tiene prioridad sobre un resorte físicamente perfecto.

El volumen del objeto sostenido participa en los barridos de colisión. Si se interpone una pared resistente, el objeto no la atraviesa mientras el personaje pasa por un lateral. Evitar autocolisión continua entre jugador y objeto; restaurar colisiones de manera segura al soltar.

Lanzamiento: dirección apuntada, anticipación opcional y combinación controlada de impulso propio con velocidad del portador. Daño y velocidad limitada por curvas de gameplay. No emplear energía cinética ilimitada que destruya todo a la primera pasada.

Prohibido usar un escombro cosmético de Niagara como si fuera automáticamente un objeto físico. Promover solo fragmentos grandes identificables al sistema de interacción.

## 5. Combate continuo, no una arena que detiene el vuelo

Kit inicial de fuerza: golpe rápido, golpe cargado, embestida al atacar durante impulso, agarre, lanzamiento y evasión/recuperación. Animaciones breves combinables; no una cinemática por cada pared.

El mismo ataque cambia de resultado según contexto: cerca desplaza; cargado desequilibra; con impulso puede lanzar al enemigo contra una fachada. La asistencia ayuda a interceptar objetivos dentro de alcance, pero no teletransporta ni atraviesa obstáculos sólidos.

Enemigos ligeros pueden agarrarse directamente. Enemigos resistentes exigen desequilibrio previo o una apertura visible. Un enemigo no puede soltarse de un agarre sin señal. El jugador debe poder evitar ataques, recuperarse de un lanzamiento y cancelar una persecución.

Primer oponente: un rival volador que se aproxima, anuncia una embestida, ataca, recibe golpes y puede agarrarse. Primero una pelea contra uno; después pequeños grupos. Las torretas y drones son variantes posteriores, no requisitos para activar el primer combate.

La IA usa distancias relativas y oportunidades; no conoce siempre la posición tras obstáculos ni reaparece encima del jugador. Preservar desplazamiento y altura de combate; no forzar retorno inmediato a una arena invisible.

Personajes humanos y rescates: misma interfaz de transporte con respuestas distintas. El primer test de agarrar personas usa un combatiente/maniquí preparado. Multitudes, tráfico y simulación social quedan fuera del primer alcance.

## 6. Destrucción que modifica las decisiones

Tres categorías visibles y consistentes:

1. Props libres/anclados: carros, postes, bloques y contenedores. Se levantan, arrancan o rompen según reglas visibles.
2. Estructuras interactivas: paneles, muros y secciones de fachada con volumen y colisión preparados. Sus huecos deben poder atravesarse.
3. Entorno lejano o resistente: representación estable, sin simular todos los edificios. No usar exactamente la misma señal visual para piezas rompibles y resistentes.

Primer conjunto de prueba, cantidad ajustable tras medición: tres apariencias de vehículo, varios bloques/postes, seis paneles rompibles y una fachada parcial. Un edificio de demostración puede perder secciones; no se promete colapso libre de toda la ciudad.

Impactar debe generar nuevas posibilidades: hueco transitable, pieza grande reutilizable o pérdida de cobertura. Polvo, cristal y fragmentos pequeños aportan presentación, con duración y presupuesto limitados. No todas las partículas colisionan ni se guardan.

Destrucción persistente durante la sesión de la zona. Reiniciar restaura geometría, colisiones, enemigos y objetos; no destruye preferencias ni progresión. No reconstruir una pared frente al jugador salvo al usar explícitamente reinicio.

## 7. Dos personajes, las mismas reglas físicas

### Fuerza superhumana

Usa lo que existe: arranca, transporta, golpea y lanza. Más énfasis en contacto, impulso y uso del mapa. Su fantasía no necesita un repertorio de rayos ni un árbol de cincuenta habilidades.

### Forjador de energía

Crea herramientas prefabricadas, no modelos arbitrarios generados con IA durante la partida:

- Escudo: intercepta objetos/ataques; resistencia finita y lectura clara.
- Puño: golpe direccional a distancia corta/media con el mismo sistema de daño.
- Martillo: impacto pesado, más preparación y zona de efecto controlada.
- Pinza: agarra objetos o enemigos elegibles mediante el sistema compartido y puede devolverlos.

Límite inicial propuesto: dos construcciones persistentes; las transitorias se retiran al terminar. Al intentar superar el límite se muestra cuál se reemplazará. La energía limita construcciones fuertes, no el vuelo ordinario. En el sandbox se puede desactivar el coste para experimentar.

El mismo carro lanzado por el personaje físico puede interceptarse con escudo y sujetarse con pinza. Un martillo rompe las mismas paredes que una embestida. No dos arquitecturas de daño/agarre ni excepciones manuales por cada combinación.

La primera entrega física contiene solo al personaje de fuerza. El forjador se incorpora cuando el bucle compartido ya funciona; no se elimina del objetivo.

## 8. Exploración diseñada para volar

Primer mapa: pocas manzanas densas, un puerto/área industrial para objetos grandes y una periferia abierta para acelerar. Edificios bajos/altos, puentes y túneles producen rutas; un solar vacío enorme no sustituye exploración.

Recorridos que se cruzan: calle para precisión, azoteas para saltos y trayectos cortos, avenida/periferia para impulso y cielo abierto para persecución. Los impactos pueden abrir atajos. Colocar puntos visibles desde altura: torre, antena, grúa y señal de desafío.

Exploración sin trabajo repetitivo: retos opcionales de vuelo, precisión de lanzamiento o combate físico. Ofrecer un desafío al acercarse, no llenar la pantalla de iconos. No misiones de recorrer kilómetros para pulsar un botón ni recogida de cien objetos para desbloquear vuelo.

Cada región posterior debe justificar el viaje con otra forma de jugar: cañón de maniobras, puerto de cargas pesadas, estación de combate tridimensional. No ampliar terreno antes de que el desplazamiento sea divertido.

Planeta posterior: visualmente continuo, regiones detalladas y destruibles. Ascender y aterrizar conserva controles; en espacio se pierde la referencia de suelo y se ofrece orientación/horizonte manual. No simular órbitas científicamente ni imponer combustible para salir del planeta. Habrá puntos de interés visibles y frenado asistido opcional al solicitar aproximación, no viajes vacíos interminables.

## 9. Presentación, interfaz y comodidad

Cámara con anticipación y distancia dependiente de velocidad, ajustes suaves de FOV y recuperación estable después de un agarre. Evitar que el objeto sostenido tape todo el objetivo. FOV máximo, temblor, estelas, motion blur y asistencia serán configurables.

Velocidad se transmite con referencias cercanas, sonido, estelas y cambios visuales, no obligando a usar blur. Un impacto combina sonido por material, reacción corta, impulso y ruptura visible. La sacudida de pantalla no debe impedir apuntar la acción siguiente.

HUD mínimo: objetivo de interacción, energía si corresponde, salud en desafío y referencia de velocidad/altura opcional. Mostrar incompatibilidad de agarre sin mensajes largos. Controles visibles contextualmente al principio y ocultables.

Sandbox: reinicio de zona, reaparición de vehículos/rival, cambio de personaje cuando exista, energía/vida infinitas y pausa. Preferencias guardadas localmente; juego sin API de IA, tienda ni conexión obligatoria durante una partida.

Daño corporal posterior: separación de piezas preparadas con presentación estilizada y ajuste para desactivarla. No vísceras ni anatomía explícita. Debe cambiar una reacción/capacidad o aportar lectura, no bloquear el núcleo divertido con semanas de preparación de todos los modelos.

## 10. Entregas revisadas: jugabilidad antes que catálogo

| Entrega | Trabajo | Evidencia para cerrar |
|---|---|---|
| D0 — Linux y vuelo | E00 nativo + mapa gris + cámara + despegue, impulso, freno y recuperación | Paquete Linux ejecutado; recorrer un circuito durante 10 minutos sin perder control |
| D1 — Cadena física | E01/E02: carro -> transporte -> lanzamiento -> hueco -> atravesarlo | 30 ciclos de agarre por tipo; 50 impactos y 20 reinicios registrados, sin colisiones fantasma |
| D2 — Sandbox jugable | Un rival volador, distrito legible, audio, retos opcionales y opciones | Sesión de 15 minutos alternando vuelo, combate y reinicio; valoración del jugador y fallos anotados |
| D3 — Asimetría | Cuatro construcciones compartiendo daño y agarre | Interceptar/devolver el mismo carro; romper la misma pared con ambos personajes |
| D4 — Expansión vertical | Laboratorio orbital Linux independiente, después conexión regional | Diez viajes superficie-espacio-regreso y una prueba con objeto sujeto sin perder estado |

Las duraciones y cantidades son criterios de prueba, no estimaciones del tiempo de desarrollo ni resultados obtenidos. El daño corporal es opcional después de D2; nunca bloquea D0–D2.

## 11. Matriz de aceptación para Codex

- G01: vuelo con límites de fotograma distintos, registrando frecuencia real. Aceleración/distancia no dependen arbitrariamente de FPS; probar 30/60 y otras tasas solo si el hardware las alcanza.
- G02: soltar impulso y frenar en recta/curva. No teletransporte ni cambios ocultos de velocidad al acercarse a un enemigo.
- G03: vuelo y objeto sostenido frente a pared delgada, esquina y suelo a la máxima velocidad permitida del prototipo. Barrido del jugador y carga; cero penetraciones no intencionadas en el conjunto de pruebas.
- G04: 30 agarres/lanzamientos por objeto; destruir el objeto mientras está sujeto; morir, pausar y reiniciar durante transporte. Sin referencia colgante ni gravedad/colisión incorrectas.
- G05: 50 impactos en paneles y bordes; hueco transitable, fragmento grande reutilizable, reinicio coherente. Medir tiempos de actualización de colisión.
- G06: 20 agarres/lanzamientos de enemigo, incluyendo suelo/aire, ragdoll y recuperación. Sin capacidades activas de un estado anterior.
- G07: 20 reinicios de zona. Contar actores/cuerpos y muestrear memoria después de estabilización; sin crecimiento persistente no explicado ni fragmentos duplicados.
- G08: perder/restaurar foco, cambiar mapeo y relanzar ejecutable. Sin teclas atascadas; preferencias persistentes y texto de controles correcto.
- G09: paquete Linux con Vulkan, resolución/calidad/hardware/driver identificados. Objetivo inicial de diseño: 60 FPS, sujeto al hardware real. Registrar promedio, p95/p99, peor tirón, física y memoria, separando primera ejecución y recorrido calentado. No inventar cumplimiento.
- G10: prueba humana: puntuar control, sensación de velocidad, claridad de impactos y deseo de repetir del 1 al 5. Objetivo de aceptación creativa: al menos 4 en control/velocidad y ninguna molestia que impida jugar. La IA no puede sustituir esa valoración con un log.

No cerrar D0 solo porque compila. No cerrar D1 con un vídeo de la demo del autor. No considerar el éxito de pruebas sin render como evidencia de Vulkan, VFX o calidad de cámara.

## 12. Lo que no haremos en la primera versión

Campaña, multijugador, tráfico inteligente, miles de NPC, economía, árbol de habilidades, físicas permanentes de todo el planeta, corte arbitrario de cualquier cuerpo, herramientas de construcción ilimitadas o generación de assets por IA durante gameplay.

Estas exclusiones son decisiones de alcance. No reducen los pilares: velocidad, libertad, combate físico y destrucción útil. El objetivo es jugar antes de ampliar.
