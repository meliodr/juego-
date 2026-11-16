# S3.5 — contrato de referencia del laboratorio

**Semántica del receptor de laboratorio conservado.** La partida utiliza el [contrato canónico](../ImpactBeyond/Source/ImpactBeyond/IB_Integration/ImpactContract/README.md) y su sesión, evitando dos mutadores de salud. El [estado de integración](IB_S7_SOURCE_OVERVIEW.md) registra el snapshot actual; esta nota no certifica la ejecución de Unreal.

Implementación: `ImpactBeyond/Source/ImpactBeyond/IB_S35/IBS35Types.h`, `IBS35ReceiverComponent.*` y reglas compartidas `IBS35Policy.h`.

| Campo | Semántica y unidades |
|---|---|
| `AttackId` | FGuid nuevo por ataque o lanzamiento. Todos sus contactos conservan el mismo ID. |
| `DamageKind` | QuickPunch, HeavyPunch, HeldTool, ThrownObject. El integrador reconciliará otros tipos de S4. |
| `HealthDelta` | Solicitud finita <= 0; el receptor la reemplaza por el delta realmente aplicado al limitar salud a [0, InitialHealth]. 0 admite empuje sin daño. |
| `RequestedHealthDelta` | Salida del receptor: conserva la solicitud antes del clamp. Evita perder daño estructural solicitado cuando la salud restante es menor. No aplicar ambos campos a la salud. |
| `Impulse` | Vector kg·cm/s, espacio mundo. Golpes limitados por masa del blanco × MaxDeltaSpeed. |
| `InstigatorId` | FGuid del portador/origen del ataque, válido durante la sesión offline. No atribuir automáticamente al jugador eventos de otro origen. |
| `InstigatorActor` | Referencia débil opcional al actor original. Ayuda a adaptar la atribución S4 sin convertir automáticamente GUID desconocido en jugador; puede expirar sin invalidar InstigatorId. |
| `TargetId` | FGuid de la vida actual del receptor. Un ResetReceiver genera otro ID. No es un ID persistente de guardado. |
| `TargetPart` | Referencia débil al primitive realmente golpeado; validado contra Body y su propietario. Conserva selección de cañón/junta; no es un PartId persistente ni serializable. |
| `DamageResponseProfile` | FName resuelto por el receptor, no por el atacante. El laboratorio ofrece salud y límite de empuje; no implementa reglas policiales. |
| `Contact`, `RelativeVelocity` | Punto mundial cm y velocidad relativa cm/s. La curva de puño es una estimación reproducible; el lanzamiento toma velocidad pre-física. |
| `SurfaceMaterial` | Nombre del physical material si el hit lo proporciona; None se registra explícitamente cuando falta. |
| `AttackSeconds`, `bAirborne` | Tiempo desde inicio del ataque y estado aéreo del emisor del puño. Un objeto lanzado no se clasifica como un personaje aéreo. |
| `bPhysicsResolved` | True para choque ya resuelto por Chaos: registrar daño, **no aplicar otro impulso**. |

`Receive(Message, Body)` devuelve false si el mensaje es inválido, pertenece a otra vida, ya se recibió ese AttackId o se agotó el historial. Rechaza DamageKind fuera del enum, edad negativa, salud inválida, Body de otro actor y TargetPart contradictorio/caducado antes de consumir el ID. En éxito modifica Message con parte, daño solicitado y perfil/delta/impulso resueltos y publica OnDamage. El emisor de puño publica OnHit con ese mismo mensaje resuelto. La deduplicación vive en el receptor, por tanto varios colliders del mismo actor no multiplican el daño. Debe haber **un receptor por blanco lógico**.

El historial conserva hasta 4.096 ataques por vida. No expulsa IDs antiguos: al llenarse rechaza nuevos impactos hasta un reset explícito, para impedir replays por rotación de caché. Es un límite del laboratorio que exige decisión de integración para una sesión larga. Los resets completos también cancelan ataques/lanzamientos; reiniciar únicamente un blanco no es un reset del mundo.

Los valores de daño son tuning provisional, no un modelo estructural: golpe rápido 20, pesado 60; impacto lanzado depende de masa y velocidad normal, limitado a 300. Material y velocidad viajan en el mensaje para la respuesta S4, pero no se inventa una tabla universal de resistencia. Los muros del laboratorio son sólidos y receptores; no simulan destrucción ni prometen huecos.

## Integración actual

El host conecta S3.5 a S4 mediante ImpactContract y conserva identidad de sesión/vida, parte y atribución. No castear enums por ordinal, truncar GUID, confundir edad con reloj absoluto ni conectar dos receptores mutadores en cadena. El receptor legacy permanece para pruebas aisladas.

Las pruebas nativas integradas se ejecutan con `Tools/IBCI_Native.py`. La comparación S35/S4 se actualizó para exigir rechazo del replay con timestamp renovado. Los tests Unreal incluidos requieren ejecución y registro separado contra el snapshot final; no se infiere colisión o jugabilidad desde las políticas puras.
