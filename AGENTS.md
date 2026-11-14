# AGENTS.md — IMPACT: BEYOND

## Misión

Crear un sandbox superheroico 3D divertido mediante reutilización gratuita. Priorizar vuelo frenético, agarres, lanzamientos y destrucción. No convertir la primera entrega en un proyecto de planeta completo.

Leer README.md, docs/MASTER_PLAN_V05.md, docs/FREE_RESOURCE_AUDIT_V05.md, docs/CODEX_QA_PLAYBOOK_V05.md, docs/CITY_SOLAR_TELEPORT_V04.md, docs/COMBAT_ROSTER_V03.md, docs/GAMEPLAY_SPEC.md, docs/SANDBOX_DESIGN.md, docs/COOP_PLAN.md, docs/LINUX_BASELINE.md, docs/REUSE_CATALOG.md y docs/REUSE_PLAN.md antes de modificar arquitectura o importar dependencias.

Precedencia: MASTER_PLAN_V05 para orden/criterios, FREE_RESOURCE_AUDIT_V05 para nuevas licencias y recursos, CODEX_QA_PLAYBOOK_V05 para autonomía/QA; LINUX_BASELINE para plataforma; GAMEPLAY_SPEC para controles y núcleo individual; COMBAT_ROSTER_V03 para H01/H02 y enemigos; CITY_SOLAR_TELEPORT_V04 para ciudad, exploración solar y H03; SANDBOX_DESIGN para modos/escenarios no sustituidos; COOP_PLAN para dos jugadores, autoridad, pausa online y pruebas tempranas. Las exclusiones antiguas de multijugador se refieren a la primera entrega, no a una arquitectura permanentemente individual. El catálogo conserva candidatos y reservas, no aprobaciones de compatibilidad.

## Decisiones creativas vigentes — 2026-11-14

- El usuario rechazó los nombres VECTOR y PRISMA. No usarlos en interfaz, conceptos ni nuevas clases. H01/H02 son identificadores internos, no nombres artísticos. Los nombres finales siguen pendientes.
- Mantener al combatiente de fuerza y al genio cósmico masculino de la referencia aprobada. H01: traje grafito con placas claras y acentos rojos. H02: hombre de cabello corto rizado, traje oscuro con plata/oro y energía esmeralda, manos espectrales y manifestaciones de genio. No sustituir H02 por la heroína genérica inicial.
- H02 no depende de una fruta del diablo, ni usa traje o símbolo de Linterna Verde. No recuperar la iconografía de franquicias de la primera referencia.
- Eliminar el arquetipo de enemigo constructor/energético genérico. Esto no elimina al héroe de creación ni convierte todos los rivales en copias de él.
- Incorporar H03, desplazador espacial de teletransporte original: traje índigo, fracturas luminosas azul-violeta; Blink, contra-blink, golpe de reaparición, regreso a marca. Sin símbolos/rasgos copiados de Nightcrawler. Personaje NO creado todavía, sin nombre definitivo.
- Mundo objetivo: ciudad muy densa y vertical con destrucción por capas y perforación funcional, planeta principal a escala reducida, Luna + dos planetas visitables en regiones pequeñas. Sol visible, no aterrizable. No desarrollar una simulación estelar completa ni publicar un mapa GTA/Marvel.
- Robots, kaijus y superhumanos son las tres familias. Robots/kaijus pierden piezas con efectos sobre capacidades y con fragmentos importantes reutilizables. Separación estilizada sin vísceras ni anatomía explícita.
- Diez enemigos diseñados no son alcance simultáneo: primer robot modular pequeño, rival de vuelo y después kaiju mediano de prueba. No modelar diez rigs ni añadir una forma colosal antes del bucle básico.
- Documentos anteriores pueden contener nombres/identidades históricos. Aplicar COMBAT_ROSTER_V03 cuando entren en conflicto. El arte de conversación no está automáticamente subido al repositorio ni implica un modelo 3D importable.

## Reglas ejecutivas adicionales — v0.5

- Trabajar **por gates** L00 -> F01 -> C01 -> P01 -> D01 -> E01 -> S01 -> H02/H03 -> K01 -> N01 -> W01/W02. El primer prototipo debe jugarse, nunca comenzar importando todo el catálogo. La prioridad es vuelo, carro y hueco transitable; sistema solar completo y red pública no bloquean esa secuencia.
- La licencia de Quaternius **puede diferir por pack/fecha**: páginas anteriores CC0; nueva QAL (agosto 2026) restringe redistribución standalone de packs bajo ella. Verificar licencia ligada al archivo descargado. No confundir Standard gratuito con Source de pago. Cuidado con licencias de Fab.
- Preferir robots Sci-Fi Essentials Standard y mechas/monstruos animados CC0 como base provisional; ninguna malla se considera desarmable hasta preparar módulos y aprobar pruebas. No instalar a la vez dos frameworks de desmembramiento.
- Unreal 5.8/Linux: hardware recomendado 32GB RAM/8GB VRAM, Vulkan, y toolchain versión específica según requisitos actualizados. **No inventar las especificaciones reales del usuario**. Si no hay GPU/editor accesible, sólo completar documentación/código estático e indicar el bloqueo.
- Unreal MCP es experimental y sus operaciones editor/game thread deben ser serializadas. No exponer puerto ni afirmar que Codex editó uassets sin evidencia. La IA no evalúa por sí sola la sensación de jugar.
- Mantener pruebas N0/N1 de red desde primer movimiento/agarre, pero no crear lobby/relay ni sistemas de Internet antes de pasar el sandbox local. El modo individual sin conexión debe funcionar siempre.

## Estado inicial

Este repositorio contiene documentación. No contiene todavía un proyecto Unreal. Ningún candidato del catálogo se ha compilado o ejecutado durante la investigación inicial. No describir documentos, personajes propuestos ni matrices de pruebas como un juego implementado.

## Plataforma y jugabilidad

- Linux nativo con Vulkan. No depender de Wine/Proton, DLL de Windows ni servicios gráficos exclusivos de un fabricante.
- No imponer distribución ni versión del motor sin inspeccionar el entorno real. Usar toolchain/documentación correspondientes a la versión elegida. No confundir un contenedor de chat con el equipo del usuario.
- Todo candidato requiere inspección de fuentes/descriptores y pruebas de compilación, empaquetado y ejecución en Linux. README, licencia MIT o binario Windows no acreditan compatibilidad.
- R02/REPIRK condicionado a portabilidad del agarre; R03 en reserva hasta confirmar Linux, licencia y acceso. Pared Chaos modular como respaldo. R01/Ephemera es laboratorio separado, no base obligatoria.
- Orden: L00 Linux -> D0 vuelo/control/cámara -> distrito urbano de prueba -> D1 carro/pared y piezas robóticas -> D2 rival y sandbox -> H02/variedad/kaiju -> H03 blink seguro -> cooperativo regional -> ciudad ampliada y laboratorio planeta->órbita->luna->dos planetas. Preparar N0/N1 temprano; no esperar la campaña espacial para probar red. N0/N1 se prueban durante D0/D1. No construir lobby/Internet antes de la primera diversión individual.
- Vuelo sin combustible; interacción compartida; no autofrenadas bruscas invisibles, menús por cada agarre ni generación de objetos con IA durante gameplay.
- Todos los héroes implementados funcionan solos y pueden repetirse en cooperativo. El núcleo de H02 reutiliza golpe, agarre, escudo y martillo con presentación de genio; brazos extra y avatar colosal son ampliaciones.
- Teleport de H03: usar primero APIs nativas de Unreal y comprobaciones de cápsula/destino; nunca dentro de paredes, sin colisión cargada ni a regiones no preparadas. El servidor valida teletransportación en cooperativo. No reutilizar scripts/assets de mods GTA o Marvel.
- Para ciudad explorar Quaternius Downtown City MegaKit Standard CC0 (solo parte gratuita), Kenney y PCG nativo; PCGEx MIT solo si simplifica trabajo real. Para sistemas planetarios evaluar Ephemera MIT o Cosmic Architect MIT en laboratorio Linux aislado; no fusionar ambos ni importar paquetes de pago. Para arte espacial Quaternius Ultimate Space / Kenney Space/Station CC0.
- Reutilizar acciones de H01 para el rival sin darle conocimiento perfecto de entradas/posición. Primer robot con unas pocas piezas; no corte universal ni combinaciones infinitas de amputación.
- Identidad de cómic 3D estilizado, siluetas legibles y materiales consistentes. Priorizar modelos provisionales/piezas rígidas antes que capas/ropa complejas.
- Compilar no demuestra diversión. Obtener feedback humano de control, velocidad y claridad además de pruebas técnicas.

## Dos jugadores y límites

- El usuario pidió dos jugadores a futuro. Cooperativo LAN y después Internet con anfitrión jugador/cámara individual es la propuesta actual. No afirmar que escogió explícitamente online frente a pantalla dividida.
- Preparar autoridad y contratos desde el inicio; probar dos conexiones con movimiento, carro y pared antes de consolidar donantes. 'Activar Replicates' no resuelve todo.
- No usar jugador 0 como propietario global de cámara, poderes, inventario o streaming. Entradas/cámara/UI locales; daño, agarres y cambios estructurales con autoridad definida.
- Un solo portador por objeto y un solo desprendimiento por PartId/versión. Validar solicitudes por actor poseído; limpiar propiedad en muerte, desconexión, destrucción y reinicio. No dos clientes decidiendo daño definitivo.
- Replicar estado jugable de estructuras/piezas y pocos objetos importantes; escombros cosméticos sin colisión/daño jugables locales. No asumir Chaos determinista ni depender solo de eventos transitorios para daño persistente.
- Movimiento personalizado integrado con predicción/replicación de CMC cuando aplique. No mover por Tick con SetActorLocation y declarar red resuelta.
- Individual puede pausar; online menú/pérdida de foco libera entradas pero no detiene el mundo. No pausa global por impacto ni cámara lenta unilateral que cambie simulación.
- Fuego amigo desactivado por defecto. Agarre de aliado requiere aceptación individual y posibilidad de soltarse; no basta una opción del anfitrión.
- Primera región cooperativa acotada y anunciada; no prometer órbita/superficie simultáneas, migración de anfitrión, PvP, pantalla dividida o conexión universal sin configurar red.
- No contratar servicios, abrir puertos, instalar VPN, conectar proveedores ni modificar red sin autorización específica. Ninguna IA externa es necesaria en runtime.

## Presupuesto y permisos

- Compras de assets/plugins: RD$0. No usar ediciones Pro/Source de pago, pruebas temporales o dependencias que exijan comprar después.
- No asumir acceso ilimitado a servicios IA ni contratar APIs para runtime.
- No copiar repositorios sin licencia explícita. No inferir licencias por ser públicos ni por otros proyectos del autor.
- Conservar LICENSE, copyright y avisos al incorporar código permitido; documentar cambios propios.
- Revisar arte por separado del código. Mantener personajes y marcas originales.
- Repositorio público: no publicar credenciales, rutas personales, datos privados ni paquetes de Fab/tiendas cuya redistribución fuente no esté permitida.
- No contactar autores, publicar mensajes ni cambiar visibilidad/permisos sin autorización adicional.

## Reutilización

1. Elegir un subsistema y prueba pequeña. Consultar catálogo antes de escribirlo entero.
2. Inspeccionar archivos, licencia y dependencias; fijar commit/versión. README no prueba que assets/Blueprints existan.
3. Importar lo mínimo en un experimento aislado. Justificar un framework completo.
4. Ejecutar pruebas del plan, Linux y puertas cooperativas pertinentes. Registrar fallos y coste cualitativo de adaptación.
5. Conservar una sola solución principal por sistema; no superponer frameworks incompatibles.
6. Dejar de buscar cuando una opción gratuita cumple la prueba y permite avanzar al sandbox.

## Seguridad de ejecución

Inspeccionar scripts de terceros antes de ejecutarlos. No ejecutar instaladores desconocidos ni desactivar controles de seguridad. No instalar plugins globalmente ni modificar el motor sin necesidad/autorización. No acciones destructivas de Git, sobrescritura de trabajo existente ni cambios en otros repositorios.

MCP es opcional y de editor; verificar disponibilidad en Linux antes de prometer automatización. No exponerlo a Internet ni hacerlo dependencia del ejecutable. No modificar configuraciones existentes a ciegas.

## Evidencia y estados

Estados: DESCUBIERTO, DOCUMENTADO, FUENTE_INSPECCIONADA, COMPILADO, EJECUTADO, ACEPTADO, BLOQUEADO. No saltar de enlace a ACEPTADO. Compilación no demuestra jugabilidad, rendimiento ni calidad visual.

Para cerrar una tarea, registrar:

- Fuentes/commit upstream, licencia y archivos importados.
- Versión real de Unreal, distribución Linux, compilador, GPU/driver y hardware.
- Comandos/pasos reproducibles, mapa, resultado esperado y observado.
- Logs de compilación/empaquetado; pruebas ejecutadas y fallidas.
- Resolución/calidad/hardware con mediciones; no FPS sin contexto. Separar primera ejecución y repeticiones.
- En red: anfitrión/cliente, procesos/equipos, RTT, pérdida/jitter y configuración. PIE limpio no sustituye paquetes Linux ni prueba de latencia.
- Para desarme: PartId, ataques deshabilitados, número de piezas generadas y resultado de reinicio/reconexión.
- Limitaciones y cambios propios. No inventar porcentajes.

No se fija parche del motor aquí: comprobar instalación/experimentos antes de hacerlo. Un selector de documentación actualizado no autoriza migrar a Preview.

## Primera tarea técnica pendiente

Inspeccionar Linux y preparar la base mínima Unreal. Obtener paquete nativo y circuito de vuelo con aceleración, frenado, recuperación y cámara. Probar N0; después R02 y pared según D1/N1. No descargar catálogo entero ni empezar migración planetaria.

No declarar terminado sin abrir, ejecutar y empaquetar donde exista motor funcional. Sin Unreal, completar solo inspección/documentación útil e indicar qué prueba no pudo ejecutarse. No crear Blueprints ficticios ni presentar código no compilado como validado.
