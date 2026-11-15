# IMPACT BEYOND — Laboratorio radial y contrato de regiones

## Recorrido ciudad–costa

El distrito urbano termina en `X = 620 m`. El terreno costero empieza allí y la autovía continúa hacia el mirador en `X = 1783 m`. El primer recorrido transcurre en el nivel urbano `Lvl_IB_C00` y vuelve por la misma autovía sin cambiar de mapa. Las coordenadas en este documento son las del nivel actual. La primera verificación de persistencia consiste en alterar un vehículo o panel preparado en la ciudad, pasar por el mirador y volver a comprobar su estado.

| ID estable | Área | Límite del corredor inicial |
| --- | --- | --- |
| `earth.city.primary` | Distrito, plaza y avenida | Oeste de `X = 620 m` |
| `earth.coast.east` | Autovía, periferia y mirador | Desde `X = 620 m` hacia el este |

Para la futura activación de regiones, usar una banda de solape `X = 600–640 m`. La región activa cambia hacia el este al llegar a 640 m y vuelve a ciudad al llegar a 600 m. La banda evita alternancias al volar cerca del límite y mantiene preparada la colisión de ambos lados. El límite describe el corredor y podrá sustituirse por volúmenes contiguos al ampliar el terreno.

## Coordenadas y estado

- **Global.** Marco cartesiano centrado en el planeta, unidades Unreal en centímetros y `FVector` de doble precisión. Guardar un `BodyId` estable junto con posiciones globales que lo requieran. En el laboratorio, `Up = normalizar(Posición − Centro)` y la gravedad apunta en dirección `−Up`. El radio de la esfera de prueba es de 1 km.
- **Local.** Cada región define su origen global y una rotación ortonormal con +Z radial y +X tangente. Convertir posiciones mediante `Global = Origen + Rotación(Local)`. Rotar velocidades y direcciones sin sumar el origen. El nivel actual conserva su marco local con +Z vertical. El punto de partida para ensayar gravedad regional es `UCharacterMovementComponent::SetGravityDirection` en un mapa de prueba.
- **Persistencia.** Identificar los actores persistentes mediante `(RegionIdDeOrigen, IdDeActorEstable)` y guardar un esquema con versión. Aplicar un registro de cambios al cargar la región, con estados como panel abierto o reparado y transformación de un vehículo desplazado. Si un vehículo cambia de región, mantener su ID y registrar su ubicación actual para evitar duplicados. Descartar escombros transitorios al descargar. El reinicio explícito del circuito limpia sus cambios de prueba. Validar primero el retorno en la misma sesión y después la restauración desde `USaveGame`.

## Precarga por trayectoria

Usar posición y velocidad medidas para proyectar el trayecto y pedir contenido de las regiones cuyos límites intersecte. Para el primer ensayo, `T = máximo(1,5 s, tiempo de carga p95 + 0,5 s)` y `distancia = velocidad × T + 100 m`. A 320 m/s y con el mínimo de 1,5 s se anticipan 580 m. Al acelerar, incluir el avance previsto durante ese horizonte. Dar prioridad al suelo y las colisiones del corredor, y conservar la región anterior mientras el jugador pueda alcanzarla de nuevo durante el mismo horizonte. Medir el primer vuelo y el regreso antes de ajustar el margen. Explorar una segunda fuente de streaming nativa de World Partition adelantada para el siguiente prototipo.

## Verificación aislada

Ejecutar `Automation RunTests ImpactBeyond.Planet.RadialLab` en una sesión de Unreal con automatización habilitada. La prueba verifica la dirección de la gravedad en polos y ecuador, la conversión local/global en un origen lejano, el desnivel de un arco de 200 m respecto a su plano tangente y una caída sobre la esfera. El ensayo matemático funciona sin mapa, assets ni renderizado. El guardado, la precarga y la colisión de un personaje sobre terreno curvo quedan para verificación independiente al integrar los contratos.
