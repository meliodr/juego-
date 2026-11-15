# Circuito jugable de la manzana

La build Windows abre en `Lvl_IB_C00`. Para probar con DualSense, usar el acceso **ImpactBeyond** de la biblioteca de Steam con Steam Input activado.

| Acción | Teclado | DualSense |
|---|---|---|
| Agarrar o soltar el carro | G | Círculo |
| Lanzar en la dirección de la cámara | T | Triángulo |
| Reiniciar la manzana | R | Cruceta abajo |

Acércate al carro, agárralo y llévalo al aire. Apunta hacia el panel central de la fachada y lánzalo. Si el impacto abre el panel, atraviesa el hueco y reinicia. El panel requiere un impacto del carro lanzado con impulso suficiente.

El carro usa el volumen gris de IB-C00, con física de cuerpo rígido de Unreal al soltar y lanzar. Mientras se sostiene, su volumen hace un barrido independiente del héroe. El agarre se suelta si un obstáculo deja la carga demasiado atrás. La pared es un panel preparado. Al romperse, se retira su colisión y se simulan cuatro fragmentos temporales. Reiniciar recarga el mapa y restablece la posición del héroe, el carro y la pared.

Las dos animaciones de vuelo se editaron sobre el maniquí de la plantilla. En suspensión, brazos bajos y piernas en ligera asimetría. En crucero, un brazo avanza por delante, el otro queda recogido y el cuerpo se inclina con la velocidad. Son ciclos provisionales. Se pueden regenerar desde `Tools/Author-FlightPoses.py` en el editor con el plugin de Python habilitado para esa sesión.

Para repetir la prueba del circuito sin escritorio, ejecutar `Tools/Test-FlightHeadless.ps1 -Circuit`. Usar el mismo script sin `-Circuit` para la regresión de vuelo.

La prueba automatizada verificó agarre, elevación de la carga, restitución de física al soltar, lanzamiento, apertura del panel, paso libre para la cápsula y recarga con un solo carro y un solo panel restaurados. También pasó la regresión de vuelo, que verifica que las animaciones se reproducen y que los tres niveles, frenado y colisiones básicas se mantienen. Sigue pendiente la batería extendida de transporte en esquinas, impactos y reinicios, junto con la valoración visual de las posturas.
