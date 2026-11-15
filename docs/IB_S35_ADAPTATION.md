# S3.5 — adaptación aislada y reproducción

El laboratorio añade tres componentes pequeños: Grip (propiedad/transporte), Punch (ventana de contacto) y Receiver (contrato/deduplicación). AIBS35Object aporta un root UBoxComponent físico y una apariencia sin colisión. No crea otro motor de física: usa MoveComponent con sweep, AddImpulse y CCD de Chaos. El único volumen físico continúa con QueryAndPhysics y bloquea WorldStatic/WorldDynamic durante el agarre.

## Reutilización y límites

Se revisaron `ImpactCircuitComponent.*`, `ImpactGripMesh.*`, `Tests/ImpactFlightTest.cpp`, la variante Combat, `docs/IB_MVP_URBANO.md`, AGENTS y las skills gameplay/QA locales. La selección por distancia y visibilidad, barrido del chasis y separación apariencia/collider provienen del diseño existente. La variante Combat ya ofrece ataques por sweep y notifies, pero exige otro Character y su cadena de animación: se adapta solamente ese patrón a un componente compatible con H01. No se modifica su código.

El circuito antiguo solo publica objetivos IK desde su propio tipo; `ImpactGripMesh::SolveArm` es privado. Duplicar el solver haría dos implementaciones. **No se duplicó ni se fingió integración IK:** Grip publica Left/RightContact o sockets, peso y carga gestual. El derivado conserva Manny/AnimBP al duplicar/reparentar el Blueprint, pero las manos todavía requieren el adaptador siguiente. Las esferas verdes muestran objetivos, no certifican contacto de manos.

Propuesta de patch para el integrador, **no aplicada** a archivos compartidos:

1. Introducir un proveedor de objetivos de agarre que permita a ImpactGripMesh consultar Grip o el circuito legado. Reusar SolveArm sin copiarlo. Escoger una sola interacción activa por personaje.
2. Usar GetGripWeight/GetGestureLoad y estado Held/Charging para las poses Carry/Throw ya instaladas; actualmente el circuito original no conoce la carga S3.5.
3. Reconciliar OnAttackCue Windup/Contact/End con la animación elegida. Se ofrece UIBS35ContactNotify; bUseNotifyContact solo se activa tras comprobar el clip y su ventana. La ruta predeterminada es curva temporal reproducible, no un clip de puñetazo validado.
4. Retirar bindings antiguos del personaje integrado solo cuando el nuevo circuito esté aceptado. El derivado de laboratorio reemplaza sus propias teclas de agarre/lanzamiento/reset y dos crucetas sin editar Character principal.
5. Conservar las regresiones `ImpactBeyond.Circuit.Headless`, `ImpactBeyond.Circuit.TransportCycles`, Flight y Landing tras integrar.

El transporte conserva orientación mundial: evita el problema conocido de que MoveComponent no barre rotación. No sigue giros de muñeca; ese comportamiento deliberadamente limitado debe evaluarse visualmente. Si el portador supera alcance/velocidad de carga o aparece bloqueo, suelta sin impulso. No garantiza transportar a velocidad extrema del vuelo S2; el ensayo a 320/368 m/s con carga sigue pendiente. El reset falla sin mover el objeto cuando su spawn está ocupado.

La elevación separa primero la carga de la cápsula y luego la acerca. Antes de reactivar física, una posible penetración con el portador se separa mediante sweep vertical. Si un techo impide separarla, el objeto permanece sujeto/colisionable y la carga de lanzamiento queda cancelada; no se fuerza un impulso ni se teletransporta. Probar explícitamente ese caso y el fin de vida del portador en Unreal.

## Perfiles y procedencia

Car Sport 1.200 kg, Sedan 1.450 kg, Electric 1.650 kg; Container 2.200 kg; Pole 85 kg; Rock 350 kg; LoosePart 45 kg. Son valores iniciales de laboratorio, no mediciones de modelos reales. Dos escalas adicionales: Rock 0,5× (masa 1/8) y LoosePart 1,5× (masa 3,375×). Todos usan caja real ajustable; no colisión convexa compleja simulada. No usar escala del Actor para falsear dimensiones: configurar HalfExtent/contactos/masa.

El generador referencia exclusivamente recursos ya integrados por S2: ConceptCar y taxi/coche eléctrico de Kit Cidade. Inspección de metadatos del checkout previo: ConceptCar 6.301.783 bytes, taxi 107.732, eléctrico 108.634, BP_ThirdPersonCharacter 54.802. Esto acredita archivos locales completos frente a punteros pequeños; no acredita carga, licencia de cada dependencia ni calidad visual del nuevo laboratorio. La procedencia y límites existentes están en `docs/IB_MVP_URBANO.md` y scripts de donantes. No se copió ningún asset binario al commit.

City Sample Vehicles y Game Animation Sample no se usan ni se descargaron; no se confirmó instalación en este encargo. Quaternius permanece alternativa futura sin descarga ni afirmación nueva de revisión legal. Materiales/arte final pertenecen a S2.5. Las formas básicas de los props son proxies explícitos de laboratorio.

## Reproducir tras conseguir turno exclusivo

1. Checkout completo de esta rama basado en el snapshot fijo, conservando dependencias instaladas. El checkout de este encargo es sparse de código para ahorrar espacio; no puede abrir el mapa con assets ausentes.
2. Coordinar con las otras tres sesiones **un solo turno de build/editor/cook**. Ausencia de procesos en una consulta no equivale a reservar el equipo. No se obtuvo ese turno en esta entrega.
3. Compilar ImpactBeyondEditor Win64 Development con el motor instalado UE5.8.4 y UBT. No cambiar Build.cs, uproject, Config ni plugins. Si falta el módulo Python de editor, detener el generador y resolver mediante el integrador; no activarlo automáticamente.
4. Ejecutar `Tools/IBS35_AuthorLab.py` desde ese editor. Prevalida los tres coches y BP base; duplica/reparenta solo en `/Game/IB_S35`. Rechaza sobrescribir un mapa existente. Sus únicos assets nuevos son perfiles, BP_H01, BP_GameMode y `/Game/IB_S35/Lvl_IB_S35_Interactions`.
5. Abrir el laboratorio; verificar spawn/poses/materiales/colisiones, y ejecutar `Automation RunTests ImpactBeyond.S35` en modo juego con Chaos. Un proceso NullRHI prueba gameplay, no DX12 ni manos.
6. Ejecutar la matriz visual/velocidad de `IB_S35_EVIDENCE.md`, capturar resultados y revisar control con DualSense físico. Guardar solo assets propios/referencias permitidas; no publicar packs Fab.

Controles del laboratorio: G/Círculo agarrar o soltar; T/Triángulo cargar y soltar para lanzar; Backspace/Crear cancelar carga manteniendo agarre; clic izquierdo/Cruceta arriba golpe rápido; clic derecho/Cruceta derecha pesado; R/Cruceta abajo reset. F/R3 y controles básicos de H01 siguen heredados. Las crucetas de impacto/travesía se reasignan solo en este derivado; V/B conservan esas funciones por teclado. Hardware y foco/cancelación requieren prueba humana.

Rollback: descartar esta rama/laboratorio. No hay cambios en producción que revertir, ni migración de datos, compra, instalación o cambio de seguridad.
