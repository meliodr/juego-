# IMPACT: BEYOND — Personajes, enemigos y repertorios v0.3

Fecha: 2026-11-14. Estado: DISEÑO PROPUESTO, NO IMPLEMENTADO NI PROBADO. Linux nativo; assets/plugins RD$0; individual y futuro cooperativo para dos jugadores. Este documento prevalece sobre las propuestas anteriores de identidad, nombres, repertorio y enemigos. No cambia las reglas de portabilidad, permisos y pruebas de red de LINUX_BASELINE y COOP_PLAN.

## 1. Decisiones del usuario y límites de esta propuesta

- Retirar los nombres VECTOR y PRISMA. No son los nombres de los protagonistas. Usar internamente H01 y H02 hasta que el usuario elija nombres.
- Conservar los dos protagonistas ya presentados: combatiente superhumano de fuerza y genio/forjador cósmico masculino de energía esmeralda. No volver a la constructora genérica de la primera lámina.
- H02 conserva la referencia aceptada: cabello negro corto y rizado, piel morena, ojos verdes durante la manifestación, traje negro/grafito con placas plateadas, bordes dorados y foco geométrico verde. Sin fruta del diablo, sin icono de Linterna Verde ni traje copiado. El diseño del genio y sus manifestaciones es importante, no solo cuatro herramientas flotantes.
- Retirar al enemigo constructor/energético genérico. Esto NO elimina al héroe H02 ni sus poderes de creación. Los enemigos se organizan en robots, kaijus y superhumanos con comportamientos propios.
- Robots y kaijus tienen piezas desmontables con consecuencias funcionales; no solo un efecto de muerte. Daño orgánico estilizado, sin vísceras ni detalle anatómico explícito.
- Los nombres de enemigos y tamaños de este documento son propuestas creativas, no aprobaciones definitivas, marcas comprobadas ni dimensiones ya probadas.
- No importar código, descargar paquetes, contratar APIs o inventar Blueprints como consecuencia de este documento.

## 2. Lenguaje de combate compartido

El jugador combina desplazamiento, golpe, apuntado, agarre, carga y evasión. Se evitan ruedas de veinte habilidades, secuencias cinematográficas obligatorias y QTE de pulsación repetida. Las acciones principales conservan movimiento y pueden interrumpirse en ventanas explícitas. Ninguna acción oculta teletransporta al objetivo a través de un obstáculo.

Estado del portador: libre -> preparando acción -> sosteniendo -> lanzando/soltando -> libre. El agarre de una pieza todavía conectada usa una transición distinta: seleccionada -> dañada -> unión expuesta -> arrancado contestable -> desprendida. Un brazo intacto no se arranca por el mero hecho de pulsar agarrar cerca.

Cuando se sostiene algo, atacar lanza o usa el objeto según la acción elegida; no dispara también un poder. Soltar no lanza accidentalmente. Los enemigos resistentes se pueden derrotar por daño normal o desarme: no convertir cada encuentro en una secuencia obligatoria de puntos débiles.

Hay recuperación aérea y protección breve contra encadenamiento de incapacitaciones. Sin invulnerabilidad permanente al repetir evasión. En modo libre pueden desactivarse costes y daño al jugador. El vuelo normal nunca requiere combustible.

Los impactos tienen una curva de potencia limitada: la trayectoria importa, pero no se convierte toda la energía cinética en daño sin límite. El alcance, cancelaciones, costes y ventanas se ajustarán jugando; no son valores implementados.

## 3. H01 — Combatiente superhumano, nombre pendiente

### Diseño y carácter corporal

Hombre atlético y robusto, cabello oscuro con mechones hacia atrás, rostro visible. Base de traje grafito ajustada, placas claras en hombros y pecho, acentos rojos en brazos y laterales; conservar la dirección de la lámina anterior. Silueta de hombros anchos, manos y antebrazos contundentes. Sin capa obligatoria, casco que oculte su expresión ni propulsores: vuela por su poder.

Se mueve con decisión, pero no como un tanque lento: puños cerca del cuerpo al acelerar, brazos abiertos al frenar y hombro adelantado en embestida. Sus efectos son presión, polvo, aire y material roto, no proyectiles mágicos de colores. Fuerza transmitida por desplazamiento del objetivo, sonido y entorno.

### Repertorio

| ID | Movimiento | Intención/contexto | Resultado | Respuesta del enemigo / alcance |
|---|---|---|---|---|
| H01-A | Serie de ruptura | Golpes breves encadenados | Dos golpes preparan un tercero que desplaza; versión ascendente o descendente según dirección | Defensa, evasión y distancia; no un combo interminable |
| H01-B | Embestida | Atacar durante impulso | Cuerpo usado como proyectil; puede romper una pieza preparada y seguir al otro lado | Preparación visible y corrección limitada; fallar obliga a maniobrar |
| H01-C | Caza aérea | Confirmar persecución después de lanzar | Acelerar tras el objetivo con ayuda de orientación breve | No teletransporte, no atravesar paredes sólidas ni capturar desde cualquier distancia |
| H01-D | Presa y arrastre | Agarrar enemigo vulnerable | Transportarlo; cerca de una superficie rompible, arrastrarlo contra ella | Ventana de escape anunciada, duración limitada; choque resistente libera/resuelve, no penetra |
| H01-E | Desarme | Agarrar una unión expuesta | Arrancar brazo robótico, cuerno o pieza autorizada | El objetivo puede interrumpirlo; solo un arrancado activo por unión |
| H01-F | Intercepción | Agarre dirigido ante objeto entrante elegible | Atrapar o desviar un carro, dron o escombro importante y devolverlo | Anticipación clara, tamaño/velocidad máximos de diseño; no aspirar todo el escenario |
| H01-G | Caída de impacto | Ataque pesado orientado hacia abajo desde aire | Impacto descendente, desequilibrio y destrucción localizada | Área anunciada; no elimina toda una manzana |
| H01-H | Arma improvisada | Sostener poste/brazo/pieza apropiada | Barrido corto o golpe pesado; lanzamiento siempre disponible | Un conjunto pequeño de categorías/poses, no animación única por cada objeto |

Defensa: evasión y recuperación compartidas; guardia breve de antebrazos es ampliación, no exige cambiar el mapeo de apuntado. Los ataques fuertes dejan una apertura breve. Ninguna debilidad quita el vuelo normal.

Secuencia objetivo: interceptar dron -> lanzarlo al hombro del robot -> romper la unión -> arrancar el brazo -> usarlo contra una pared -> perseguir al rival por el hueco. Se prueba por sistemas, no como una animación prefabricada.

## 4. H02 — Genio cósmico, nombre pendiente

### Diseño y personalidad

Conservar el hombre de la referencia aceptada, no sustituirlo por otro héroe genérico. Cabello negro rizado, barba corta, mirada verde activada. Traje oscuro con hombros plateados, filigrana dorada sobria, cristal geométrico propio en el pecho y faldones divididos cortos. Los adornos rígidos y materiales sencillos son la primera implementación; no bloquear el juego por simulación de ropa.

Sus poderes parecen extensiones vivas de su voluntad: manos espectrales nacen detrás de hombros/antebrazos; remolinos y brazaletes luminosos conectan herramientas y cuerpo. Combina calma visual con gestos rápidos. Al volar deja una estela de genio; no se necesita anillo ni fruta para explicar el poder. El origen narrativo permanece pendiente.

### Repertorio

| ID | Movimiento | Intención/contexto | Resultado | Límite/contrajuego |
|---|---|---|---|---|
| H02-A | Brazos del genio | Ataque básico dirigido | Una mano espectral amplía el alcance del puñetazo; remate de dos manos | Alcance corto/medio y dirección clara, no rayo automático |
| H02-B | Mano soberana | Agarre a distancia | Sujetar, transportar, desarmar o devolver el mismo objeto que usa H01 | Mismas condiciones de vulnerabilidad, anclaje y dueño exclusivo |
| H02-C | Rebote | Escudo orientado y acción de devolución | Detener trayectoria y devolver un objeto significativo hacia el apuntado | La devolución requiere apuntar; escudo finito, no esfera invulnerable |
| H02-D | Martillo celeste | Ataque cargado de herramienta | Formar un martillo conectado al gesto y golpear objetivo o estructura | Preparación visible, corrección limitada, daño compartido |
| H02-E | Prensa | Dos manos alrededor de un objetivo | Sujetar brevemente, desequilibrar o tensar una pieza expuesta | No inmoviliza un kaiju completo; agarra una zona válida y puede romperse |
| H02-F | Lazo astral | Proyección de un lazo a objetivo autorizado | Retener a un enemigo ligero o dificultar una maniobra pesada | Ruptura por resistencia/daño; sin cadena de control ilimitada |
| H02-G | Paso astral | Evasión | Desplazamiento corto con silueta energética y estela; volver al vuelo | La presentación no autoriza atravesar paredes intactas ni terreno |
| H02-H | Despertar: seis brazos | Manifestación avanzada temporal | Cuatro brazos espectrales adicionales: patrón de golpes o agarre y golpe | Elegir patrón; no seis IA ni seis objetos físicos independientes en la primera prueba |
| H02-I | Manifestación colosal | Ampliación posterior | Avatar gigante de torso/manos detrás del héroe, capaz de agarrar una zona de un coloso o dar un golpe enorme | Espacio válido, blanco y presupuesto acotados; no agrandar la cápsula sin resolver colisiones |

Las primeras cuatro capacidades se apoyan en los sistemas de golpe, agarre, escudo y martillo ya previstos, pero su presentación debe ser claramente la del genio. Prensa, lazo y manifestaciones se añaden después. No simular ciudades de constructos ni ejércitos de criaturas como condición de la primera versión.

Formas de diseño: corporal (traje y poder normal), manifestada (brazos extra y torso energético superpuesto), colosal (evento de poder acotado). La forma astral aporta silueta y movilidad; no es invulnerabilidad/no-clip permanente. La energía limita herramientas potentes/persistentes, no volar. Modo libre ofrece energía y manifestación ilimitadas dentro del presupuesto técnico.

Secuencia objetivo: detener carro -> sujetarlo -> lanzarlo para descubrir una articulación -> acercarse en vuelo -> pinzar la pieza -> golpear con otro brazo. Crear es una forma de luchar con el mundo, no abrir un menú de objetos cada pocos segundos.

## 5. Robots: piezas industriales reutilizables

### RBT01 — Aguijón (aprox. 1–1,5 m)

Silueta triangular compacta, dos turbinas separadas y cañón central. Metal oscuro, carcasas cerámicas claras y luz ámbar; no confundir su color con la energía del héroe. Pasadas cortas, ráfaga anunciada y picado tras encender sus turbinas. Partes: turbina izquierda/derecha y cañón. Sin una turbina pierde control y debe aterrizar/caer; sin cañón deja de disparar. Se puede agarrar entero y lanzar contra otro enemigo. No convierte todo su casco en una bomba automática.

### RBT02 — Carcelero (aprox. 2,5 m)

Humanoide de torso estrecho con antebrazos de pinza desproporcionados y sensor hundido. Abre ambos brazos para anunciar una captura, intenta sujetar y arrojar; también empuja y patea. Partes: pinzas, placa pectoral y rodilla preparada. Arrancar la pinza elimina esa captura; romper la rodilla lo obliga a apoyarse o caer. Núcleo descubierto por daño a la placa. La pinza desprendida es una maza agarrable. Primer robot recomendado: extremidades rígidas preparadas y pocas articulaciones, no corte libre universal.

### RBT03 — Demoledor (aprox. 5–6 m)

Máquina asimétrica: martinete a la derecha, garra de carga a la izquierda, patas anchas con anclajes. Golpea el suelo, arranca objetos autorizados y los arroja, o fija pies antes de embestir. Módulos: martinete, garra, anclaje de una pierna y placa del núcleo. Pierde ataques exactamente cuando pierde el módulo. Dañar apoyo permite tumbarlo durante una ventana. Su martinete arrancado sirve de arma pesada; no continúa golpeando desde un brazo inexistente.

### RBT04 — Coloso de asedio (aprox. 18–25 m; posterior)

Andamio blindado caminante, espalda de grúa plegada, hombros artilleros y tres niveles de placas en pecho. Ataca con barridos, descarga anunciada y aplastamiento. Romper cañones elimina bombardeo; quitar placas permite atacar núcleo; perder un apoyo cambia postura y patrón. Reutiliza familias de materiales y módulos, pero requiere su propia prueba de escala. No promete colapso libre de la ciudad ni escalada manual sobre todo su esqueleto. No agrandar simplemente al soldado y multiplicar su salud.

## 6. Kaijus: partes grandes, violencia estilizada

### KAI01 — Rhazkar (aprox. 10–12 m)

Bestia terrestre bípeda de cuello corto, brazos gruesos, dos cuernos inclinados, caparazón oscuro con fisuras cobrizas y cola de maza. Silueta propia, sin copiar un monstruo de franquicia. Carga tras bajar la cabeza, barre con cola y golpea con ambos brazos. Piezas iniciales: cuerno principal, punta segmentada de cola y placa del hombro. Sin cuerno sigue embistiendo, pero pierde perforación; sin maza pierde el barrido pesado; sin placa expone zona vulnerable. Cuerno como lanza y segmento de cola como arma. Añadir separación de un brazo solo después de resolver su animación reducida. Cortes con tapa oscura mineral/energética, polvo y destello breve, sin anatomía explícita.

### KAI02 — Nheru (aprox. 18 m de envergadura; posterior)

Depredador alado de alas anchas como cuchillas flexibles, patas posteriores y cola estrecha; tonos pizarra/índigo y garganta luminosa. Picados curvos, coletazo y ráfaga de presión anunciada por expansión del cuello. Piezas: sección externa de ala, coraza de garganta y punta de cola. Dañar ala provoca aterrizaje forzado anunciado, no caída sin control infinita. En tierra conserva saltos, garras y ataques de cuello debilitados. Perder una parte cambia la fase sin exigir otra barra de vida completa. La placa alar grande puede convertirse en proyectil; no todas las membranas se vuelven objetos físicos.

### KAI03 — Uldra (aprox. 20–24 m; posterior)

Criatura de caparazón alto y patas robustas, con pocos nódulos cristalinos voluminosos que brillan antes de emitir pulsos. No crea objetos ni ejércitos: su energía es una descarga biológica localizada. Martillea con cola, proyecta un haz desde un órgano externo y anuncia pulsos de área. Romper cada nódulo cancela el ataque asociado o deja una zona segura; retirar placas descubre tejido energético estilizado. Cristales desprendidos pueden usarse como proyectiles una vez, con un comportamiento acotado. No regeneración instantánea que borre los avances del desarme.

## 7. Superhumanos: rivales, no copias del forjador

### SH01 — Malek (nombre provisional)

Hombre fuerte, pelo corto, rostro con cicatriz y traje granate/carbón con antebrazos descubiertos y piezas claras mínimas. Pelea con hombro, agarres, golpes cargados y vuelo. Finta una embestida, frena y busca un agarre; puede usar los mismos carros del jugador bajo las mismas reglas. Una embestida fallada deja apertura. El daño visible es traje/armadura y postura; no necesita el sistema de despiece de kaijus. Primera IA superhumana: reutilizar acciones del héroe, no acceso perfecto a posición/entradas del jugador.

### SH02 — Saira (nombre provisional; posterior)

Mujer atlética de silueta estrecha, cabello corto hacia atrás, traje cobalto oscuro con marcas claras en pantorrillas y antebrazos. Velocista aérea: realiza pasadas en arco, amaga con una estela y golpea en la siguiente aproximación. Debe preparar y terminar sus curvas; no giro instantáneo ni persecución imposible de evadir. Anticipar una pasada permite agarrarla o poner un objeto en su ruta. No quitarle el vuelo por destruir un prop genérico del escenario.

### SH03 — Odran (nombre provisional; posterior)

Superhumano corpulento con zonas del torso que se iluminan al acumular impactos y traje ceniza con protecciones segmentadas. Acumula parte de los golpes frontales al adoptar una guardia anunciada y devuelve una descarga corta. No es invulnerable ni anula todo daño cinético: daño limitado, acumulación finita y ventana de liberación. Se contrarresta flanqueando, agarrando durante su preparación o cambiando el ritmo; no obliga a usar un personaje específico. No construye herramientas, barreras ni criaturas.

## 8. Desarme como sistema, no premio decorativo

Contrato propuesto por pieza: PartId estable; unión y hitbox; salud/estado; condición de arrancado; ataques que habilita; representación conectada y separada; tipo de objeto resultante; estado persistente. Especificación conceptual, no clases implementadas.

Estado: intacta -> dañada -> unión expuesta -> desprendida o inutilizada. Daño suficiente puede romper directamente una pieza frágil. El núcleo/cuerpo puede seguir recibiendo daño para evitar un puzle obligatorio en todos los encuentros.

Al desprender: una sola pieza nueva; desactivar ataque/colisión anterior; cambiar capacidades y animación; producir objeto interactivo solo si está en presupuesto. Si no hay presupuesto, usar la misma consecuencia funcional y una representación no interactiva señalada; no prometer agarrar un fragmento cosmético. Piezas interactivas prioritarias nunca desaparecen mientras un jugador las sostiene.

No obligar a cada enemigo a seguir vivo tras perder cualquier combinación de extremidades. Definir para cada enemigo unas pocas combinaciones válidas; las demás provocan derrota controlada. No duplicar la complejidad de animación por permitir cientos de combinaciones inútiles.

## 9. Cooperativo y encuentros

Ambos héroes deben funcionar solos y se permiten elecciones repetidas. Ejemplos: detener la carga del robot mientras otro arranca la pieza; lanzar un cuerno para que el compañero golpee o redirija; pinzar un ala expuesta mientras el otro impacta; hacer un relevo de carro sin dos propietarios simultáneos.

Presa/arrancado concurrente de una misma pieza: servidor autoritativo elige una solicitud; la otra recibe una cancelación legible. Pérdida de una extremidad desactiva su ataque para todos. Cada desprendimiento tiene identidad/versión para evitar duplicados al reconectar o repetir mensajes. Las manos espectrales no son un segundo servidor de física. No hit-stop ni cámara lenta global por cada puñetazo online; efectos locales no cambian el tiempo compartido.

Un kaiju puede pelear con robots si se configura hostilidad entre facciones. Un selector simple de escena fija equipos, enemigos, cantidad y agresividad; no necesita un director narrativo complejo. Ajustes por defecto conservadores y límites medidos, sin prometer cien actores activos.

En solo no hay acciones que requieran obligatoriamente dos héroes. Agarrar al compañero requiere aceptación y puede cancelarse. Fuego amigo desactivado por defecto. Colosos no inmovilizan al jugador en una cinemática larga.

## 10. Alcance y comprobaciones

Primera incorporación: H01 con vuelo, golpe, agarre, lanzamiento e impacto + Carcelero con dos piezas separables + pared. Malek añade duelo aéreo reutilizando acciones. H02 empieza con brazos, mano, escudo y martillo; probar diferencias sobre los mismos objetos. Rhazkar mediano con tres piezas es el primer laboratorio kaiju. No implementar diez enemigos antes de jugar.

Después: Aguijón/Demoledor, técnicas avanzadas del genio, otras variantes. Coloso, kaijus alados, forma colosal y SH02/SH03 son ampliaciones condicionadas a diversión y recursos. Ningún tamaño es un compromiso hasta probar cámara, vuelo, animación, colisión y rendimiento.

Pruebas nuevas para Codex (pendientes):

1. Un ataque que usa un brazo deja de estar disponible al separarlo; 20 ciclos de reinicio sin ataques fantasma.
2. Dos intentos simultáneos sobre la misma pieza producen un único desprendimiento y dueño. Registrar también reconexión/reinicio.
3. Transportar una pieza durante vuelo no la atraviesa contra una pared resistente ni cambia su peso de juego entre clientes.
4. La pieza rota vuelve como objeto agarrable cuando corresponde y no aparece duplicada en cuerpo/mundo.
5. Prensa, lazo y agarre respetan resistencia; no bloqueo permanente del enemigo por alternar personajes.
6. Un enemigo alado dañado pasa a su estado terrestre y conserva colisión/capacidades válidas.
7. Ninguna manifestación del genio genera daño por un brazo puramente visual ni a través de un obstáculo no destruido.
8. Probar derrota por daño directo y por desarme, solo y cooperativo; ninguna estrategia es obligatoria por un error de invulnerabilidad.
9. La opción sin separación orgánica conserva salud, ataques cancelados y colisión jugable compartida.
10. Prueba humana: claridad del objetivo agarrable, disfrute de movimiento, sensación de diferencia entre héroes y ganas de repetir. Logs y láminas no sustituyen jugar.

No se implementó ni compiló ninguna de estas capacidades durante esta revisión. Las referencias visuales son las imágenes del usuario y los conceptos aceptados de esta conversación; no se han subido sus binarios al repositorio público.
