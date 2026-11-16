# Evidencia actual de la candidata S7

C++ `fa75f7f`; autoría `730b39f`; configuración/validador de paquete `17d7f91`. Paquete actual: `8b56a20`, `s7-package-20261116T060244Z` (37,01 s), con el mismo ejecutable C++ `fa75f7f`. La candidata tiene build, autoría, cook y dos mediciones aprobados; el gate final de materiales detectó agua sin soporte de instancias y requiere una corrección de autoría/recook; **no se declara aceptación humana de S7**. Rigen los criterios de [PLAN_ACTIVO.md](PLAN_ACTIVO.md).

## Gates observados

| Gate | Resultado y alcance |
|---|---|
| CI pública `a645030` | Integrated source checks `43813155209` y Lab audit tests `43813155278` PASS; 13 suites nativas |
| Compilación | Game `fa75f7f` PASS, `s7-build-20261116T054825Z`, 114,26 s. Último Editor: `6b4631d`; cambio posterior limitado a QA de framebuffer/GPU y dependencias RHI |
| Autoría | Partida generada con cero dependencias ausentes. Corrección final `ead1d59`, `s7-author-20261116T060201Z`: PASS; once materiales verificados, solo `agua` cambiado y guardado localmente |
| Producción | PlayableSession, S3 Contacts —seis superficies— y S7 PreparedEncounter PASS, `s7-tests-20261116T053308Z`; grafo de ticks sin ciclos ni warnings |
| Reentrada S3 | Diez regresos físicos PASS en S3Lab, `s7-tests-20261116T053404Z`; no equivale a tres circunnavegaciones renderizadas en producción |
| Funcionales DX12 anteriores | Cuatro suites PASS sobre paquete `17d7f91`, `s7-tests-20261116T054558Z`: PlayableSession, RealInput, S3 Contacts y PreparedEncounter; cero recursos ausentes/materiales de reserva |
| Paquete actual | Cook/paquete `8b56a20` PASS, `s7-package-20261116T060244Z`, 37,01 s; cuatro funcionales DX12 con gate de contenido en curso |
| Steam | Acceso actualizado al ejecutable actual y Activar Steam Input comprobado en UI; arranque visible pendiente |

Los resultados Unreal fueron comunicados por el integrador, único operador de editor/UBT/cook. Las revisiones y fallos anteriores se conservan en el [manifiesto de evidencia](IB_S7_PUBLIC_EXPORT.json); no invalidan ni se confunden con los runs corregidos.

## Dos perfiles previos a la corrección del agua

Run `s7-tests-20261116T055356Z`, 90,008 s y 5.397 fotogramas. DX12 con audio y reloj real, salida **1920×1080**, escala interna **83%**, calidad **2** y límite **60 FPS**. Fortnite estaba ausente; se mide el equipo en su estado real, según la preferencia del usuario.

| Métrica | Primera medición `055356Z` | Segunda medición `055617Z` |
|---|---:|---:|
| FPS medios | 59,94 | 59,9848 |
| Fotograma p95 / p99 | 16,7181 / 16,9259 ms | 16,684 / 16,7149 ms |
| Fotograma máximo | 50,018 ms | 26,2070 ms |
| Fotogramas >50 ms | 1 | 0 |
| GPU media / p95 / p99 | 6,144 / 6,8109 / 7,2065 ms | No informado en este resumen |

Ambas mediciones pasaron sobre el mismo ejecutable SHA-256 `6C7741F10E1350CF56A8A80C84B34300CBB2A29E33E8E3BB3B09DD5DEEAE4337`. La segunda no tuvo fotogramas >33,33 ms. Sus resultados permanecen ligados a ese paquete anterior a la corrección del agua; falta una medición confirmatoria del recook.

El resultado no es rendimiento sin límite ni render interno nativo 1080p; tampoco garantiza la misma tasa en recorridos, densidades o condiciones diferentes. Las pruebas funcionales anteriores tenían Fortnite activo y no se usan como benchmark.

El gate final detectó falta de instancing en `/Game/IB_Donors/CityKit/hotel/hotel/Materials/agua`. La corrección de autoría `730b39f` pasó: once materiales verificados, solo `agua` cambió; los otros diez ya tenían instancing. No hay cambios C++. El recook actualizado pasó; cuatro funcionales con el gate de contenido están en curso y queda un perfil confirmatorio. No se considera resuelto por el PASS de las aserciones de gameplay.

## Contratos S7 y límites

- Robot con Hammer/Claw funcionales y kaiju con Horn/Tail/Shoulder, dos ataques anunciados, consecuencias de desarme y cuerpo vencible sin desmontar todas las partes. Políticas y runtime cubren deduplicación, una pieza física por PartId y restauración.
- Sesión y héroes comparten ImpactContract, receptores, agarre y persistencia. El encuentro preparado y el grafo de ticks pasaron en producción.
- La rueda equipa sin activar, cancela y conserva el poder anterior al refrescar. Cada héroe ofrece seis poderes y el HUD distribuye seis sectores; no se afirma presentación correcta de 7–8 sectores aunque la política los admite.
- RealInput inyecta teclado, mando y stick mediante PlayerController. No certifica DualSense físico, selección direccional con ratón, pérdida de foco ni sensación; estas comprobaciones se mantienen separadas.
- Escala provisional: robot 4 m y kaiju 8 m. La sesión crea amenazas por región y activa las cercanas; residencia/coste tras viajes y aceptación artística requieren su propio recorrido.

## Pendiente para cerrar esta entrega

1. Validar el paquete recocinado con la autoría del agua corregida: cuatro funcionales más gate de contenido y un perfil confirmatorio sobre el paquete actualizado.
2. Arranque visible desde Steam y revisión de imagen/audio, piezas, telegráficos, rueda y persistencia de la partida entregada.
3. Registrar los límites de las pruebas acotadas frente a la QA sostenida del plan; no equivalen a certificar todos sus recorridos y ciclos. DualSense físico y valoración humana permanecen separados.
4. Publicar la evidencia final y reconciliar los PR cubiertos/sustituidos conservando su historial. S8 sigue después de este cierre.
