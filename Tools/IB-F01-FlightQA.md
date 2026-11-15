# IB-F01 · Prueba de vuelo sin escritorio interactivo

Base: `b65f9a6` de `prototype/w01-third-person-windows` (PR #4).

## Reproducción

Desde la raíz de una copia de trabajo aislada en Windows, con Unreal Engine 5.8.4 y el toolchain del proyecto:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tools\Test-FlightHeadless.ps1
```

`-EngineRoot` permite indicar una instalación existente. El script compila Editor Win64 Development y arranca un proceso propio mediante `UnrealEditor-Cmd.exe -game -nullrhi` con paso de simulación fijo de 1/60 s. Devuelve un error si falla la compilación, el proceso vence o el log carece de un resultado exitoso para la prueba exacta. El log queda en `ImpactBeyond/Saved/Logs/IB-F01-*.log`. El paso fijo es un parámetro de simulación.

La prueba reutiliza Unreal Automation, el Blueprint Third Person poseído, su controlador y el CharacterMovementComponent. El suelo y la pared son cajas transitorias en memoria, fuera del recorrido del mapa base. Se muestrean velocidad, modo y posición de la cápsula. El avance llama a `DoMove`, entrada pública de la locomoción del personaje. Las demás acciones se inyectan en el controlador local. Los archivos de prueba se pueden retirar revirtiendo el commit de QA.

## Revisión visual de «camina en el aire» · pendiente

En una build Win64 DX12, grabar el personaje de perfil y desde tres cuartos con desenfoque desactivado y efectos mínimos. Registrar el modo de movimiento, nivel, velocidad y el estado del AnimBP junto al vídeo. Tomar primero una referencia de un segundo de carrera en suelo. Hacer luego el mismo recorrido con despegue, dos segundos de estacionario, dos segundos de crucero por nivel, ascenso, descenso, freno y recuperación de carrera.

- **Fallo de animación** si se repite un ciclo completo de pisada izquierda/derecha en cualquier ventana estable de dos segundos con modo Flying, o si el AnimBP permanece en su estado de locomoción terrestre más de 0,25 s después del despegue.
- **Fallo de silueta** si estacionario y crucero al nivel 3 conservan la misma postura erguida de la carrera terrestre durante un segundo completo. Para hacer reproducible la comparación inicial, comprobar en perfil una diferencia de al menos 15° en la inclinación del torso entre estacionario y crucero al nivel 3. Es un criterio propuesto de revisión, sujeto a dirección de animación.
- **Fallo de transición** si reaparecen pisadas mientras la cápsula sigue en Flying durante el frenado o si, al tocar suelo y reanudar avance, continúa visible la pose aérea durante más de 0,25 s en Walking.
- **Fallo de control** si el personaje pierde altitud manteniendo solo avance en un tramo libre de crucero, si no responde a los controles independientes de ascenso y descenso, o si el freno no deja una pausa aérea controlable. Registrar por separado el aspecto visual y la trayectoria observada.

En el C++ de la base, el avance usa el yaw de la cámara, el ascenso y el descenso se controlan aparte y la rotación automática solo tiene tasa de yaw. El propio ticket describe la pose aérea como provisional. Hace falta observar el AnimBP y la pose en DX12 para atribuir el reporte a la animación, a la cámara o a la combinación de ambas.

## Resultado local · 2026-11-15

**EJECUTADO, PASS** para `ImpactBeyond.Flight.Headless`. El script salió con código 0. Entorno Windows, UE 5.8.4 CL 58902052, Editor Win64 Development, MSVC 14.44.35228, Windows SDK 10.0.26100.0, Intel Core i9-14900HX, RTX 4070 Laptop GPU (driver 32.0.16.2114). Ejecución con NullRHI.

| Caso | Resultado observado |
|---|---|
| Suelo → vuelo → suelo | 30 ciclos con ascenso de la cápsula y regreso a Walking mediante descenso |
| Carrera → salto → vuelo → caída → carrera | El mismo personaje pasó por los modos esperados y volvió a superar 400 cm/s en el suelo |
| Niveles | 900, 1800 y 3200 cm/s de velocidad real. 20 cambios alternos entre los niveles 2 y 3 |
| Deriva vertical | Al nivel 1, +7,17 cm entre los fotogramas de prueba 84 y 144 tras activar el vuelo durante un salto. En el segundo de crucero entre los fotogramas 179 y 239, +0,16 cm al nivel 1 y 0,00 cm redondeados al nivel 2 y 3 |
| Frenada | Desde el nivel 3, a los 20 fotogramas 23,196 m/s al soltar el avance y 15,332 m/s con freno y avance mantenidos. La velocidad con freno cayó por debajo de 0,05 m/s con modo Flying activo |
| Barrido de cápsula | Pared de 4 cm, aproximaciones frontal y oblicua con pico de 32 m/s. Máximo X relativo de 11963,0 y 11962,9 cm frente a la cara de pared en 11998 cm |
| Entradas simuladas de gamepad | R1 subió de nivel, L1 lo bajó y R3 activó la caída en el aire |

Evidencia local: `ImpactBeyond/Saved/Logs/IB-F01-a500337a4818457cbcf9369e947352a4.log`, desde `Test Started` hasta `Test Completed. Result={Success}`. La prueba y la revisión del log se hicieron en el mismo encargo.

**Pendiente para IB-F01.** Ejecutar la revisión visual anterior, validar ajustes de cámara y pose, repetir con el DualSense físico, ensayar esquinas y realizar el recorrido jugable de diez minutos con métricas de FPS en DX12. Decisión **CONTINUAR** con la revisión visual de IB-F01.
