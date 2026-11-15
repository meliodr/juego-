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
