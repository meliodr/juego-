# S7 — Corrección de presentación tras prueba humana

El usuario jugó el paquete `s7-package-20261116T060244Z`, abierto desde Steam, y lo calificó **2/10**: animaciones extrañas y conjunto de personaje, cámara y escenario insuficiente para validarlo como juego. Aclaró que el desplazamiento es rápido pero la animación y los efectos no acompañan. La corrección conserva esa rapidez; no intenta resolver la presentación reduciendo las velocidades.

## Causas confirmadas por código

- El pawn integrado reproducía Walk a ritmo fijo hasta 700 cm/s, aunque su velocidad normal era 650 cm/s; el sprint de 1800 cm/s reproducía Jog a ritmo fijo.
- Salto, caída, despegue y aterrizaje no tenían estados visuales completos en el pawn integrado. Las poses de vuelo habían perdido la inclinación y el balanceo que aplicaba el controlador anterior.
- La selección de pose aérea dependía de la marcha, incluso cuando la travesía producía una velocidad diferente.
- La mezcla de transformaciones en espacio de componente acortaba segmentos del esqueleto durante rotaciones grandes. Se debe mezclar en espacio local y reconstruir la jerarquía antes del IK.
- Las estelas eran dos líneas cian con una historia de 0,25 s: su longitud crecía mucho con la velocidad. El impulso sonoro podía activarse por pulsar turbo sin aceleración real.
- La presentación urbana heredaba un gran plano asfaltado y edificios separados en retícula; cargar materiales y mallas no bastaba para producir una escena convincente.

Son hallazgos de código y metadatos. No equivalen a una inspección visual de contenido protegido ni a una nueva aprobación humana.

## Reutilización y alcance

Reutilizar el BlendSpace bidimensional `BS_Idle_Walk_Run` de la plantilla (dirección −180°..180°, velocidad 0..600 cm/s), clips Jump/Fall/Land y las poses ya autorizadas. Recuperar la inclinación y el balanceo existentes en el marco de gravedad local. Conservar controles, cinco marchas, velocidades, daño, guardado, ciudades e identidad de objetos. Corregir el mezclado de poses sin sustituir el sistema de agarre.

Reutilizar la presentación de aire/audio y los materiales, fachadas y vegetación disponibles. El relleno urbano tendrá presupuesto limitado y exclusiones para los encuentros existentes. No se añaden compras, plugins ni bibliotecas por anticipación. Los recursos Fab y derivados siguen fuera del repositorio público.

## Aceptación

1. Compilar Editor/Game y ejecutar la regresión de mezcla, estados de presentación y controles sobre la nueva revisión.
2. Probar transiciones tierra→salto→vuelo→crucero→freno→aterrizaje, repetidas y en gravedad local. Evitar clips de caminar en el aire, poses que se reinician continuamente y deformación entre estados.
3. Empaquetar y verificar DX12, contenido, encuentros y recorrido de rendimiento. Medir cambios de coste; no trasladar los 59,93 FPS de la versión anterior a esta candidata.
4. Abrir la nueva versión mediante Steam con Steam Input. La evaluación humana de animación, cámara, sensación de velocidad y escena decide si la presentación mejoró; una prueba automatizada no puede conceder esa aceptación.

El paquete anterior permanece como respaldo. S8 no avanza durante esta corrección.
