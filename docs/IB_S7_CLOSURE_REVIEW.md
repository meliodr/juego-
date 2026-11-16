# Versión jugable S1–S7: validación de entrega

C++ `fa75f7f`, autoría `730b39f`, configuración/validador `17d7f91`. Paquete final local `8b56a20`, run `s7-package-20261116T060244Z`. **Build, autoría, pruebas funcionales del paquete y perfil DX12/audio aprobados.** La evaluación humana y el mando físico se registran aparte.

## Evidencia final

| Gate | Resultado y alcance |
|---|---|
| CI pública `8acc379` | Integrated source checks `43815842275` y Lab audit tests `43815842311` PASS; 13 suites nativas |
| Compilación | Game `fa75f7f` PASS, `s7-build-20261116T054825Z`, 114,26 s. Último Editor `6b4631d`; cambio posterior limitado a QA de framebuffer/GPU y dependencias RHI |
| Autoría | `s7-author-20261116T060201Z` PASS: once materiales verificados; solo `agua` cambió y se guardó localmente. Partida generada sin dependencias ausentes |
| Paquete final | `s7-package-20261116T060244Z` PASS, 37,01 s |
| Funcionales del ejecutable | Doce suites PASS, `s7-tests-20261116T055829Z`, ejecutable exacto `fa75f7f` |
| Funcionales del paquete DX12 | `s7-tests-20261116T060433Z` PASS: PlayableSession, RealInput, S3 Contacts y S7 PreparedEncounter; cero recursos ausentes/materiales de reserva |
| Reentrada S3 | Diez regresos físicos PASS en S3Lab, `s7-tests-20261116T053404Z` |
| Steam | Arranque visible verificado: JUGAR pasó a DETENER y apareció ImpactBeyond (64-bit Development PCD3D_SM6). Ejecutable final, Steam Input activado, mapa Playable, DX12 y audio Realtek confirmados; sin conceder red ni cambiar firewall |

SHA-256 del ejecutable probado: `6C7741F10E1350CF56A8A80C84B34300CBB2A29E33E8E3BB3B09DD5DEEAE4337`. El ajuste final del agua solo cambió contenido local; la comprobación funcional y el perfil siguientes utilizaron el paquete actualizado. El [manifiesto](IB_S7_PUBLIC_EXPORT.json) conserva revisiones e intentos históricos separados de estos resultados.

## Perfil del paquete final

Run `s7-tests-20261116T060613Z`: **90,011 s, 5.395 fotogramas**, recorrido local de 208,47 m y 75,01 s de vuelo. DX12 con audio y reloj real, salida **1920×1080**, escala interna **83%**, calidad **alta/2**, límite **60 FPS**. Fortnite ausente; se midió el equipo en su estado real: Intel Core i9-14900HX y NVIDIA GeForce RTX 4070 Laptop GPU, confirmados por los metadatos CPU/RHI del log.

| Métrica | Resultado |
|---|---:|
| FPS medios | 59,9253 |
| Fotograma p95 / p99 | 16,6848 / 16,7421 ms |
| Fotograma máximo | 93,0082 ms |
| Fotogramas >50 ms | 1 |
| GPU media / p95 / p99 | 6,8739 / 7,6963 / 8,1459 ms |
| VRAM máxima de toda la GPU | 2.831 MiB |

VRAM corresponde al total de la GPU, no únicamente al juego. El perfil es un ensayo local con límite de FPS y render interno escalado; no garantiza la misma tasa en otras rutas, densidades o condiciones. Los dos perfiles previos del mismo ejecutable se conservan como antecedentes, con su contenido y configuración identificados.

## Alcance verificado y límites

- Robot con dos piezas funcionales y kaiju con tres partes, dos ataques anunciados, consecuencias de desarme y cuerpo vencible sin desmontar todo. Políticas/runtime cubren deduplicación, una pieza física por PartId y restauración.
- Sesión y héroes comparten impacto canónico, receptores, agarre y persistencia. Encuentro preparado, contactos y grafo de ticks pasaron en producción.
- Cada héroe ofrece seis poderes; la rueda equipa sin activar, cancela y conserva el poder anterior al refrescar. El HUD presenta seis sectores; no se anuncia presentación para siete u ocho.
- RealInput valida eventos simulados mediante PlayerController. DualSense físico, sensación, revisión visual humana y pruebas sostenidas más amplias del plan son alcances diferentes; los resultados actuales no se convierten en aceptación humana.
- Los regresos S3Lab no equivalen a tres circunnavegaciones renderizadas. El perfil local tampoco certifica coste máximo tras visitar todas las regiones.

El arranque visible desde Steam quedó verificado. El log de las 06:23:23 UTC confirma `/Game/IB_Playable/Lvl_IB_Playable` con `IBPlayGameMode`, DX12 y audio Realtek; el proceso corresponde al ejecutable final. La publicación de fuentes y reconciliación de PR pueden avanzar con los gates aprobados. Después, la valoración del usuario orientará refinamientos y el siguiente sprint, sin presentar S8 como entregado.
