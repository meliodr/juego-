# Revisión de cierre S7

Estado al 16 de noviembre de 2026: **implementación integrada en validación**. Fuentes C++ candidatas `fa75f7f` (base jugable `6b4631d` más corrección QA), autoría `742ff49` y configuración/validador de paquete `17d7f91`; no constituyen una build entregada o aceptada. Esta revisión aplica los contratos de [PLAN_ACTIVO.md](PLAN_ACTIVO.md), consolidados de PR17/22/23; no añade alcance a la cola.

## Contratos frente al código

| Contrato | Implementación observada | Evidencia y límite |
|---|---|---|
| Robot pesado con dos piezas funcionales | `IB_S7/Wave8Threats/ThreatModel.h`: Hammer habilita el golpe; Claw habilita el agarre. `IBS7TThreatActor` aplica colisión, movimiento radial, captura acotada y liberación | Política nativa y RuntimeContract cubren consecuencias, desprendimiento y reset. Falta confirmar lectura visual y comportamiento sostenido en el paquete |
| Kaiju con tres partes y dos ataques anunciados | Horn modifica daño de Charge, Tail habilita TailSweep, Shoulder modifica vulnerabilidad. El cuerpo puede llegar a cero sin desprender todas las piezas. Hay anticipación temporizada, trayectoria fijada y avisos distintos | Políticas y runtime automatizados; apariencia, timing percibido, sonido y contacto en el recorrido final requieren DX12/audio |
| Una pieza física por desprendimiento | Actor S7 y objeto S3.5 conservan PartId, desactivan collider/módulo adherido y crean una pieza agarrable. Snapshots rechazan estados incompatibles y renuevan vida del receptor | RuntimeContract y PreparedEncounter cubren partes, replay, colisión antigua, agarre y restauración. Nueva prueba de grafo de ticks en `6b4631d` aún pendiente |
| Daño compartido con los tres héroes | Sesión registra receptores y dirige mensajes mediante ImpactContract; poderes, golpes y partes reutilizan la misma autoridad | PlayableSession pasó sin render en `761a2b5`; repetir la regresión afectada después de la corrección de ticks |
| Rueda equipa sin activar, con cancelación y poder anterior | `Radial/PowerWheel.h` acepta 6–8 slots, zona muerta, disponibilidad/cooldown y selección anterior. `UpdateSlots` conserva historial al refrescar. Host bloquea acciones mientras está abierta | Suite nativa de refresco y prueba UE; RealInput inyecta eventos reales de PlayerController. El HUD actual distribuye seis sectores y cada héroe ofrece seis: no afirmar que presenta 7–8 sectores correctamente |
| Ratón, mando y restauración de input | Componente acumula desplazamiento de ratón o lee stick, cancela al perder foco, libera su bloqueo de cámara y no modifica dilatación global | RealInput cubre Tab, D-pad, stick, confirmar/cancelar y supresión modal. No cubre selección direccional por ratón, pérdida de foco ni DualSense físico; quedan en revisión de controles |
| Regiones, persistencia y presupuesto | Sesión crea un robot por región visitada y un kaiju en puerto/montaña; activa amenazas cercanas dentro de la burbuja física. Guardado captura cuerpo/partes y rechaza reset/guardado incompatible con piezas retenidas | No equivale a una única instancia global de cada enemigo. Medir residencia/coste tras visitar cuatro ciudades y regresar; no inferir presupuesto aprobado de un test local |

Los modelos se escalan provisionalmente a 4 m de robot y 8 m de kaiju en el host. La lectura de escala y las piezas animadas requieren revisión a cámara de juego. Las pruebas de política no certifican colisiones del motor, presentación o sensación.

## Resultados registrados

Los resultados Unreal siguientes fueron comunicados por el integrador, único operador de Unreal. Este frente no ejecutó editor, UBT, cook ni paquete.

| Revisión / run | Resultado observado | Alcance |
|---|---|---|
| Público `534f0d4` / Actions `43812082671`, `43812082685` | CI verde | Recetas, sintaxis, publicación y 13 suites nativas. No se extiende a commits posteriores |
| `9ca77e1` / `s7-build-20261116T045837Z` | Editor + Game PASS | 224,52 s y 210,82 s respectivamente |
| `acf4156` / `s7-tests-20261116T051706Z` | 12 funcionales PASS; Editor también aprobado por el integrador | Sin render: combate integrado, S6, runtime S7, encuentro preparado, refresco de rueda, vida/persistencia S5, ciudades, golpes, aterrizaje y agarre/lanzamiento/reset |
| `s7-tests-20261116T052404Z` | Tres pruebas S4 PASS | ThirdPerson con GameMode S4. H01, ResponseContract y PreparedCollision; sustituye el estado anterior de PreparedCollision pendiente y no certifica por sí solo el mapa de producción |
| `761a2b5` / `s7-tests-20261116T052319Z` | Integration.PlayableSession PASS | Mapa de producción sin render |
| `761a2b5` / `s7-tests-20261116T052438Z` | Playable.RealInput PASS | DX12 offscreen, eventos simulados de teclado/mando. Fortnite estaba activo: no es una medición de rendimiento ni prueba de mando físico |
| Autoría `761a2b5` | Contenido generado y cero referencias ausentes; proceso terminó con exit 3 | Fallo posterior durante teardown de Python; resuelto por la ejecución posterior `b341476` |
| `6b4631d` / `s7-build-20261116T052603Z` | Editor + Game PASS (Editor: 181,87 s) | C++ congelado; regresión específica registrada debajo |

La regresión de producción `s7-tests-20261116T053308Z` aprobó **PlayableSession, S3 Contacts (seis superficies) y S7 PreparedEncounter**, incluida comprobación de grafo de ticks; cero warnings de ciclos. El batch terminó FAIL únicamente por S3FlightReentry: requiere los fixtures car01/wall01 del laboratorio, que producción omite intencionadamente. La repetición en S3Lab **pasó con diez regresos físicos**, run `s7-tests-20261116T053404Z`, sin alterar la prueba. Su alcance sigue siendo la fixture S3Lab; no certifica por sí sola circunnavegaciones o viajes completos en producción.

La autoría integrada en `b341476` **pasó con exit 0 y cero dependencias ausentes**, run `s7-author-20261116T053232Z`: queda resuelto el fallo de teardown. El script exacto `742ff49` aplicó la [receta de instancing S25](IB_S25_SOURCE_REBUILD.md), guardando los siete materiales únicamente en la integración local. Todavía faltan revisión visual/cook y el paquete; no se exportan esos binarios.

El primer paquete `a91b163` (`s7-package-20261116T053439Z`) **se retiró como candidato**: el cook pasó, pero la prueba DX12 detectó cinco recursos S3 ausentes y viewport 888×500 por DPI. `17d7f91` añade los directorios de materiales S25 y mallas del árbol a AlwaysCook, activa High DPI, fuerza la resolución de pruebas y falla ante referencias ausentes o materiales de reserva. El nuevo cook/paquete `s7-package-20261116T054101Z` pasó con cero errores y 807 paquetes. Cuatro suites funcionales del paquete pasaron en DX12 en `s7-tests-20261116T054558Z`: PlayableSession, RealInput, S3 Contacts y S7 PreparedEncounter, sin recursos ausentes ni materiales de reserva. El perfil anterior falló en la lectura QA de imagen (`InRHITexture null`); no produjo CSV ni resultados de rendimiento. `fa75f7f` conserva el framebuffer Slate durante la lectura y registra la GPU de RHI en lugar del adaptador primario de pantalla. Game pasó en `s7-build-20261116T054825Z` (114,26 s); la última compilación Editor corresponde a `6b4631d`. El recook actualizado está en curso. Fortnite estaba activo en las cuatro pruebas DX12 anteriores; el usuario lo cerró antes de la repetición de perfil y pidió medir el equipo como está. La carga concurrente se registrará al capturar.

La fuente de prueba RealInput conserva la advertencia explícita sobre mando físico. La rama pública mantiene `final_source_commit: null` y `is_final_release_snapshot: false` hasta que el integrador fije la entrega real.

## Pasos de cierre ya previstos

1. Conservar los recibos con su mapa: FlightReentry pasó diez regresos en S3Lab; PlayableSession, PreparedEncounter y S3 Contacts pasaron en producción sin ciclos de ticks. La autoría terminó limpia y sin dependencias ausentes.
2. Terminar el recook de Game `fa75f7f` ya compilado y repetir el perfil que falló antes de producir CSV; comprobar el ejecutable actualizado con DX12/audio y el acceso Steam vigente. Verificar cuerpos/piezas, telegráficos, seis sectores de rueda, ratón, pérdida de foco y persistencia al viajar/regresar.
3. Completar el recorrido continuo y cargas de QA pendientes del plan: colisión rápida/carga, destrucción y resets, reloj/civiles, acciones H02/H03, desarme y reinicios. Registrar p95/p99, VRAM y tirones con hardware, calidad y resolución; no usar la sesión concurrente con Fortnite como benchmark.
4. Separar evidencia de input simulado, DualSense físico y valoración humana. Actualizar fuentes y documentación con la revisión efectivamente probada, pasar CI y reconciliar los PR cubiertos/sustituidos con precisión.

S8 —pausa completa y comienzo del espacio— sigue después de este cierre. El estado actual no demuestra aceptación S7 ni justifica ampliar el alcance para ocultar fallos de la candidata.
