# IMPACT: BEYOND — Herramientas verificables y un sandbox con sociedad propia

**Fecha de revisión: 2026-11-15. Estado: AUDITORÍA REMOTA + DECISIONES FUTURAS; NO IMPLEMENTACIÓN.**

Este documento complementa `ROADMAP_SPRINTS_02_A_08.md` por decisiones recientes del usuario. No inicia trabajo de sprints futuros ni modifica el código, assets, plugins o editor de una sesión de Codex activa. Se mantiene Windows/DX12, UE5.8.4 fijado en el proyecto, un jugador offline, presupuesto RD$0, reutilización máxima, cinco agentes como máximo y autorización antes de instalaciones, cambios de sistema, reclamaciones Fab/aceptación de licencias o merges a main. El permiso previo para obtener assets gratuitos no autoriza instalar herramientas por iniciativa propia.

## 1. Visión de producto expresamente confirmada

**El jugador decide salvar la ciudad, destruirla, explorar o ignorar una emergencia.** El juego ofrece poderes, físicas, interacciones y eventos emergentes; NO una campaña lineal, cadenas obligatorias de misiones o vuelos bloqueados por reputación. Robots, kaijus e invasiones alienígenas pueden aparecer cuando sus sistemas estén implementados. Atenderlos es opcional. El mundo puede cambiar como consecuencia, pero no se fuerza un game over ni un viaje obligatorio por ignorar un aviso.

Decisiones del usuario: menú de pausa en **Sprint 8**, sin retirar el comienzo del espacio; identidad original y lore ambiental en **Sprint 10**; héroes aliados no jugables que actúan ocasionalmente en **Sprint 15**. Selector radial de poderes solicitado para una etapa futura: se propone entregar su primera versión en S7, después de consolidar H02/H03 en S6, y unificarlo con el menú en S8. S7 para la rueda es propuesta de producción, no fecha impuesta por el usuario. S9 y S11–14 no reciben alcance nuevo cerrado por este documento.

## 2. Auditoría de herramientas: lo comprobado y lo desconocido

Último HEAD jugable publicado inspeccionado: `20e6a0081e33ab1abb7c8e7b67b55a4bc44a5443`, rama `prototype/ib-urban-mvp` / PR #18. Un checkout local puede contener cambios sin push: no se infiere que Codex esté detenido o que haya terminado.

| Evidencia reabierta | Hallazgo | Límite de la conclusión |
|---|---|---|
| `ImpactBeyond/ImpactBeyond.uproject` | Declara ModelingToolsEditorMode, StateTree y GameplayStateTree | No es inventario completo de plugins cargados: puede haber dependencias y ajustes locales |
| `ImpactBeyond/Source/ImpactBeyond/ImpactBeyond.Build.cs` | Incluye EnhancedInput, AnimationCore, UMG, Slate y módulos StateTree | Son dependencias declaradas; no prueba uso efectivo de cada función durante juego |
| `docs/IB_MVP_URBANO.md` y archivos Tools publicados | Documenta CMC, IK, HISM, interior cubemap, scripts Python de edición, pruebas de física y CSV | Sí hay reutilización nativa. No confundir scripts existentes con plugin externo instalado |
| Raíz del HEAD jugable | Tiene `.agents`, AGENTS.md, ImpactBeyond, README, Tools y docs; no contiene `.codex` ni `.mcp.json` publicados | Una configuración global/local de Codex puede existir fuera de Git; no se afirma ausencia local de MCP |
| PR #9, rama governance, `.codex/config.toml` | `agents.enabled=true` y `max_concurrent_threads_per_session=4` | Ajustes documentados por OpenAI actualmente, pero aún deben comprobarse versión y configuración efectivamente cargada en el Codex local |
| Perfiles de agentes preparados en PR #9 | Cuatro perfiles de auditoría de solo lectura | Esto no equivale a cuatro implementadores trabajando; el integrador puede convertirse en cuello de botella |
| AGENTS.md del HEAD jugable | Conserva referencias a instalación pendiente y antiguo arranque W00, además de instrucciones válidas nuevas de Steam Input | Reconciliar hechos de estado, sin borrar controles/permisos/decisiones vigentes al incorporar gobernanza |
| Informe de rendimiento del MVP | Describe medición DX12 fuera de pantalla, audio apagado, entradas simuladas y saltos de posición | Resultado útil de un ensayo, NO certificación audiovisual con mando ni recorrido continuo de regiones |

**MCP no está acreditado como activo en esta revisión.** No se ha conectado este chat al editor local. No se han observado llamadas MCP ni logs locales del servidor. El repositorio y una configuración escrita no permiten afirmarlo.

## 3. Protocolo obligatorio para el siguiente encargo de Codex

Antes de implementar, reunir una ficha breve de capacidades reales: commit y worktree, estado de cambios locales, versión de Codex, versión de Unreal, plugins efectivamente cargados, endpoints MCP configurados y operativos, número de subagentes realmente creados, roles/rutas exclusivas y evidencia de la build anterior. No publicar credenciales, configuración privada o rutas personales.

- Consultar `codex --version`, `codex mcp list` y, cuando la interfaz instalada lo admita, `/mcp`. Una entrada configurada no basta: comprobar conexión y una llamada real. Usar las opciones de la versión instalada, no copiar parámetros de otra edición.
- PR #9 es configuración preparada, no desplegada. Incorporar las secciones útiles al AGENTS vigente sin sustituirlo entero ni perder cambios de Steam Input. Reconciliar PLAN_ACTIVO/TAREAS_ACTIVAS con evidencias actuales; no repetir W00 por un encabezado histórico.
- Conservar un coordinador y hasta cuatro agentes. Asignar **resultados y propiedad por archivo**, no solo títulos: A1 integrador; A2 animación/presentación aislada; A3 mundo y activos aislados; A4 mecánicas/datos en componentes separados; A5 QA y perfil. La auditoría puede ser solo lectura, pero un agente implementador necesita permiso de escritura limitado a su trabajo, expresamente definido. No ignorar silenciosamente perfiles read-only.
- Un solo propietario del `.umap`, `.uasset` o AnimBP final; llamadas al editor y cook serializados. Worktrees separados no hacen seguro editar simultáneamente el mismo asset binario. No ejecutar cinco compilaciones pesadas a la vez en la laptop.
- Elegir por necesidad un recurso principal y un respaldo, verificar archivos/licencia/dependencias y probarlo en un área pequeña; detener investigación cuando una opción funcione. No usar un plugin porque aparezca en un vídeo, ni programar una alternativa sin revisar lo existente.
- Separar gates: compila; test headless; DX12 visual; audio activo; input físico; aceptación del jugador. Repetir los tests relevantes en el commit que se entrega, no heredar PASS de otra rama.
- Una rama de integración de sprint, un PR revisable de entrega y build identificada por SHA. Commits internos pequeños. No crear PR adicionales por cada arreglo visual sin una necesidad concreta; no fusionar main ni cerrar ramas de otros agentes automáticamente.
- Cada fallo visual confirmado por el usuario debe convertirse en una prueba reproducible y condición de cierre. Una animación que se reproduce no demuestra una silueta natural. Medir tiempo de reintentos/errores y tiempo hasta build jugable, no cantidad de commits ni líneas.

### Herramientas a evaluar, sin instalarlas ahora

**Unreal MCP nativo de 5.8:** Epic lo documenta como experimental. Puede inspeccionar y editar actores/materiales y ejecutar pruebas; serializa llamadas sobre el game thread. Su servidor tiene módulos runtime, por lo que no debe suponerse excluido automáticamente del paquete. Evaluar en checkpoint aislado, solo loopback, inicio manual, herramientas mínimas y sin autoaprobación. Probar lectura, cubo temporal, guardado/reapertura y una automatización; medir beneficio frente a scripts existentes. Parar el servidor y comprobar que la build distribuida no lo inicia. El generador de configuración Codex puede negarse a sobrescribir TOML existente: hacer merge conservador, no eliminar configuración privada.

**Python Editor Scripting / Editor Utility:** conservar para tareas de lotes deterministas y reproducibles. MCP para inspección/ajuste interactivo; script para repetir una operación validada; C++/Blueprint para juego. MCP no reemplaza la revisión visual ni arregla por sí solo una mala animación.

**CommonUI + UMG:** candidato nativo para navegación mando/ratón, foco y capas de pausa/selector. El proyecto ya usa UMG; no migrar todo el HUD por defecto. **UltimateUI de DanialKama**, con licencia MIT en su repositorio y menú de pausa/configuración, es respaldo a evaluar en S8, con revisión independiente de iconos/arte de terceros y compatibilidad UE5.8.4. No se ha importado ni probado.

**StateTree:** ya declarado. Reutilizarlo para estados de eventos y NPC cuando se necesite, no añadir otro framework de IA antes de comparar. Performance/audio/cámara siguen verificándose con Automation Tests, CSV e Insights existentes.

Fuentes oficiales consultadas: [Epic MCP 5.8](https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8), [Codex configuración](https://learn.chatgpt.com/docs/config-file/config-reference), [Codex subagentes](https://learn.chatgpt.com/docs/agent-configuration/subagents), [Codex MCP](https://learn.chatgpt.com/docs/extend/mcp?surface=cli), [CommonUI](https://dev.epicgames.com/documentation/unreal-engine/overview-of-advanced-multiplatform-user-interfaces-with-common-ui-for-unreal-engine), [StateTree](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-state-tree-in-unreal-engine), [UltimateUI](https://github.com/DanialKama/UltimateUI).

## 4. Selector de poderes: variedad sin otro conflicto de controles

Primera versión propuesta S7; preparación de datos compartidos durante S6. Rueda radial inspirada en la legibilidad de un selector de armas, no copia del arte o código de GTA V. Seis espacios iniciales, hasta ocho visibles por página si el mando lo permite. Mantener para abrir, stick/ratón para resaltar, soltar para seleccionar; centro muerto cancela, pulsación rápida vuelve al poder anterior. Botón se asigna después de inventariar conflictos con vuelo, sprint, frenada, agarre y niveles; no ocupar R1/L1 silenciosamente.

Datos por poder: ID estable, nombre/icono, contexto válido, efecto mecánico, coste/recarga opcional, animación, audio/VFX, condiciones de cancelación y prueba. La rueda selecciona; **no dispara al soltar por accidente**. Distinguir equipar de activar. Mantener accesos rápidos y disponibilidad de locomoción. Colecciones propias de H01/H02/H03; preset creativo permite combinaciones si no compromete colisión. No duplicar daño, agarre ni teletransporte para cada icono.

Tiempo lento opcional al abrir, predeterminado conservador; una pausa total es accesibilidad configurable en modo offline. Gestionar time dilation desde un único dueño para que al cerrar vuelva al valor previo, incluso tras abrir pausa o perder foco. Testear carga de lanzamiento, láser sostenido, velocidad alta, objeto agarrado y reset al abrir/cerrar la rueda.

Reutilización: UMG/Enhanced Input actuales como principal. Se inspeccionaron menús radiales públicos, pero no se incorpora CurvedLayout sin licencia verificada ni SP Radial Menu sin precio final RD$0 y prueba UE5.8.4. QuickMenu es para el editor, no sustituye un selector de poderes runtime. Evitar confundir ejemplos/documentación de un producto con un plugin gratuitamente reutilizable.

## 5. Sprint 8 — Espacio inicial Y menú de pausa completo

Se conserva ciudad → altura orbital → regreso con estado persistente. Se añade menú completo de pausa (Options/Escape como propuesta a validar): continuar, mapa, poderes/equipamiento, controles, ajustes de vídeo/audio/cámara/accesibilidad, guardar/cargar cuando el guardado por región esté validado, reiniciar zona con confirmación y salir.

Pausar debe congelar correctamente física, daño, eventos y reloj del mundo; el menú recibe input y no lo filtra al héroe. Reanudar no debe lanzar un carro, emitir un láser o mantener sprint porque se perdió un evento de soltado. Guardados versionados y seguros con objetos transportados, destrucción, hora global y estado de evento. Ajustes de 60 FPS/TSR/resolución interna explícitos; no etiquetar 83% y 100% como la misma configuración sin explicarlo. Una pausa técnica mínima anterior no adelanta el alcance visual completo de S8.

## 6. Sprint 10 — Identidad original y memoria del mundo, no campaña

El usuario pide nombres de ciudades, edificios emblemáticos, estatuas, lore y más interacción. La entrega será una biblia breve de identidad y un conjunto de lugares **con función jugable**, no una novela o una cadena de cinemáticas.

Nombres exclusivamente propuestos para revisión creativa: **Vértice** (centro vertical), **Puerto Ferral** (industria/puerto), **Nácar** (costa/cultura) y **Altavera** (montaña/ciencia). No son nombres aprobados, marcas comprobadas ni una obligación de renombrar los IDs internos. Las ciudades mantienen ID estable aunque cambie su nombre.

Cada ciudad: un edificio reconocible desde vuelo, un monumento, una institución, un espacio cotidiano y un incidente propio. Ejemplos: Torre de Coordinación, Memorial del Primer Impacto, museo de tecnología recuperada, hospital, central eléctrica, refugio, mercado o laboratorio. Darles estados intacto/dañado/inutilizado/en reparación y consecuencias razonables: un apagón afecta farolas; un rescate permite evacuación; una antena averiada cambia avisos. No bloquear el sandbox ni el viaje por destruir una estructura.

El lore aparece mediante placas breves, señalización, noticias de radio, murales, conversaciones cortas y cambios visuales. Una estatua de un protector puede mostrar un grafiti después de daños causados por el jugador. No prometer una IA generativa de NPC en runtime: usar frases y variaciones escritas, activadas por hechos del mundo. Reutilizar edificios/modulares de Fab y figuras riggeadas autorizadas para poses de estatua; editar lo imprescindible. No modelar otra ciudad desde cero para tener identidad.

Pedir aprobación creativa del usuario para nombres finales, símbolos y tono, sin detener el trabajo reversible de estructura y assets provisionales.

### Inspiraciones: extraer principios, no personajes ni propiedad intelectual

- **Invincible:** tomar escala del conflicto, organizaciones de respuesta y coexistencia de amenazas humanas/extraterrestres. No copiar GDA, trajes, nombres, símbolos ni historia. Fuente primaria consultada: [Ubisoft, universo Invincible y GDA](https://www.ubisoft.com/en-us/game/invincible-guarding-the-globe/news-updates/6fOCZNisnSl2FStfbHm6Au/major-gda-alert-invincible-guarding-the-globe-is-out-now).
- **Young Justice:** tomar redes de héroes, generaciones, mentores y una base que sugiera actividad fuera del protagonista. No copiar equipo ni sede. Fuente primaria: [DC, Young Justice: Outsiders](https://www.dc.com/blog/2019/01/04/heres-what-you-need-to-know-before-starting-young-justice-outsiders).
- **Dispatch:** tomar emergencias, disponibilidad de héroes, especialidades, fricción personal y comunicaciones. NO adoptar su estructura narrativa episódica. Fuente del editor: [Dispatch en Steam](https://store.steampowered.com/app/2592160/Dispatch/), [AdHoc](https://adhocstudio.com/).
- **Ben 10 / GALVAN:** estudiar legibilidad del selector y cómo un kit de acciones cambia la interacción con el mismo entorno. No asumir permiso para extraer aliens, animaciones o código de un ejecutable.

## 7. Eventos opcionales y libertad de salvar o destruir

Crear un director de eventos orientado a datos, no un generador infinito de misiones ni un LLM. Reutilizar regiones/IDs, daño/impulso, IA y enemigos existentes. Estados propuestos: elegible → aviso → activo → escalada opcional → resuelto/expirado → consecuencias → recuperación. Datos: semilla, región, origen, facción, severidad, presupuesto de actores/efectos, objetivos locales, duración, tiempo entre eventos y causas del resultado.

Robots: asalto a infraestructura, robo de tecnología, evacuación. Kaiju: aparición anunciada por mar/tierra y avance limitado. Invasión alienígena: reconocimiento, llegada localizada y retirada/contención; no miles de unidades en cada ciudad. Los tres tipos se habilitan solo cuando sus assets, IA y pruebas estén listos; no reinterpretar el catálogo como implementación.

El usuario puede intervenir, provocar, observar o ignorar. Consecuencia no equivale a castigo obligatorio: el incidente puede resolverse de otra forma, dejar daños locales o continuar; nunca exigir volver inmediatamente ni bloquear vuelo/poderes. Separar amenaza mundial, alerta local, reputación regional y relación individual. Registrar la causa real del daño, no culpar al usuario por escombros enemigos. No premiar rescates provocados artificialmente por el jugador de forma ilimitada.

Dos presets propuestos: **Mundo reactivo**, con memoria y consecuencias; **Laboratorio libre**, con ajustes de daño/reputación/frecuencia, generación manual de encuentros y reset. No exigir heroísmo como condición para disfrutar del juego.

Presupuesto inicial de ensayo: un evento mayor activo y hasta dos menores en el área relevante, ajustado por perfil real. Simular estados/resúmenes lejos del jugador, no físicas y combates completos. Guardar daño y resultados; rehidratar actores al acercarse sin duplicaciones. No reconstruir mágicamente una ciudad al descargarla; elegir reparación gradual, intervención de servicios o reset explícito.

## 8. Sprint 15 — Héroes aliados NPC ocasionales

Propuesta inicial de tres aliados originales: defensor que contiene amenazas, rescatista que evacua y técnico que protege/repara infraestructura. Cada uno tendrá nombre, presencia reconocible, especialidad, zona preferida, disponibilidad y relación con jugador/ciudad. No son compañeros permanentes ni nuevos personajes seleccionables.

Pueden llegar a un incidente, salvar civiles mientras el jugador pelea, detener un ataque, intercambiar una frase y marcharse. Si el jugador causa destrucción, advertir, priorizar rescate, negarse a ayudar o defender la ciudad según contexto. Evitar policía omnisciente, teleports visibles de aparición, invulnerabilidad narrativa y aliados que resuelven todo quitando diversión.

Compartir primitivas de movimiento/daño/agarre, StateTree y componentes de habilidades. Usar modelos y clips autorizados de Fab/CC0 con cambios mínimos. NPC activos solo cerca de escenas relevantes; estado abstracto de disponibilidad lejos. Memoria de pocos hechos concretos, no simulación ilimitada de toda sociedad.

Aceptación: cada aliado interviene en una escena repetible, puede coincidir con otros eventos sin duplicarse, reacciona de modo distinto al rescate/daño causado, se retira limpiamente y mantiene 60 FPS con audio/render real. No considerar esta entrega realizada por tener un modelo que camina.

## 9. Investigación del juego fan de Ben 10

**Identificación todavía provisional:** el candidato que mejor coincide con un sandbox 3D de poderes es **GALVAN, de Haez**. La página oficial lo identifica como juego Windows, en desarrollo, mundo abierto, fan game y Unreal Engine. En la consulta mostraba 0.19.4, versión itch 11. Revisadas página del autor y búsquedas de repositorio: no se encontró enlace a fuente pública ni licencia abierta para su código. Esto NO demuestra que no exista código privado; simplemente no acredita reutilización. No contactar al autor ni descargar/ejecutar el juego sin una necesidad/autorización concreta.

- Fuente del creador: https://haezdev.itch.io/galvan
- Página del creador en Game Jolt: https://gamejolt.com/games/galvan/918236
- Alternativa plausible: **Hero Time, de Muskito**, listado oficial Game Jolt como fan game Unity inspirado en Protector of Earth. No se encontró allí licencia/repositorio fuente: https://ssr.gamejolt.net/games/herotime/578793
- **Omnitrix Arena**, de leonardont, sí publica repositorio Unity y licencia Apache-2.0, pero su README admite modelos extraídos de juegos comerciales y declara uso de investigación. No adoptar el repositorio completo ni tratar su arte como assets libres: https://github.com/leonardont/Omnitrix-Arena

La conclusión es de búsqueda documental, no de una auditoría de binarios ni de jugar la demo. No identificar con certeza el juego que recuerda el usuario sin nombre, imagen o enlace. Estudiar principios visibles no requiere importar assets de franquicias.

## 10. Bloque de mejora para futuras instrucciones

"Primero identifica la build y capacidades reales de esta sesión; informa qué herramientas y agentes están operativos, no solo configurados. Reutiliza la solución existente que pase la prueba con menor complejidad. Distribuye trabajo implementable con dueño por archivo y único editor. Cada entrega debe preservar la libertad de salvar, destruir o ignorar eventos y debe aportar interacciones observables, no solo arquitectura. Prueba animación, cámara, sonido y mando en la build de ese commit; no equipares un PASS headless o una media de FPS con aceptación humana. No amplíes el sprint actual con el lore, menú o aliados futuros. Actualiza las fuentes activas con hechos verificados y conserva una build anterior. Si MCP no aporta ahorro medido, usa scripts nativos y continúa; no instales plugins para justificar avance." 

**Hecho en esta revisión:** lectura de fuentes y planificación en rama documental. **No hecho:** conexión/instalación MCP local, activación de CommonUI/UltimateUI, ejecución de Codex, modificación del gameplay, aceptación de licencias o merge a main.
