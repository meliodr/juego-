# IMPACT: BEYOND — Skills de IA y sistemas de superpoderes (investigación v1.3)

Fecha: 2026-11-15. **Estado: INVESTIGACIÓN; NO SE HA INSTALADO NINGUNA SKILL/PLUGIN NI IMPLEMENTADO NINGÚN PODER.** Plataforma activa Windows/UE 5.8; UE todavía estaba instalándose en la captura previa. Hardware reportado RAM 64GB, RTX4070 8GB VRAM; CPU/driver/VS/SDK pendientes de verificar. Un jugador offline. Cooperativo muy lejano. Presupuesto RD$0.

## A. Dos conceptos que NO se deben confundir

1. **Skills de Codex/ChatGPT:** carpetas SKILL.md con procedimientos para escribir código, preparar un proyecto, integrar dependencias, perfilar y comprobar; no crean funciones automáticamente ni equivalen a un plugin jugable.
2. **Gameplay Abilities:** ataques, teletransporte, invocaciones y efectos del juego; estos sí deben implementar C++/Blueprints, animaciones, físicas, sonidos, VFX y reglas.
Solo traer una skill externa si aporta indicaciones NUEVAS, verificadas y compatibles con nuestro Windows/UE 5.8. No instalar 70 skills y 50 agentes. No hacer que Codex trate el README de otro proyecto como autoridad sobre las reglas de IMPACT.

## B. Skills externas investigadas directamente

| Repo | Skill concreta | Evidencia / licencia | Decisión |
|---|---|---|---|
| https://github.com/maystudios/claude-skills | unreal-gas | SKILL.md inspeccionado: ASC, GameplayAbility, Effects, Attributes, Tags, Tasks, Cues; repo MIT (LICENSE inspeccionada) | **ESTUDIAR** cuando se compare GAS; no necesitamos habilidad de replicación ni red aún |
| https://github.com/maystudios/claude-skills | unreal-thirdparty | SKILL.md inspeccionado: Build.cs, bibliotecas y dependencias/ABI; MIT | **APROVECHAR IDEAS** para adaptar C++ MIT de ManOfSteel/PlotBuilder/REPIRK; no instalar dependencias por defecto |
| https://github.com/maystudios/claude-skills | unreal-blueprint-codegen | SKILL.md inspeccionado: generar .uasset vía módulo C++ de editor y APIs internas de grafos; dice **validado hasta UE 5.7**, UE5.8 no verificado | **RESERVA** si MCP/editor scripting no basta; alto riesgo de API |
| https://github.com/Skyrant/gamedev-agent-skills | unreal-cpp-gameplay | SKILL.md inspeccionado, orientado a UE5.8; repo licencia Apache-2.0 | **ÚTIL** para plantillas UE C++/Build.cs, sin necesidad de instalar router entero |
| https://github.com/Skyrant/gamedev-agent-skills | unreal-niagara | SKILL.md inspeccionado, UE5.8: Sistemas, emisores, parámetros User y FX | **ÚTIL** al crear efectos de H02/H03 e impactos |
| https://github.com/Skyrant/gamedev-agent-skills | game-feel | SKILL.md inspeccionado: capas de feedback, hitstop, easing y shakes; disciplina común | **ÚTIL** para epicidad y fluidez, NO congelar la física sin control |
| https://github.com/Skyrant/gamedev-agent-skills | performance-optimization, physics-tuning, unreal-packaging | catálogo inspeccionado, Apache-2.0 | **RESERVA** cuando exista build para medir |
| https://github.com/UnrealXu/codex-skills | ue-build-assistant | README inspeccionado: skill para builds UE y diagnóstico | **RESERVA**, evaluar licencia de directorio antes de copiar |
| https://github.com/merlinhu1/codex-game-studio | flujos/roles de game studio | README: gran colección de agentes/skills/workflows | **REFERENCIA**, no trasplantar toda la organización |
| Oficial OpenAI | [Guía actual de skills](https://learn.chatgpt.com/docs/build-skills) y [skill creator](https://github.com/openai/codex/blob/main/codex-rs/skills/src/assets/samples/skill-creator/SKILL.md) | El antiguo repositorio openai/skills aparece como deprecated en favor de plugins/guía actual | **FUENTE DE FORMATO**: mantener skills cortas y específicas, no duplicar documentos |

Las skills propias existentes en .agents/skills/ ya cubren Linux histórico, gameplay, licencias, animación, QA, ciudad y eventos. **No añadir ninguna skill nueva ahora**; aprovechar primero los procedimientos consultados como referencias. Si el primer poder complejo demuestra un vacío real, crear una única skill de abilities en la siguiente fase.

## C. Sistemas de poderes reutilizables

| Código/proyecto | Evidencia concreta inspeccionada | Riesgo / uso |
|---|---|---|
| Unreal GAS (motor) https://dev.epicgames.com/documentation/unreal-engine/gameplay-ability-system-for-unreal-engine | Documentación oficial UE5.8: AbilitySystemComponent, abilities, gameplay effects, tags, attributes, cues, tasks | **CANDIDATO PRINCIPAL PARA SISTEMA AMPLIO** solo después de probar primero dos o tres acciones; GAS no implementa superpoderes listos por sí solo |
| https://github.com/samuellapointe/GASam | README + .uproject + LICENSE.txt inspeccionados; MIT; ejemplo mínimo con Fireball Blueprint, Lightning Bolt C++, efectos, atributos y cues. **README dice 5.6.1; descriptor actual .uproject indica 5.7**: fijar commit y versión antes de ensayar | **PRIMER LABORATORIO GAS**; aislado, sin migrar todo el juego |
| https://github.com/Thyke/ProjectAscend | README + .uproject + LICENSE inspeccionados; MIT, UE5.5, Leap Slam y pipeline de abilities por DataAssets/fragments. .uproject incluye TargetingSystem, GameplayStateTree y ProjectCleaner de Marketplace | **RESERVA DE DISEÑO MODULAR**; dependencias que pueden encarecer port a 5.8 |
| https://github.com/tranek/GASDocumentation | Recurso MIT ampliamente usado para patrones de GAS y errores frecuentes | **REFERENCIA TÉCNICA**, complejidad de red irrelevante ahora |
| https://github.com/jessebr2/UE5-Cpp-GAS-Code-Samples | Muestras de abilities/attributes/dodge/projectile; README dice NO standalone compilable, sin Blueprints/content | **REFERENCIA DE CÓDIGO**, licencia individual a verificar antes de copiar |
| https://github.com/BoilTask/ActionRPG-UE5 | Actualización ActionRPG de Epic a UE5.7, combate/IA/GAS | **RESERVA COMPLEJA**, condiciones de Epic/licencia original, no base del proyecto |
| https://github.com/ProdiG66/ManOfSteel | MIT C++ vuelo/combate, pero mayoría de animaciones UAsset bajo punteros Git LFS incompletos | Fuente para H01 locomoción; no extraer personajes de franquicia |
| https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5 | MIT PhysicsGrabberComponent auditado en plan anterior | Fuente para agarre, lanzamiento e interacción; comprobar integración |
| Unreal nativo: Niagara, Chaos, Enhanced Input, Gameplay Tags y CMC | Sistemas del motor sin compra extra; no son superpoderes prefabricados | Preferirlos para acciones físicas y presentación |

## D. Repertorio mínimo de poderes y cómo ensamblarlo

La mecánica central de caminar/correr/volar debe estar en movimiento nativo; no puede depender de una GameplayAbility con cooldown ni combustible. Las **acciones** pueden ser abilities si su complejidad lo justifica.

**Primitivas reutilizables compartidas**:
- Targeting/Trace: seleccionar objetivo/parte válida sin apuntado omnisciente.
- Damage & Impact: AttackId, golpe confirmado, daño, knockback, material y deformación/hueco preparado.
- Grab/Carry/Throw: una entidad sujetada, sockets/IK, colisión propia de la carga, lanzamiento.
- ApplyImpulse/Recover: física limitada + animación, anti-softlock.
- SpawnConstruct: mesh prefabricada, duración/coste opcional, escalas de VFX Niagara.
- TeleportSafe: validar destino/cápsula/celda cargada y carga sujetada.
- Cooldown/Ability Tags: solo si se habilita modo desafío; Creativo permite efectos ilimitados.
- ModularDamage: PartId intacto/dañado/ausente y ataque deshabilitado por pieza, sirve robots/kaijus.

**H01 fuerza**: jab, golpe pesado, embestida, arrancar postes/armaduras, cargar/lanzar enemigos y carros; vuelo como movimiento común.
**H02 genio esmeralda**: mano/pinza que usa Grab/Carry, puño/martillo que usan Impact, escudo que intercepta, gestos breves/manifestaciones múltiples; sin anillo ni fruta.
**H03 teletransporte**: blink libre seguro, contra-blink y golpe al reaparecer; reposicionamiento respeta colisión, sin telefrag automático.
**Robots/kaijus**: ataques configurados desde el mismo Damage/Impact y partes; perder propulsor/ala/cola cambia capacidad, no solo apariencia.
**Eventos**: robo/invasión/meteorito reutilizan actores y poderes, no crean un subsistema de habilidades nuevo.

## E. ¿GAS o sistema pequeño? Decisión temporal

- Para **W00-W04 y E1/E2**: NO instalar GAS por defecto. H01 caminar/correr/volar en CMC, y golpes/agarre con componentes simples. Objetivo: un bucle divertido mínimo antes de un framework.
- Después del primer sandbox E3: probar en un proyecto **aislado** GASam/UE5.8 con un único ataque, un escudo y un cooldown, y comparar contra nuestras acciones simples: costes de portabilidad, latencia de input, flexibilidad para H02/H03, mantenimiento de blueprints y rendimiento.
- **Adoptar GAS** si la comparación demuestra ahorro real al componer habilidades combinables, efectos y estados. En caso contrario mantener interfaz de habilidades simple con DataAssets/Gameplay Tags. No mezclar dos fuentes de daño al mismo tiempo.
- No reutilizar toda la implementación de GASShooter/Lyra: la red está aplazada y las bases grandes comprometen velocidad de desarrollo.

## F. Validaciones de primera habilidad

PWR-01: 30 golpes H01, distintos ángulos, un impacto por AttackId/TargetId.
PWR-02: 20 agarres de carro/enemigo, sin atraviesos ni objetos duplicados.
PWR-03: H02 mano y escudo, mismo objeto/manipulador que H01; defensa cambia resultado real, no solo VFX.
PWR-04: H03 50 blinks válidos/inválidos, destino no aparece dentro de sólidos o zonas no cargadas.
PWR-05: robot pierde parte, ataque asociado se deshabilita y fragmento reutilizable.
PWR-06: comparar habilidades simples vs GAS en un prototipo aislado bajo UE5.8 Windows con CPU/GPU/VRAM medidos; ninguna habilidad puede bloquear de modo arbitrario caminar/correr/volar.
PWR-07: prueba humana breve: control, claridad, potencia y repetición, sin retrasar el input.
**Status real al escribir:** estos son tests definidos, NINGUNO ejecutado. Donantes no instalados.

## Fuentes y nota legal

- Guía de skills OpenAI https://learn.chatgpt.com/docs/build-skills
- Docs GAS oficial https://dev.epicgames.com/documentation/unreal-engine/gameplay-ability-system-for-unreal-engine
- Clases y Effects https://dev.epicgames.com/documentation/unreal-engine/using-gameplay-abilities-in-unreal-engine
- Licencias root MIT/Apache verificadas de maystudios, Skyrant, GASam, ProjectAscend; subassets de terceros se comprueban individualmente.
- GitHub público no autoriza copiar cualquier asset, ni el juego compartido privadamente con amigos elimina las condiciones de licencia.
