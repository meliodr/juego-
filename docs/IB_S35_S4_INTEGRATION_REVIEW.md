# S3.5 ↔ S4: revisión local del contrato y adaptadores

Fecha: 2026-11-15. **Revisión y pruebas nativas; no integración Unreal.** Se leyó el checkout S4 indicado por coordinación, sin modificarlo. Ref local observada: `prototype/ib-s4-response-lab`, `3b7c5c7780b454e2c80af2fce2fae8523663c3b6`. Las conclusiones se fijan a los bytes SHA-256 de `IB_S35_S4_NATIVE_RESULTS.json`, no solo a un HEAD que podría tener cambios locales.

Se inspeccionaron `IBS4ResponseComponent.h/.cpp`, `IBS4Policy.h`, `IBS4PreparedTarget.h/.cpp`, `IBS4PowerComponent.h/.cpp` y `IBS4Threat.cpp`, además de su documento de laboratorio. No se añadieron includes IB_S4 al runtime S3.5, componentes al personaje/mapa global, ni cambios a S4. El único include cruzado está en una herramienta de prueba nativa fuera de Source.

## Divergencias verificadas

| Aspecto | S3.5 | S4 observado | Efecto de una conversión directa |
|---|---|---|---|
| Tipo | QuickPunch=0, HeavyPunch=1, HeldTool=2, ThrownObject=3 | Bullet=0, Missile=1, Laser=2, Punch=3 | HeavyPunch pasa a Missile: H01 recibe 0 daño y empuje; QuickPunch se vuelve bala inmune. Tool/Thrown no tienen equivalente semántico S4. |
| Daño de entrada | HealthDelta <= 0 | Amount >= 0 | Enviar -60 como Amount es rechazado. Negar **la solicitud** es necesario para una ruta explícita de puño. |
| Daño de salida | HealthDelta es pérdida efectiva después del clamp | OnResponse emite Result.healthDelta de Policy, antes del clamp del componente | Con 10 HP y solicitud 60: S3.5 emite -10; S4 emite -60 aunque también deje HP=0. PreparedTarget acumula ese -Delta para BreakThreshold. |
| Impulso | FVector en kg·cm/s; se aplica a primitive simulado y se limita por masa | FIBS4Hit solo tiene Direction. Policy produce pushSpeed cm/s para Missile; CMC AddImpulse(..., true), limitado a 1.200 y con cooldown | 120.000 kg·cm/s / 1.200 kg son 100 cm/s. Usar 120.000 directamente en push da 1.200 por clamp: 12×. Un Punch S4 no aplica el impulso S3.5. |
| Choque físico | bPhysicsResolved=true evita otro impulso tras Chaos | No existe ese flag en FIBS4Hit | Volver a tratar el choque como empuje de arma puede duplicar la respuesta. No convertirlo en Missile para conseguir knockback. |
| AttackId | GUID 128 bits por golpe/lanzamiento | int64 positivo de secuencia estática privada en MakeHit | Truncar GUID causa colisiones; generar otro ID por callback pierde dedup. Un contador del adaptador independiente puede colisionar con el contador nativo S4. |
| Tiempo | AttackSeconds es edad relativa; lanzamiento admite contactos hasta 10 s | IssuedAt es tiempo absoluto del UWorld; mensaje válido 2 s, tolerancia futura 0,01 s | Edad 0,1 enviada a un mundo en t=100 es vieja; un lanzamiento de 4 s falla si se toma inicio como IssuedAt. Hay que acordar qué evento se fecha. |
| Dedup | 4.096 IDs por vida; no expulsión; fail closed al llenar | 256 IDs vivos, expiry=issued+2,01; reutiliza slots caducados | La saturación y duración difieren. Refrescar IssuedAt de un ID viejo lo hace aceptable de nuevo en S4: el adaptador debe conservar el tiempo original del evento. |
| TargetId/reset | GUID explícito por vida; reset renueva ID y limpia ledger/counters | Target implícito en componente; RestoreHealth conserva ledger/cooldown; ObjectId FName de PreparedTarget sirve a snapshot | No equiparar RestoreHealth con una nueva vida. El mismo payload puede tener tratamiento distinto tras reset. |
| Instigador | GUID; ahora también actor débil opcional del emisor real | AActor* usado para atribuir incidentes/alerta | Nunca convertir GUID desconocido en jugador. Con actor destruido se requiere política de atribución, no un actor inventado. |
| Pieza | Antes solo Contact y TargetId; ahora TargetPart preserva primitive exacto | FIBS4Hit.Part; robot exige Hit.Part == Pieces[1] | La posición sola no identifica el cañón: perder Part impide desarme o permite asignarlo incorrectamente. |
| Respuesta | DamageResponseProfile FName del receptor genérico | bH01, Surface enum, MissilePushSpeed y PushCooldown | El nombre/material no contiene inmunidad H01 ni anti-stunlock. No se puede sustituir el receptor S4 por el genérico y conservar la política automáticamente. |

La salud de S3.5 no se considera una segunda autoridad para los blancos S4. `SurfaceMaterial` (physical material FName) tampoco es `EIBS4Surface`: hace falta una tabla explícita Metal/Concrete/Glass y rechazo/fallback documentado para materiales desconocidos.

## Correcciones propias realizadas

1. **Información de contacto perdida:** `FIBS35DamageMessage` conserva TargetPart e InstigatorActor como referencias débiles. Punch y ThrownObject las rellenan con el contacto y origen reales. El receptor exige Body del mismo actor, rechaza parte contradictoria/caducada y publica la pieza validada. No convierte el puntero en identidad persistente.
2. **Solicitud destruida por el clamp:** RequestedHealthDelta conserva el daño pedido mientras HealthDelta sigue representando pérdida real. Esto permite a un futuro consumidor estructural distinguir resistencia y salud, sin aplicar salud dos veces. La operación usa la precisión de la salud almacenada; un delta inferior a la precisión float no se reporta como pérdida efectiva.
3. **Entrada inválida aceptable por el receptor:** se rechazaron DamageKind desconocido, AttackSeconds negativo y salud no finita/fuera de rango antes del ledger. Antes esos tipos/edades podían consumir un ID y modificar salud. ValidDamageRequest y AppliedDamageDelta son código compartido entre runtime y pruebas ligeras.

Se añadieron casos Automation de Body ajeno, pieza incorrecta, enum inválido, edad negativa, ID no consumido al rechazar, clamp y parte publicada. **Están preparados, no ejecutados**: el turno pesado sigue reservado a S2.5.

## Secuencia de adaptación propuesta, sin aplicar

1. Elegir **un único receptor mutador** por blanco. Enrutar antes de aplicar daño. No conectar OnDamage de S3.5 a Receive de S4 y mantener ambos activos: la deduplicación es local y no impide dos mutaciones/FX/impulsos en dos receptores.
2. Traducir QuickPunch/HeavyPunch a Punch con switch explícito, preservando categoría original en metadatos. HeldTool/ThrownObject quedan sin traducción hasta añadir un tipo/consumidor acordado; no fingir que son láser o misil. Para solicitudes entrantes a S4 usar Amount=-HealthDelta **antes de resolver**, no el delta clamped. Para observadores posteriores usar RequestedHealthDelta, sin volver a ejecutar una segunda salud.
3. Resolver TargetId a actor/receptor/vida y validar TargetPart. S4 HandleResponse es privado y modifica estructura al recibir su evento; no hay hoy una API pública equivalente que admita el mensaje S3.5 completo sin pasar por su salud. Ese adaptador exige una revisión de integración posterior.
4. Compartir un asignador de IDs o un registro sin colisiones que incluya origen/sesión. No es seguro usar hash/truncado de GUID ni un segundo contador que desconozca MakeHit. Para el sentido S4→S3.5, reutilizar la misma traducción GUID para cada ID del mismo mundo/evento, nunca NewGuid por contacto.
5. Acordar reloj, TTL y evento: fecha de emisión/impacto y vida del blanco. No cambiar IssuedAt al reentregar. Para proyectiles >2 s, decidir explícitamente si ampliar validez o representar un evento de contacto estable por ataque/blanco; no hacer refresh silencioso. Distinguir restaurar HP de respawn/nueva vida y conservar cooldown cuando corresponda.
6. Aplicar física una sola vez. Si bPhysicsResolved, no añadir impulso. Si es CMC, usar conversión a delta-velocidad con masa/política explícitas; preservar inmunidad bala/misil H01, límite y cooldown S4. Mantener efecto cosmético separado del resultado aceptado.

## Agarre de piezas: otro bloqueo real

S4 usa PhysicsHandle sobre UPrimitiveComponent etiquetado IBS4Grabbable/IBS4Held, masa máxima predeterminada 250 kg y radio envolvente. Su cañón desprendido es Pieces[1] del mismo actor robot; no es AIBS35Object. S3.5 busca actores AIBS35Object con root UBoxComponent y Lease; el coche/contenedor pesa 1.200–2.200 kg. Añadir un tag S4 a un objeto S3.5 no reconcilia las reservas, masas, collider ni los resets.

El integrador deberá reemplazar la reserva PhysicsHandle por un adaptador del agarre elegido, o transferir una pieza preparada a un cuerpo canónico desactivando su cuerpo anterior. No mantener dos cuerpos ni dos dueños de la misma pieza. El tag IBS4Held que bloquea restaurar el cañón tampoco refleja automáticamente la Lease S3.5. Este encargo no cambia esas rutas de S4.

## Pruebas y evidencia

- `Tools/IBS35_TestPolicy.ps1`: **2.613 aserciones**, C++17 MSVC /W4 /WX, incluyendo 18 casos nuevos de validación/clamp. Mismos helpers usados por el receptor S3.5.
- `python Tools/IBS35_CompareS4.py`: **280 aserciones** compilando los headers nativos reales de ambos laboratorios. Reproduce diferencias de enum, signo, delta solicitado/aplicado, reloj/TTL, replay, capacidad, unidades e inmunidad H01. El runner comprueba nombres/orden de enums de los headers UE y hashes de S4 antes/después.
- `IB_S35_S4_NATIVE_RESULTS.json`: resultado observado, snapshot S4 y hashes. El PASS significa que los contraejemplos se reprodujeron, **no que los laboratorios sean compatibles**.
- Primer intento del runner: bloqueado al escribir en TEMP administrado. Se movieron los artefactos a una subcarpeta nueva de la tarea S3.5 autorizada; no se cambiaron permisos ni se borraron carpetas ajenas. La prueba posterior terminó.

Pendientes: UHT/C++ Unreal, serialización/lifecycle de referencias débiles, ruta real de cañón vs torso, H01/CMC, choques Chaos, foco y pruebas DX12. Durante esta revisión no hubo Editor/Cook/build pesado, nuevos intentos de push/PR ni solicitud de autorización al usuario. La publicación estaba bloqueada entonces; la autorización posterior y su estado se registran en `IB_S35_EVIDENCE.md`.
