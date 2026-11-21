# IMPACT: BEYOND — Plan activo

## Recursos Fab — seguimiento del 21/11/2026

La [selección vigente](FAB_SELECCION_ACTIVA.md), el [catálogo de 91 fichas](FAB_CATALOGO_MAESTRO.csv) y la [auditoría lote 04](FAB_AUDITORIA_LOTE04_2026-11-21.md) son **investigación externa**, NO cambios integrados en S7. Distribución: 12 seleccionados, 14 por probar, 43 reservas, 22 descartados. Reclamación e instalación siguen NO_VERIFICADAS; pruebas UE5.8 NO_EJECUTADAS. La build S1–S7 validada al 16/11 no se modifica. Prioridad de mejora visual: una sola manzana/malecón con DZ Sample Trees, City Props Sample, una fachada modular y Water System nativo. Los nuevos candidatos de animación, VFX, vehículos y audio son comparaciones opcionales en mapas aislados. No instalar paquetes masivos, no introducir plugins sin verificar coste, licencia y dependencia, no degradar arte ni VRAM de la build estable. Las promociones oficiales actuales expiran 1/12: Absorption FX y Post-Apocalyptic Factory para pruebas acotadas; Advanced Companion Drone sigue descartado por alcance. Mantener uso privado, offline y compras RD$0.


Actualización: 16 de noviembre de 2026. **Versión jugable S1–S7 integrada y validada mediante build, autoría, paquete, pruebas funcionales y perfil DX12/audio.** [Evidencia final](IB_S7_CLOSURE_REVIEW.md). Publicación/reconciliación en cierre; arranque visible desde Steam verificado con el ejecutable final y Steam Input activado. Evaluación humana del control y mando físico separada; no se afirma aceptación de todos los ensayos sostenidos del plan.

Este documento consolida las decisiones de [PR17](https://github.com/meliodr/juego-/pull/17), los problemas de [PR22](https://github.com/meliodr/juego-/pull/22) y el alcance de [PR23](https://github.com/meliodr/juego-/pull/23). Las instrucciones históricas de laboratorios sobre S2 no ordenan rehacerlos ni bloquean la integración autorizada por el usuario. La cola está en [TAREAS_ACTIVAS.md](TAREAS_ACTIVAS.md).

## Dirección vigente

- Sandbox de superhumanos para un jugador offline: salvar, destruir, explorar o ignorar incidentes. Sin campaña obligatoria, vuelo bloqueado por reputación ni game over impuesto por ignorar avisos.
- Windows/DX12 y proyecto/toolchain fijados. Linux y cooperativo quedan fuera del alcance actual.
- Reutilizar primero componentes, rigs, clips, materiales y recursos legales existentes. Crear solamente la adaptación mínima necesaria. Nuevas compras: RD$0.
- No entrenar IA, proporcionar bytes NoAI a herramientas generativas ni publicar paquetes Fab o derivados privados. Una ruta suave o recibo no demuestra posesión ni prueba del recurso.
- Un integrador, propiedad por archivo y un único responsable de Unreal/UBT/cook/paquete. Conservar la build anterior verificable. Separar input simulado de DualSense físico con Steam Input.
- H01 fuerza, H02 genio cósmico masculino esmeralda y H03 teletransportador masculino índigo/negro, con identidad original. No recuperar VECTOR/PRISMA ni copiar personajes, símbolos o trajes ajenos. Nombres artísticos futuros pendientes.

## Alcance de la candidata hasta S7

| Fase | Resultado observable exigido | Criterio de cierre |
|---|---|---|
| S1–S2, regresión | Caminar, correr, saltar, volar, niveles, frenar, cargar/lanzar objetos, aterrizaje de impacto y reset | Ruta continua; colisión del héroe y de la carga; preservar controles y estado al cancelar/reiniciar |
| S2.5, presentación | Fachadas reales, calles/aceras PBR, vegetación y aterrizajes legibles | Revisión a cámara de juego; proxies invisibles permitidos, primitivas no presentadas como arte final; procedencia y reconstrucción |
| S3, planeta | Cuatro núcleos visitables con colisión, siluetas diferenciadas y una interacción local; mapa, minimapa y destino útil | Viajes entre los cuatro núcleos, tres circunnavegaciones y regreso con persistencia; polos/fronteras sin saltos ni duplicados |
| S3.5, interacción | Perfiles de coche, contenedor, poste, roca y pieza; golpes en suelo/aire con timing | Barrer volumen cargado, un propietario por objeto, restaurar física al soltar/destruir/reset, deduplicar impactos |
| S4, combate | Combo, uppercut, golpe pesado/aéreo; láser/telequinesis de ensayo; destrucción preparada, robot pequeño/dron y respuesta policial | Huecos/cráteres con colisión; desarme elimina capacidades; recuperación acotada; vuelo rasante no fuerza caminar |
| S5, mundo reactivo | Reloj global, iluminación, 3–6 civiles, huida/refugio y un incidente opcional | Día 36 min configurable 18/72; persistencia y hora local coherente; alerta, reputación y amenaza separadas; un evento y cooldown |
| S6, tres héroes | H01/H02/H03 en una partida con mundo, cámara, carga y daño compartidos | H02 escudo/mano/mazo; H03 blink seguro, contra-blink y ataque después de llegar; selector sin acciones filtradas |
| S7, amenazas y rueda | Un robot pesado, un kaiju mediano preparado y rueda con mando/ratón | Robot con dos piezas funcionales; kaiju con tres partes y dos ataques anunciados; rueda equipa sin activar y conserva poder anterior |

Son criterios y límites de ensayo, no afirmaciones de pruebas aprobadas. La rueda S7 era una propuesta de producción en PR17 y forma parte de la integración actual solicitada hasta S7. El menú completo permanece en S8. No se exige arte definitivo para probar mecánicas, pero un placeholder debe identificarse y no presentarse como acabado aceptado.

## Contratos y correcciones que bloquean el cierre

1. Un mensaje canónico de impacto y deduplicación por AttackId/TargetId. La presentación no decide daño, movimiento, reputación ni alerta. PartId estable y un desprendimiento por pieza.
2. Balas policiales y misiles de tanque producen cero daño de salud a H01; el misil puede empujarlo con recuperación acotada. La inmunidad no se propaga a otros blancos. Sin alerta, la policía no dispara.
3. Cápsula y carga respetan paredes de 4 cm, esquinas, techos y suelo. Probar regímenes aplicables de 12/90/180/320/368 m/s y registrar velocidad realmente alcanzada y dt. Matemáticas aisladas no certifican colisión de Unreal.
4. Gravedad, trazas, impulsos, orientación y velocidad usan el marco local. Región no lista impide transiciones/blinks inseguros. H03 valida ruta, cápsula final, carga y caída; rango de ensayo 25–40 m configurable.
5. Un reloj planetario: cambiar de ciudad no lo reinicia. Guardado versionado compatible con datos anteriores. Atribución distingue jugador, enemigos y entorno; rescates fabricados no generan recompensa ilimitada.
6. Destrucción por piezas/estados preparados con presupuesto de cuerpos. No afirmar fractura arbitraria de todo el planeta. Las amenazas pueden caer sin resolver cada pieza como rompecabezas.
7. Selector, mapa y rueda priorizan su input. Confirmar, cancelar, perder foco y refrescar cooldowns no disparan, saltan, lanzan carga ni borran el poder anterior. Restaurar tiempo y estado al cerrar.
8. H02 reutiliza agarre y daño de H01; H03 no implementa una segunda autoridad de daño. El mismo objeto transportado y el mismo muro deben funcionar con los tres héroes.

## Validación y evidencia

Registrar por separado: política estática, C++ nativo, UHT/build Editor y Game, automatización Unreal, cook/paquete, arranque DX12, imagen/audio, input simulado, mando físico y aceptación humana. Cada resultado identifica commit, mapa, comando, configuración, resultado y límite. No trasladar PASS de otra rama/build.

Alcance heredado de QA: S3 tres vueltas y viajes a cuatro núcleos; S3.5 ciclos por categoría y golpes sin contactos dobles; S4 50 roturas/20 resets más inmunidad, desarme y vuelo rasante; S5 tres ciclos acelerados y un ciclo completo de 36 minutos con DX12/audio; S6 30 acciones por tipo H02 y 50 destinos válidos/inválidos H03; S7 desarme, recogida de piezas, reinicios, persistencia y selección/cancelación repetida de rueda.

La sesión final debe permitir 10–15 minutos alternando movimiento, carga, combate, mundo y héroes sin crash, softlock, colisión fantasma ni pérdida de estado. Registrar p95/p99, tirones y VRAM con hardware/resolución/calidad. **1080p/60 FPS es objetivo, no resultado presumido.** Sensación, potencia y deseo de repetir requieren evaluación del usuario.

## Secuencia de entrega

1. Congelar candidata local y corregir los gates relevantes.
2. Exportar fuentes del commit aprobado a rama limpia desde origin/main, conservando documentación existente y excluyendo binarios nuevos, rutas personales e historial privado. [Contrato de publicación](IB_S7_PUBLICATION.md).
3. Ejecutar CI y validación local sobre la versión exacta. Documentar provisión privada/regeneración de assets: un clone público no garantiza arte ni paquete.
4. Integrar un PR revisable y cerrar labs cubiertos o sustituidos con lenguaje preciso, preservando ramas. No afirmar que fuentes ausentes fueron integradas.
5. Entregar build identificada, actualizar acceso Steam según las reglas del proyecto y comunicar controles/límites comprobados. No marcar S7 aceptado sin evidencia.

## Después de S7

S8 conserva el comienzo del espacio —ciudad, altura orbital y regreso con estado— y añade pausa completa, controles, ajustes y guardado coherente. S9 amplía a ocho ciudades y biomas tras medir escala/coste. S10 desarrolla nombres originales, hitos, lore ambiental y sociedad sin campaña lineal. S11–S14 quedan reservados sin alcance cerrado. S15 añade héroes aliados NPC ocasionales, no compañeros permanentes ni nuevos héroes seleccionables.

Los nombres propuestos y tamaños metropolitanos anteriores no son compromisos artísticos aprobados. Luna, otros planetas y Sol visual siguen en la visión espacial; no se anuncian entregados en S7. Catálogos Fab/plugins se consultan por necesidad concreta, sin convertirlos en importaciones obligatorias. El historial previo del plan permanece en Git y los informes de cada entrega.
