# IMPACT: BEYOND — Diseño del sandbox v0.2

Fecha: 2026-11-14. Estado: PROPUESTA DE DISEÑO, sin implementación ni pruebas jugables. Linux nativo; compras de assets/plugins RD$0. Nombres de personajes y lugares provisionales, no una comprobación comercial de marcas.

Esta extensión de GAMEPLAY_SPEC.md concreta identidad, personajes, enemigos y modos. COOP_PLAN.md incorpora la nueva intención del usuario: dos jugadores en una entrega posterior, con pruebas técnicas tempranas. Los controles y la base Linux previos siguen vigentes salvo las excepciones multijugador indicadas allí. No se ha creado arte final ni un proyecto Unreal durante esta revisión.

## 1. Promesa

Un patio de juegos de poder superhumano: entrar, volar, encontrar o convocar un rival, convertir el mapa en armas y repetir una pelea distinta. Primero diversión controlable; después tamaño del mundo. No es una campaña obligatoria, un simulador militar ni un catálogo de poderes sin interacciones.

Bucle: ver una oportunidad -> acercarse volando -> golpear/agarrar/crear -> transformar el escenario -> aprovechar la nueva ruta o proyectil -> continuar o reiniciar. El personaje debe ser divertido incluso antes de que aparezca un enemigo.

Reglas: vuelo sin combustible; poderes principales disponibles al elegir personaje; mismo lenguaje de interacción para jugadores e IA; destrucción con consecuencias; recuperación rápida; ningún trámite de inventario entre dos golpes. El modo cooperativo no exigirá usar un personaje específico.

## 2. Dirección visual: cómic 3D industrial luminoso

Propuesta: formas grandes, materiales sencillos y coherentes, contraste legible, impactos exagerados y ciudad estilizada. No mezclar personajes low-poly con calles fotorealistas, ni depender de detalles pequeños que no se leen volando.

| Capa | Diseño propuesto | Regla de lectura |
|---|---|---|
| Ciudad | Concreto marfil/gris, asfalto oscuro, vidrio azulado, maquinaria ocre | Menos saturada que personajes y poderes |
| VECTOR | Grafito, marfil y acentos coral; torso y antebrazos contundentes | Silueta compacta y angular, impulso frontal |
| PRISMA | Traje oscuro, placas claras separadas, energía turquesa/jade | Silueta ligera, herramientas geométricas abiertas |
| Enemigos | Metal carbón, cerámica gris, núcleos ámbar | Formas distintas por función, no solo recolores |
| Peligro | Trayectoria, postura y señal sonora; color como refuerzo | Nunca depender únicamente de rojo/verde |
| Destrucción | Fracturas y apoyos reconocibles; polvo breve, fragmentos grandes claros | El efecto no debe ocultar la siguiente decisión |

Sombras estilizadas y contornos son opciones artísticas, no dependencias de un shader comprado. Empezar con materiales nativos sencillos; evaluar cualquier efecto bajo Vulkan. Los carros agarrables reciben una señal contextual al apuntar, no un borde brillante permanente sobre todo el mapa.

La violencia se presenta con abolladuras, armaduras rotas, reacciones, polvo y separación de piezas preparadas. No vísceras ni detalles anatómicos explícitos. Desactivar el efecto corporal cambia la presentación, no la colisión o el daño compartidos entre jugadores.

### Reutilización de arte

Tomar una base humanoide gratuita autorizada y compartir esqueleto si las pruebas de escala/retargeting lo permiten. Diferenciar mediante proporciones visuales moderadas, placas rígidas, materiales, poses y VFX; no encargar dos esqueletos completos antes de jugar. Evitar capa larga, pelo complejo y simulaciones de ropa como requisitos iniciales. Las construcciones salen de primitivas/mallas reutilizables y un material común.

Los recursos Kenney/Quaternius son candidatos del catálogo existente: verificar los archivos realmente incluidos y las licencias. No se aprueba un paquete nuevo ni se garantiza compatibilidad por esta propuesta visual.

## 3. Dos personajes jugables

### VECTOR — el mundo como arma

Arquetipo original de fuerza e impulso. Traje grafito con segmentos marfil, antebrazos coral, rostro visible y marca diagonal propia. Sin capa ni iconos de franquicia. Cuerpo proyectado hacia delante al acelerar; antebrazos abiertos al frenar; sonido de presión y aire, no propulsores mecánicos.

| Acción | Resultado y decisión |
|---|---|
| Golpe rápido | Interrumpe o desplaza; permite continuar moviéndose |
| Golpe cargado | Desequilibra y lanza, con preparación visible |
| Embestida | Convierte una trayectoria bien elegida en impacto; fallar exige corregir |
| Agarre | Recoge objeto libre o combatiente elegible; no atraviesa obstáculos para alcanzarlo |
| Arrancado | Libera postes/piezas preparadas después de resistencia breve y cancelable |
| Transporte y lanzamiento | Conservar movimiento; apuntar y elegir trayectoria, no abrir inventario |
| Recuperación/evasión | Recuperar vuelo tras un golpe; sin cadenas indefinidas de pérdida de control |

El daño por velocidad usa una curva con límite. No basta rozar un objeto a velocidad extrema para eliminar toda la ciudad. La dificultad es elegir ruta, frenar y aprovechar aperturas; no vigilar una barra de gasolina.

Debilidad de diseño: menor alcance que PRISMA, carga grande limita giro, una embestida fallada ofrece una apertura breve. Ninguna debilidad debe apagar la fantasía básica de volar.

Secuencia de referencia: arrancar un poste -> desviar un dron -> recoger un carro -> lanzarlo contra el rival -> atravesar la fachada dañada -> continuar en el aire. Las acciones son sistemas combinables, no una animación única.

### PRISMA — fabricar la herramienta adecuada

Arquetipo original de construcción energética. Silueta más ligera, armazones de antebrazo y piezas dorsales rígidas pequeñas; energía turquesa/jade dibujada como marcos que se ensamblan. No anillo ni traje identificable de una franquicia. La identidad es geométrica y técnica, no un catálogo de rayos de colores.

Comparte vuelo, frenado, evasión y golpe básico. Puede ganar solo. Su recurso de concentración solo limita construcciones potentes; no bloquea el vuelo ni toda capacidad de ataque.

| Construcción inicial | Función | Límite/contrajuego |
|---|---|---|
| Escudo | Interceptar una trayectoria o proteger una aproximación | Área y resistencia finitas; no invulnerabilidad esférica |
| Puño | Empujar, interrumpir y lanzar desde distancia corta/media | Dirección y alcance limitados |
| Martillo | Golpe pesado sobre enemigo o estructura | Preparación visible; cambiar de dirección tiene coste de control |
| Pinza | Sujetar, transportar y devolver objetos o enemigos elegibles | Usa el mismo dueño y estados del agarre de VECTOR |

Propuesta inicial: máximo dos construcciones persistentes; los golpes transitorios terminan tras la acción. Reemplazar una construcción muestra cuál desaparecerá. Energía infinita disponible en modo libre. No generación de mallas arbitrarias mediante una API de IA durante la partida.

Debilidad de diseño: las herramientas grandes exigen orientación y preparación; un rival cercano puede forzar reposicionamiento. No convertirlo en soporte indefenso ni obligar a encontrar un compañero fuerte.

Secuencia de referencia: bloquear un carro -> sujetarlo con la pinza -> devolverlo a un rival -> acercarse con vuelo -> martillo sobre una columna -> salir por el hueco.

### Personajes futuros

No añadir un tercer protagonista a la primera entrega. Dos personajes con interacciones completas tienen prioridad sobre seis apariencias con acciones repetidas. Ambos pueden repetirse en cooperativo, con acentos, iconos y etiquetas de jugador diferentes. No bloquear la elección por una cuota de clase.

## 4. Enemigos que cambian la manera de pelear

Una familia visual común, provisionalmente 'la Contención': drones y combatientes con armaduras tecnológicas. Su función es responder a la presencia superhumana, no requerir una campaña para entender el sandbox.

Primero un humanoide rival. Después un dron geométrico y una variante pesada del humanoide. Interdictor y jefe llegan tras demostrar diversión con esa combinación. Las siluetas y conductas importan más que cinco barras de salud diferentes.

| Enemigo | Diseño | Conducta | Respuesta del jugador | Prioridad |
|---|---|---|---|---|
| Rival | Humanoide estrecho, aletas pequeñas, núcleo vertical | Vuela, persigue, anuncia embestida y ofrece recuperación tras fallar; agarre enemigo después | Frenar, esquivar, contraatacar y lanzarlo contra el mapa | Primero |
| Avispa | Dron triangular con tres brazos y núcleo visible | Pasadas y disparos anunciados; mantiene distancia, no teletransporte permanente | Agarrarlo o interceptarlo y usarlo como proyectil | Sandbox ampliado |
| Ancla | Humanoide ancho, placas frontales y apoyos reconocibles | Resiste frontalmente, se ancla a una superficie y arroja cargas; no vuela bien | Flanquear, romper apoyo preparado o desequilibrar antes de agarrar | Variante del rival |
| Interdictor | Unidad de módulos separados y balizas visibles | Crea un corredor peligroso o campo localizado anunciado | Esquivar o destruir emisor; no quitar vuelo a toda la región | Reserva |
| Bastión | Variante élite blindada, placa/núcleo central y módulos separables | Alterna embestida, lanzamiento y recuperación; usa piezas del entorno | Quitar módulos, cambiar ángulo y aprovechar aperturas | Jefe posterior, no gigante nuevo |

Bastión no será inicialmente un monstruo del tamaño de un rascacielos. Reutiliza locomoción y un subconjunto de ataques del rival, con armadura preparada. Su tamaño debe respetar animaciones y colisiones probadas; 'escalar un personaje' no se acepta como solución automática.

### Contrato de comportamiento justo

Percepción -> seleccionar objetivo -> buscar posición -> anunciar -> atacar -> recuperarse. La IA respeta superficies y obstáculos actualizados. El vuelo necesita detección y evasión tridimensional; no asumir que una ruta de suelo resuelve el aire.

El enemigo pesado abre una oportunidad antes de un agarre; el ligero puede agarrarse directamente. La misma condición funciona con manos físicas y pinza. No hacen falta armas de un color concreto para dañar a un tipo.

Limitar ataques de alta amenaza simultáneos por jugador y espaciar avisos fuera de cámara. Punto de partida para probar: un atacante de contacto comprometido y apoyo a distancia, no todo el grupo pegando a la vez. El coordinador asigna amenazas, no controla al jugador ni se presenta como una IA invencible.

Escalar dificultad por combinaciones, trayectorias, ventanas de reacción y coordinación antes que multiplicar vida. El rival no conoce posiciones detrás de cualquier obstáculo ni reaparece encima de la cámara. Si queda atascado, registrar el fallo y buscar una ruta/retirada; no teletransportar ataques silenciosamente.

No mantener una cadena de agarres/ragdoll sin escape. Recuperación defensiva con breve protección contra nuevo agarre, anunciada mediante postura/VFX. Probar también esa regla con dos jugadores golpeando al mismo enemigo.

## 5. Destrucción y combate: reglas comunes

Cada impacto identifica autor, acción, objeto, superficie, dirección y potencia de juego limitada. La ejecución, el daño, el impulso y el efecto visual son responsabilidades distintas. Un mismo impacto no puede aplicar daño cada fotograma de contacto como si fueran golpes nuevos.

| Material/categoría | Respuesta propuesta |
|---|---|
| Cristal o panel ligero preparado | Paso y fragmentos visuales; poco freno |
| Muro modular | Requiere potencia suficiente; se abre una sección real |
| Placa reforzada | Desvía/detiene hasta debilitar un apoyo o superar un umbral explícito |
| Columna preparada | Se desprende o cambia el estado de una sección; no cálculo estructural universal |
| Carro/contenedor | Transporte, lanzamiento y variantes de daño; explosiones solo donde tenga sentido |

Ancla no caerá al romper una textura cualquiera: solo apoyos que el sistema haya definido y mostrado. El edificio puede tener módulos intermedios perforables y núcleo resistente. No esconder todas las excepciones detrás de fachadas idénticas.

Un agujero modifica colisión, visión y rutas de aproximación. Los escombros grandes seleccionados pueden reutilizarse; polvo/cristales no. La colisión del agarre incluye la carga y el portador. Destruir un objeto sujeto libera el estado de ambas partes de forma segura.

La destrucción permanece durante la sesión. Reconstrucción por reinicio explícito; no cerrar una pared delante del jugador para recuperar rendimiento. Cambiar de zona puede descargar físicos menores conservando el estado jugable compacto.

## 6. Primer mapa: Distrito Fractura

Nombre provisional. Una región de prueba con tres espacios conectados y un horizonte convincente, no un mapa planetario detallado completo.

**Avenida Vertical:** tramos rectos para acelerar, cruces para cambiar rumbo, fachadas preparadas, azoteas y pasajes. Referencia visual: torre inclinada con antena. Evitar una cuadrícula sin decisiones.

**Puerto de Carga:** contenedores, carros, muros de almacén y grúas de silueta fuerte. Referencia visual: grúa doble. Objetos grandes disponibles donde los enemigos pesados se posicionan; no obligar a buscar munición durante minutos.

**Cantera Abierta:** espacio para velocidad y combate amplio, bloques preparados, desniveles y túneles cortos. Referencia visual: arco de roca. No prometer excavación volumétrica ilimitada.

Diseñar recorridos calle -> azotea -> cielo y puerto -> almacén -> costa que se conecten. Los huecos abren rutas alternativas. El escenario no debe obligar a volver al suelo entre acciones.

La expansión posterior conserva planeta visible/navegable y regiones detalladas, con estación orbital y asteroides. Cada destino debe aportar otra interacción: espacios interiores/exteriores en estación, objetos masivos y combate alrededor de rocas. La órbita no bloquea la entrega urbana.

## 7. Modos y ritmo de sesión

Tres conjuntos de reglas sobre el mismo mapa, no tres juegos separados:

- Libre: generar objetos y enemigos de un catálogo pequeño, elegir poderes, opciones de vida/energía infinita y reinicio de zona. No puntuación obligatoria.
- Confrontación: elegir rival o pequeña oleada; seguir la pelea por el distrito; reintento inmediato, no campaña.
- Desafíos: pruebas breves de trayectoria, lanzamiento o combinación de acciones; optativas y desbloquean como mucho presentación, no el vuelo básico.

El jugador elige si quiere refuerzos reactivos. Un medidor de intensidad, desactivable, avisa antes de enviar más enemigos; no castigar toda exploración con persecuciones infinitas.

Puntuación de estilo opcional: variedad, dirección, uso del entorno y cooperación. No premiar exclusivamente repetir la acción más rentable; excluir eventos duplicados. Nunca exigir subir de nivel para acceder a la fantasía principal.

Reaparición segura cerca del combate y reinicio que restaure actores, estados y colisiones. El juego distingue derrota de un desafío de borrar el sandbox. Controles guardados y acciones reasignables.

## 8. Cooperación diseñada, no cinemáticas dobles

VECTOR + PRISMA: uno desequilibra o lanza; el otro intercepta, protege o remata. VECTOR + VECTOR: relevos de golpes, flanqueo y lanzamientos encadenados. PRISMA + PRISMA: un escudo habilita aproximación y el otro crea presión desde un ángulo diferente.

Ejemplos por dificultad de implementación:

1. **Relevo:** un jugador lanza un carro, el otro lo intercepta con pinza y lo devuelve. Requiere la transición lanzado -> sujeto, no una animación cooperativa nueva.
2. **Apertura:** un jugador rompe el apoyo preparado del Ancla; el otro aprovecha el desequilibrio. Ninguna comprobación de clase es necesaria.
3. **Martillo y pared:** uno lanza al rival; el otro cambia su trayectoria con un golpe hacia un módulo rompible. Se combinan impactos independientes.
4. **Rescate opcional:** agarrar a un aliado derribado que lo acepte y apartarlo del peligro. Se pospone hasta validar transporte de jugadores.

Todos los desafíos centrales tienen solución individual. No añadir compañero IA obligatorio. Fuego amigo desactivado por defecto; modo caos opcional con acuerdo. Agarrar a un aliado requiere aceptación explícita temporal y permite soltarse. No basta que el anfitrión habilite caos para perder ese control.

Cooperativo en dos equipos con cámara individual es la dirección propuesta. Pantalla dividida, versus competitivo y viaje independiente a superficie/órbita quedan fuera de la primera entrega a dos jugadores. Ver COOP_PLAN.md para separación de alcance y pruebas.

## 9. Prioridades y entrega

| Entrega | Contenido jugable | Fuera de alcance |
|---|---|---|
| Base | Linux nativo, vuelo y cámara; prueba técnica temprana de dos conexiones | Lobby completo, campaña |
| Cadena física | VECTOR provisional, carro y pared; prueba de autoridad del agarre | Todos los edificios y cuerpos destruibles |
| Primer sandbox | Un rival volador, sector urbano pequeño, reinicio y ajustes | Cinco enemigos, planeta |
| Sandbox ampliado | Avispa y Ancla reutilizando reglas; puerto/cantera como expansión | Sistemas económicos, tráfico complejo |
| Dos héroes | PRISMA con cuatro construcciones; mismos objetos/daño | Generación arbitraria de objetos |
| Cooperativo jugable | Dos jugadores en una región; combos, errores de red controlados | Pantalla dividida, PvP, migración de anfitrión |
| Expansión | Orbital, Interdictor/Bastión y daño corporal solo según resultados | Crecer por acumular funciones |

La preparación de red no desplaza las primeras pruebas de diversión, pero los recursos principales no se consolidan sin una prueba pequeña de red. No asignar fechas ni porcentajes sin ejecutar la base.

## 10. Criterios creativos y jugables para Codex

Todos pendientes. Los números describen muestras de prueba, no garantías de rendimiento ni plazos.

- S01: distinguir VECTOR, PRISMA y las tres amenazas básicas en silueta sin depender del color; revisión humana en movimiento y a distancia de combate.
- S02: volar, frenar, agarrar, lanzar, atravesar y recuperar control sin abrir menús; veinte secuencias registradas.
- S03: diez encuentros del rival desde calle/azotea/cielo; anuncia ataque, respeta obstáculos y permite recuperación. Sin teletransporte ni bloqueo permanente.
- S04: Avispa, Rival y Ancla provocan respuestas diferentes explicables por el jugador; no basta cambiar daño/vida.
- S05: tres soluciones al mismo encuentro: cuerpo a cuerpo, objeto físico y construcción. Ninguna clase obligatoria.
- S06: perforación realmente transitable, pérdida de cobertura y fragmento reutilizable; veinte reinicios con actores/colisiones restaurados.
- S07: cada héroe completa solo el encuentro básico con todas sus opciones iniciales. Sin farmeo, compañero IA ni conexión externa.
- S08: repetir tres combos cooperativos sin cinemática ni inmovilizar cámara; validar también dos héroes iguales.
- S09: todos los efectos de cámara configurables y señal redundante para peligro; no parar el mundo online para dar peso a un golpe.
- S10: prueba humana breve con notas sobre control, poder, claridad, frustración y ganas de repetir; no sustituirla por una puntuación inventada por el agente.

## 11. Estado y siguientes acciones

Diseño documentado, no arte entregado ni funcionalidades implementadas. El próximo trabajo ejecutable sigue siendo inspeccionar Linux y obtener un proyecto empaquetado; después demostrar vuelo + carro + pared. No descargar nuevos frameworks o crear cinco enemigos antes de esa prueba.

Este documento contiene propuestas originales de diseño. Las capacidades de motor y las precauciones técnicas de multijugador se sustentan por separado en las fuentes de COOP_PLAN.md y LINUX_BASELINE.md.
