# IMPACT: BEYOND — Cola activa de trabajo

## Catálogo Fab y revisiones — 2026-11-15

Leer [FAB_SELECCION_ACTIVA.md](FAB_SELECCION_ACTIVA.md) y [FAB_CATALOGO_MAESTRO.csv](FAB_CATALOGO_MAESTRO.csv) antes de elegir assets. Actualmente hay 52 entradas gratuitas públicas (8 SELECCIONAR, 5 PROBAR, 27 RESERVAR y 12 DESCARTAR), **ninguna marcada como reclamado/instalado** porque no se accedió a la cuenta privada. Se creó tarea recurrente para intentar actualizar cada 3 días desde 18/11/2026, sin compras ni entrenamiento de IA.

**Tickets opcionales futuros, NO ANTES DE W00/W01:** FAB-OWNED-01 (leer Mi biblioteca con acceso real, paginar y reconciliar UUIDs con CSV; sin reclamar ni descargar), FAB-IMPORT-01 (importar solo 1 pack seleccionado en sandbox, licencia específica, notas y test UE5.8/Windows). Mantener la primera experiencia moviéndose sin necesitar ningún pack nuevo.


## Decisión de plugins v1.4 — no bloquea arranque

Referencia: [PLUGIN_SELECTION_V14.md](PLUGIN_SELECTION_V14.md). Primero Windows/UE5.8 (IB-W00/W01); el núcleo de proyecto usará Enhanced Input y componentes del motor. **No activar plugins externos antes de comprobar C++/editor.** Experimentos condicionales:
- IB-PLUGIN-MCP (opcional tras W01): activar Unreal MCP/AllToolsets solo en editor local, crear cubo y guardar, verificar apagado en paquete. Si no hay agente local, NO bloquea desarrollo.
- IB-PLUGIN-RDM (solo tras D01 baseline Chaos): comparar Realtime Destructible Mesh Free/Fab (Windows UE5.7+) frente a Chaos, 50 impactos y 20 resets, latencia de colisión y consumo; requiere términos Fab aceptados por usuario.
- IB-PLUGIN-PCGEX (solo tras PCG ciudad): ramo UE5.8 exacto de PCGEx MIT, grafo de calles/parcelas, performance y cook en sandbox; integrar si simplifica realmente.
- IB-PLUGIN-RMC (reserva): Realtime Mesh Component Core MIT si el proyecto requiere malla procedural propia y alternativas no funcionan. No instalar antes.
Ningún experimento se marca ACEPTADO por documentación; versiones, licencias, build, mapa, rendimiento y rollback son obligatorios.


## Actualización de arranque Windows — 2026-11-15 (manda sobre la tabla histórica)

Unreal Engine 5.8 está **en instalación** según screenshot; Bridge y Fab en cola. Primera tarea **IB-W00**: verificar que editor abre, CPU/GPU/DirectX 12/espacio y herramientas MSVC/SDK; después **IB-W01**: crear proyecto Third Person y generar primer ejecutable Windows. Detalles y requisitos en [WINDOWS_INICIO_V12.md](WINDOWS_INICIO_V12.md). En la tabla histórica **IB-L00 está sustituido por IB-W00/IB-W01**. El texto previo relativo a Linux/Vulkan, paquete Linux y CPU del equipo Linux se interpreta como futura portabilidad, NO primer gate. Los demás tickets mantienen orden/descripción y se validan sobre Windows. Sigue un jugador offline y nada de pruebas N0/N1. La captura no demuestra instalación completada.  


Actualización: 2026-11-14. Fuente de alcance: [PLAN_ACTIVO.md](PLAN_ACTIVO.md).

**Estado real: ninguna tarea de juego ha comenzado ni está aceptada.** Esta es una cola documentada, no issues asignadas a agentes ni instrucciones ya enviadas a Robert. El primer bloqueo operativo es disponer del Windows real autorizado para inspeccionar y probar.

## Próxima tarea

**IB-W00 → IB-W01 — Confirmar Windows/Unreal/compilador y crear la base reproducible.** No comenzar otro sistema antes de obtener su evidencia. Este turno autoriza planificación; no sustituye permiso para instalar motor, cambiar drivers, ejecutar terceros o acceder a la máquina.

## Orden y dependencias

| Orden | Ticket | Entrega / dependencia | Responsable propuesto | Estado |
|---|---|---|---|---|
| 1 | IB-W00/W01 | Verificar Windows/UE 5.8.4 y ejecutar la plantilla Third Person | Ejecutor local + QA | W00 y W01 aceptados. Siguiente: IB-FLT-AUD. [Evidencia local](IB_W00_W01_VALIDACION.md) |
| 2 | IB-FLT-AUD | Selección de código y clips sobre la base Windows validada | Auditor de recursos + integrador | ACEPTADO. [Evidencia y decisión](FLIGHT_REUSE_AUDIT_V09.md). Siguiente: IB-GND00 |
| 3 | IB-GND00 | Caminar, correr, esprintar y saltar, antes de vuelo | Integrador de gameplay | EJECUTADO; validación manual pendiente |
| 4 | IB-F01 | Vuelo, cámara y poses; GND00 + FLT-AUD | Gameplay/animación | EJECUTADO; pruebas completas pendientes |
| 5 | IB-C00 | Una manzana y fachada de prueba; F01 | Mundo/arte técnico | PLANIFICADO |
| 6 | IB-P01 | Agarre y lanzamiento; F01 + escenario simple | Gameplay | EJECUTADO en el carro; [prueba y pendientes](IB_CIRCUITO_JUGABLE.md) |
| 7 | IB-D01 | Hueco y piezas; P01 + C00 | Gameplay/destrucción | EJECUTADO en un panel; [prueba y pendientes](IB_CIRCUITO_JUGABLE.md) |
| 8 | IB-E01 | Robot modular y rival aéreo, de uno en uno; D01 | Gameplay/IA | PLANIFICADO |
| 9 | IB-S01 | Sandbox integrado + prueba humana; anteriores | Integrador + QA + usuario | PLANIFICADO |

No hay tickets de red activos. N0/N1/N02 y otros requisitos multijugador de documentos históricos están aplazados. Los nuevos IDs y los aquí conservados sustituyen el orden de las tablas antiguas.

## IB-L00 — Ficha histórica de Linux, **sustituida por IB-W00/IB-W01** (ver WINDOWS_INICIO_V12.md)

**Propósito:** saber sobre qué entorno se construirá y producir una base repetible, no descargar todos los recursos.

**Quién:** Robert solo si se le asigna y confirma acceso; de lo contrario, un ejecutor local autorizado con Codex. QA no presumirá independencia si lo realiza el mismo agente.

**Entradas conocidas:** Linux; 64 GB RAM; RTX 4070 8 GB VRAM. **Pendiente de inspeccionar:** CPU, modelo preciso/driver de GPU, distribución, Vulkan, almacenamiento, motor/toolchain y estado real del checkout. No volver a preguntar por RAM/GPU.

**Procedimiento:** primero lectura de información y estado de Git; después comprobar si Unreal está instalado. No cambiar de distribución, reinstalar drivers o actualizar el motor por iniciativa propia. Si falta instalación, entregar el bloqueo y los pasos necesarios, no fingir una prueba. Con entorno y permisos apropiados, usar plantilla mínima C++ Third Person; variante Combat solo si existe en la versión elegida. Fijar motor/toolchain después del smoke test.

**Ámbito propuesto:** `Game/ImpactBeyond/` para el proyecto; `docs/evidence/IB-L00.md` para reporte; scripts propios mínimos en `Tools/` si resultan necesarios. Estas carpetas no se consideran existentes hasta crearlas.

**Aceptación:** compilar, cocinar/empaquetar y abrir el ejecutable Linux fuera del editor; verificar entradas, cámara, audio, colisión y salida. Registrar comandos, versiones y logs; separar primera ejecución del recorrido repetido. Sin experiencia gráfica disponible no aprobar render/rendimiento.

**No tocar:** recursos de ciudades/planetas, MCP público, drivers, otros repositorios ni permisos globales. **Alternativa:** plantilla más simple del mismo motor estable si falla la variante Combat. **Salida bloqueada:** indicar exactamente si falta acceso, instalación, compilador o GPU.

## IB-FLT-AUD — Elegir lo que realmente se puede reutilizar

**Propósito:** resolver la cadena mínima de dependencias de ManOfSteel y la disponibilidad real de clips.

**Leer:** auditoría de vuelo v0.9 y skill de licencias. Fijar commit del donante, revisar LICENSE, `Flight.cpp/.h` y dependencias; comprobar LFS mediante los mecanismos permitidos cuando se disponga de entorno. Un puntero no es un clip importado. Examinar mirrors solo tras procedencia/permisos.

**Entrega:** inventario por archivo: origen, revisión/edición, licencia de código/arte, completo o puntero, dependencia, uso deseado y alternativa. Seleccionar una base/rig y clips imprescindibles, no cientos de archivos.

**Aceptación:** saber qué partes se copiarían con permiso y qué faltantes impiden integrar. Nada de marcar Linux compatible por leer código.

**Descartar como base:** el proyecto completo si exige restaurar muchos Blueprints y sistemas ajenos al vuelo. **Alternativa definida:** CMC nativo más clips autorizados y poses propias mínimas.

## IB-GND00 — Un personaje que se mueve bien en tierra

**Resultado:** reposo, caminar, correr/esprintar, girar, saltar, caer y aterrizar con el mismo Pawn. Parámetros editables y controles compatibles con la futura transición aérea.

**Técnica propuesta:** reutilizar plantilla/animaciones accesibles; no fusionar ALS, GASP y otra locomoción. Proxies admitidos.

**Aceptación:** 30 ciclos caminar→correr→parar→girar y pruebas de rampas/esquinas/azoteas; sin entradas atascadas al perder foco. Paquete Win64 y captura del movimiento. Deslizamiento de pies o cámara brusca se registra y corrige antes de ampliar contenido.

**Evidencia Windows — 2026-11-15.** Implementado en ImpactBeyondCharacter sobre el CMC, las acciones Enhanced Input y las animaciones del Third Person. WASD corre (500 cm/s), Ctrl izquierdo + movimiento camina (220 cm/s) y Shift izquierdo + movimiento esprinta (700 cm/s). Si se presionan ambos, prevalece esprintar. Son límites editables en el Blueprint del personaje. El modificador se consulta al procesar movimiento y solo aplica en suelo. La configuración existente de Unreal limpia las teclas al perder el foco.

**Prueba del paquete.** Compilaron Editor y Game Win64 Development. AutomationTool completó el empaquetado y el ejecutable cargó Lvl_ThirdPerson en DX12 SM6. Las inyecciones de teclas mediante la consola de Enhanced Input registraron 220 → 500 → 700 → 500 cm/s como límites del CMC. En la serie repetida se registraron 32 pares consecutivos 220 → 500; se enviaron también entradas de parada y giro, sin instrumentar giro ni calidad visual en cada ciclo. La serie tuvo una lectura incompleta que se repitió. Los ensayos de foco, rampas y esquinas con teclado físico y la sensación del sprint siguen pendientes. Usar Win64 conforme al plan vigente. No iniciar IB-F01 hasta cerrar la validación.

## IB-F01 — Vuelo y transición continua

**Resultado:** despegue, estacionario, aceleración/boost, giro, freno, descenso, aterrizaje y recuperación; correr→saltar→volar→aterrizar→correr sin cambiar personaje.

**Donante:** subconjunto permitido elegido en FLT-AUD; movimiento nativo como respaldo. No heredar cambios de posición sin comprobación de colisión. El movimiento controla la pose; el montaje no secuestra el vuelo.

**Aceptación:** 30 ciclos suelo-aire-suelo, 20 cambios de régimen, esquinas y paredes delgadas a la velocidad máxima permitida en el prototipo. Recorrido de 10 minutos y prueba humana de control/velocidad/claridad. Registrar tasas de fotogramas realmente alcanzadas, no afirmar 30/60 por configuración solamente.

**Límite:** no planetas, agarres ni velocidad física ilimitada. Los clips faltantes se resuelven con pocas poses, no reconstruyendo una biblioteca entera.

**Implementación Windows.** El personaje cambia entre caminar, caer y volar con CharacterMovementComponent y la cápsula de la plantilla. Vuelo dirigido por la cámara en horizontal, ascenso y descenso independientes, estacionario al soltar, freno explícito y transición a suelo al bajar contra una superficie caminable. La build de desarrollo muestra nivel y velocidad real. La pose en vuelo sigue siendo provisional.

| Acción | Teclado/ratón | DualSense (asignaciones de gamepad) |
|---|---|---|
| Moverse / cámara | WASD / ratón | Stick izquierdo / derecho |
| Saltar / esprintar en suelo | Espacio / Shift izquierdo | Cruz / L3 |
| Entrar o salir de vuelo | F | R3 |
| Subir o bajar nivel | E / Q | R1 / L1 |
| Ascender / descender | Espacio / C | R2 / L2 |
| Freno en vuelo | X | Cuadrado |

Topes iniciales de la lista editable: 9, 18 y 32 m/s. Se podrá añadir un cuarto nivel de alta velocidad. El quinto se diseñará como una extensión que parte de estar en el cuarto y vuelve a él al terminar. No se fijan aún su duración ni umbrales, pendientes de un mapa de prueba adecuado.

**Evidencia.** Editor y juego Win64 Development compilaron y AutomationTool completó el paquete. El ejecutable cargó Lvl_ThirdPerson en DX12 SM6. Prueba visual de teclado: despegue con F, estacionario a 0 m/s y selección del nivel 3/3. Pendientes la batería completa de frenado, aterrizaje, colisiones y la validación de eventos con DualSense físico. Windows detectó el dispositivo. Si Unreal no recibe eventos de gamepad directamente, se puede probar la ruta XInput mediante [Steam Input](https://partner.steamgames.com/doc/features/steam_controller/legacy_mode). Próxima entrega agrupada: circuito gris IB-C00, con recta, giro y recorrido repetible de vuelo y aterrizaje para validar más casos en una sola sesión.

**Corrección de animación del prototipo.** El clip se detenía al entrar en vuelo y al cambiar de pose. Ahora se usa el ciclo aéreo de la plantilla en bucle, con reproducción más pausada en suspensión y más rápida en avance. La inclinación acompaña la velocidad y la animación terrestre se restaura al salir del vuelo. La prueba automatizada de Win64 incluye reproducción activa en despegue y en los tres niveles de crucero.

## IB-C00 — Calle de pruebas legible

**Resultado:** una manzana con avenida, giro, callejón, distintas alturas y azoteas; fachada pequeña preparada para el futuro D01.

**Donante:** un único kit urbano permitido y solo las piezas necesarias. Antes de un generador PCG complejo, construir un bloque de referencia reproducible para comprobar escala y colisión.

**Aceptación:** recorrer a pie y volando; orientación clara, escalas coherentes, materiales básicos y límites de recursos registrados. No declarar la manzana destructible hasta D01. Si los assets bloquean importación, usar proxies sin bloquear pruebas de gameplay.

## IB-P01 — El carro no puede ser un adorno pegado a las manos

**Resultado:** seleccionar, agarrar, transportar, soltar y lanzar un carro y un bloque; tamaño/estado de carga coherentes. Comprobar la colisión de la carga, no solo la cápsula del héroe.

**Donante:** REPIRK mínimo frente a Physics Handle/transporte asistido nativos. Un solo propietario lógico y una fuente de movimiento del objeto sostenido, también en single-player.

**Aceptación:** 30 ciclos por tipo en suelo y aire, frenadas, esquinas, soltar cerca de obstáculo, destruir la carga y reiniciar mientras se sostiene. Sin objetos duplicados, atraviesos ilegítimos ni colisiones/gravedad mal restauradas.

**Descartar:** resorte inestable que obligue a limitar todas las acciones. Respaldo: transporte controlado que conserve colisión y retorno a física al soltar.

## IB-D01 — Perforar, atravesar y reutilizar

**Resultado:** el carro abre una sección de fachada, el héroe la atraviesa y una pieza importante queda utilizable. Reinicio restaura geometría y colisión.

**Dos pruebas acotadas:** A) paneles/Chaos nativos preparados; B) componente de soporte de PlotBuilder en estructura pequeña, sin importar tienda, inventario ni bucle de construcción. Documentar qué mejora B frente a A antes de elegir. UE_Physics puede servir de ejemplo, no otra dependencia obligatoria.

**Aceptación:** 50 impactos en varias direcciones/bordes y 20 reinicios. Agujero transitable y ausencia de colisión fantasma en el conjunto ensayado; conteo de cuerpos/actores y memoria estable tras limpieza; p95/p99 y mayor tirón registrados. La prueba no acredita destrucción universal del mapa.

**Alternativa:** colapso preparado sin grafo estructural si portar PlotBuilder deja de ser una integración pequeña.

## IB-E01 — Oponentes útiles, uno cada vez

**Primero:** robot con 2–3 módulos preparados. Perder el arma/propulsor cambia realmente sus ataques/movimiento; una pieza se reutiliza como proyectil.

**Después:** rival humanoide de vuelo, con ataque anunciado, evasión, golpe recibido, agarre y recuperación. Reutiliza reglas del héroe pero no tiene conocimiento perfecto del input.

**Aceptación:** 20 secuencias de desarme y reinicio sin duplicar partes, 20 agarres/lanzamientos de rival, ninguna capacidad que dependa de una pieza ausente. Si el arte definitivo tarda, usar rig/modelos provisionales legales.

## IB-S01 — Primera entrega para jugar

**Resultado:** una manzana o distrito corto, H01, objetos, fachadas, robot y rival, sonidos, generador de encuentros y reinicio. Ampliar a 2–4 manzanas solo si la primera ya cumple perfil y control.

**Aceptación integrada:** una sesión de 15 minutos alternando caminar, correr, vuelo, agarres, pelea, daño estructural y reinicio. Registrar crashes, softlocks, posiciones ilegales, latencia percibida y rendimiento con configuración/hardware reales. Usuario puntúa control, potencia/claridad y deseo de repetir; objetivo inicial 4/5. Un resultado malo provoca ajuste, no una nueva lista de features.

**Congelación del alcance:** no ciudadanía, reputación, mundos ni H02/H03 como condición de cierre. Esta es la demostración del núcleo divertido.

## Después de S01: reserva ordenada, no tareas abiertas

E4: H02 sobre daño/agarre compartidos → H03 con destinos seguros → kaiju mediano con piezas preparadas. Cada personaje y enemigo es una tarea separada.

E5: incidentes y atribución → 3–6 ciudadanos → alerta/karma configurables → robo simple → meteorito en zona preparada → miniinvasión con enemigos existentes. Consultar documentos temáticos, sin activar multitudes antes de perfiles.

E6: laboratorio orbital separado → regreso a la ciudad conservando estado → Luna → cada planeta adicional con una región interactiva. Sol solo visual. El cooperativo no figura en esta reserva de entregas.

## Formato obligatorio de entrega

Ticket y commit; qué cambió; qué se reutilizó y de dónde; prueba ejecutada con pasos/resultado; motor/hardware/configuración; logs/captura/mediciones pertinentes; revisión humana si aplica; fallos y límites; decisión CONTINUAR/CORREGIR/BLOQUEADO. No presentar una tarea como asignada o ejecutada por haber escrito su ficha.
