# S4 — Laboratorio aislado de respuesta

Fecha: 2026-11-15. Base fija: `1d01db54b63250ed0aacb0704a5ac16290324e32`, PR #19. Rama: `prototype/ib-s4-response-lab`. Destino del PR draft: `prototype/ib-sprint02`. **Estado: CÓDIGO + política C++ nativa compilada/ejecutada; integración Unreal IMPLEMENTADA SIN VALIDAR.** No es Sprint 7.5 ni una integración de los otros laboratorios.

Revisión posterior focalizada: [IB_S4_STATIC_REVIEW.md](IB_S4_STATIC_REVIEW.md). Resultado nativo actualizado: **2.010 checks**, en `IB_S4_REVIEW_NATIVE_RESULTS.json`. Se conserva el reporte inicial de 1.902 como evidencia histórica. El evento de respuesta ahora distingue solicitud estructural y pérdida efectiva de HP; el ledger conserva un límite retirado de seriales; reset/cannon/cargo recibieron las correcciones documentadas. No implica compatibilidad S3.5 ni validación Unreal.

## Qué hay realmente

El cambio añade clases exclusivamente en `ImpactBeyond/Source/ImpactBeyond/IB_S4`, scripts `Tools/IBS4_*` y esta documentación. No modifica Character, Circuit, Landing, GameMode compartido, Build.cs, .uproject, Config, mapas anteriores ni carpetas S2.5/S3/S3.5. No introduce módulos, plugins, servicios o paquetes descargados. RD$0 de compras/servicios adicionales; no se consultó ninguna API de modelos de pago.

| Pieza | Comportamiento implementado en código | Límite actual |
|---|---|---|
| ResponseComponent + Policy | H01: bala/misil humanos sin pérdida de salud; misil limitado a 1.200 cm/s y cooldown mínimo 0,5 s (valor de escena 1,1 s); ID por ataque/ledger por blanco; rechazo de repetidos, caducados, futuros y no finitos | No es el contrato global de daño; no conectado al H01 compartido |
| LabPawn + FlightIntent | CMC nativo, vuelo rasante con cápsula, intención explícita para pasar a caída/aterrizar; Enhanced Input teclado/mando; microshake ajustable 0–0,4°, solo al aceptar empuje | Pawn de ensayo independiente; AnimBP existente provisional, no poses finales |
| Threat | Dos ACharacter de policía: patrulla acotada → advertencia 2 s → apuntado 1 s → disparo; retiro a corta distancia o después de ráfaga; alerta expira; requiere causa atribuida y verificada | Patrulla simple en zona abierta, sin NavMesh/cobertura estratégica; falta actuación policial/arma/uniforme final |
| Missile | ProjectileMovement, esfera colisionable y visible provisional; explosión limitada a 350 cm/16 actores, oclusión, un contacto por actor; no daño H01 ni teleport | Desplazamiento 3–8 m por medir; falta malla de tanque autorizada cargada y Niagara final |
| Powers | Láser cámara→ojos→blanco (evita disparar alrededor de esquinas); máximo 10 muestras/s, daño metal/hormigón/cristal; telequinesis PhysicsHandle solo sobre IBS4Grabbable; dueño único y barrido de esfera envolvente completa; lanzamiento con comprobación de obstáculo | Modo creativo, masa ≤250 kg, radio de carga ≤160 cm; debe reemplazarse por adaptador S3.5 al acordar contrato |
| PreparedTarget | Vidrio/puerta, muro de 220×260 cm libres después de romper tapón; cráter preparado 5×5 m, profundidad central 60 cm, escalones 30 cm y piso continuo; cañón robótico separable desactiva disparo y se vuelve agarrable | Geometría de prueba con formas del motor; ninguna destrucción arbitraria de mallas |
| Reset/Save | Reset por nivel; restauración local rechaza ocupantes/cargas agarradas; limpia fragmentos propios; snapshot versionado por ObjectId restaura estados estructurales y capacidades | No persiste pose exacta de objetos sueltos, salud parcial, alertas ni estado de otros sprints; carga puede ser parcial si hay ocupantes y devuelve false |
| Presentación | Beam/impacto geométricos provisionales, sonido procedural breve por material, HUD y estado de advertencia; hooks Blueprint para Niagara | No se ha escuchado/renderizado. No hay humo/polvo Niagara, ni se afirma fotorealismo |

`IBS4_AuthorLab.py` construye **solo** `/Game/IB_S4/Lvl_IB_S4_Response` después de compilar. Requiere cargar el Manny y AnimBP existentes, rechaza mapa destino existente o mapa actual sucio, y guarda únicamente su mapa. **El .umap todavía NO se ha generado**: no hubo turno coordinado de Editor. El código crea los fixtures de esa escena durante BeginPlay. El lanzador de tanque utiliza una caja explícita si no se configura una malla autorizada; esto no satisface el gate artístico del tanque.

## Reutilización y decisiones

Se inspeccionaron `ImpactCircuitComponent`, `ImpactLandingComponent`, `ImpactGripMesh`, `ImpactFlightFeedback` y la interfaz nativa de plantilla `ICombatDamageable` antes de crear las piezas S4. El circuito tiene estado/grabado privado, cambio a cinemático y tags de su mapa; extenderlo para agarrar a distancia requeriría modificar archivos prohibidos. La interfaz de combate mezcla daño/impulso/muerte y carece de identidad/tipo de ataque. Por ello el laboratorio usa un receptor opt-in y PhysicsHandle del motor, sin registrarlos como API global ni asociarlos automáticamente al personaje existente. El integrador decidirá después si el receptor se adapta a la interfaz existente o al contrato S3.5.

No se importa el framework de REPIRK, un sistema de conducción, GAS, PlotBuilder ni Realtime Destructible Mesh. Los estados de malla con colisión y los pocos cuerpos Chaos satisfacen el ensayo sin modificar Build.cs. La ausencia de dependencia Niagara en Build.cs obliga a mantener los hooks visuales para un Blueprint posterior; no se ocultó ese pendiente con una importación implícita de plugin.

Se leyeron AGENTS y las skills locales `impact-gameplay-gate`, `impact-qa-review`, `impact-license-audit`, `impact-civic-systems`, junto con PLAN_ACTIVO/TAREAS_ACTIVAS, FAB_SELECCION_ACTIVA/CSV, COMBAT_ROSTER y el plan paralelo/documentación S3–5 de la rama documental. Las referencias históricas a Linux, networking o planificación solamente quedan sustituidas por el encargo Windows offline autorizado. No se encontraron archivos de memoria versionados en el checkout. Ningún subagente fue creado.

## Ejecución reproducible

1. Desde este checkout: `python Tools/IBS4_Check.py --report docs/IB_S4_NATIVE_RESULTS.json`. Compila únicamente el ejecutable pequeño de política con MSVC `/W4 /WX`, verifica rutas permitidas y sintaxis Python. Es independiente del motor; sus resultados no certifican física.
2. Acordar con las otras sesiones un turno exclusivo de Editor/build. La ausencia momentánea de procesos no constituye una reserva. El script exige `-ExclusiveEditorSlotConfirmed` y también rechaza procesos Unreal activos; esa bandera **no es un lock compartido**.
3. En una copia completa propia de la base y esta rama, con assets legales disponibles: `pwsh -File Tools/IBS4_RunLab.ps1 -Phase Build -ExclusiveEditorSlotConfirmed`. Esto compila `ImpactBeyondEditor` con máximo dos acciones paralelas. No ejecutar en un checkout ajeno.
4. Con Python Editor ya disponible (el script no habilita plugins): `pwsh -File Tools/IBS4_RunLab.ps1 -Phase Author -ExclusiveEditorSlotConfirmed`. Si se dispone de tanque ya importado y autorizado, configurar `IBS4_TANK_ASSET` con su ruta de objeto antes de ejecutar; de lo contrario queda caja de ensayo.
5. `pwsh -File Tools/IBS4_RunLab.ps1 -Phase Automation -ExclusiveEditorSlotConfirmed`. Suites preparadas: `ImpactBeyond.S4.H01Response`, `PreparedCollision` (20 ciclos), `MissileFlightRecovery` (20 proyectiles reales, alturas alternas 0,5/3 m, medidas y recuperación). **No ejecutadas en esta entrega.** El proceso NullRHI no valida sonido/imagen.
6. `-Phase DX12` usa DX12 1920×1080 y audio para captura técnica. Un test visible dirigido al usuario debe seguir el gate existente: paquete vigente desde Steam con Steam Input; este script no altera accesos de Steam ni constituye prueba DualSense física.
7. Capturar un recorrido inicial y otro repetido por separado con Unreal Insights/CSV Profiler. Normalizar CSV a `frame_ms,gpu_ms,vram_mb`, luego `python Tools/IBS4_SummarizeCapture.py captura.csv --run-label repetido --resolution 1920x1080 --rhi DX12 --audio on`. Los opcionales ausentes quedan null. Registrar calidad/driver/hardware y archivos fuente, no rellenar cifras ausentes.

El checkout de esta sesión es sparse (código/herramientas/docs); no duplica assets ni caches. Un build/editor posterior necesita materializar contenido legalmente disponible. Se verificó `Engine/Build/Build.version` instalado: UE **5.8.4**, CL 58902052, y MSVC **14.52.37025**. Espacio libre observado al inicio: ~149,5 GB decimales. El catálogo local de Codex muestra Sol/Astra/Terra; no hay evidencia suficiente del modelo/esfuerzo activo de esta sesión para afirmar “Extra High seleccionado”.

## Controles del laboratorio

| Acción | Teclado/ratón | Gamepad previsto |
|---|---|---|
| Mover / mirar | WASD / ratón | Sticks izquierdo/derecho |
| Ascender / descender | Espacio / Ctrl | R1 / L1 |
| Láser mantenido; lanzar si sostiene | Botón izquierdo | R2 |
| Agarrar / soltar sin lanzar | E | Cuadrado |
| Puño / lanzar carga | Q | Triángulo |
| Intención vuelo / aterrizar | F | R3 |
| Alerta explícita de ensayo | G | Options |
| Reset por recarga del nivel | R | Special Left (validar traducción Touchpad/Steam Input) |

El HUD identifica proxy art y advertencias; no hay autoataque por ver o rozar al héroe. El botón de alerta registra una causa de prueba explícita. Un impacto atribuible al jugador contra un blanco preparado puede activar la alerta; un impacto de enemigo no se le atribuye al jugador. No se implementa karma ni castigo por volar.

## Aceptación y bloqueos

| Gate | Ejecutado aquí | Pendiente |
|---|---|---|
| Política 20 balas H01 | PASS C++ nativo, sin HP/impulso, duplicados rechazados | 20 impactos trazados reales, FX/audio, ausencia de interrupción |
| 20 misiles + anti-stunlock | PASS política de 20 espaciados + 20 rápidos, HP=0 delta, cooldown | 20 proyectiles/medidas/colisión/recuperación, prueba junto a paredes y suelo |
| Láser metal/hormigón/cristal | PASS 20 muestras por cada material + dedup y clamp de hitch | 20 usos reales por material, aim/oclusión/beam |
| Telequinesis | Código revisado; ninguna ejecución física | 20 ciclos incluyendo pared, esquinas, rotación, caída, destrucción del owner, throw y reset |
| Muro/cráter | PASS diseño numérico de 20 layouts; suites físicas preparadas | 20 fracturas/20 cráteres con cápsula, colisión/visibilidad, save/load/reset sin fragmentos |
| Alertas | PASS 10 secuencias de causa/advertencia/oclusión/retiro/expiración | Patrulla y ataques reales, reacción animada |
| Robot | Capacidad vinculada a cañón; separación idempotente en código | Fixture físico, daño selectivo, cancelación de disparo, no duplicados al agarrar/reset |
| Vuelo bajo | Componente/contrato y suite de misil preparados | 20 recorridos 0,5–3 m a 30/60 fps, pendientes/esquinas, láser/puño al suelo |
| Compilación Unreal, mapa y DX12 | NOT RUN | UHT/C++, authoring, audio, Niagara, fps p95/p99, VRAM/hitches, Steam/DualSense |

Resultado nativo exacto en `IB_S4_NATIVE_RESULTS.json`. La misma persona/agente implementó y revisó; no es revisión independiente. No se han medido FPS, VRAM ni calidad visual. No hay build nueva entregable ni aceptación humana. **Bloqueadores vigentes:** la coordinación reservó el siguiente turno exclusivo de Unreal/build/cook a S2.5 y ordenó que S4 no lo inicie hasta su liberación; tanque/robot final y Niagara/animación policial sin assets cargados y validados. El siguiente integrador debe resolverlos antes de marcar S4 jugable/aceptado.

## Integración y rollback

El PR es draft y no se fusiona. S3.5 no es dependencia. Mantener un solo dueño por cuerpo al integrar; retirar el adaptador creativo S4 antes de activar el agarre común sobre sus tags. Traducir AttackId/TargetId/instigator/perfil y la intención de aterrizaje en una revisión dedicada; no copiar el Pawn entero sobre H01. Para descartar el ensayo basta no integrar la rama o retirar sus carpetas en una rama nueva: ningún archivo compartido fue cambiado.
