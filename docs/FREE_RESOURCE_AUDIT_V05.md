# IMPACT: BEYOND — Auditoría adicional de recursos gratuitos v0.5
Fecha 2026-11-14. **Estado = documental/fuente inspeccionada** salvo indicación explícita; ningún candidato nuevo se ejecutó en Linux. Complementa REUSE_CATALOG.md (R01–R28) y CITY_SOLAR_TELEPORT_V04.md. No se promete que los archivos fuente de un modelo sean libres para republicarse sin verificar licencia exacta.

## Nuevos hallazgos para ensamblaje
| Recurso | Evidencia pública y licencia | Qué sirve | Qué NO garantiza | Decisión |
|---|---|---|---|---|
| Quaternius Sci-Fi Essentials Kit (nov. 2024) | 65 modelos catálogo completo, Standard gratis ~60–70%, CC0 indicada en página original: https://quaternius.com/packs/scifiessentialskit.html | robots animados, armas, cajas y atrezzo; primera extracción selectiva | ni 65 archivos incluidos en gratis, ni mallas divididas en brazos/cabeza; Source Unreal importado es otra edición | **PROBAR PRIMERO** para robot |
| Quaternius Animated Mech Pack (mar. 2021) | 4 mechas, animados, CC0, FBX/OBJ/Blend/glTF: https://quaternius.com/packs/animatedmech.html | robot pesado/boss visual y animaciones | no partes desmontables ni física modular integrada | **PROBAR DESPUÉS** del robot básico |
| Quaternius Animated Monster Pack (ago. 2018) | 4 monstruos animados, CC0: https://quaternius.com/packs/animatedmonster.html | cuerpos y animaciones para minikaiju provisional | escalado no equivale a ataques grandes, navegación, escala de daño y colisiones | **PROBAR** en laboratorio kaiju |
| Quaternius Animated Robot Pack (oct. 2018) | 1 robot, CC0: https://quaternius.com/packs/animatedrobot.html | maniquí robot simple con animaciones | estilo infantil quizá no encaje con mundo semirrealista | **RESERVA** solo prototipo gris |
| Quaternius Downtown City MegaKit (may. 2026) | más de 300 piezas en paquete completo, ~60–70% libre en Standard, CC0 en página: https://quaternius.com/packs/downtowncitymegakit.html | bloques densos y rascacielos | Standard no incluye automáticamente Unreal project, shaders de ventanas, colliders propios de Source | **PRIORIDAD CIUDAD** |
| Quaternius Ultimate Space Kit (mar. 2023) | 92 modelos, CC0: https://quaternius.com/packs/ultimatespacekit.html | cuerpos planetarios visuales, enemigos/objetos espaciales | no sistema de planetas con gravedad y aterrizaje real | **RESERVA W01/W02** |
| Kenney Space Kit (2020) | 150 archivos, licencia CC0: https://kenney.nl/assets/space-kit | piezas espaciales/satélites/escenografía | no física orbital ni terreno jugable | **RESERVA W02** |
| PCG Extended Toolkit / PCGEx | MIT archivo LICENSE; README oficial menciona soporte Linux y ramas por versión: https://github.com/PCGEx/PCGExtendedToolkit | redes de calles, grafos, zonas, filtros para poblar ciudad | no ciudad completa ni plug&play sin instalación/compatibilidad; Pro separado | **RESERVA**, PCG UE nativo primero |
| OpenWorldGenerator / Archengius | MIT; descriptor OpenWorldGenerator.uplugin con módulo Runtime y PCG: https://github.com/Archengius/OpenWorldGenerator | generador de terreno/PCG en editor y runtime | repositorio corto (4 commits observados), sin evidencia 5.8/Linux/rendimiento; demasiado invasivo frente a UE Landscape | **NO BASE**, reserva experimental |
| BBBomber/Dismemberment | MIT LICENSE comprobada; fuente C++ con cortes de estáticas/esqueléticas; https://github.com/BBBomber/Dismemberment | referencias de separación dinámica y hit detection | README reconoce sin tapas en limbs, mallas agujereadas, colisiones convexas inexactas; no copia directa para no-gore ni caos masivo | **DESCARTADO PARA V1**, estudiar ideas solamente |
| Thyke/Dismemberment | MIT publicada; descriptor inspeccionado declara EngineVersion 5.4.0; https://github.com/Thyke/Dismemberment | partes esqueléticas separables con datos | migrar a 5.8/Linux y preparar cada malla; no sustituye robot rígido | **RESERVA TRAS ROBOT** |
| REPIRK/Modular-Mechanics-Toolkit-UE5 | MIT LICENSE, componente PhysicsGrabber inspeccionado, README indica 5.7/Win64; https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5 | agarre, soltado, carga, lanzamiento, eventos | no juego importable completo, no certificado Linux, masa/resorte puede vibrar a gran velocidad | **PRIMER EXPERIMENTO AGARRE** |
| Ephemera / 0xWerz | MIT, archivo proyecto UE5.8, algunos plugins limitados a Win64: https://github.com/0xWerz/ephemera | ejemplo de planeta/luna y desplazamiento con IA que ensambla código | no combate superheroico ni build Linux certificado | **LABORATORIO APARTE** |
| Cosmic Architect / narratech | MIT y FastNoiseLite MIT, autores documentan UE5.7: https://github.com/narratech/TFG-Bravo-Perez-Rubio | generación procedural planetaria | no aterrizaje/combate/destrucción integrados ni UE5.8/Linux verificados | **ALTERNATIVA ORBITAL** |
| UE 5.8 Third Person Combat template | Primer recurso Epic interno del motor: https://dev.epicgames.com/documentation/unreal-engine/variants-in-game-templates | personaje, ataques/enemigos básicos; estándar limpio | no vuelo supersónico, robots/destrucción, ni planeta | **BASE POR EVALUAR**. Linux packaged smoke test obligatorio |
| GASDocumentation / tranek | Ejemplo público de Gameplay Ability System UE5.3: https://github.com/tranek/GASDocumentation | principios para efectos/replicación de habilidades | no solución lista para importar a 5.8 y usar 3 héroes; sample envejecido | **REFERENCIA**, no dependencia |

## Política de licencias 2026: importante
- Las páginas de packs de Quaternius consultadas aún muestran **CC0** para esos packs, pero la nueva **QAL v1.0 (28 agosto 2026)** se aplica a material liberado BAJO esa licencia y prohíbe redistribución standalone de assets: https://quaternius.com/license.html. No asumir todos los recursos CC0 para siempre. Guardar copia local de la licencia y ficha vinculada al archivo/fecha descargado. No subir archivos QAL a GitHub público. Cuando un recurso sí sea CC0 específico, se puede seguir su licencia, verificando contenido y origen.
- Kenney: paquetes individuales CC0, pueden descargarse gratis sin comprar All-in-One: https://kenney.nl/assets/space-kit.
- Códigos MIT: conservar avisos/copyright en distribuciones de código; licencia de código no implica permisos para logos/personajes/modelos propietarios usados en demo.
- No descargar/modificar archivos de juegos comerciales como GTA/Spider-Man/Invincible/Nightcrawler para luego empaquetarlos. Un mod gratuito puede seguir dependiendo de licencia cerrada.
- UE para juegos: royalty estándar 5% sobre ingresos brutos del producto por encima del primer US$1M de por vida (con excepciones); uso inicial sin pagar por motor, no significa EULA de assets indiscriminada: https://www.unrealengine.com/license.
- La EULA UE cambió en octubre 2026 (reglas sobre IA); ello no cancela licencias de Fab ni de artistas: https://www.unrealengine.com/eula-change-log/unreal.

## Qué no acumularemos
No descargar todos los assets ni ponerlos en Content por si acaso. Por cada sistema: 1 paquete principal + 1 fallback máximo. No mezclar decenas de packs de estilos divergentes. No importar UAssets/UE proyectos de ediciones Source pagas. No hacer backend/IA en tiempo de ejecución. No asumir un archivo de un repositorio público puede copiarse si no hay licencia.
Para kaiju: elegir SOLO UN rig, 3 puntos de desarme funcionales y escalas adecuadas. Para robots: UNO con cabeza/brazo/propulsor desmontables de forma preparada. El trabajo de cortar y volver a riggear assets es más caro que cambiar material o agregar sockets: experimentarlo antes de prometer demasiadas partes.

## Checklist de prueba por recurso
1. Identificar URL directa, paquete exacto, licencia y fecha, edición gratuita o Source, y commit de fuente cuando sea GitHub.
2. Inspeccionar scripts/README y dependencias antes de ejecutarlas. Ninguna instalación global sin autorización.
3. Descargar a área de prueba, listar archivos reales, formatos, mallas, texturas, esqueleto; no inferir archivos del material promocional.
4. Importar solo 1–3 mallas/animaciones relevantes en nivel de prueba. Probar materiales, escala, colisión, uso de memoria, rig/retarget, desempeño Linux/Vulkan.
5. Documentar COMPILE/EXECUTE/ACCEPTED con logs y paquete Linux; no prometer rendimiento por vídeos del autor. Si falla por API/versiones o licencias, reservar/descartar.
6. Aprobar y bloquear nueva búsqueda de alternativa para ese sistema salvo regresión demostrada.

## Fuentes técnicas del motor
Unreal Linux requisitos y toolchain versionado: https://dev.epicgames.com/documentation/unreal-engine/linux-development-requirements-for-unreal-engine?lang=en-US
Chaos prefracturado y clusters: https://dev.epicgames.com/documentation/en-us/unreal-engine/destruction-overview
Chaos rendimiento y limitación de bodies: https://dev.epicgames.com/community/learning/tutorials/KWeX/unreal-engine-a-tech-artist-s-playbook-for-chaos-performance
World Partition no es un sistema solar: https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine
Unreal MCP experimental: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8
