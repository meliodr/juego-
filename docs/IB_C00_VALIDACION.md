# IB-C00 — Validación de la manzana gris

## Ampliación para vuelo

`Tools/Extend-IB-C00.py` amplía el pavimento hasta X=620 m y reutiliza geometría y materiales de Third Person para añadir una avenida de 26 m, una transversal de 30 m, una plaza, dos pasos elevados y edificios de hasta 78 m. Los actores del recorrido de agarre y destrucción conservan sus posiciones. El escenario sigue en fase de geometría de prueba.

El vuelo adapta el cálculo de inclinación por trayectoria y velocidad angular de [ManOfSteel, UFlight](https://github.com/ProdiG66/ManOfSteel/blob/d107fd72606f33fc7062e2e473bfab330ee403ea/Source/ManOfSteel/Private/Components/Flight.cpp). Se conserva la [licencia MIT del donante](LICENSE-ManOfSteel.txt). La cámara, el control del personaje y las animaciones usan la plantilla que ya estaba integrada.

## Recursos integrados

- **ManOfSteel, `d808f32`.** Se portaron partes de `UFlight::SetCamLag`, `CheckFlightTime`, `OnHit` y `ASuperman::MoveCharacter` para los niveles 4 y 5. El nivel 4 llega a 72 m/s y el 5 aumenta ese límite progresivamente hasta 105 m/s en tres segundos. Se seleccionan con E/R1 y se bajan con Q/L1. Frenar con X/Cuadrado conserva el control del personaje.
- **REPIRK.** El lanzamiento usa la secuencia de liberación, impulso independiente de la masa y vibración de [`PhysicsGrabberComponent`](https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5/blob/main/Private/PhysicsGrabberComponent.cpp), con su [licencia MIT](LICENSE-REPIRK.txt). Los contactos de las manos y el barrido del chasis siguen definidos por el circuito.
- **Kit Cidade Low Poly, `ed5ba11`.** Cuatro mallas GLB y sus materiales se importaron como recursos de Unreal y se colocaron en siete lotes. Cada instancia tiene colisión de caja para sobrevolar y aterrizar. `Tools/Import-CityDonor.py` regenera la importación apuntando `IB_CITY_DONOR` a una copia del [repositorio de kayquer](https://github.com/kayquer/kit-cidade-lowpoly/tree/ed5ba11d026ffc6c990035d117c7886b40ed3fb7). [Licencia CC0](LICENSE-CityKit.txt).

Quedan por integrar las dependencias de combate, visión térmica, montajes y efectos del donante. En el Git del donante hay archivos de animación que son punteros LFS, incluido `A_Heatvision_Loop.uasset`.

## Fachadas urbanas

El distrito reutiliza ventanas, portales, cornisas y revestimiento PBR del kit [Modular Urban Apartments Facade de Poly Haven](https://polyhaven.com/a/modular_urban_apartments_facade), con [licencia CC0](https://polyhaven.com/license). Se seleccionaron cuatro módulos del glTF de 1K para vestir 28 edificios. Los módulos se renderizan en cuatro grupos de instancias jerárquicas y las colisiones siguen en los volúmenes del circuito. La escenografía low poly previa se oculta en el mapa. Los cristales se importan como superficie opaca oscura.

Para actualizar la colocación en el editor, ejecutar `Tools/Apply-RealisticFacades.py`. La primera importación requiere que `IB_POLY_FACADE` apunte al archivo glTF descargado con su binario y texturas. Con los módulos importados, el script reutiliza los recursos ya guardados.

**Estado:** EJECUTADO para geometría y colisión automatizada. Aceptación jugable pendiente.
**Base:** `b65f9a6` de `prototype/w01-third-person-windows`. Cotas según [plano y ficha del PR #3](https://github.com/meliodr/juego-/pull/3) (`d088330`).

Abre `/Game/IB_C00/Lvl_IB_C00` y pulsa Play desde el PlayerStart. El mapa usa el GameMode Third Person del proyecto. Empieza mirando al este en (10, 0) m. Sigue los marcadores hasta la esquina de (180, 0) m y gira hacia +Y. Las dos parejas de edificios de la avenida dejan pasos laterales de 6 m. El PlayerStart está a 100 cm sobre la calle para dar holgura a la cápsula de 96 cm de semialtura.

El mapa reutiliza un cubo nativo por volumen, texto e iluminación de Unreal. Contiene el carro como un volumen estático de ensayo y una fachada de cuatro tramos sólidos con panel central editable como actor separado. Los rótulos y marcas de segmentos son visuales. Coste RD$0. No requiere assets de Fab ni plugins nuevos para cargar el mapa.

## Prueba reproducible

Compilación local de `ImpactBeyondEditor Win64 Development` en UE 5.8.4 (CL 58902052), Windows 11 25H2, i9-14900HX, MSVC 14.44 y SDK 10.0.26100.0. Se usó `-NoHotReloadFromIDE` para compilar la copia aislada con Live Coding activo en otra sesión.

Desde la raíz del repositorio, ejecuta este comando en PowerShell. Abre una instancia independiente con Null RHI, carga el mapa desde disco, ejecuta la prueba y sale.

```powershell
$ue = 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
& $ue "$PWD/ImpactBeyond/ImpactBeyond.uproject" "-ExecutePythonScript=$PWD/Tools/Check-IB-C00.py" -unattended -nullrhi -nosplash -NoLiveCoding -stdout
```

- **C00-01 PASA.** Límites medidos del suelo, proxy y panel, y posiciones de marcas a X=20, 130 y 180 m. La recta mide 110 m y el tramo de frenada mide 50 m. Entre los centros del carro y la pared hay 58 m.
- **C00-02 parcial.** `IB-C00 BOUNDS AND CAPSULE SWEEPS PASS` después de 25 barridos con radio de 42 cm y semialtura de 96 cm. Los casos cubren el despegue, el giro, la calle norte, cinco puntos de suelo, el carro, el panel intacto, una fachada lateral, una azotea y nueve trayectorias en los 34 m detrás de la diana. El modo commandlet de Python se descartó para la prueba física porque no devolvió el impacto esperado. La prueba pasó en el editor aislado en modo desatendido.
- **Pendiente.** Recorrido manual a pie y en vuelo, legibilidad visual, DX12 y rendimiento. La prueba fue ejecutada por quien construyó el mapa.

**Decisión:** CONTINUAR con la revisión jugable en Win64/DX12. Para revertir, revertir el commit de IB-C00.
