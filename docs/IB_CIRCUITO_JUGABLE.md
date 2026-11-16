# Circuito jugable de la manzana

La build Windows abre en `Lvl_IB_C00`. Para probar con DualSense, usar el acceso **ImpactBeyond** de la biblioteca de Steam con Steam Input activado.

| Acción | Teclado | DualSense |
|---|---|---|
| Esprintar mientras te mueves | Mantener Shift izquierdo | Mantener L3 |
| Activar vuelo o dejarte caer | F | R3 |
| Agarrar o soltar el carro | G | Círculo |
| Cargar y lanzar en la dirección de la cámara | Mantener y soltar T | Mantener y soltar Triángulo |
| Reiniciar la manzana | R | Cruceta abajo |

Acércate al carro, agárralo y llévalo al aire. Apunta hacia el panel central y mantén el botón de lanzamiento hasta ver **ALTA**. Suelta el botón, atraviesa el hueco si el impacto abre el panel y reinicia. Una pulsación breve permite probar un lanzamiento ligero. El panel requiere suficiente velocidad de aproximación e impulso perpendicular al impacto.

El carro usa el volumen gris de IB-C00, con física de cuerpo rígido de Unreal al soltar y lanzar. Mientras se sostiene, su volumen hace un barrido independiente del héroe. El agarre se suelta si un obstáculo deja la carga demasiado atrás. La pared es un panel preparado. Al romperse, se retira su colisión y se simulan cuatro fragmentos temporales. Reiniciar recarga el mapa y restablece la posición del héroe, el carro y la pared.

Las poses de suspensión, crucero y carga usan el esqueleto del maniquí. En crucero, un brazo guía la trayectoria y el cuerpo se inclina con la velocidad. El despegue adapta `MM_Jump` en 0,43 s y tiene una variante con carga. El impulso de lanzamiento conserva el IK de las manos hasta soltar el chasis. Los clips se pueden regenerar desde `Tools/Author-FlightPoses.py` con el plugin de Python del editor habilitado para esa sesión.

Para repetir la prueba del circuito sin escritorio, ejecutar `Tools/Test-FlightHeadless.ps1 -Circuit`. Usar el mismo script sin `-Circuit` para la regresión de vuelo.

La regresión H01 en Windows verificó 30 despegues desde reposo, 20 secuencias de sprint con giro, frenada y despegue desde carrera, y 20 recorridos desde el agarre hasta el paso por el panel abierto y el reinicio del mapa. El carro se detuvo contra un voladizo que dejaba pasar la cápsula del héroe. Reiniciar durante una carga restauró un solo carro y un solo panel.

Se probaron 10 lanzamientos por potencia. Las velocidades máximas observadas fueron 16,05, 40,49 y 67,99 m/s. Los impactos de la prueba ligera conservaron la colisión del panel y los de la prueba fuerte abrieron el paso. La respuesta con un DualSense físico requiere una sesión por Steam Input.
