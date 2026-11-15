# IMPACT: BEYOND · MVP urbano

Win64 Development, DX12 SM6. Abrir **Steam → Biblioteca → ImpactBeyond** con Steam Input activado.

## Recorrido

1. Sal desde la Plaza de Inicio. Mantén el sprint en la avenida y pulsa salto cuando hayas ganado velocidad. Activa el vuelo durante el salto. Sube marchas con R1 o E y frena con Cuadrado o X.
2. Prueba una azotea. En vuelo bajo, mantén L2 o C hasta tocar una superficie caminable. Regresa a la avenida, gira en el Sector Coches y usa el deportivo de la zona.
3. Agarra, despega con el coche, apunta al panel de la fachada, carga hasta **ALTA** y suelta para lanzar. Atraviesa el hueco.
4. Regresa a la avenida, avanza por la transversal señalizada y cruza al Centro de Impacto. Salta o vuela por encima del círculo. Activa el impacto desde el aire sin una carga en las manos. Las cuatro piezas alrededor del círculo reaccionan a la onda. También se pueden agarrar y lanzar.
5. Reinicia y repite desde la plaza.

| Acción | Teclado y ratón | DualSense |
|---|---|---|
| Mover / cámara | WASD / ratón | Stick izquierdo / derecho |
| Caminar / correr / sprint | Ctrl izquierdo / WASD / Shift izquierdo + WASD | Stick suave / stick completo / mantener L3 |
| Salto o supersalto con velocidad de sprint | Espacio | Cruz |
| Entrar o salir del vuelo | F | R3 |
| Ascender / descender | Espacio / C | R2 / L2 |
| Subir / bajar nivel de vuelo | E / Q | R1 / L1 |
| Frenar en vuelo | X | Cuadrado |
| Agarrar o soltar | G | Círculo |
| Cargar y lanzar al soltar | T | Triángulo |
| Aterrizaje de impacto | V | Cruceta arriba |
| Rendimiento / Alta | F2 | Cruceta izquierda |
| Reiniciar mapa | R | Cruceta abajo |

## Implementación y respaldo

| Frente | Reutilización | Respaldo y prueba |
|---|---|---|
| Movimiento | CharacterMovement, acciones de la plantilla, Manny y poses de vuelo ya integradas | Salto convencional por debajo del umbral. Apex, avance, aterrizaje y paso a vuelo en `ImpactBeyond.Flight.Headless`. |
| Interacción y destrucción | IK, barrido, impulso y panel de PR anteriores. La malla de la plantilla ConceptCar se ajusta al chasis físico existente | Coche CC0 del Kit Cidade si falta el modelo principal. La colisión física de caja conserva el resultado repetible de las pruebas. |
| Ciudad | Mapa compacto, módulos Poly Haven CC0, HISM, geometría y acabados ya presentes | Se conserva un solo nivel con colisiones resueltas al cargar. El cielo urbano distante es decoración sin colisión. |
| Ventanas | InteriorCubemap de Unreal, material opaco e iluminación procedural en un máximo de 72 módulos próximos | Vidrio opaco convencional en el resto del distrito. Las habitaciones se representan en el material y carecen de colisión interior. |
| Impacto | Barrido y aterrizaje de CharacterMovement, impulso radial en cuerpos preparados, sonido procedural y anillo nativo | Un panel ligero etiquetado reacciona al umbral de impacto. El circuito de coche conserva su propio panel. |

El coche principal procede de `Templates/TemplateResources/Standard/ConceptCar/Content/Car` de Unreal 5.8. Los ejemplos de la carpeta Templates se encuadran en las secciones 1 y 5(b) del [acuerdo de licencia de Unreal](https://www.unrealengine.com/eula/unreal). El coche de respaldo usa `models/veiculos/carro.glb` del [Kit Cidade Low Poly CC0](https://github.com/kayquer/kit-cidade-lowpoly/tree/ed5ba11d026ffc6c990035d117c7886b40ed3fb7).

Los scripts de editor `Tools/Author-UrbanMVP.py`, `Tools/Author-MVPInteriors.py` y `Tools/Author-MVPImpactPose.py` regeneran la decoración, la prueba de ventanas y la pose de descenso. El paquete incluye el resultado de los scripts.

## Verificación

- Los barridos del mapa verificaron el circuito original, las anchuras de calle, el acceso a la plaza de impacto y su panel. La decoración usa 442 instancias en 25 grupos, con distancias de dibujo para accesorios pequeños.
- La prueba de vuelo verificó los cinco niveles, 20 supersaltos y 30 ciclos básicos de despegue. El apex medido de supersalto fue 15,00 m. La muestra que aterrizó sin activar vuelo recorrió 40,93 m; la muestra con transición a vuelo recorrió 32,93 m en su ensayo. La cápsula golpeó una pared de 4 cm a 105 m/s de frente y en ángulo.
- Pasaron 20 ciclos de agarre, transporte, lanzamiento, paso y reinicio, con 10 tiros por potencia. Los ensayos de impacto repitieron 10 descensos y comprobaron empuje sobre cuerpos etiquetados con exclusión de objetos no elegibles.
- La primera vuelta comparable del paquete, a 1920×1080 y escala interna del 100 %, midió 129,44 FPS de media, 7,73 ms por fotograma, p95 de 8,67 ms y p99 de 9,17 ms. Los dos fotogramas por encima de 50 ms ocurrieron al recargar. Es una captura automatizada DX12, sin límite de FPS y con saltos de posición de la prueba.
