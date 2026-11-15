# S3 — laboratorio planetario, cuatro hubs y mapas

Fecha: 2026-11-15. Base inmutable: `1d01db54b63250ed0aacb0704a5ac16290324e32` (`prototype/ib-sprint02`, PR #19).
Rama: `prototype/ib-s3-planet-lab`. Laboratorio aislado; no integración global, S7.5 ni merge a main.

**Estado: CÓDIGO, núcleo matemático COMPILADO/EJECUTADO fuera de Unreal. Módulo UE IMPLEMENTADO SIN VALIDAR.**
No existe una build jugable S3 verificada ni un `.umap` generado en esta entrega. Autoría, UHT/UBT, colisión, DX12, controles humanos y rendimiento requieren turno exclusivo confirmado. Que no aparezca un proceso de Unreal en un instante no demuestra que otra sesión haya cedido el turno.

## Qué se reutiliza y por qué hay código nuevo

Se leyeron AGENTS, PLAN_ACTIVO, TAREAS_ACTIVAS, catálogo/selección Fab y las skills locales `impact-gameplay-gate`/`impact-qa-review`; las órdenes actuales autorizan este laboratorio y sustituyen la antigua prohibición de iniciar S3. También se revisaron `IB_SPRINT02_PLANET_LAB.md`, `ROADMAP_SPRINTS_02_A_08.md`, `SPRINT09_OCHO_CIUDADES_Y_BIOMAS.md` y `PLAN_PARALELO_S25_S3_S35_S4_20261115.md` (los tres últimos en la rama documental).

S2 contiene `Tests/ImpactRadialLab.cpp`: prueba numérica de radio 1 km y caída, sin mundo jugable, gestor de ciudades ni guardado planetario implementados. Se conserva intacta. UE 5.8.4 instalado incluye `UCharacterMovementComponent::SetGravityDirection`, confirmado en su header local. Se reutilizan CMC, capsule sweeps, Chaos, USphereComponent, ISM, SkyAtmosphere, UMG y USaveGame. Build.cs ya depende de UMG/Slate; no se modifican dependencias.

El código nuevo es el adaptador esférico mínimo para este laboratorio: matemática compartida verificable, manifiesto de datos, residencia/estado, variante de personaje y HUD. No se duplica el movimiento global ni los sistemas S3.5/S4. No se importó código de Ephemera: queda como referencia por auditar licencia/revisión/bytes/dependencias, sin plugin instalado o clonado. Tampoco se habilita PCG/World Partition por anticipación: la esfera y sus marcos deben existir primero; WP no transforma un mundo plano en esfera.

## Coordenadas, IDs y escala

Todas las longitudes del contrato son centímetros, dobles. `IBS3Geometry.h` es el mismo núcleo que consumen runtime y pruebas C++ nativas. Marco global centrado en el cuerpo; geografía con latitud [-90,90], longitud [-180,180), altitud sobre radio. Las conversiones admiten origen global lejano. Centro singular devuelve `valid=false`.

Revisión estática posterior: [hallazgos, correcciones y regresiones](IB_S3_STATIC_REVIEW.md). El manifiesto usa **revisión 2**: positivo este apunta hacia -Y en el meridiano cero, coherente con X=norte/Y=este/Z=arriba del actor UE. Revisión 1 se rechaza al cargar; no se reinterpreta un save anterior. El Actor planeta admite solo traslación; configurar radio en el manifiesto antes de iniciar, no mediante escala del Actor ni mutaciones del manifiesto durante vuelo.

`UIBS3WorldManifest` contiene BodyId, revisión, radio, gravedad, velocidad de ensayo, frenado, margen, estimación inicial p95, burbuja física y array abierto de recetas de ciudad. Ninguna lógica de Pawn/streaming contiene cuatro ramas por CityId. Añadir ciudad 5–9 consiste en añadir datos únicos. Semilla y escala de edificios están en cada receta. Los polígonos construidos en este primer laboratorio siguen siendo pequeños; modificar FootprintRadius no genera automáticamente kilómetros de ciudad.

| BodyId | CityId | RegionId de origen | Latitud / longitud | Seed |
|---|---|---|---|---|
| BODY_EARTH_LAB | CITY_CENTRAL | earth.city.primary | 0 / 0 | 1101 |
| BODY_EARTH_LAB | CITY_PORT | earth.city.port | 0 / 90 | 1102 |
| BODY_EARTH_LAB | CITY_COAST | earth.city.coast | 0 / 179.5 | 1103 |
| BODY_EARTH_LAB | CITY_MOUNTAIN | earth.city.mountain | 75 / -90 | 1104 |

Se conserva el RegionId central de S2. No se migra su mapa plano ni sus saves: el integrador deberá definir explícitamente el transform local/global y la migración. `earth.coast.east` sigue reservado al corredor S2. Las regiones geográficas de laboratorio usan `earth.cell.N`, 12 sectores longitudinales × 6 bandas y dos caps polares; los IDs de objeto no se recalculan al cruzarlas.

Clave de objeto: `BodyId/OriginRegionId/ObjectId` (`wall01`, `car01`, `object01`). `CurrentRegionId` es metadato de ubicación y puede cambiar; no altera identidad. Save schema 1 almacena posición/rotación/escala global, velocidades, daño, descubrimiento y destino. Guarda centro global y radio/revisión del manifiesto. Cambio incompatible o datos no finitos/duplicados deshabilitan escritura y preservan el slot original; no hay migración silenciosa. F5/cruceta abajo guarda explícitamente. No se sobrescribe durante teardown.

Radio inicial de ensayo 25 km, gravedad 980 cm/s² y travesía 270 m/s son datos provisionales. Se ensayó matemáticamente también 60 y 90 km sin cambiar el algoritmo. A 25 km + 500 m de altura, 270 m/s produce 593,411946 s; no incluye acelerar, girar o aterrizar y no es una medición jugada. Conservar revisión/slot distintos para experimentar con otro radio.

## Colisión, movimiento y presentación

`AIBS3PlanetWorld` contiene una esfera analítica de colisión estática (no un cuerpo simulado gigante). Es la superficie física global legítima; la malla orbital es solo presentación. Un cap curvo preautorado gira bajo el jugador como LOD visual cercano; no reconstruye malla ni colisión durante vuelo extremo. Se oculta sobre 3 km de altitud. La esfera visual tiene 65.024 triángulos y cap 32.768; sus bytes OBJ se generan localmente, no se publica un pack de terceros.

Los hubs instancian 169 losas curvadas, dos calles cruzadas abiertas, pad central y 16 volúmenes con distinta escala/semilla. Cada uno recibe muro removible con colisión realmente deshabilitada al romperlo, auto de ensayo y objeto con impulso. **Todos son placeholder técnico**. No se afirma puerto/montaña/costa artísticamente realistas ni se reconstruye City Sample. S2.5 aportará arte después.

`UIBS3Movement` deriva de CMC únicamente para este Pawn de laboratorio. Tierra/caída usan gravedad radial CMC. Vuelo integra arcos sobre la esfera de altitud actual y barre la cápsula en subpasos de hasta 4 m / 1/60 s, orientando +Z radial. Entrada vertical cambia altura; sin ella se mantiene altura. Transporte paralelo del frente evita singularidad de yaw en polos. No usa teleports de jugador ni snap de longitud. Si hay colisión se detiene; si se agota presupuesto de pasos se informa y se detiene. El uso de `TeleportPhysics` queda limitado a hidratar props y a fixtures de QA explícitos, nunca la circunnavegación.

La preparación de gravedad/límites se ejecuta **antes de CMC::TickComponent**, con dependencia explícita del Tick del planeta. El límite de subpaso incluye aceleración. `flightStep` comparte la integración de ascenso/curvatura con la suite nativa; aceleración y velocidad se transportan entre subpasos. Se comprueba el segmento completo contra la envolvente de cada ciudad pendiente antes de mover, incluyendo extensión de cápsula; una retirada desde dentro sigue permitida. Detener por esa barrera muestra el motivo y no simula un collider listo.

Caminar, gravedad, cápsula rotada y esfera Chaos de gran radio aún necesitan ensayo de motor. La exactitud matemática no certifica contactos ni penetración. El cap cercano se aproxima por triángulos: a R=25 km su error máximo de arista medido es 2,1973 cm, frente a 376,4483 cm de la malla orbital. A R=90 km esos errores escalan; la colisión sigue siendo analítica. Transición LOD, sombreado y horizonte deben revisarse visualmente antes de aceptación.

## Residencia, precarga y persistencia

Hay un único ActiveCity; ciudades próximas pueden tener colisión precargada, el resto proxies. Distancia de precarga = velocidad × p95 + velocidad²/(2×frenada) + margen. Valor inicial conservador p95=1,5 s; a 320 m/s, freno 80 m/s² y margen 300 m resulta 1.420 m. Se mide desde solicitud hasta preparación del estado físico, con hasta 64 muestras; el presupuesto mantiene el suelo conservador inicial. No se presenta como p95 real medido fuera de Unreal.

Selección incluye distancia geodésica y altura, histéresis de 400 m y revisión a 10 Hz. Antes de entrar se comprueba estado físico de calles, pad y edificios. Si no está listo, velocidad admisible deriva del presupuesto de frenada; no se permite fingir collider cargado. Esta primera implementación crea hubs pequeños sincrónicamente con componentes nativos y espera su estado físico cuando es diferido. No equivale a streaming asíncrono de paquetes urbanos reales ni garantiza ausencia de hitches: la sustitución por cargas/celdas preautorizadas se hará tras medir.

La comprobación actual revisa FBodyInstance de cada instancia y pad, además del cuerpo planetario. La carga de USaveGame valida poses e identidad contra defaults del manifiesto en un candidato y solo aplica el conjunto completo al terminar. Estado vivo inválido suspende guardado/descarga y conserva el último delta válido. Contactos/query real, carga diferida y reentradas siguen pendientes de motor.

Solo los props a menos de 1.200 m del jugador simulan; salir captura delta y destruye el actor, entrar rehidrata una sola instancia por clave. La selección usa la posición global guardada del objeto, no su ciudad de origen; un objeto movido a otra celda no vuelve a aparecer en su punto original. Se mantienen 12 registros representativos iniciales, no árboles/edificios completos. La esfera global no simula fragmentos lejanos. Un objeto descargado queda congelado, no resuelve una trayectoria balística fuera de burbuja.

## UMG y controles

`UIBS3MapWidget` calcula datos a 10 Hz y dibuja con UMG, sin SceneCapture2D. Mapa global equirectangular: ciudades, jugador, descubierto y destino. Minimap azimutal con rumbo arriba: rango 2 km, norte, flecha/marker de destino, distancia, lat/lon, altura, rumbo y región. Usa el marco transportado del Pawn, por lo que el minimapa no gira 180° al cruzar un polo. La brújula geográfica marca rumbo indeterminado en el polo; un destino exactamente antípoda se marca ambiguo, no se inventa una dirección. El mapa global tiene la costura cartográfica habitual; el mundo no tiene costura.

| Acción | Teclado | Mando expuesto como gamepad/Steam Input |
|---|---|---|
| Mover / mirar | WASD / ratón | sticks |
| Vuelo / caída | F | R3 |
| Alternar velocidad de travesía | Shift | L3 |
| Subir / bajar en vuelo | E / Q | R2 / L2 |
| Frenar | X mantenida | Cuadrado mantenido |
| Saltar | Espacio | Cruz |
| Interacción | G | Círculo |
| Mapa global | M | View/panel según Steam Input |
| Siguiente destino | Tab | Cruceta derecha |
| Guardar | F5 | Cruceta abajo |

Los bindings existen en código; ningún test físico de DualSense ni sesión por Steam se ha ejecutado. Este Pawn técnico no contiene todavía modelo/animaciones de H01. Para despegar, activar vuelo y ascender; para aterrizar, desactivar vuelo y usar caída radial. No se sustituye el acceso Steam existente por un laboratorio sin build validada.

## Reproducción y gates pendientes

Sin Unreal y sin coste adicional:

```powershell
./Tools/IBS3_TestCore.ps1 -OutputDirectory ../qa-core
./Tools/IBS3_TestCore.ps1 -Suite Review -OutputDirectory ../qa-review
python Tools/IBS3_TestAuthoring.py ../qa-geometry
```

Tras **coordinar explícitamente un turno exclusivo** con otras sesiones, revisar los scripts y ejecutar secuencialmente:

```powershell
./Tools/IBS3_RunUnreal.ps1 -Mode Build -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode Author -ExclusiveTurnConfirmed
./Tools/IBS3_RunUnreal.ps1 -Mode RuntimeTests -ExclusiveTurnConfirmed
```

El guard comprueba procesos y un mutex cooperativo; no sustituye coordinación con sesiones que no usan ese mutex. Build se limita a dos acciones. Author usa plugins Python/editor scripting nativos solo en el proceso, sin modificar .uproject. Crea `/Game/IB_S3/Lvl_IB_S3_PlanetLab`, manifiesto y mallas originales; rechaza sobrescribir assets/mapa existentes. El HUD UMG es nativo; cualquier Blueprint de UI futuro se crea solamente bajo Content/IB_S3.

`ImpactBeyond.S3.Manifest` prepara pruebas de novena ciudad/radio 90 km/IDs duplicados. `ImpactBeyond.S3.RuntimeResidency` prepara trazas de suelo en cuatro hubs, 10 descargas/reentradas, persistencia de muro/auto, unicidad y lectura de un archivo USaveGame de prueba con nombre GUID. Usa colocación de fixtures y **no cuenta como vuelo continuo, aterrizajes, reinicio completo del juego ni rendimiento**. El archivo temporal de QA es el único save que elimina.

`ImpactBeyond.S3.SaveValidation` prepara archivos de prueba con pose, identidad, duplicados y revisión inválidos; debe verificar rechazo completo, conservación del estado anterior y bloqueo de sobrescritura. Tampoco se ejecutó. Las suites nativas actuales suman **436.423 aserciones PASS** (336.251 Core + 100.172 Review); no sustituyen ninguno de estos tests de motor.

Después faltan: cuatro aterrizajes CMC; suelo/aire/giros en ecuador y ambos polos; cinco cruces físicos; 10 retornos con los tres objetos alterados y reinicio real; minimapa/mapa en tierra/aire/dateline/polos; 320 m/s con freno y fallo de precarga; vuelta continua ≥1 (roadmap general pide 3); empaquetado Win64 DX12; 1080p con audio, FPS/p95/p99/VRAM; Steam Input/DualSense y aceptación humana. No simular QA independiente: el mismo agente implementó y verificó las pruebas ligeras.

## Integración, procedencia y rollback

Solo se añaden rutas IB_S3 / IBS3_* / docs/IB_S3_*. Ningún Character/Circuit/GameMode global, mapa IB_C00, .uproject, Build.cs, configuración o checkout ajeno cambiado. Rama aislada desde clon parcial propio para evitar permisos/races del checkout activo. Sin compras, cuentas API nuevas, credenciales, permisos, instalaciones o cambios de seguridad: RD$0.

Fuentes técnicas oficiales consultadas: [CMC gravity](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UCharacterMovementComponent/SetGravityDirection), [SkyAtmosphere properties](https://dev.epicgames.com/documentation/unreal-engine/sky-atmosphere-component-properties-in-unreal-engine). APIs se cotejaron con headers instalados UE5.8.4. Catálogo local ofrece gpt-6-astra y gpt-6.1-sol con xhigh; **no se verificó ni cambió el modelo/esfuerzo efectivo de la sesión**.

El integrador decidirá qué adaptar a H01 y cómo sustituir los bloques por S2.5 después de validar. Sin merge automático. Rollback: no incorporar esta rama, o revertir sus commits y retirar su mapa de laboratorio; el mapa principal y build previa no dependen de ella. No hay que cambiar ni restaurar settings globales.
