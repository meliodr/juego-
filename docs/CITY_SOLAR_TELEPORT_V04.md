# IMPACT: BEYOND — Ciudad destructible, sistema solar compacto y tercer héroe (v0.4)
Fecha: 2026-11-14. **ESTADO: diseño/selección documentada; NADA COMPILADO NI PROBADO en Linux.**
Prioridades del usuario: sandbox sin historia obligatoria; sensación de ciudad densa como Spider-Man, pero con destrucción; explorar un planeta pequeño, despegar al espacio, ver un sol NO visitable, visitar al menos una luna y otros dos planetas; personaje de teletransporte; reutilizar todo lo útil, crear solo lo imprescindible; Linux nativo, compras RD$0, cooperativo 2 jugadores posterior.

## 0. Precedencia
Este documento actualiza **dirección de mundo, recursos urbanos/espaciales y roster de 3 héroes**; complementa COMBAT_ROSTER_V03.md (H01/H02 siguen válidos), GAMEPLAY_SPEC.md (vuelo/agarre), COOP_PLAN.md (red) y LINUX_BASELINE.md (plataforma). No autoriza contenido de GTA/Marvel ni licencias no confirmadas. Nombres artísticos H01/H02/H03 pendientes; VECTOR y PRISMA están rechazados. No reintroducir enemigo constructor genérico.

## 1. Promesa y bucle jugable
Poder recorrer una ciudad vertical reconocible; despegar, acelerar y perseguir; agarrar vehículo/robot/enemigo o conjurar una construcción; lanzar, perforar una fachada y aprovechar el hueco; repetir con nuevas combinaciones. **Moverse sin misión debe ser divertido**. No hay campaña obligatoria ni grind para desbloquear vuelo. Sandbox con generación de enemigos/objetos y reinicio de zona.
Tres héroes que funcionen de manera autónoma y compartan daño, colisiones, agarre y destrucción:
- **H01 Fuerza superhumana**: vuelo rápido, embestida, golpes, agarrar carros/personas preparadas y arrancar partes del mundo.
- **H02 Genio cósmico esmeralda**: diseño masculino ya aprobado; invoca manos, escudo, puño, martillo, pinza, manifestaciones posteriores de genio. NO fruta del diablo, NO traje/icono de Linterna Verde.
- **H03 Espaciodesplazador (nombre pendiente)**: estética original negro/índigo con cortes de luz azul-violeta y silueta atlética; sin apariencia o símbolos de Nightcrawler. Comparte movimiento básico para explorar y viaje espacial, pero domina el combate con saltos cortos y reposicionamiento.

## 2. Diseño de ciudad: densidad antes que kilómetros
Primer escenario: **distrito urbano de prueba de ~1–2 km por lado como objetivo de contenido, no capacidad medida**, con avenida larga de alta velocidad, 3–5 alturas de edificios, azoteas jugables, callejones, tráfico/props sencillo, zona industrial con objetos pesados, un parque y conexión a periferia abierta. Evitar kilómetros de calles vacías. Centro comercial de rascacielos, anillo portuario y terreno exterior. Siluetas reconocibles desde el cielo, referencias de altura para navegación.
No reproducir Nueva York/Marvel ni mapear GTA. La referencia Spider-Man es **densidad, verticalidad y recorrido**, no copiar edificios/texturas.

### Destrucción de ciudad (realista para RD$0)
- **Capa V (visual)**: cristales, marcas, chispas, polvo.
- **Capa P (perforable)**: paneles de fachada y muros de sección preparada con colisión intercambiable; la abertura deja paso.
- **Capa I (interactuable)**: postes, carros, placas, vigas, fragmentos grandes con interfaz de agarre.
- **Capa C (colapso)**: 1–3 edificios de demostración con colapso parcial controlado, no simulación estructural universal.
- Entorno distante = mallas/LOD, sin Chaos activo para cada edificio. Estabilidad y presupuesto de fragmentos prioritarios.
- Las fachadas P deben aparecer repartidas por el distrito para dar sensación de mundo destructible, no limitarse a un único muro de laboratorio. No prometer que TODOS los edificios son enteramente fracturables. Los estados deben persistir durante la sesión, y reinicio restaurarlos.
- Mantener interacción bidireccional: vehículo puede romper pared; bloque/placa desprendida puede ser nuevo proyectil. Validar hueco sin colisión fantasma.
- Para un futuro de dos jugadores, servidor decide estados de daño/huecos y reserva de objeto; partículas locales sin daño.

## 3. Sistema solar compacto, jugable y escalable
**Contenido objetivo, no mapa ya construido:**
1. **Planeta inicial:** esfera ficticia pequeña y visualmente continua; distrito urbano muy detallado + periferia/biomas simplificados, otras costas visibles desde altura.
2. **Luna:** destino visitable, menor gravedad, cráteres, estación/minas y rocas agarrables.
3. **Planeta rojo:** destino visitable, cañones, bases abandonadas y variantes de robots; pequeña región de aterrizaje real.
4. **Planeta tormentoso/azulado:** destino visitable, mesetas/estructuras flotantes o tecnología, enemigos y objetos interactivos; pequeña región de aterrizaje real.
5. **Sol:** astro visual luminoso y fuente de iluminación; **no es destino, no requiere geometría aterrizable ni física solar**.

No fabricar a mano superficies infinitas. Separar: (A) mapa local con física y destrucción, (B) proxy esférico/LODs planetarios e iluminación de atmósfera, (C) navegación espacial con cuerpos distantes. Guardar identidad de planeta/región + transformación local; no poner todas las ciudades y planetas con miles de físicas en una escena.
**Continuidad perceptiva**: ascenso por atmósfera -> representación orbital con streaming -> recorrido espacial -> acercamiento -> zona de aterrizaje. Las transiciones pueden usar precarga, cambio de representación y montaje a gran velocidad; no afirmar continuidad geométrica total sin demostrarla. Primer laboratorio: ciudad -> órbita -> regreso. Después luna; después planetas 2/3. Distancias y aceleración COMPRIMIDAS por ritmo de juego; no miles de segundos de viaje. Entre cuerpos un modo de travesía de alta velocidad con referencia visual, no teletransportación obligatoria.
Tamaño del planeta se calibra con horizonte, radio de curvatura y altura de edificios; NO fijar radio definitivo antes de prueba. Georreferencia/local origin para escena, doble precisión para coordenadas globales; física solo local. Bajo Vulkan medir streaming/VRAM.

## 4. H03: kit de teletransporte jugable (no clon de Nightcrawler)
Diseño original: persona ágil con ropa grafito/índigo segmentada, líneas azul-violeta como fracturas espaciales y partículas angulares; sin cola, orejas o traje copiado. Aparece un 'desfase' breve, no el humo o animación de otra franquicia.
**Set inicial reutilizable:**
- **Blink direccional**: salto de distancia corta al punto válido de mira; conserva orientación y una fracción configurable de inercia.
- **Blink de altura**: variante hacia arriba/en diagonal para escapar y enlazar combo aéreo.
- **Contra-blink**: teletransporte evasivo breve tras señal de amenaza, con ventana y recarga.
- **Golpe de reaparición**: salida del blink -> golpe con impulso limitado; no telefrag ni atravesar cuerpos para daño automático.
- **Marca y regreso (después)**: ancla de retorno en región cargada, no teletransporte instantáneo interplanetario.
- **Intercambio posicional (experimento posterior)**: intercambiar con un objeto marcado y autorizado, sujeto a colisión/red.
Reutilizar **Teleport/TeleportTo** nativo de UE y traces/capsule tests; no instalar un plugin antes de probar la solución nativa. Validar volumen de cápsula en destino y objeto sostenido, techo/piso/pared, distancia y línea de visión según modalidad. Nunca aparecer dentro de sólido, región sin colisión cargada, posición ilegal o personaje sin consentimiento. Si falla el punto, reubicar en uno seguro o rechazar sin gastar recursos. Para blink a través de estructuras, solo atravesar rutas explícitamente permitidas o regiones ya precargadas.
**Sensación:** tres blinks encadenados posibles con límites en desafío; en modo libre se puede eliminar coste. Poder seguir volando/explorando como otros héroes para que el viaje orbital no sea exclusivo de H01/H02. Para cooperativo validar destino en servidor, predecir VFX local y replicar estado de reaparición.
**NO hacer primero:** cortes del tiempo, portales persistentes con render recursivo, teletransporte interestelar instantáneo, intercambio sin límites o sistema de partículas gigantes.

## 5. Recursos evaluados — usar lo mínimo
| Necesidad | Candidato gratuito real | Estado y decisión |
|---|---|---|
| Ciudad de rascacielos modular | Quaternius Downtown City MegaKit STANDARD; ~60–70% del catálogo total disponible gratis, CC0. https://quaternius.com/packs/downtowncitymegakit.html | **PRIMERA EVALUACIÓN**. Contar meshes realmente descargados; edición Source pagada DESCARTADA. Materiales/colisiones Unreal exclusivos del Source NO asumidos gratuitos. |
| Calles y props | Kenney City Kit Roads CC0 https://kenney.nl/assets/city-kit-roads | **RESERVA/COMPLEMENTO** por escala/estilo. |
| Industria y vehículos | Kenney City Kit Industrial + Car Kit CC0 https://kenney.nl/assets/city-kit-industrial | **COMPLEMENTO** para puerto y objetos lanzables. |
| Distribución urbana | PCG nativo UE https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-overview | **USAR** primero sin plugins adicionales. |
| Redes de calles/parcelas más complejas | PCG Extended Toolkit, MIT, autores mencionan Linux https://github.com/PCGEx/PCGExtendedToolkit | **RESERVA** sólo si PCG nativo queda corto. Validar versión y empaquetado Linux. No edición Pro. |
| Fracturas | Chaos UE integrado | **USAR** para paneles preparados. |
| Teleportación | K2_TeleportTo + trazas/cápsula + World Partition streaming source https://dev.epicgames.com/documentation/unreal-engine/BlueprintAPI/Transformation/Teleport | **USAR** como base, luego prueba Linux/red. |
| Arquitectura planeta -> luna | Ephemera, MIT, UE5.8, código de referencia espacial https://github.com/0xWerz/ephemera | **LABORATORIO APARTE**. Publica sólo ejecutable Windows; Content reconstruible; plugins Win64 opcionales. No certifica Linux. |
| Generador planetario | Cosmic Architect, MIT, UE 5.7 https://github.com/narratech/TFG-Bravo-Perez-Rubio | **ALTERNATIVA** si Ephemera no ahorra suficiente trabajo; versión Linux/5.8 pendiente. |
| Planetas, rocas, espacio | Quaternius Ultimate Space Kit (92 modelos CC0) https://quaternius.com/packs/ultimatespacekit.html | **EVALUAR** para visuales/prototipo, sin inventar interior/destructibilidad. |
| Estaciones y naves | Kenney Space Kit (150 modelos CC0) https://kenney.nl/assets/space-kit y Space Station Kit (90 CC0) https://kenney.nl/assets/space-station-kit | **EVALUAR DESPUÉS** de primera órbita. |
| Planetas/datos de referencia | NASA imágenes: medios generalmente no sujetos a copyright EE.UU., pero normas/marcas/terceros varían https://www.nasa.gov/nasa-brand-center/images-and-media/ | **NO NECESARIO** para primer prototipo; preferir materiales originales/CC0. |
| Mods de GTA V/Spider-Man | Scripts/assets de juegos ajenos | **NO IMPORTAR**. GTA scripts y assets dependen de motor distinto; no asumir permiso a extraer modelos, mapas, personajes ni código. Tomar ideas de jugabilidad, no archivos protegidos. |
| City Sample entero | Epic Fab, gratuito pero pesado; ficha "Allows usage with AI: No" | **NO INTEGRAR COMO BASE**. Puede estudiarse documentación pública; revisar licencias antes de cualquier uso con IA. |
| Blender/Houdini externo | Blender gratis; CityGenerator de AlexKorotkin basado en Houdini 22 + City Sample | **NO DEPENDER** de un pipeline con requisitos de licencias/herramientas no comprobadas. |

## 6. Orden obligatorio para Codex (reutilización antes de escribir)
**A. Linux base**: detectar instalación UE/clang/Vulkan/GPU/VRAM; proyecto ligero y ejecutable empaquetado, sin asumir editor del usuario accesible.
**B. Verticalidad urbana**: importar 1 set CC0, construir 4–8 manzanas densas con calles y azoteas, un circuito de vuelo y techo alto; comprobar escala/colisiones/LOD.
**C. Física y destrucción**: carro -> agarrar -> lanzar -> hueco real en fachada -> recuperar objeto/cascote; una pieza de robot desmontable.
**D. Enemigos + héroes**: primer superhumano rival y robot, H01; integrar H02 con sus manifestaciones iniciales; H03 blink seguro antes de construir niveles planetarios con él.
**E. Mundo exterior + órbita**: zona exterior para acelerar; laboratorio separado con esfera, referencia solar, atmósfera y retorno; integrar sólo lo verificado.
**F. Luna y dos planetas**: primero cuerpos visuales con navegación; luego 1 región jugable diferenciada por cuerpo. Sol sólo visual. Planeta no necesita todas sus celdas al mismo tiempo.
**G. Cooperativo**: pruebas locales desde B/C de dos jugadores; LAN funcional después del bucle divertido. Servidor autoriza daño, hueco, agarre, blink y estado planetario; no PvP hasta cerrar PvE.
La exploración al espacio nunca debe bloquear el desarrollo del sandbox urbano.

## 7. Pruebas de aceptación (a crear; no realizadas)
- **CITY-01** 10 min de vuelo sin misión entre edificios y azoteas; usuario puntúa control/velocidad/diversión; evitar pasillos visualmente repetitivos.
- **CITY-02** 30 lanzamientos de carro contra fachadas desde 3 direcciones; hueco transitable, sin colisión fantasma, reinicio consistente.
- **CITY-03** 10 circuitos con precarga, ida y vuelta sin desaparecer edificios/colisiones cercanos.
- **TP-01** 50 blinks: calle, tejado, cornisa, en vuelo, cerca de pared, destino bloqueado, objeto transportado; 0 posiciones dentro de sólidos.
- **TP-02** 20 blinks repetidos con RTT artificial al menos en 2 conexiones; no duplicar daño ni quedar desincronizado.
- **PLANET-01** despegar -> ver planeta y sol -> órbita -> regresar a edificio marcado 10 veces, sin perder orientación/estado.
- **PLANET-02** luna y dos planetas visibles; cada uno con región jugable real antes de anunciarlo como visitable.
- **COOP-01** dos jugadores ven el mismo hueco/pieza lanzable y mismo destino seguro de teleport; estado autoritativo.
Medir hardware/resolución/frame times/VRAM reales, no declarar 60 FPS por documentación. Registrar clones de prueba en Linux con licencias y versiones exactas.

## 8. Estado y regla de parada
Este repositorio tiene **documentos, no juego**. Ningún recurso nuevo está importado, no hay paquete Linux ni planetas jugables. Candidatos "EVALUAR" no son dependencias aceptadas. Ante incompatibilidad: usar primero APIs/plantillas nativas y alternativa más simple. **Detener búsqueda cuando un paquete CC0 gratuito aprobado cumple la prueba de una sección**; no ampliar catálogo en lugar de jugar.
