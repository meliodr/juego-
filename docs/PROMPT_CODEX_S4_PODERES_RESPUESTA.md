# PROMPT PARA PEGAR EN CODEX — SPRINT 4 · PODER, DESTRUCCIÓN, POLICÍA Y TANQUE

**Eres UN agente Codex implementador de combate/destrucción y respuestas del mundo, en LABORATORIO AISLADO.** Hay tres sesiones más: no crees subagentes ni toques personaje/planeta/mapa final. Las siguientes mecánicas deben ser jugables sobre un escenario independiente, construidas sobre física/Chaos, AnimBP, Enhanced Input y componentes del motor. El usuario desea un H01 casi invulnerable a armas humanas convencionales, pero susceptible al empuje de un misil: no inventar muerte por policías/tanques.

Repo https://github.com/meliodr/juego-, base fija común \`1d01db54b63250ed0aacb0704a5ac16290324e32\`, PR #19. Worktree independiente \`../impact-s4-response\`, rama \`prototype/ib-s4-response-lab\`. Lee \`AGENTS.md\`, \`docs/COMBAT_ROSTER_V03.md\`, \`docs/SPRINTS_03_04_05_CIUDADES_PODERES_TIEMPO.md\`, \`docs/PLAN_PARALELO_S25_S3_S35_S4_20261115.md\` en rama documental. Examina clases existentes de impacto/agarre, no duplicar un motor de daño global. Los contratos S3.5 se acordarán DESPUÉS: no depender de archivos de ese agente hasta integración.

**Solo escritura:** \`ImpactBeyond/Source/ImpactBeyond/IB_S4/**\`, \`ImpactBeyond/Content/IB_S4/**\`, \`Tools/IBS4_*\`, \`docs/IB_S4_*\`. No cambiar \`ImpactBeyondCharacter.*\`, \`ImpactCircuitComponent.*\`, \`ImpactLandingComponent.*\`, \`IB_C00/Lvl_IB_C00.umap\`, \`ImpactBeyond.Build.cs\`, \`ImpactBeyond.uproject\`, Config, ni carpetas de otros agentes. Crear ActorComponent/Blueprint/Pawn derivado propios para probar, sin pisar la implementación compartida.

## Escena mínima obligatoria

Una calle realista existente o mapa de prueba con fachada destructible, suelo preparado para cráter, dos policías humanos, un tanque, un dron/robot con una parte desmontable y varios objetos con física. El arte definitivo vendrá de S2.5; jamás afirmar fotorealismo si solo hay proxies. Modelos principales desde biblioteca autorizada/CC0, nunca modelar policías y tanque desde cero como primera opción.

**Policía humano:**
- Reutilizar modelos humanos de City Sample Crowds (Fab Free, rig Epic, **sin uniforme policial asegurado**) si archivo instalado y derechos reales, o humano existente permitido/Manny provisional. Elegir ropa/materiales sencillos, buscar clips de patrulla, advertencia, apuntado y retroceso; animaciones de arma solo mediante paquete legal/ya instalado. IA sencilla con patrulla→alerta/advertencia→apuntar/disparar→retirarse/cubrirse, activación por causa real/alerta de zona, **no disparar a H01 sin motivo**.
- Balas simuladas como ataques de juego con FX/SFX y trace o proyectil pequeño, pero **H01 sufre cero pérdida de salud y cero muerte**; impacto visual/sonoro discreto (destellos/ricochet o reacción de tela), sin interrumpir todos sus movimientos. Si hay sistema de HP futuro, su perfil humano-bala tiene \`HealthDelta=0\`. La policía puede recibir daño según las reglas del sandbox, pero NPC no es objetivo obligatorio.
**Tanque:**
- Una malla de tanque autorizada y gratuita si está disponible localmente; explorar modelos gratis con licencias verificables (p. ej. T-72 FBX de Nekis5_3D dice uso personal/comercial pero **no incluye texturas**; no tratarlo como asset fotoreal listo). No instalar framework de conducción ni misiles realistas. Turret simple con advertencia antes de disparar.
- Misil con VFX/Audio de Niagara/MetaSound o paquetes gratuitos legalmente adquiridos; físicas colisión e impulso radial limitado. Al impactar H01, **HealthDelta=0**, pero aplicar knockback perceptible y recuperable (parámetro inicial desplazamiento ~3–8m según vuelo/masa ficticia, no fuerza balística literal), cooldown anti-stunlock; no tirarlo infinitamente ni encajarlo bajo suelo. Tanque estático en primera escena.
**Robot:** un enemigo simple con una pieza funcional separable (cañón o propulsor): al perderla deshabilita su habilidad y pieza pasa a interactuable. Reutilizar Free Stylized Sci-Fi Scout Bot / Sci Fi Robot 13 solo si licencia/bytes/compatibilidad verificados; si no, laboratorio con malla legal existente y placeholder **explícito**.

## Poderes y mundo destruible

1. Dos prototipos H01: **láser/visión térmica** apuntado con cámara, daño por material/trace y beam Niagara; **telequinesis experimental creativa** que usa componente físico/Physics Handle para seleccionar, desplazar y lanzar objeto con colisión; se reservará identidad energética propia de H02 para S6, no crear dos sistemas de agarre global. No copiar nombres/efectos de superhéroes de franquicias.
2. **Destrucción por capas**: vidrio/puerta, muro con hueco realmente transitable y suelo con cráter **de profundidad/colisión real** en patch preparado (no solo decal); usar Chaos/Geometry Collections o estados de mallas con colisión. Pocos fragmentos físicos, otros cosméticos; no activar física de todo el planeta y no declarar cualquier triángulo destructible. Si evalúas PlotBuilder o Realtime Destructible Mesh, solo en sandbox con permisos/licencia exacta y beneficio medido vs Chaos nativo; no instalar plugins por defecto.
3. **Vuelo bajo**: preparar test de H01 0.5–3m sobre suelo, sin \`MOVE_Walking\` involuntario. El personaje principal se cambia después por integrador; crea componente/contrato de intención de aterrizaje y prueba con tu Pawn/derivado independiente. Mantener colisión/rastreo válido al pegar puñetazo/laser sobre el suelo.
4. **Cámara y efectos**: hitstop local opcional y breve, microshake de misil/puño de intensidad configurada, humo/polvo/escombros, sonido por material; no sacudidas constantes ni screen shake mareante. VFX gratuitos principal [Niagara Examples Pack Epic](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600), alternativas Free Magic Niagara y CC0, solo si los bytes están autorizados y realmente disponibles.

## Evidencias de aceptación S4

- 20 tiros policía al H01: cero daño, cero muerte, impacto cosmético adecuado y sin stunlock. Policía no ataca sin condición; registra origen alerta.
- 20 misiles de tanque a H01: cero daño/muerte, retroceso medido y recuperación, colisión segura en vuelo y cerca del suelo, varios impactos seguidos sin bloqueo permanente.
- 20 usos de láser contra metal, hormigón y cristal, un daño por tick/objetivo controlado; 20 usos de telequinesis sin atravesar pared.
- 20 fracturas de pared y 20 cráteres en parche preparado: volumen realmente transitable o hundido, colisión/visibilidad coherentes, reset/guardado sin fragmentos fantasmas.
- 20 recorridos de vuelo rasante manteniendo modo aéreo y collider, con golpes suelo y sin entrar en caminar por proximidad.
- Robot pierde parte y acción asociada; QA contra duplicado de ataques y partes.
- Build/QA DX12 1080p aspirando a 60fps, con audio y VFX; registrar fps p95/p99/VRAM/hitches sin fingir pruebas. No abrir Editor ni cook si otro Codex lo está usando; dejar código y tests pendientes si falta turno. **No crear una ciudad completa en S4, no implementar H02/H03 ni espacio.**

**Publicación:** un PR draft contra \`prototype/ib-sprint02\`, commit, mapa de laboratorio solo en \`Content/IB_S4\`, componentes aislados, test y tabla de resultados/licencias, sin merge automático ni instalar plugins/descargar Fab/reclamar licencias. El integrador mañana reconciliará los contratos S3.5/S4 y conectará los efectos sobre el H01 y la ciudad de S2.5, tras validar que el mundo sigue funcionando.
