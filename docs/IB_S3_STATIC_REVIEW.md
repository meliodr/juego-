# S3 — revisión estática adicional y regresiones ligeras

2026-11-15. Revisión del commit de entrega local `2add2edd64b3bb0270e442aa80dfb50b0c08fbb4`, sin abrir Editor, cook ni compilación UE. S2.5 conserva el turno pesado. Publicación previamente denegada: no se reintentó ni se cambió el método. Revisión realizada por el mismo implementador, no auditor independiente.

## Defectos reproducidos y correcciones

Se compiló una nueva suite contra el núcleo original: **5 aserciones, 5 FAIL**. Después se corrigieron las funciones reales y se amplió la suite, incluyendo el integrador que ahora llama PhysFlying. Resultado final: **100.172 aserciones, 0 fallos**. La suite original volvió a pasar sus **336.251 aserciones**.

| Hallazgo demostrado | Corrección | Evidencia / alcance |
|---|---|---|
| Al mirar al norte, `Up × Forward` apuntaba al oeste geográfico; cámara y brújula tenían este/oeste invertidos. | Longitud positiva hacia -Y en meridiano cero. Marco geográfico consistente con actor UE: X=norte, Y=este, Z=arriba. | Falló antes; pasa en 10.000 marcos aleatorios. No es inspección visual UMG. |
| `prefetch(NaN,...)` convertía NaN a velocidad cero por el orden de `std::max`. | Validar antes de operar; entradas inválidas devuelven presupuesto infinito conservador. | FAIL→PASS. |
| `safeSpeed` con frenada NaN devolvía NaN. | Datos inválidos producen velocidad admisible cero. | FAIL→PASS, además de límites extremos. |
| `geographic` aceptaba radio NaN como resultado válido. | Rechazo explícito de radio/origen singulares y celdas inválidas sin convertir NaN a entero. | FAIL→PASS y regresión de región. |
| La raíz `sqrt((brake*p95)^2+2*brake*distance)-brake*p95` perdía el resultado por cancelación. | Raíz racionalizada con `hypot`. | FAIL→PASS; 10.000 presupuestos aleatorios verificados. |
| UE 5.8.4 inicializa `bTickBeforeOwner=true`: nuestro Actor::Tick configuraba gravedad/límites después de mover. | Preparación dentro de `UIBS3Movement::TickComponent`, antes de Super; dependencia explícita del Tick del planeta. | Código del motor inspeccionado en MovementComponent.cpp, constructor y RegisterComponentTickFunctions. Orden real pendiente de motor. |
| Subpaso limitado por velocidad antigua omitía la aceleración; podía superar 4 m. | `boundedStep` incluye aceleración semimplícita en el límite. | Función consumida por runtime; regresiones desde reposo hasta 1.000.000 m/s de entrada numérica. No se declara esa velocidad jugable. |
| La aceleración quedaba en el marco anterior durante subpasos; a 320 m/s generaba componente radial superior al umbral de entrada vertical. | Determinar intención vertical una vez, transportar aceleración junto a velocidad/frente y mantener altura explícitamente. | Caso reproduce componente falsa >1 cm/s²; transporte la elimina. |
| Combinar arco a radio viejo con desplazamiento radial alteraba el recorrido real. | `flightStep` integra la tasa angular `vt/(r+vr*t)` mediante log1p, con límite estable para vr≈0. | Ascenso/descenso, longitud máxima y conservación de velocidad; dos vueltas numéricas del helper real de runtime. |
| Solo limitar velocidad no comprueba que un segmento extremo atraviese una ciudad pendiente cuyos extremos estén fuera. La cuadrícula física también excedía el radio geográfico nominal en las esquinas. | Envolvente conservadora de geometría + cápsula, prueba del segmento completo antes del sweep; detener con estado explícito. Permitir retirada monótona si ya se estaba dentro. | Casos de cruce, paso lejano, retirada, entrada más profunda, NaN y overflow PASS. Es una barrera de preparación declarada, no un collider fingido. |
| El flag de estado físico del componente no demuestra que cada cuerpo ISM exista. | Revisar FBodyInstance válido del pad y cada instancia; movimiento espera cuerpo planetario válido. | API cotejada en PrimitiveComponent.h, InstancedStaticMesh.cpp y BodyInstance.h. Query/contactos reales pendientes. |
| Un save con quaternion cero o escala cero era finito y podía hidratar una pose física inválida. También aceptaba identidad/tipo/origen ajenos al manifiesto. | Validador de pose compartido y claves/kind/origen contra los defaults autorizados del manifiesto. | Pose válida/inválida probada nativamente. Identidad/serialización mediante nuevo test UE preparado, no ejecutado. |
| La carga escribía directamente en Deltas y, al rechazar una fila, vaciaba el registro. BeginPlay lo rellenaba después con defaults; el método de carga por sí mismo no preservaba el estado anterior. | Validar contra defaults mediante candidato completo y sustituirlo solo al terminar; guardar y descargar se suspenden si el estado vivo es inválido. | Refuerzo del flujo de validación y fixtures UE preparados: cinco archivos inválidos, conservación del estado anterior y prohibición de sobrescribir slot. No se afirma fallo jugado previo ni PASS del test UE. |
| Referencias a hubs/props destruidos podían permanecer en el registro y bloquear reentrada o desreferenciar un actor inválido. | Eliminar referencias caducadas antes de evaluar residencia, reteniendo el último delta válido. | Corrección de ciclo de vida inspeccionada; regresión física pendiente. |

## Compatibilidad de coordenadas

El manifiesto pasa a **revisión 2** porque cambia el signo de Y de las posiciones geográficas. Saves de revisión 1 se rechazan conservando el archivo; no se reinterpretan posiciones antiguas. No se había generado ni ejecutado el mapa S3 ni creado saves S3 durante este encargo. BodyId/RegionId/CityId y clave de objeto no cambian.

`Frame` expresa componentes geográficos este/norte/arriba; para una rotación de actor UE usar norte/este/arriba. No construir un quaternion directamente con columnas E/N/U. Los hubs construyen su rotación con `MakeFromZX` y un frente tangente, sin convertir ese marco geográfico reflejado en una matriz de rotación.

La posición persistida continúa siendo `ActorLocation + World.OriginLocation`; hidratar resta el origen actual. Velocidades y rotaciones no suman el origen. Se conserva centro global junto a radio/revisión. El Actor planeta admite traslación; escala/rotación del Actor incompatibles se rechazan, y el radio se ajusta mediante el manifiesto antes de iniciar la sesión.

## Pruebas ejecutadas

```powershell
./Tools/IBS3_TestCore.ps1 -Suite Review -OutputDirectory ../qa-review-final
./Tools/IBS3_TestCore.ps1 -Suite Core -OutputDirectory ../qa-core-reviewed
```

MSVC 14.52.37025, C++17, /W4 /WX /O2. Review: **100.172 PASS**. Core: **336.251 PASS**. El helper de vuelo real produjo error máximo de retorno **0,0000723808 cm** y deriva radial **0,0000231871 cm** en sus dos vueltas numéricas, con origen global lejano. Estas cifras son de cálculo doble en CPU; no corresponden a Chaos, cápsula, frame time ni posición jugada.

Se comprobó sintaxis PowerShell de los scripts modificados y alcance exclusivo. El generador OBJ no cambió; mantiene la evidencia de topología previa. El test nuevo de Unreal `ImpactBeyond.S3.SaveValidation` está añadido al runner, **PREPARADO/NO EJECUTADO**.

## Siguiente turno exclusivo de Unreal

1. UHT/UBT de la rama local revisada. Verificar los cambios de TickComponent y acceso FBodyInstance contra la compilación real.
2. Autoría del mapa desde cero bajo Content/IB_S3 con manifiesto revisión 2. Confirmar X=norte/Y=este local, girar a la derecha aumenta rumbo hacia el este, flecha y mapa coinciden.
3. Ejecutar Manifest, RuntimeResidency y SaveValidation. Si Chaos crea cuerpos de forma diferida, convertir las comprobaciones instantáneas del fixture de reentrada en comandos latentes acotados; no marcar preparado como PASS.
4. Cuatro aterrizajes, polos, dateline, 320 m/s con freno y descarga/collider no preparado. La barrera debe informar el motivo, permitir retirada y reanudar tras estar listo.
5. Diez reentradas con muro, auto y objeto movidos, migración física entre regiones y reinicio real del proceso con USaveGame. Los fixtures actuales no sustituyen ese reinicio.
6. Vuelta continua, UMG visible tierra/aire, build empaquetada DX12, Steam Input/DualSense, audio y métricas 1080p/FPS/VRAM.

S3 sigue en **CÓDIGO; núcleo nativo COMPILADO/EJECUTADO; UE sin validar**. No se solicitó nada al usuario dormido, no se lanzó proceso pesado ni se reintentó push/PR.
