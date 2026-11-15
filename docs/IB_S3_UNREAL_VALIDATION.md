# S3 — validación Unreal aislada, 15 noviembre 2026

Continuación local del commit publicado `2f08add14a341bfef1f3fd91a1c3c0c6b7e56a12`, PR draft #27. Checkout: `C:/Users/jarry/Documents/Codex/2026-11-15/task-2/impact-s3-planet`, rama `prototype/ib-s3-planet-lab`, base inmutable `1d01db54b63250ed0aacb0704a5ac16290324e32`. Esta validación no integra S2.5, no cambia el mapa principal y no publica sus cambios nuevos.

## Ocupación y entorno

Antes de iniciar se consultaron procesos y líneas de comando mediante CIM elevado por el mecanismo habitual. No había Unreal Editor, UBT, AutomationTool/cook, ShaderCompileWorker, dotnet, MSBuild, cl o link activos. El mutex `Local\ImpactBeyond_UnrealExclusive` estaba disponible. La reserva de S2.5 indicaba CLOSED, exit 0, RELEASED a las 17:28:50 UTC. Reserva S3 escrita a las 17:53:32 UTC en `../IBS3_editor-turn.json`. Cada proceso toma además el mutex. Nunca se cerraron procesos ajenos.

Unreal 5.8.4 CL 58902052; UBT seleccionó **MSVC 14.44.35228** (instalación BuildTools 14.44.35207), Windows SDK **10.0.26100.0**, .NET integrado 10.0. Se limita la compilación a dos acciones. El MSVC 14.52 usado por las pruebas matemáticas anteriores es un compilador distinto; no se confunden sus resultados con UBT.

## Correcciones demostradas durante ejecución

1. UBT rechazó `auto* W=Pilot->Planet` en UMG. Se usa el tipo explícito y `.Get()`.
2. Unity juntaba unidades de la base S2 que declaran `PanelTag` y `SampleRate` en espacios anónimos. Runner con `-DisableUnity`, sin tocar Character/Circuit/Landing/FlightFeedback compartidos.
3. El KillZ plano predeterminado de UE está a aproximadamente -10 km; destruiría el personaje al sur de la esfera de 25 km. Autoría fija KillZ a -1.000.000.000 cm únicamente en el WorldSettings del mapa S3.
4. El fixture que generaba una fila duplicada usaba `TArray.Add(Array[0])`, prohibido por la comprobación de autoalias del contenedor. El proceso terminó con código 3. Se copia primero la fila a una variable local; los tres tests originales pasaron después.
5. Unreal reescribió DefaultEngine.ini en el primer arranque (normalización de hardware y valores automáticos de un módulo del Editor). Se restauró exactamente HEAD, comprobado limpio antes del arranque. El runner usa `-nowrite`, cuyo comportamiento se verificó en ConfigCacheIni.cpp. No se conserva ningún cambio de configuración, seguridad o credenciales generado por el Editor.
6. Un ensure previo al comienzo de los tests reveló que W/A/S/D/E/Q no son `Axis1D`. Ahora se lee su estado digital desde PlayerController antes del movimiento. Ejes de ratón/gamepad siguen enlazados como ejes. La regresión de contactos inyecta press/release W dentro de Unreal, sin usar teclado del escritorio.
7. Capturas DX12 reales mostraron fachadas/suelo negros y nombres solapados en el mapa global. Se reemplaza la captura ambiental desde el centro interior del planeta por referencia al cubemap ambiental instalado del motor, con luces móviles. No se copia su contenido. UMG incorpora fondo legible, índices y leyenda separada. Corrección del mapa existente mediante `IBS3_RepairLighting.py`, limitada al mapa S3 y una luz de cada tipo.
8. El fixture visual reiniciaba su reloj y después evaluaba el timeout del caso anterior; acabó con FAIL tras cinco imágenes aunque el proceso devolvió 0. El runner detectó la falta de PASS. Se retorna al siguiente frame al avanzar de caso y se guardan las nuevas capturas en carpetas GUID para impedir falsos positivos por imágenes antiguas.

## Evidencia completada

Los logs están bajo `ImpactBeyond/Saved/IB_S3_QA/`. La salida generada no se añade indiscriminadamente al repositorio.

| Prueba | Estado y alcance |
|---|---|
| UHT/UBT | PASS tras las correcciones. Logs `build-01-failed.log`, `build-02-pass.log`, `build-03-pass.log`, `build-04-pass.log`. |
| Autoría | PASS, `Author-41b260b7bc764e6fa5f1937027c7291a.log`. Map, DataAsset y dos StaticMesh originales guardados en Content/IB_S3. |
| Manifest | PASS, `RuntimeTests-5efc7e5c4f9d4d0ea09817ac79e87167.log`. |
| RuntimeResidency | PASS en el mismo log: cuatro trazas de colisión, diez ciclos de descarga/rehidratación de fixtures, IDs únicos y muro roto conservado. Son ciclos sintéticos de residencia, no vuelo. |
| SaveValidation | PASS en el mismo log: cinco archivos incompatibles/corruptos, sin aplicación parcial ni sobrescritura. |
| Contacts, primera ejecución | PASS, `Contacts-0fdfb7ad4ab44b158092a4b8b62bdf89.log`: cuatro hubs y ambos polos, caída de 20 m, contacto CMC, giro y marcha de más de 4 m. 3,45–3,49 s por sitio. El ensure de entrada anterior al test motivó la corrección y repetición. |
| Contacts, teclado corregido | PASS, `Contacts-61c1eb9b638144bab30dd1b4f603da32.log`, proceso 80860. Seis sitios con press/release W simulado y movimiento CMC; 3,45–3,51 s. Sin el ensure de Axis1D. No es prueba de teclado físico o gamepad. |
| RestartWrite | PASS, proceso 27148; `RestartWrite-b2060fd5857f4643b250585f937949ad.log`. Slot GUID nuevo, sin tocar save de usuario. |
| RestartVerify | PASS, proceso distinto 75784; `RestartVerify-a0af8967ee72451bbd330c28a23cb396.log`. BeginPlay recupera descubrimiento, destino, muro roto, auto y objeto movidos, una instancia por ID. Diferencias de posición frente a save: muro 0,000 cm, auto 0,002 cm, objeto 0,001 cm. Tolerancia de asentamiento declarada en test: 150 cm. |
| VisualEvidence, primera ejecución | FAIL, `VisualEvidence-d8abb9614b9e486bb19de9c223488345.log`. Se generaron cinco PNG reales 1920×1080; se inspeccionaron central, puerto y mapa global antes de corregir contraste/solapamiento/timeout. Se preservan en Captures-before. |
| RepairLighting | PASS, `RepairLighting-e1796fc508e54189b3676fb85c2f6e73.log`, Editor cerró con código 0. DefaultEngine.ini sigue idéntico a HEAD. |
| VisualEvidence corregido | PASS, `VisualEvidence-495edacd442347dc9adcdc8caa966f3d.log`, proceso 18320. Siete PNG reales 1920×1080 inspeccionados: cuatro hubs en suelo, mapa global a 100 m, minimapa a 800 m, órbita a 50 km. Carpeta `Captures/A6ADBFBA4EF254A045D6F1A595DA4EFB`. |
| FlightReentry | PASS, `FlightReentry-edecce2968db4e8796aa453f00497c47.log`, proceso 21740. Diez salidas/reentradas por vuelo CMC a 320 m/s, veinte cambios ciudad/celda, 185,882 s de juego. Auto movido y muro roto conservados, una instancia por ID. Veinte frenadas de 216,882–216,928 m, dentro del límite conservador de 660 m (v²/2a + 20 m). El frenado ocurre en varios frames; incluye fricción CMC, no solo deceleración constante. |

Los fixtures de aterrizaje se colocan explícitamente al comienzo de cada caso y después dejan actuar caída, contacto y marcha reales. No demuestran una ruta volada entre las cuatro ciudades. El reinicio usa dos procesos y un slot `IB_S3_QA_Restart_<GUID>`; no es una prueba de reinstalación, migración entre versiones o recuperación ante corte de energía.

## Reproducción

Coordinar turno y comprobar propietarios antes de usar `-ExclusiveTurnConfirmed`. El runner no obtiene permiso por esa bandera; solo verifica procesos/mutex adicionales.

```powershell
./Tools/IBS3_RunUnreal.ps1 -Mode Build -ExclusiveTurnConfirmed
# Author solamente en un checkout sin mapa/assets S3 generados; se niega a reemplazarlos.
./Tools/IBS3_RunUnreal.ps1 -Mode Author -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode RuntimeTests -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode Contacts -ExclusiveTurnConfirmed
$slot='IB_S3_QA_Restart_'+[guid]::NewGuid().ToString('N')
./Tools/IBS3_RunUnreal.ps1 -Mode RestartWrite -QASaveSlot $slot -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode RestartVerify -QASaveSlot $slot -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode VisualEvidence -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode FlightReentry -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode Circumnavigation -ExclusiveTurnConfirmed
```

## Limitaciones y diagnóstico de entorno

El checkout es sparse: al registrar CDOs de clases S2 se notifican assets ausentes de Car, IB_Donors, IB_Flight y plantillas. El mapa S3 utiliza formas del motor y sus propias mallas; estos mensajes no se ocultan ni se corrigen editando clases compartidas. El resultado no acredita un paquete global. No se ejecutó cook ni paquete distribuible, Steam Input/DualSense físico, audio, aceptación de sensación de control o perfil 1080p/60 FPS.

El mismo agente implementa y verifica: no es QA independiente. Todos los hubs siguen siendo **placeholder técnico**, no realismo aprobado.

Pendientes concretos: paquete Win64/cook y lanzamiento por Steam; DualSense físico y sensación de control; perfil de frame time/p95/p99/GPU/VRAM/audio; tres vueltas exigidas por el roadmap general; capturas de mapa/minimapa durante cruce polar; fallo de carga diferida inyectado; migración física de props entre hubs y prueba de world-origin rebasing en el motor; rutas urbanas largas y colisiones bajo estrés. No se sustituye un control físico/manual por los eventos de entrada simulados del fixture. Las capturas prueban render y legibilidad técnica, no una sesión completa jugada por una persona.

El render se inicia con `-d3d12 -RenderOffscreen -windowed -ResX=1920 -ResY=1080 -ForceRes`, sin entrada de escritorio. RHI seleccionó NVIDIA GeForce RTX 4070 Laptop GPU, controlador 621.14 (32.0.16.2114), fecha 27 octubre 2026. El umbral de calentamiento de automatización es 10 FPS y no certifica 60 FPS; `t.MaxFPS 60` es solo un límite. No se ejecuta SceneCapture2D cada frame.

El inventario genérico del reporte Automation lista Intel UHD Graphics, mientras el log explícito de selección RHI indica adaptador 0 NVIDIA; se conservan ambos datos. CPU i9-14900HX, 64 GB, Windows 11 25H2 10.0.26200.9711. No se infiere rendimiento a partir de ese hardware.

FlightReentry usa una única colocación inicial a 500 m y modifica los objetos representativos como fixture. Después solo introduce avance/freno y gira al detenerse; no escribe posiciones durante los viajes medidos. Los retornos físicos se ensayan alrededor de CITY_CENTRAL. El objeto pequeño desplazado sí se valida en el reinicio, no se afirma que los tres objetos se alteraron físicamente en cada uno de los diez retornos. La deriva radial se imprime como 0,000000 cm con seis decimales; no significa exactitud física infinita.

## Pruebas adicionales y cierre del turno

**Circumnavigation PASS**, proceso 40472 sin render, log `Circumnavigation-5e28d684f0c04720a7b4f11740ac3c1f.log`. Una vuelta ecuatorial completa con CMC real, radio 25 km y altura 500 m: **594,932818 s (9 min 54,93 s)**, velocidad pico **270 m/s**, **15 cruces de región**, incluyendo puerto, costa y antimeridiano. Retorno a **762,171147 cm** del inicio, dentro de la tolerancia de una muestra conservadora de 10 m; no se fuerza un snap al punto inicial. Frenada final **170,782 m**. Deriva radial impresa **0,000000 cm**, precisión del log y no promesa de exactitud física infinita. Una colocación inicial explícita; ninguna escritura de pose durante la vuelta. No es una vuelta con render, aterrizajes intermedios o prueba de choque contra obstáculos.

El proceso cerró con código **0**. A las **18:33:27 UTC** no había procesos Unreal/UBT/AutomationTool/ShaderCompileWorker/dotnet/MSBuild/cl/link y el mutex estaba disponible. Reserva S3 `CLOSED / RELEASED`, sin terminar procesos ajenos. La entrada de escritorio se declaró libre para Figma; S3 nunca envió teclado/ratón al escritorio.

`Tools/IBS3_SummarizeEvidence.py` genera inventario con SHA256 de logs/capturas y snapshot actual de fuentes, además de `flight-progress.csv`. Los hashes de fuentes son del estado actual, no se atribuyen retroactivamente a los intentos fallidos. Nuevos cambios y assets quedan en commit local; sin push, actualización remota de PR ni merge en esta validación.
