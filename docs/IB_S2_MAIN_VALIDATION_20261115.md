# S2: base integrada, alcance de validacion y procedencia

La candidata combina main `538fd68dd76c69e72d84831e3320e9108775a7fd` con S2
`1d01db54b63250ed0aacb0704a5ac16290324e32`, conservando ambos padres en
`0f51da71db813ca2275b42b1fb2d5630ecf32e27`. Ese merge local no tuvo conflictos.
El incremento incorpora el proyecto base y su historia, no los laboratorios S2.5-S4.
No representa un paquete nuevo ni una actualizacion del acceso de Steam.

## Validacion inicial ejecutada

Sobre `0f51da71`: UE 5.8.4 CL 58902052, MSVC 14.44.35228 (directorio 14.44.35207),
Windows SDK 10.0.26100.0. UHT/UBT Win64 Development de Editor y Game PASS, exit 0,
83.52 s y 76.28 s respectivamente. Se uso `-DisableUnity -MaxParallelActions=2
-NoUBA -NoHotReload -NoHotReloadFromIDE`; evita las colisiones de nombres de
unidades anonimas de la base al agrupar fuentes en Unity.

Seis procesos independientes de UnrealEditor-Cmd, todos exit 0 y Automation Success:

| Suite (prefijo ImpactBeyond.) | Alcance comprobado |
| --- | --- |
| Flight.Headless | Capsula contra muro a 320/368 m/s, movimiento, vuelo y saltos |
| Circuit.Headless | Circuito original cargado desde IB_C00 |
| Circuit.TransportCycles | 20 ciclos completos, 10 lanzamientos por cada una de tres potencias, reinicios y obstaculo exclusivo de carga |
| Circuit.CityVehicles | Regresion de los vehiculos del mapa |
| Landing.Headless | 10 aterrizajes de impacto; entradas simuladas de teclado/gamepad |
| Planet.RadialLab | Matematica radial en Unreal; no es el laboratorio S3 |

8/8 tests Python del gate PASS. Sintaxis de 15 Python y 2 PowerShell PASS.
Los 880 paquetes uasset/umap tienen cabecera Unreal real; no son punteros LFS.
No hubo cambios versionados de Config, Source o Content durante la ejecucion.
No se detectaron errores Automation, LogStreaming, LogLinker/LogUObjectGlobals,
ensure o fatal en los patrones revisados. Avisos de DLL opcionales de profiler
aqProf/Vtune/WinPix no se interpretan como assets faltantes del juego.

## Regresion adicional de carga

`f2769ed35f3fbd1a085ce93d868719b65e19b316` amplia TransportCycles con cuatro casos:
320 y 368 m/s, frontal y oblicuo, contra un muro de 8 cm que deja libre la capsula.
Cada caso coloca el fixture una vez y establece una velocidad inicial explicitamente;
despues usa CMC y el TickComponent real de la carga. Mide velocidad de aproximacion
del coche, limite alcanzado por su volumen, paso del heroe y liberacion por bloqueo.
No acredita un viaje continuo de aceleracion ni todas las formas/rotaciones posibles.
La revision paso UHT/UBT Editor (77.67 s), Game (74.41 s) y las seis suites de nuevo.
Velocidades de carga medidas: 32000.0, 36800.0, 32015.9 y 36815.8 cm/s.
El frente del coche quedo en 5260.0/5260.1 cm ante el muro situado en 5260.0 cm,
dentro de la tolerancia de 8 cm; el heroe paso y el objeto bloqueado se libero.
Resultado y SHA256 de logs en [evidencia](IB_S2_MAIN_EVIDENCE_20261115.json).
Los logs originales permanecen locales; los hashes permiten contrastarlos, no
sustituyen una ejecucion independiente. Ambos turnos usaron mutex exclusivo y
finalizaron sin terminar procesos. Ultimo cierre: 21:14:59 UTC; inventario posterior
vacio y mutex ausente. El arbol de juego validado es
`ca4b0e1c3b8c8ef8dc4d4abc82f0e0958873528f`; los cambios siguientes son solo evidencia.

## Auditoria de publicacion

Revisada el 2026-11-15. No se adquieren ni descargan packs nuevos.

| Familia | Archivos | Evidencia y condicion |
| --- | ---: | --- |
| Ejemplos instalados de Epic | 760 | SHA256 identico a archivos de Templates de UE 5.8.4, incluidos ThirdPerson, variantes y ConceptCar |
| Material de plantilla modificado | 1 | MI_PrototypeGrid_TopDark: blob inicial `bc399156a502e0b5dc756e8ef22c309ce25f8a90` coincide con Templates; modificado en MVP `a8b9366` |
| Mapa/material de interiores propios | 2 | IB_C00; scripts de autoria versionados |
| Poses derivadas de plantilla | 12 | IB_Flight; scripts Author-FlightPoses/FlightSpeedPoses/MVPImpactPose |
| Kit Cidade Low Poly | 81 | Carro 6, CityKit 57, SprintVehicles 18; licencia CC0 conservada |
| Poly Haven UrbanFacades | 24 | Cuatro modulos seleccionados, materiales/texturas importados; CC0 |

La [EULA de Unreal, secciones 1 y 5(b)](https://www.unrealengine.com/eula/unreal)
define los Examples por su procedencia en Samples/Templates y permite distribuirlos,
incluidas modificaciones, en fuente u objeto. Esta comprobacion se limita a esos
Examples: no extiende ese permiso a recursos de Fab o al codigo del motor.

Kit Cidade: [revision fijada](https://github.com/kayquer/kit-cidade-lowpoly/tree/ed5ba11d026ffc6c990035d117c7886b40ed3fb7)
y [CC0 original](https://github.com/kayquer/kit-cidade-lowpoly/blob/ed5ba11d026ffc6c990035d117c7886b40ed3fb7/LICENSE),
conservada en LICENSE-CityKit.txt. Fachadas: [asset original](https://polyhaven.com/a/modular_urban_apartments_facade)
y [licencia CC0 de Poly Haven](https://polyhaven.com/license), que permite redistribucion.

Adaptaciones de codigo: [ManOfSteel MIT](https://github.com/ProdiG66/ManOfSteel/blob/d808f32ccab77dfbe2ba4b5f1cd29c2571eaa422/LICENSE)
y [REPIRK MIT](https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5/blob/main/LICENSE).
Ambos avisos se conservan en docs/LICENSE-*.txt. No se importan modelos ni clips de
Superman ni se atribuye licencia MIT a arte externo de esos repositorios.

Escaneo heuristico de todos los archivos versionados: cero patrones de claves
privadas, tokens GitHub, claves AWS/OpenAI o asignaciones largas de credenciales;
cero ejecutables PE, DLL/PDB, paquetes cocinados, archivos comprimidos o claves.
No es garantia universal de ausencia de secretos. Logs de maquina, binarios de build
y datos temporales quedan locales; el JSON publico solo contiene nombres relativos,
resultados y hashes. Dependencias declaradas: modulos/plugins nativos de Unreal;
no hay plugin binario externo en el descriptor o Build.cs inspeccionados.

## Reproduccion y limites

Coordinar turno exclusivo y verificar procesos/reservas antes de cualquier ejecucion.
Usar el mutex `Local\ImpactBeyond_UnrealExclusive`; no terminar procesos al vencer
un timeout. El runner historico Test-FlightHeadless.ps1 no aplica este protocolo y
no fue usado para esta validacion.

Compilar con Build.bat, targets ImpactBeyondEditor e ImpactBeyond, Win64 Development,
la ruta absoluta del uproject y los flags anteriores. Para cada suite ejecutar
UnrealEditor-Cmd con el proyecto, mapa ThirdPerson para Flight o IB_C00 para las otras,
`-game -NullRHI -unattended -nosound -NoLiveCoding -nowrite -UseFixedTimeStep -FPS=60`,
`-ExecCmds="Automation RunTests ImpactBeyond.<suite>"`,
`-TestExit="Automation Test Queue Empty"` y rutas unicas de abslog/ReportExportPath.
Exigir exit 0 y el resultado Success del nombre completo, no solo ausencia de crash.

Sin cook, paquete actualizado, DX12, audio, benchmark sostenido ni DualSense fisico.
El guardado/streaming S3, contratos S3.5/S4 y S7 no se integran ni se declaran cerrados.
CI solo ejecuta los ocho tests del gate Git; las pruebas Unreal fueron locales.
La revision fue realizada por el mismo integrador, no QA independiente.
Conservar las ramas de autor; revertir el merge a main con `git revert -m 1 <merge>`.
