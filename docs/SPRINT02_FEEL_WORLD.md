# Sprint 2 · Velocidad, fluidez y periferia

Base jugable `20e6a00` del PR #18. El ejecutable sigue en el acceso **ImpactBeyond** de Steam.

| Vuelo | Mando / teclado | Regímenes |
|---|---|---|
| Crucero | Modo inicial | 12, 35 y 90 m/s |
| Travesía | Cruceta derecha / B | 180 y 320 m/s |
| Subir o bajar dentro del modo | R1/L1 / E/Q | La quinta marcha progresa desde la cuarta |
| Turbo y freno | Mantener L3/Shift y Cuadrado/X | Turbo ×1,15. El freno tiene prioridad. |

La avenida principal se prolonga por el extremo este (X = 620 m) hacia los apartaderos y el mirador costero. La ciudad y la costa comparten nivel y estado durante el recorrido de ida y vuelta. Los coches interactivos comparten el componente de agarre y regresan al reiniciar el mapa.

Se reutilizan CharacterMovement, SpringArm, rig y clips de Manny, ConceptCar de la plantilla de Epic y modelos `carro`, `taxi` y `carro_eletrico` del [Kit Cidade Low Poly CC0](https://github.com/kayquer/kit-cidade-lowpoly). Los materiales del terreno y mar, las formas de vegetación y la iluminación usan recursos instalados de Unreal y del proyecto. El mar es una superficie visual y la carretera conserva colisión independiente.

Las variantes aéreas se generan con `Tools/Author-FlightSpeedPoses.py`. `Tools/Author-Sprint02World.py` genera la periferia y los vehículos. El viento usa una voz procedural local y la estela tiene un límite fijo.

## Verificación

- Pasaron vuelo, transporte, aterrizaje de impacto, seis vehículos y laboratorio radial. El barrido de la cápsula se comprobó a 320 m/s de frente y en oblicuo, y a 368 m/s en oblicuo. El barrido de la carga se comprobó por separado en el circuito a velocidad baja.
- Primera medición Win64 DX12, 1080p, audio WASAPI activo, fuera de pantalla y con entradas simuladas. Alta al 100 % registró 143,94 FPS de media, p95 de 8,53 ms, p99 de 9,59 ms y 2.514 MiB de VRAM de pico. Rendimiento al 75 % registró 181,12 FPS, p95 de 6,68 ms, p99 de 7,64 ms y 2.035 MiB de pico. Hubo dos cuadros de más de 50 ms en cada vuelta cerca de la recarga. Se alcanzaron 367,99 m/s en la autovía. La preparación del coche y la plaza de impacto se hizo mediante saltos de posición del test.
- El paquete de las mediciones sustituyó un material en las colinas. El paquete de entrega incorpora el flag de instancias correspondiente. La sesión visible de 10–15 minutos con DualSense físico y validación del sonido siguen pendientes.
- El [laboratorio radial](IB_SPRINT02_PLANET_LAB.md) es una prueba aislada. El recorrido de ida y vuelta conserva el mismo estado del nivel cargado. Persistencia en disco y precarga de regiones quedan como contratos del sprint siguiente.
