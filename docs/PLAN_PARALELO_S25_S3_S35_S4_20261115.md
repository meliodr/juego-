# IMPACT: BEYOND — Recuperación visual y cuatro laboratorios paralelos (S2.5 / S3 / S3.5 / S4)

Fecha: **2026-11-15**. **ESTADO: DISEÑO + ANÁLISIS, NO IMPLEMENTACIÓN.** El usuario ejecutará cuatro encargos Codex en paralelo y revisará los resultados después. No afirmar que las tareas ya se están ejecutando o que existen builds. No alterar trabajo local ni hacer merge. Presupuesto RD$0, un jugador offline, Windows/DX12/UE5.8.4, RTX 4070 Laptop 8GB/64GB RAM y SSD disponible a verificar.

## 1. Diagnóstico de qué falló

Base real inspeccionada: PR #19, \`prototype/ib-sprint02\`, commit \`1d01db54b63250ed0aacb0704a5ac16290324e32\`, dependiente de #18. El informe \`docs/SPRINT02_FEEL_WORLD.md\` declara 5 marchas, 6 autos físicos, 8 estacionados y carretera costera. \`Tools/Author-Sprint02World.py\` construye autovía con \`/Engine/BasicShapes/Cube\`, agua con \`Plane\`, colinas con \`Sphere\` y árboles con \`Cylinder+Cone\`; adicionalmente importa auto/taxi/carro eléctrico de Kit Cidade **Low Poly**. Ese es el origen concreto de la decepción visual, no el hardware. El criterio de aceptación del S2 era funcional y técnico; **no exigía comparación visual con una referencia City Sample a idéntica cámara, iluminación y distancia**, y el QA ofscreen/audio/input simulado no probó sensación real. Aprendizaje: geometría provisional es válida SOLO en sandbox, no se acepta como ciudad/periferia final.

Epic ofrece City Sample UE5.8 **renovado con PCG interno, PCG Primitives y compatibilidad MCP**: https://dev.epicgames.com/documentation/unreal-engine/city-sample-pcg-for-unreal-engine?lang=en-US . Su demo contiene 18 grafos interdependientes y streaming NO queda listo por defecto: hay que particionar los grafos/actores (ej. celda 128m). La versión anterior con Houdini no es obligatoria para aprovechar la de 5.8. **No portar los 18 grafos completos por inercia**. La malla principal de Unreal mantiene el juego; City Sample es referencia y donante selectivo, no reemplazo automático de movimiento.

**Patrón de objetivo artístico del usuario:** realismo percibido objetivo de ~1/3–1/2 del City Sample en núcleos urbanos; ratio subjetivo de calidad, NO reducción matemática de VRAM, triángulos o shaders. El arte debe tener formas, PBR, materiales, geometría de silueta, calle, autos y vegetación coherentes. Rechazar literalmente cubos/conos/esferas visibles como árboles, montañas, casitas o costa finales.

## 2. Referencias verificables y permisos

| Recurso | Fuente | Selección |
|---|---|---|
| City Sample 5.8 completo | https://www.fab.com/listings/4898e707-7855-404b-af0e-a505ee690e68 ; ya figura reclamado en Mi biblioteca auditada PR #6; bytes locales sin confirmar | **Referencia visual**: empezar por \`Small_City_LVL\`, si está instalado y hay espacio; no clonar/importar todo |
| City Sample PCG 5.8 | https://dev.epicgames.com/documentation/unreal-engine/city-sample-pcg-for-unreal-engine?lang=en-US | Reutilizar ejemplos de reglas/calles/parcelas/vegetación y subgrafos SOLO tras inspección |
| City Sample Buildings | https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5 | Free en ficha, >2,000 módulos, UE-Only, **no se ha demostrado posesión/compatibilidad efectiva individual** |
| City Sample Vehicles | https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f | Free en ficha, 13 vehículos, UE-Only; preferir 2–3 modelos reales de aquí en arte S2.5 |
| City Sample Crowds | https://www.fab.com/listings/903037e9-e1ac-4f41-96e8-1683c6fa7ad4 | Free en ficha, 6 tipos de cuerpo, prendas, Blueprint; **no incluye demostración de uniformes policiales/armas**, adaptar y retarget |
| Game Animation Sample | https://www.fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016 | Free en ficha, 500+ animaciones; clips selectivos, no Motion Matching entero |
| Electric Dreams Environment | https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306 | Free UE-Only, ejemplo de bosque/PCG fotoreal; pesado, estudiar estructura y migrar SOLO subassets permitidos |
| Content Examples | https://www.fab.com/listings/4d251261-d98c-48e2-baee-8f4e47c67091 | Free, ejemplos de materiales/física/UI nativos para adaptar conceptos |
| Automotive Materials | https://www.fab.com/listings/5dd132fe-ee32-4e8c-9cd3-7496547dfb29 | Free, 164 materiales/texture UE4 antiguos; validar compatibilidad y coste real en 5.8 |
| Poly Haven | https://polyhaven.com/license | CC0, HDRI/PBR/3D realista, alternativas ligeras sin Fab |
| AmbientCG | https://ambientcg.com | Materiales PBR CC0 sujetos a comprobar ficha/archivos |
| T-72 free model (opcional) | https://nekis5-3d.itch.io/t-72-3d-model-free | FBX, gratuito uso comercial según autor, **sin texturas**, licencia precisa y procedencia antes de importar, no primera elección para arte final |

**Steam**: juegos y Workshop NO son catálogo general de assets licenciados para otros juegos. Se pueden estudiar demostraciones y jugabilidad; descargar elementos de Workshop para incorporarlos a IMPACT solo con licencia **explícita por elemento** para redistribución en un juego independiente. No extraer mallas/código/audio de juegos comerciales.

Fab Standard permite incluir activos en un juego y modificar, no redistribuirlos como pack fuente independiente en GitHub público (https://www.fab.com/eula). NoAI: no proporcionar bytes de assets etiquetados a modelos generativos. El usuario permite investigar recursos gratis y descargar CC0 verificado necesario; **reclamación Fab, aceptar nuevos términos, instalar plugins, modificar Windows, borrar personales, o hacer merge a main requieren autorización específica**. El usuario piensa descargar City Sample él mismo; no iniciar esa descarga en cuatro sesiones.

## 3. Cuatro agentes/sesiones independientes, sin colisiones de trabajo

**Misma base inmutable inicial:** \`1d01db54b63250ed0aacb0704a5ac16290324e32\` (último S2 publicado al preparar); cada sesión verifica que no haya una base distinta acordada para todos antes de empezar. No basarse en \`main\` (todavía principalmente docs) ni en HEAD mutable sin fijar SHA.

| Encargo | Rama | Únicos propietarios | Entrega |
|---|---|---|---|
| **S2.5** Realismo | \`prototype/ib-s25-realism-lab\` | \`Content/IB_S25/**\`, \`Tools/IBS25_*\`, \`docs/IB_S25_*\` | Distrito realista pequeño fotográfico, captura/benchmark de comparación |
| **S3** Planeta, 4 ciudades, mapas | \`prototype/ib-s3-planet-lab\` | \`Source/ImpactBeyond/IB_S3/**\`, \`Content/IB_S3/**\`, \`Tools/IBS3_*\`, \`docs/IB_S3_*\` | Laboratorio esférico y cuatro zonas visitables, mapa global/minimapa, streaming/persistencia |
| **S3.5** Interacciones y puñetazos | \`prototype/ib-s35-interactions-lab\` | \`Source/ImpactBeyond/IB_S35/**\`, \`Content/IB_S35/**\`, \`Tools/IBS35_*\`, \`docs/IB_S35_*\` | Agarre genérico de cinco clases, 2 golpes, variantes de coches, API para S4 |
| **S4** Poderes, destrucción y policía | \`prototype/ib-s4-response-lab\` | \`Source/ImpactBeyond/IB_S4/**\`, \`Content/IB_S4/**\`, \`Tools/IBS4_*\`, \`docs/IB_S4_*\` | Policía humana, tanque con misiles de empuje, robot, láser/telequinesis, pared/suelo destruible con tests |

**Archivo compartido prohibido para cuatro sesiones:** \`ImpactBeyondCharacter.cpp/.h\`, \`ImpactCircuitComponent.cpp/.h\`, \`ImpactBeyondGameMode\`, \`Content/IB_C00/Lvl_IB_C00.umap\`, \`DefaultEngine.ini\`, \`ImpactBeyond.uproject\`, \`ImpactBeyond.Build.cs\`, \`docs/PLAN_ACTIVO.md\`, \`docs/TAREAS_ACTIVAS.md\`, \`AGENTS.md\`, la carpeta raíz de \`.codex\`, incluso si se usan worktrees. Si un cambio allí es imprescindible, dejar parche textual **sin aplicar** en la carpeta propia y pedir integración posterior por un único integrador.

**Un editor Unreal / una compilación/cook pesada a la vez por laptop**. Son cuatro sesiones Codex, NO 4×5 agentes. Cada chat trabaja como **UN agente** y no crea subagentes adicionales mientras estén las cuatro sesiones abiertas. Las cuatro pueden investigar/codificar en ramas y worktrees aislados; antes de abrir editor, comprobar si hay otra ejecución en curso, pedir/verificar turno de editor o dejar los tests editor como pendientes. No afirmar PASS por haber preparado scripts. No se ofrece mecanismo mágico de lock entre chats; si no hay coordinación segura, no intentar cuatro editores a la vez. Worktrees separados no duplican seguridad del editor ni de recursos externos.

**No crear cuatro proyectos completos**: usar worktrees separados, preferir script/datos/modulos pequeños, mantener caches compatibles con consentimiento, revisar espacio libre (último ~115GB histórico); reservar margen >40GB. Ni una sola sesión puede borrar/cambiar assets/cache personales o migrar versiones del motor.

## 4. Contratos compartidos antes de integrar (no deben duplicar sistemas en simultáneo)

- **Identidad:** BodyId / RegionId / CityId / ObjectId persistentes y semillas, interfaz de ubicación global doble, marco local tangencial. S3 dueño de los contratos espaciales; otros solo usan identificadores provisionales en sus respectivos laboratorios.
- **Interacciones:** categorías \`Car\`, \`Container\`, \`Pole\`, \`Rock\`, \`LoosePart\`; cada tipo tiene masa, collider, puntos de agarre, límite de transporte, perfil de golpe/daño y reinicio. S3.5 dueño futuro del contrato físico común. S4 implementa **adaptadores aislados** y no declara una segunda API global incompatible como verdad del juego.
- **Daño:** golpe/policía/bala/misil/láser/impacto describen \`DamageType\`, \`HealthDelta\`, \`Impulse\`, \`InstigatorId\`, \`TargetId\`, cooldown/hit dedup y \`DamageResponseProfile\`. La respuesta se parametriza por tipo de blanco/arma.
- **H01 contra policía/tanque:** balas humanas \`HealthDelta=0\` y respuesta cosmética (sin muerte); misil tanque \`HealthDelta=0\`, **empuje/knockback limitado**, recuperación y resistencia a stunlock, nunca mata al protagonista. Otros enemigos/poderes pueden usar reglas futuras diferentes; no convertir H01 en inmune a todo sin decisión.
- **Eventos:** policía patrulla/advierten y escalada por situación/reputación/causa, no disparar automáticamente al ver al jugador. Tanque aparece solo en prueba preparada o cuando se active alerta. Nada de violencia contra civiles como requisito de test. A futuro mundo permite salvar, destruir e ignorar.
- **Presentación:** gameplay determina daño físico, VFX/animación/audio se sincronizan, no deciden si el golpe acertó; screenshot/clip de cámara y materiales para aceptación visual.

**Integración posterior (NO a cargo de estas cuatro sesiones por la noche):** único integrador revisa cuatro PR, compara con snapshot, decide contrato común, cherry-pick selectivo o adapta cambios, resuelve conflictos, recompila y prueba otra build, y deja cuatro laboratorios separados si falla. No hacer merge automático a main.

## 5. Gates de evidencia por encargo

Cada uno debe entregar commit SHA + PR draft contra \`prototype/ib-sprint02\` + script de reproducción + assets/licencias + estado exacto \`DOCUMENTADO | CÓDIGO | COMPILADO | HEADLESS | DX12 | HUMANO\`. Ramas sin pruebas no se declaran terminadas. Un bloqueador puede producir resultado parcial honesto, pero nunca presentación de demo low-poly como "realismo final".

**S2.5:** 5 vistas comparables City Sample Small City vs distrito nuevo; materiales/escala/densidad/naturaleza/cars, sin cubos/cono evidentes en núcleo; DX12 1080p/60fps objetivo con luz/sonido y DR con render visible, p95/p99/VRAM; revertible.
**S3:** cambiar entre 4 hubs físicos y vuelta planetaria en prototipo, mapa/minimapa sin salto de longitud/polos, 10 retornos con persistencia, test distinto de integración artística S2.5.
**S3.5:** 30 agarres por categoría, 30 golpes, anim/contacto, sweep del objeto transportado y reset, no duplicación/portador; pruebas de carro existente intactas cuando integrador enlace.
**S4:** 20 contactos de bala policía H01 sin daño, 20 explosiones tanque con retroceso/recuperación sin muerte, 10 bloqueos/reaccciones IA, 20 impactos de láser, 20 telequinesis y 20 cráteres colisionables/reset; 60fps objetivo medido cuando editor esté libre.

**Fuentes de verdad:** PRs #18/#19; \`docs/IB_MVP_URBANO.md\`, \`docs/SPRINT02_FEEL_WORLD.md\` del PR #19, \`docs/IB_SPRINT02_PLANET_LAB.md\`; [roadmap](ROADMAP_SPRINTS_02_A_08.md), [sprints 3–5](SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md), [S9 ocho ciudades](SPRINT09_OCHO_CIUDADES_Y_BIOMAS.md). No confundir el antiguo plan de \`main\` W00 con el estado nuevo real.

**Esta entrega es DOCUMENTAL. Nada de esto demuestra que cuatro agentes ya trabajaron, los assets están instalados o los sprints estén terminados.**
