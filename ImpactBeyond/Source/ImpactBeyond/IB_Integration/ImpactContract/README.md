# Contrato canónico de impactos v1 — S3.5 ↔ S4

**Nota de integración pública:** la candidata exportada usa este contrato desde Playable. El estado vigente y los gates se encuentran en [la guía S7](../../../../../docs/IB_S7_SOURCE_OVERVIEW.md). Las referencias a ramas, entregas y pruebas anteriores de esta nota son historial técnico, no estado de aceptación de la candidata actual.

Estado: núcleo C++ nativo COMPILADO/EJECUTADO; glue UE IMPLEMENTADO SIN COMPILAR.
No representa integración jugable, colisión probada, ni validación DX12. No se
ejecutaron Unreal/UBT/cook/GPU ni se publicó esta rama. Coste externo RD$0.

## Entrega y dependencia

- Rama propia: `prototype/ib-impact-contract-adapter`.
- Ancestro de la entrega inicial: `1ffe10085bb8c7e3afc44ea14a863fa05816d15a`.
- Ancestro de la corrección de binding: `3c5d3da490111c44f4f7aa85e78d67bf027dc163`.
- Respaldo previo: `backup/ib-s35-before-contract-20261116` en ese ancestro.
- S3.5 requerido por el glue: `IBS35Types.h` de ese ancestro (incluye RequestedHealthDelta,
  InstigatorActor y TargetPart). No se modifica su API ni su receptor.
- S4 código requerido: `82865397b1034ee619b9ed315dd6172291d3c1f6`, entrega completa
  `1a5451a6a8a216a4f7d74e8f6654248aadfcb106` sobre `7755e366...`; `IBS4Policy.h`,
  `IBS4ResponseComponent.h/.cpp`, y consumidor `IBS4PreparedTarget.cpp`.
  Requiere `FIBS4Outcome`, `LastOutcome`, `MakeHit`, `BindConsumer`, `ConsumeBoundHit`,
  `UnbindConsumer`, `HasConsumerBinding` y `Receive` de esa versión.
  Corrección de procedencia: el reporte inicial rotulaba 7755e366, pero sus hashes
  ya correspondían al código 82865397. Se verificaron ahora contra los blobs Git
  exactos, sin modificar el checkout S4. No atribuir esas pruebas a 7755e366.
- Destino informado por el integrador: staging `5571818a410e202d2be5b6101ef1f4aec7bf134b`,
  basado en `87d266c6cd7b6eb355cd641c97243471f0c2987d`. No se modifica ni se toma
  ese checkout como donante de trabajo incompleto.
- Todos los archivos de esta entrega viven en
  `ImpactBeyond/Source/ImpactBeyond/IB_Integration/ImpactContract/`.
  Incorporar el commit incremental identificado en el reporte final, no mezclar
  automáticamente todo el historial del laboratorio. El núcleo no requiere UE;
  el header UE se incluye explícitamente cuando ambos laboratorios estén integrados.

Se revisaron AGENTS, PLAN_ACTIVO, TAREAS_ACTIVAS y las skills locales gameplay-gate
y qa-review. Se reutilizan los receptores, las políticas y el asignador real S4;
el código nuevo solo resuelve incompatibilidades del sobre y la entrega. No se
crea motor físico, componente de personaje, sistema de salud rival ni importación
de arte. No hay cambios en Character, mapas, Build.cs, Config ni código S4.

## Archivos y entrada para consumidores

- `ImpactContract.h`: `IBImpact::Envelope`, `Outcome`, `TargetRouter`, conversiones
  explícitas y planificación de impulso adicional. C++17 sin motor.
- `ImpactContractUE.h`: `IBImpactUE::S4Bridge`, opt-in sin UObject/UHT nuevo.
  Traduce las firmas reales de ambos laboratorios. Solo S4 muta HP y emite su
  `OnResponse`; el adaptador añade impulso físico cuando corresponde.
- `Tests/ImpactContractNative.cpp`: ejecuta el núcleo y la política S4 real.
  El `main` solo existe con `IB_IMPACT_NATIVE_TEST`; no entra al juego por UBT.
- `Tests/RunNative.ps1`: compilación MSVC de una unidad, hashes S4 antes/después,
  logs y reporte fuera del repositorio. Sin instalación ni compilación Unreal.
- `Tests/ImpactBridgeAutomation.cpp`: regresión del bridge UE real con dueño,
  binding, ingress, H01, dedup y física resuelta. Solo se compila cuando S4 está
  presente y WITH_DEV_AUTOMATION_TESTS. Preparada, NO compilada/ejecutada aquí.

El integrador crea **un S4Bridge por vida del receptor**, en el game thread:

```cpp
#include "IB_Integration/ImpactContract/ImpactContractUE.h"
// Guardar la instancia; NO construir un bridge nuevo por contacto.
Bridge = MakeUnique<IBImpactUE::S4Bridge>(*Response, *BindingOwner, WorldSessionGuid,
    TargetLifeGuid, FName(TEXT("S4.Prepared.Metal")));
// S35: reemplaza el Receive antiguo, antes de cualquier mutación de HP.
const auto Record = Bridge->ReceiveS35(RawMessage, ExactContactBody, StablePartGuid);
// S4: llamada desde el callback registrado por BindConsumer, conservando MakeHit.
const IBImpactUE::S4Identity Identity{WorldSessionGuid, TargetLifeGuid,
    InstigatorGuidOrZero, StablePartGuid, bPhysicsAlreadyResolved}; // capturada con el contacto
const auto OtherRecord = Bridge->ReceiveS4(OriginalS4Hit, Identity);
```

Las dos llamadas ilustran entradas alternativas. No entregar el mismo contacto
por ambas. No llamar `UIBS35ReceiverComponent::Receive` y luego al bridge. Para
ingress S4, `Receive` invoca el callback vinculado; este llama al bridge y el bridge
termina exclusivamente en **`ConsumeBoundHit(BindingOwner.Get(), Hit, E.PhysicsResolved)`**.
No volver a llamar Receive desde Dispatch. Se necesita redirigir el punto
de entrega del productor en la candidata, decisión exclusiva del integrador.
Este commit no redirige ni adjunta nada al personaje o mapa.

Firmas exactas de la corrección (ReceiveS4 conserva su firma):

```cpp
S4Bridge(UIBS4ResponseComponent& Target, UObject& Owner,
    FGuid WorldSession, FGuid TargetLife, FName ResponseProfile);
Record ReceiveS4(const FIBS4Hit& Input, const S4Identity& Identity);
// S4Identity: SessionId, TargetId, InstigatorId, PartId, bool bPhysicsResolved=false.
```

Ejemplo de registro: Bridge, CapturedBySerial, LastRecord y NativeIngress deben
vivir en el dueño mientras dure el binding; aquí se muestran variables ya vivas.
`CapturedBySerial` es un `TMap<int64, IBImpactUE::S4Identity>` del dueño. Se llena
al contacto/encolado original, nunca al recibir un replay con la vida actual.

```cpp
NativeIngress.BindLambda([&](UIBS4ResponseComponent&, const FIBS4Hit& Hit)
{
    const auto* Identity = CapturedBySerial.Find(Hit.AttackId);
    if (!Identity) return; // sin identidad capturada, no consumir
    LastRecord = Bridge->ReceiveS4(Hit, *Identity);
});
const bool bBound = Response->BindConsumer(BindingOwner, NativeIngress);
// Solo si bBound: las entradas nativas Response->Receive(Hit) ya pasan por el bridge.
// En el teardown fuera de callbacks, antes de liberar el bridge:
const bool bUnbound = Response->UnbindConsumer(BindingOwner);
```

El constructor no registra automáticamente ni crea otro dueño. Dispatch exige
dueño vivo y binding presente, y S4 verifica que sea el dueño exacto. Sin binding,
con dueño ajeno/destruido o rechazo del consumidor, no hay fallback al laboratorio.
No cambiar bH01 temporalmente. Unbind/rebind conserva el bridge, la vida y ledger;
si no se pudo desvincular, no destruirlo dejando su callback colgante.

S6/S7 consumen `Record` **solo si `Record.Result.State == Status::Accepted`**.
Es una notificación de efectos ya pedidos, no otro comando de daño. Conservar
Attack, TargetId, InstigatorId, PartId y Profile al producir alertas/consecuencias.
Los IDs cero de instigador S4 son atribución desconocida; nunca asumir jugador.
Los WeakObjectPtrs son pistas locales, no identidad serializable. No incluir
otro adaptador de enums/signos en S4/S6/S7.

## Semántica y conversiones

| Campo | Contrato y traducción |
|---|---|
| DamageKind (`Envelope.Kind`) | Ocho valores explícitos. QuickPunch/HeavyPunch/HeldTool/ThrownObject se entregan como **S4 Punch**. Se conserva el detalle original en Record. Bullet/Missile/Laser/Punch nativos se conservan. Nunca cast ordinal; valores desconocidos se rechazan. |
| HealthDelta | Entrada S35: `HealthDelta <= 0` antes de Receive. Entrada S4: `Amount >= 0`, convertido a negativo. `Envelope.RequestedHealthDelta` conserva el pedido bruto. `Outcome.RequestedHealthDelta` es el pedido filtrado por S4 (inmunidad, tope 1000). `Outcome.HealthDelta` es pérdida real del float HP, incluso cero/sub-ULP. No es un segundo daño. |
| Destrucción S4 | `OnResponse` sigue recibiendo RequestedHealthDelta para resistencia estructural, incluso con HP agotado. El adaptador no vuelve a notificarlo ni rompe piezas por su cuenta. Mantiene Part exacta. |
| Impulse | Vector **kg·cm/s**. En S35 se preserva el pedido. En Outcome es el impulso adicional pedido por el bridge al body, limitado por masa real × MaxDeltaSpeed (2000 cm/s por defecto, igual al laboratorio S35). |
| DeltaVelocity | Vector **cm/s** de S4 `LastOutcome`, pedido a CMC con cambio de velocidad independiente de masa. No es impulso ni velocidad observada. No se reenvía a Chaos. |
| Conversión de unidades | `dv = J / masa_real_kg`; `J = masa_real_kg * dv`. Ambas funciones exigen masa positiva finita y rechazan overflow. No estiman la masa del Character ni usan un perfil de coche en su lugar. El bridge conserva los dos campos, no hace esta conversión automáticamente. |
| Física ya resuelta | `PhysicsResolved=true` produce `AlreadyResolved` y **cero impulso adicional**. Un body sin simulación produce `NotSimulating`; masa/límite inválidos producen `InvalidBodyMass`. Son resultados explícitos, no simulación verificada. Los golpes S35 contra Character sin body simulado no adquieren empuje CMC nuevo con este adaptador. |
| InstigatorId | GUID exacto de S35, o GUID de registro que el integrador suministre para el actor S4. Zero en S4 significa desconocido. Nunca derivar del puntero/nombre ni atribuir al jugador por defecto. |
| TargetId / PartId | TargetId representa una **vida** del receptor. Todas sus partes comparten dedup. PartId identifica contacto para desarme/evidencia, no otorga daño adicional. La instancia UE verifica pertenencia real del primitive al dueño. |
| DamageResponseProfile | Nombre explícito al construir el bridge; vacío se rechaza. Etiqueta de enlace/observación, no configura por sí misma bH01, Surface o cooldown S4. Esos valores permanecen en el receptor real. |
| Velocidad/material | Contact cm, RelativeVelocity cm/s solo si HasRelativeVelocity. Direction es unitario/sin unidades y separado. S4 no suministra velocidad relativa ni material físico: no inventarlos. |

Con el consumidor vinculado, `bH01=true` conserva HP para **todos** los tipos
aceptados, incluidos Punch/Laser y los golpes/lanzamientos S35 traducidos a Punch.
La inmunidad limitada Bullet/Missile solo describe el laboratorio sin binding,
al que el bridge no hace fallback. Se conserva empuje de misil máximo 1200 cm/s
y cooldown mínimo 0.5 s. PhysicsResolved reserva esa ventana sin segundo empuje.
El bridge no decide un perfil global de balance. Mantener S4 como único mutador significa
su límite de salud 100 y tope por pedido 1000; no migrar silenciosamente HP de
S35 (1000 por defecto) a ese receptor. Cualquier cambio de balance es otra decisión.

## Identidad, reloj y deduplicación

`AttackId = {Origin, SessionGuid, NativeGuid128, NativeSerialInt64}` es una unión
validada: S35 usa GUID completo y serial cero; S4 usa serial positivo y GUID cero.
No se trunca, hashea ni interpreta un GUID como int64. SessionGuid se crea una vez
por mundo/sesión y cambia al recrear/reiniciar su reloj; no por contacto.

`TargetRouter` guarda una entrada por `(AttackId, TargetId de vida)` y jamás
expulsa entradas. Máximo 4096 por vida: al llenarse rechaza, no vuelve vulnerable
al replay. No ofrece Reset; no se puede copiar/mover. `RestoreHealth` mantiene
la instancia. Para respawn real crear TargetLifeGuid nuevo y bridge nuevo; un
paquete encolado conserva su vida anterior y se rechaza. S4Identity lleva también
sesión capturada para rechazar paquetes de un mundo previo, pues FIBS4Hit no la trae.

S35 no comparte el contador privado S4. Al primer contacto con **cada vida de
blanco**, el bridge usa `UIBS4ResponseComponent::MakeHit` para obtener un serial
global y tiempo de entrega. Ese serial es un ID de transporte, conservado junto
al AttackId canónico original. El mismo lanzamiento puede golpear otro blanco
cinco segundos después con otro serial de transporte, pero el mismo AttackId.
Esto evita usar timestamps de lanzamiento vencidos o reciclar un serial retirado.
No son dos daños al mismo blanco: el ledger canónico lo impide toda la vida.

Los paquetes S4 nativos conservan su serial e IssuedAt original, sin asignación.
Un reintento tras rechazo conserva primer sobre, parte/actor y timestamp. No
refresca entrega ni aumenta daño. Si expira, queda rechazado; debe ser un ataque
nuevo real para crear otro ID. Un resultado incierto/aceptado inválido se sella
sin reintento. `false` del receptor significa **ninguna mutación**.

AttackAgeSeconds de S35 es edad relativa de gesto/lanzamiento (HasAttackAge=true).
S4 carece de edad de ataque; HasAttackAge=false. ContactWorldSeconds y S4 IssuedAt
usan `UWorld::GetTimeSeconds`, no UTC ni edad relativa. Para el núcleo, la primera
captura no puede tener más de 2 s de antigüedad ni más de 0.01 s en el futuro.
La entrada UE S35 debe llamarse en el contacto real y lo sella entonces. No sirve
para reproducir mensajes históricos fuera de ese camino. Reintentos usan el sello
original. Retroceso del reloj dentro de la misma instancia se rechaza.

El receptor S4 mantiene además su ledger de 256 paquetes vivos, TTL 2 s y marca
monótona `retiredThrough`. No se elude su rechazo por saturación/retirada ni se
fabrica un contador rival. La instancia bridge bloquea reentrancia incluso desde
OnResponse; todos los productores del blanco deben pasar por esa instancia.
Un listener que llame al receptor S4 directo incumple el contrato y puede alterar
LastOutcome: no se declara seguro ese bypass.

## Pruebas reproducibles y límites

Ejecutar desde el checkout propio con rutas locales explícitas:

```powershell
& .\ImpactBeyond\Source\ImpactBeyond\IB_Integration\ImpactContract\Tests\RunNative.ps1 `
  -S4Root <checkout-S4-82865397> -OutputDirectory <carpeta-propia-de-resultados>
```

Resultado observado tras corregir binding: **1038 aserciones PASS** con MSVC C++17,
`/W4 /WX /Od` (706 previas + 332 de política bound/reentrada/dedup/physicsResolved).
El runner usa el header real S4, no una copia de su política. Casos: todos los
enums; signo/tope/inmunidad; HP cero/overkill/redondeo; GUID completo/serial
INT64_MAX; sesión/vida inválida; part multi-contacto; replays/restauración;
límites temporales exactos y reloj que retrocede; lanzamiento tardío a nuevo
blanco; ledger nativo 256 lleno y retiro; ledger canónico lleno; reentrancia,
excepción y resultado incierto; masa inválida, overflow, dirección de impulso,
contacto ya resuelto y body sin simulación. La física solo se calcula como
plan numérico: **no** ejecuta Chaos, CMC, sweeps, UObjects ni delegates reales.
El runner también verifica estáticamente que el bridge use el endpoint bound,
dueño/guardas, firma ReceiveS4 y propagación de PhysicsResolved. No confundir esa
verificación de fuente con ejecutar el binding. La regresión real pendiente es
`ImpactBeyond.Integration.ImpactContract.BoundBridge`; la suite usa el bridge
real, no un consumidor alternativo ni un mock de UObject.

Pendiente del integrador con reserva exclusiva: compilar el header UE en una
unidad real de la candidata (su presencia sola no lo compila); redirigir las
entradas sin doble receptor; validar OnResponse/LastOutcome y destrucción de
parte durante callback; verificar IDs y perfil del registro de actores; 30 hits
suelo/aire, lanzamiento/muros y resets reales; pruebas S4 consumidor independientes;
UHT/UBT, headless, DX12, mando y rendimiento. El mismo autor ejecutó estas pruebas
nativas: no constituyen revisión independiente ni aprobación jugable.

Rollback: revertir solo el commit incremental en la candidata propia. No tocar
el respaldo S2+S3, las ramas de otros agentes ni el PR histórico #26. Esta entrega
no se publica por la instrucción vigente del usuario.
