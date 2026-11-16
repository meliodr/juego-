# H01 y H02 en Unreal Engine 5.8

Referencias de rostro y traje para IMPACT: BEYOND. Los retratos son arte conceptual generado con la herramienta integrada de imágenes. Esta guía documenta un flujo de trabajo propuesto; queda por validarlo en el editor.

| H01 · Combatiente superhumano | H02 · Genio cósmico |
| --- | --- |
| ![Retrato actualizado de H01](h01-retrato-rostro-v2.png) | ![Retrato pulido de H02](h02-retrato-rostro-v4.png) |
| Rizos cortos, gafas rectangulares, bigote y barba corta. Traje grafito, placas claras y acentos rojos. | Rostro frontal de la versión aprobada, con rizos ordenados y barba perfilada. Traje grafito, hombros plateados, ribetes dorados y cristal esmeralda. |

## 1. Rostros y cuerpos

En el editor de Unreal, habilitar MetaHuman Creator y comprobar que estén disponibles los datos de Creator. Crear dos assets MetaHuman Character. Elegir una base por proporciones faciales y ajustar con las herramientas de Creator. Usar los retratos como guía de aspecto y comprobar el resultado de frente, perfil y tres cuartos con la misma luz neutra. Para H01, comparar cejas, nariz y contorno mandibular antes de añadir las gafas. Para H02, comparar el volumen de mejillas, los párpados y la línea de la barba con la versión pulida. [MetaHuman Creator en Unreal](https://dev.epicgames.com/documentation/metahuman/metahuman-creator-in-unreal-engine)

Si los controles de Creator se quedan cortos para el parecido, esculpir o capturar primero una malla facial neutra y usar MetaHuman Identity y Conform from Identity. Esta ruta recibe una malla 3D; los retratos PNG sirven para comparar el resultado. [Epic · From Mesh para 5.8](https://dev.epicgames.com/documentation/metahuman/from-mesh?application_version=5.8)

## 2. Trajes y accesorios

- **Base compartida.** Ajustar una prenda grafito con pesos del esqueleto, comprobar hombros y cadera en poses extremas y reutilizar el material base con instancias independientes. Modelar las placas como piezas rígidas y comprobar su unión con la prenda al mover los brazos.
- **H01.** Añadir placas claras y paneles rojos. Montar las gafas como malla rígida vinculada a la cabeza. Revisar reflejos y visibilidad de los ojos, además del encaje en nariz y sienes.
- **H02.** Añadir hombreras plateadas, ribetes dorados y cristal geométrico como pieza independiente con un parámetro emisivo. Probar el cristal en reposo y al activar el poder. Si se incorporan faldones con simulación, verificar el comportamiento durante el impulso y el frenado antes de ampliarla. [Epic · Prendas paramétricas y Chaos Cloth](https://dev.epicgames.com/documentation/metahuman/parametric-asset-setup-for-metahuman?lang=en-US)

## 3. Incorporación al personaje jugable

Conservar el Pawn, su cápsula, locomoción y controles. Probar primero H01 como presentación visual del personaje existente. Si el esqueleto de arte difiere, configurar IK Rigs de origen y destino y usar un IK Retargeter para las animaciones actuales. Revisar manos, pies y alineación del cuerpo. Después conectar la selección visual de H01 y H02 al evento `OnHeroChanged` del [selector S6](../../../ImpactBeyond/Source/ImpactBeyond/IB_S6H/README.md), tomando el descriptor del héroe como fuente del estado. [Epic · IK Retargeting para bípedos](https://dev.epicgames.com/documentation/unreal-engine/retargeting-bipeds-with-ik-rig-in-unreal-engine?lang=en-US)

- **Prueba en el mismo mapa** con ambas apariencias: caminar, despegar, girar, picar, frenar, quedar suspendido, aterrizar y cambiar de héroe. Revisar que el traje no atraviese al cuerpo en las transiciones y que la cámara de vuelo mantenga legible la silueta.
- **Rendimiento:** partir de UE Optimized y verificar con LODSync las transiciones de cara, pelo, barba y cuerpo a distancia de juego. Comparar resultados en el mismo recorrido y medir tiempo de cuadro y memoria antes de fijar calidad. [Epic · Materiales de MetaHuman](https://dev.epicgames.com/documentation/metahuman/metahuman-materials-and-textures?application_version=5.8) · [Epic · LODSync](https://dev.epicgames.com/documentation/en-us/metahuman/lodsync-component-for-unreal-engine)
