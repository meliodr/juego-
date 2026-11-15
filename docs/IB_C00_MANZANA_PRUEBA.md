# IB-C00 — Plano de la manzana de prueba de H01

**Fecha de diseño:** 2026-11-15 · **Estado:** PLANIFICADO (diseño documentado, no construido) · **Plataforma:** Windows / DX12 / Unreal Engine 5.8.4 objetivo · **Costo de recursos y plugins:** RD$0.

**Fuente de autoridad:** AGENTS.md → PLAN_ACTIVO.md → TAREAS_ACTIVAS.md → WINDOWS_INICIO_V12.md → PLUGIN_SELECTION_V14.md. La auditoría Fab es referencia, no confirma propiedad, descarga ni compatibilidad.

**Única tarea de implementación activa:** IB-W00 (verificar entorno). **IB-W01** va después; este documento es preproducción para IB-C00 y NO adelanta su implementación a IB-GND00/IB-F01 ni implementa IB-P01/IB-D01. No afirmar editor operativo o resultados de pruebas sin evidencia real.

## 1. Resultado jugable integrado que queremos lograr

H01 aparece en la calle → despega → acelera entre fachadas → frena antes de una esquina de 90° y gira → encuentra y agarra un carro → transporta el carro sin perder colisión con edificios → lo lanza contra un panel central de la fachada → queda un hueco **realmente transitable** → H01 atraviesa el hueco y dispone de espacio libre detrás → pulsa reiniciar → carro, pared, colisiones y estado del héroe regresan al estado inicial. La prueba completa se integra al cerrar IB-D01, no al construir C00.

No contiene robot, oponente, kaiju, H02, H03, ciudad grande, mundo espacial, enemigos, ciudadanos, red ni arte final.

## 2. Plano medible — coordenadas en metros

Convención: +X Este, +Y Norte, +Z arriba. Unreal usa centímetros; **1 m = 100 Unreal Units**. Coordenadas del centro sobre el suelo; Z de centros de mesh se convierte según altura. Los siguientes son valores iniciales editables, NO medidas ya probadas.

Vista de planta ilustrativa, NO a escala:

~~~text
                               NORTE (+Y)
                                  ↑
 Y=155                ┌───────────────────────────────┐
                      │ ZONA LIBRE TRAS PARED: ≥30 m   │
 Y=120                ████ PARED | PANEL 6 m | ████████
                      │             ↑                 │
 Y=62                 │           [CARRO]             │
                      │             ↑ vuelo/agarre     │
 Y=20                 │          CALLE NORTE          │
                      │             ↑                 │
 Y=0    [INICIO] ======= ACELERACIÓN ==== FRENO === ↰  ESQUINA
        X=10           X=20           X=130        X=180       → +X

 Horizontal: calle 20 m de ancho, fachada a ambos lados.
 Vertical: calle 20 m de ancho, fachada a ambos lados.
 Zona trasera de la pared: libre de obstáculos, verificable en colisión.
~~~

| Elemento | Coordenada / extensión inicial | Restricción |
|---|---|---|
| Suelo base | plano X=−20..220; Y=−30..165 | simple, colisión sólida; material gris |
| Spawn H01 | (10, 0, 0), orientación +X | sin solapes, contacto correcto con suelo |
| Despegue | X=10..20, Y=0 | espacio vertical libre ≥30 m |
| Recta de aceleración | centro Y=0; X=20..130 (110 m) | calle libre **20 m** de ancho, edificios a ambos lados |
| Frenada | centro Y=0; X=130..180 (50 m) | señal en el suelo, libre de obstáculos |
| Esquina | centro (180,0); giro 90° hacia +Y | área de maniobra de ~20 × 20 m; sin bloque invisible |
| Calle norte | centro X=180; Y=20..120 | ancho 20 m; fachadas desde Y≈20, separación visual |
| Carro de ensayo | centro aprox. (180,62,0.75) | proxy 4.5 m largo × 2.0 ancho × 1.5 alto; rotado hacia +Y |
| Distancia al objetivo | carro Y=62 → pared Y=120 | ~58 m de espacio para apuntar/lanzar |
| Fachada diana | plano Y=120, X=168..192; Z=0..16 m | edificio proxy de 24 m ancho × 16 m alto, grosor 0.6 m |
| Panel central sacrificial | X=177..183 (6 m), Y=119.7..120.3; Z=0..5 m | núcleo destructible separado; rodearlo con pilares/lintel fijos |
| Hueco verificable | apertura central después del impacto, libre ≥4.5 m ancho × 3.2 m alto desde el suelo | sin colisión fantasma; se prueba con cápsula del héroe |
| Espacio tras pared | X=172..188; Y=121..155; Z=0..12 | al menos 30 m de profundidad libre y piso continuo |

Ubicación de fachada para la recta: borde de carretera en Y=±10 m, acera ligera 2 m y cajas de fachada desde |Y|≈12 m; alturas variadas 12–24 m. En la calle norte, el mismo esquema a X=180±12 m. No cerrar la esquina con edificios. Marcadores de suelo: **DESPEGUE**, **ACELERA**, **FRENA**, **CARRO**, **PARED**, **SALIDA**; usar texto/editor o materiales simples, no packs.

## 3. Selección mínima de recursos: principal y respaldo

| Necesidad | Recurso principal (usar primero) | Respaldo condicionado | Trabajo propio inevitable |
|---|---|---|---|
| Manzana/fachadas | Cubos, planos, materiales y colisión simple de Unreal | Kenney/Quaternius, solo si licencia/archivos locales verificados; City Sample Buildings solo cuando escale | Componer el trazado con medidas y marcar zonas |
| Personaje y entrada | Third Person + Enhanced Input del motor | Blueprint Third Person si C++ está bloqueado | Ajuste del movimiento y parámetros del vuelo en IB-GND00/IB-F01 |
| Carro | Proxy con cajas/mallas primitivas UE (chasis y ruedas visuales) | Un único carro de City Sample Vehicles/Sport Car si se demuestra propiedad, licencia y compatibilidad | Estados Libre/Sujeto/Lanzado, masa/colisión coherentes y reinicio |
| Transporte | Physics Handle nativo como primer ensayo con comprobación de volumen barrido | Transporte cinemático propio mínimo con sweep y política de bloqueo si el Handle vibra/atraviesa | Ignorar colisión solo con el portador cuando haga falta; nunca desactivar la colisión con mundo |
| Pared | Panel preparado + Chaos/Fracture nativos, cuando toque IB-D01 | Estados de paneles/mallas con colisiones reales; PlotBuilder solo en sandbox del ticket | Detección de impacto, apertura real, limpieza/restauración |

**Fab:** no reclamar, descargar, activar ni instalar nada. La auditoría de Fab en main y su rama específica solo aporta candidatos; los estados de Mi biblioteca/licencia/bytes y funcionamiento con UE5.8.4 permanecen pendientes. Realtime Destructible Mesh NO es dependencia inicial.

## 4. Contratos de interacción y restauración (IB-P01 / IB-D01)

**Colisión del carro:** en suelo o lanzado mantiene colisión física con mundo y masa configurable. Mientras se sostiene, el volumen del carro sigue siendo un obstáculo: hacer sweep/test de la carga además del sweep de la cápsula del héroe, responder a golpes y bloquear/reubicar la carga antes de una pared. Nunca usar attachment sin sweep con colisión mundial desactivada. Evitar auto-colisión del carro con su portador mediante exclusión específica, no global. Si Physics Handle no satisface esquina/boost, descartar y pasar al respaldo con una fuente de movimiento.

**Agarre:** un único portador lógico; una sola transición Libre → Sujeto → Libre/Lanzado; soltar/reactivar física/gravedad y restaurar respuestas de colisión originales. El impacto válido se decide por colisión/impulso real, no por Niagara. Velocidad de lanzamiento, masa y umbral de rotura son Data/EditDefaults y se ajustan tras prueba.

**Pared:** panel central preparado para romperse y dejar apertura definida. Una pieza clave puede adquirir física, máximo 3–6 fragmentos físicos significativos en esta escena inicial; resto solo visual. No dejar detrás del panel un volumen fijo o colisión de Geometry Collection que bloquee el paso. Un umbral bajo no debe romper por tocarlo caminando. El muro será real en estado Intacto y realmente transitable en estado Roto; no se promete corte arbitrario en otras fachadas.

**Reset determinista (acción IA_Reset, tecla R inicialmente):** si H01 sostiene el carro, interrumpir el agarre; limpiar impulsos, velocidades lineales/angulares, referencias de portador, handles y timers pendientes; eliminar escombros/actores temporales y proyectiles; reinstanciar o restaurar el panel y su colisión; restaurar carro en (180,62,0.75) con física/colisión/gravedad iniciales; teletransportar H01 a spawn seguro sin solapes y devolver cámara/velocidad de movimiento a estado inicial. Todo reset es idempotente; pulsar R de nuevo no duplica objetos. Evitar referencias colgantes. Guardar estado inicial al empezar la partida.

## 5. Matriz de aceptación — no ejecutada

Usar una hoja de evidencias por ticket (fecha, commit, Windows/UE/VS, versión DX12, ruta o mapa, actor y prueba, resultado REAL PASA/FALLA/BLOQUEADO, captura/log/medición, autor de prueba). Ninguna fila es un resultado observado hoy.

| ID | Gate | Acción reproducible | Aceptación inicial |
|---|---|---|---|
| W00-01 | IB-W00 | verificar Launcher, abrir UE5.8.4, inspeccionar GPU/DX12, CPU, SSD, VS/MSVC/SDK | editor realmente abre; DX12 se confirma en runtime y se registra toolchain o bloqueo; no deducir por un archivo instalado |
| W01-01 | IB-W01 | crear Third Person minimal, Play, compilar C++ si toolchain listo y empaquetar Win64 | proyecto/equipo reproducible, movimiento de plantilla y exe Win64 ejecutan; si falta MSVC: BP smoke test y C++ BLOQUEADO |
| C00-01 | IB-C00 | inspeccionar suelo/calles/medidas con regla de editor | recta 110 m, freno 50 m, esquina limpia, carro/pare diana separados 58 m, espacio tras pared ≥30 m |
| C00-02 | IB-C00 | caminar y volar por la L, techo y esquinas | ninguna barrera oculta, suelo continuo, orientación clara, sin atraviesos por fachadas |
| F01-01 | IB-F01 | 30 ciclos suelo→aire→freno→suelo y 20 cambios de régimen | mismo H01 con entrada recuperable, sin cámara atascada ni paredes atravesadas; registrar fallos |
| F01-02 | IB-F01 | acelerar en recta, frenar antes del giro, repetir con velocidad de prueba documentada (propuesta 45 m/s) | frenada dentro de los 50 m previstos **si los parámetros elegidos lo permiten**; sin túnel ni pérdida de control; ajustar velocidad/deceleración si falla |
| P01-01 | IB-P01 | 30 ciclos agarrar/transportar/soltar o lanzar en suelo y aire | un carro, sin duplicación, gravedad/masa/colisión restauradas |
| P01-02 | IB-P01 | volar cargando el carro contra lateral/esquina y probar barridos | el carro no atraviesa fachada aunque cápsula de H01 no la toque; bloqueo perceptible y sin vibración incontrolable |
| D01-01 | IB-D01 | 50 impactos de carro: centro, borde, oblicuos y con distinta fuerza, con reset cuando corresponda | solo impactos válidos abren panel, apertura reproduce umbral; sin daño duplicado ni colisión fantasma |
| D01-02 | IB-D01 | chocar intencionalmente con pared intacta y después atravesar apertura | pared bloquea antes; H01 atraviesa hueco real libre ≥4.5 × 3.2 m después; tiene 30 m despejados detrás |
| RST-01 | IB-D01 | 20 reinicios: pared intacta, pared rota, carro lanzado, carro sostenido | un carro y una pared intacta en posición inicial por reset; cero fragmentos antiguos, sin fuga creciente de actores/memoria |
| PERF-01 | IB-S01/QA | ejecutar recorrido repetido y 10 min de vuelo en build Win64 1080p con perfil documentado | registrar FPS/frame time p95/p99, GPU/CPU, VRAM y hitches; **60 FPS aspiracionales, no presumidos** |
| HUM-01 | Usuario | probar ciclo completo: despegar → acelerar → frenar → agarrar → lanzar → cruzar → reiniciar | usuario puntúa control, sensación de potencia y ganas de repetir (escala 1–5); meta inicial ≥4/5, corregir si no convence |

**Pruebas negativas imprescindibles:** R con carro sostenido, R inmediatamente al impacto, intentar pasar por el muro antes de romperlo, cargar carro contra una esquina a velocidad alta, impacto fuera del panel, carro en el hueco y choque contra los laterales del agujero, Play → Stop → Play sin persistencia accidental de rotura.

**Instrumentación temporal:** overlay de desarrollo con velocidad (m/s), estado de agarre, estado del muro, contador de carro/fragmentos, número de resets; trazas/visualización de colisión; logs con nombres de ticket y de prueba. Desactivable para build de evaluación. No medir FPS como prueba de corrección de colisiones.

## 6. Orden de ejecución y autorización

1. **IB-W00 único ticket activo.** Verificar de modo no destructivo Windows real, UE versión exacta, primer arranque Editor, DX12/GPU y VS/MSVC/SDK. Ya existe un PR en borrador #2 con auditoría de solo lectura; revisarlo antes de duplicar trabajo. No ejecutar script sin acceso local/autorización.
2. **IB-W01 al aprobar W00.** Third Person nativo; C++ si toolchain listo; Blueprint de prueba si falta compilador, registrando bloqueo C++. No instalar/alterar Visual Studio o drivers por cuenta propia.
3. **IB-FLT-AUD → IB-GND00 → IB-F01.** Un movimiento, un personaje, colisión barrida y prueba humana del giro y freno.
4. **IB-C00.** Montar la escena L con primitivas según este documento. Registrar mapa, medidas y capturas. **No exigir carro interactivo ni panel fracturado para aceptar geometría C00.**
5. **IB-P01 → IB-D01.** Agarre con sweep del carro, lanzamiento y apertura colisionable real. Cerrar el recorrido completo y realizar regresiones 30/50/20.
6. **IB-E01 → IB-S01.** Enemigos y entrega urbana mínima solo después de estabilizar el bucle. Ciudad grande, kaiju y espacio quedan fuera.

**Intervención humana:** cuando Epic Launcher indique instalación terminada, pedir verificación del primer arranque de UE y captura de versión/RHI (o evidencias del script de W00, sin rutas personales). Cuando F01/D01 estén implementados, solicitar una prueba jugable concreta con controles y qué observar, sin trasladar construcción técnica al usuario.

**Cambios de impacto reservados al usuario:** descargas/reclamaciones Fab, activar/instalar plugins externos, instalaciones o cambios del sistema, borrar archivos personales, cambios de licencia/visibilidad y merge en main. Trabajo documental reversible y PR de borrador no equivalen a aprobación de esos cambios.

## 7. Registro breve de decisiones, evidencia y bloqueos

| 2026-11-15 | Tipo | Registro |
|---|---|---|
| Decisión D-001 | Geometría gris nativa antes de Fab; circuito en L con 110 m de aceleración, 50 m de freno, carro y pared con espacio detrás |
| Decisión D-002 | Carro con colisión mundial activa incluso sujeto; Physics Handle + sweep como ensayo, respaldo transporte controlado |
| Decisión D-003 | Panel sacrificial preparado y Chaos nativo como baseline; no destruir toda la ciudad ni asumir corte arbitrario |
| Decisión D-004 | Estado inicial restaurado en R; tests de fuga, colisión y duplicación obligatorios |
| Evidencia | Revisión documental de GitHub; ninguna escena, plugin, editor, compilación ni ensayo Windows ejecutado aquí |
| Bloqueo | Primer arranque local Unreal/Windows, RHI DX12 y MSVC/SDK todavía sin comprobar |
| Próxima decisión | IB-W00 con prueba real; si pasa, IB-W01; de lo contrario documentar el bloqueo exacto |

**Rollback:** trabajo en rama separada; no modificar packs ni binarios de terceros; si una implementación futura falla, revertir solo el cambio del ticket con un commit normal, conservar el último mapa/build validado. No forzar ramas ni hacer merge sin autorización.
