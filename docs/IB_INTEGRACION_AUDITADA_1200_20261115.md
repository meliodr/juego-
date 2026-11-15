# IMPACT: BEYOND — corte técnico de integración, 15-nov-2026 12:00 AST

> **ESTADO: BLOQUEADO PARA FUSIONAR GAMEPLAY S2.5–S7; GATE DE QA INTEGRADO EN RAMA REVERSIBLE.** Esta rama parte del SHA jugable exacto de Sprint 2 y añade **solo pruebas Git/QA + este informe**, sin tocar Character, .umap, .uasset, .uproject, Config ni `main`. No se inició ningún chat Codex, descarga Fab, cook o prueba Windows/Unreal desde esta ejecución. Un plan documentado no equivale a una rama implementada.

## 1. Inventario verificado ahora

- Repositorio [meliodr/juego-](https://github.com/meliodr/juego-), `main` **`aaad4491ca3610ba22d12f764d1ed0e35ca0356f`**; es la rama de preparación/documentos, **no** la cabeza jugable más avanzada.
- Baseline con código C++, archivos `.uasset`, mapa y scripts de mundo: [PR #19](https://github.com/meliodr/juego-/pull/19) **`prototype/ib-sprint02` / `1d01db54b63250ed0aacb0704a5ac16290324e32`**, 40 archivos cambiados respecto del [PR #18](https://github.com/meliodr/juego-/pull/18) `20e6a0081e33ab1abb7c8e7b67b55a4bc44a5443`.
- [PR #17](https://github.com/meliodr/juego-/pull/17), **`bb97a37df78cf29324e8bdfe7b6e17c50cc2cffe`**: roadmap y cuatro prompts. No implementa S2.5/S3/S3.5/S4.
- [PR #20](https://github.com/meliodr/juego-/pull/20), **`11cea83ae955a5b5cb053b96b7b8709010d3bf7d`**: ficha documental Poly Haven Shrub 03 CC0; no mesh importado.
- [PR #21](https://github.com/meliodr/juego-/pull/21), **`c8708f7d7d0cafb4fdff273ce76e028fac44bd1f`**: auditoría de 08:45, herramienta de gate Git y 3 tests; **no UE**.
- [PR #22](https://github.com/meliodr/juego-/pull/22), **`2c784e3d0a64a05a2d8975904b4eab47fff96f88`**: 6 documentos de refinamiento visual/física/planeta/alerta, no implementación.
- [PR #23](https://github.com/meliodr/juego-/pull/23): **ocho prompts documentales** S5–S7 y README. Su rama original era `5d37f55f4ab10d671adfacf0474dda6f0beac86d`; durante esta auditoría se corrigió un nombre incorrecto de rama S4 en README: `prototype/ib-s4-powers-lab` → **`prototype/ib-s4-response-lab`** (el nombre usado por PR17, PR21 y PR22). No hay evidencia de ocho chats reales ni de ocho builds.
- La API enumeró 23 ramas **antes** de crear esta rama de integración, 21 PRs y **0 GitHub Actions workflow runs**. En los cuatro SHA S2/PR21/PR22/PR23 se obtuvieron **0 status checks y 0 check runs**. Consultado en este corte. La ausencia de CI NO es PASS.
- No hay en el remoto a este corte las cuatro ramas de **implementación** esperadas: `prototype/ib-s25-realism-lab`, `prototype/ib-s3-planet-lab`, `prototype/ib-s35-interactions-lab`, `prototype/ib-s4-response-lab`. Podría existir trabajo local sin push; esta auditoría no puede verlo.
- `ImpactBeyond/ImpactBeyond.uproject` indica `EngineAssociation: "5.8"`. No existe evidencia de que el ejecutable local corra específicamente **UE 5.8.4**; comprobar el patch instalado en Windows.

## 2. Qué puede probarse ahora

**Código presente en S2 (no solo planes):** distrito urbano y extensión de carretera/costa en `IB_C00`; vuelo con cinco velocidades y modo travesía/turbo/freno, cámara con `USpringArmComponent`, animaciones de vuelo, auto recogible, lanzamiento y efecto de aterrizaje, viento procedural, HUD e integración preparada para mando. [Documento S2](https://github.com/meliodr/juego-/blob/prototype/ib-sprint02/docs/SPRINT02_FEEL_WORLD.md).

**Verificación reportada en el PR19, no reproducida aquí:** Win64 DX12 1080p/100% interno, audio WASAPI activo pero ventana offscreen e inputs simulados: media 143,94 FPS, p95 8,53 ms, p99 9,59 ms, VRAM pico 2.514 MiB; **dos cuadros >50 ms** cerca de recarga. Capsule sweep reportado a 320/368 m/s, **no sweep de carga volumétrica a esa velocidad**. Se usaron teletransportes para preparar coche/escena de impacto. No permite afirmar «1080p 60FPS visible, Steam Input y audio real aceptados»; faltan 10–15 minutos sin interrupciones con mando físico y entorno visual final.

**Quedan NO jugables como fases aceptadas:** ciudad realista S2.5, cuatro ciudades/planeta esférico S3, cinco clases de objetos y golpes S3.5, policía/tanque/robot/laser/telequinesis S4, ciclo/ciudadanos S5, héroes H02/H03 S6 y amenazas S7. Los prompts únicamente describen objetivos.

## 3. Matriz de compatibilidad y dependencia (sin inventar PASS)

| Orden | Capa | Entrada real | Bloqueo para integrar en proyecto | Prueba/gate para admitir |
| --- | --- | --- | --- | --- |
| 0 | **S2 baseline** | PR19 `1d01db54`, Unreal C++/Content | DX12 visible + input real no verificados; arte costero geométrico | Conservar baseline intacto, abrir mapa y medir 10–15min |
| 1 | **S2.5 realismo** | Facetas PBR PR16 reutilizables + planes PR17/22 | No branch/código del lab; costa `Cube/Plane/Sphere/Cone` y autos low-poly | 5 vistas A/B y colisión, VRAM, licence manifest; nunca pisar `IB_C00` directamente |
| 2 | **S3 planeta** | Matemáticas de laboratorio radial + contrato de regiones en S2 | No streaming/cambio de regiones con persistencia ni branch | 2 regiones y 10 retornos con guardado/reinicio/polos, luego 4 ciudades |
| 3 | **S3.5 interacción** | `ImpactCircuitComponent` S2 | La cápsula no representa volumen del vehículo; no hay 5 categorías verificadas | Sweep del volumen acarreado a 320/368m/s, 30 ciclos por categoría, IK/contacto/dedup |
| 4 | **S4 poderes y respuesta** | Contrato documentado PR22 | Depende de S3.5 y perfil daños; no policía/tanque compilados | 20 balas y 20 misiles a H01 con `HealthDelta=0`, knockback y recuperación, cero stunlock; probar daño normal a otros objetivos |
| 5 | **S5 sol y civiles** | 2 prompts PR23 | Necesita regiones S3 y eventos/alertas S4 para no duplicar estado | Un ciclo global 36min propuesto + civil/evento sin perder FPS |
| 6 | **S6 H02/H03 + selector** | 3 prompts PR23 | API de movimiento, poderes y daño S3.5/S4 no confirmada; arte reutilizable legal | Habilidades en mapa aislado + cambio de personaje sin romper H01 |
| 7 | **S7 robot, kaiju, radial** | 3 prompts PR23 | IA/destrucción/daño/selector no disponibles; una rueda documentada no es HUD funcional | IA de 1 robot + 1 kaiju, ataques/colisión, rueda con input y QA DX12 |

**Propiedad por laboratorios:** S2.5 solo `IB_S25`, S3 solo `IB_S3`, S3.5 solo `IB_S35`, S4 solo `IB_S4`. El dueño exclusivo del pegamento `Character/GameMode/Build.cs/IB_C00` es el **integrador**, después de revisar PRs. Las ramas documentales #17/#21/#22/#23 **divergen** de S2 porque provienen de `main`: prohibido hacer `merge` masivo de ellas para simular integración.

## 4. QA implementado realmente en esta rama

Se incorporaron sin modificación de contenido desde [PR21](https://github.com/meliodr/juego-/pull/21):
- `Tools/IBAudit_ValidateLabs.py` blob SHA **`112cfd75aae11e6e809cdfba1ae8a21346a8dd65`**.
- `Tools/tests/test_IBAudit_ValidateLabs.py` blob SHA **`e20dcf60a6aae5f4b0d61d4fff21c06eac9db11b`**.

Se añadió `Tools/tests/test_IBAudit_IntegrationGates.py` para verificar propiedad de los cuatro laboratorios, negativa al editar Character/mapa común, error por base ausente y error de `--require-all` cuando faltan ramas.

**Prueba ejecutada en entorno Python/Git aislado (no en Windows):** `python -m unittest discover -s Tools/tests -p 'test_IBAudit_*.py' -v`: **8/8 tests PASS** en repositorios Git temporales, usando bytes de script/test cuyos SHA Git se compararon con los blobs de GitHub. Es una prueba de **lógica de gate**, no compilación del juego. La ejecución directa contra el clon real, C++ UHT, DX12, mando físico, audio y FPS se marca **NO EJECUTADO**.

## 5. Riesgos prioritarios antes de integrar gameplay

1. **Estética:** `Tools/Author-Sprint02World.py` coloca `Cube` como carretera/suelo, `Plane` como agua, `Sphere` como montañas y `Cone/Cylinder` como árboles; el modelo de coches proviene del kit low-poly. No igualar FPS de geometría simple con calidad City Sample. Establecer bloque A/B con fachadas PBR, asfalto y props realistas; evitar descargar City Sample completo.
2. **Colisión, cámara y carga:** `ImpactBeyondCharacter.cpp` usa `SpringArm` y activa lag por perfil de velocidad; verificar clipping, framing y FOV en cambios 90/180/320/368 m/s y vuelo rasante. `ImpactCircuitComponent.cpp` mueve el coche sostenido, pero el sweep de la cápsula reportado no valida el volumen de la carga. Priorizar barrido continuo de objeto + fallback de desatasco antes de explosiones/destrucción.
3. **Respuesta no letal + rendimiento:** S4 debe garantizar `HealthDelta=0` para bullets/missiles contra H01 por perfil, impulso del tanque con recuperación, sin stunlock ni policía que abra fuego al spawn. Hacer prueba DX12 1080p visible con audio y mando físico; meta p95 de frametime <=16,67ms, documentar p99 y >50ms hitches, VRAM real RTX 4070 laptop 8GB. Una media alta offscreen no basta.

**Fab/licencias:** la ficha oficial de [City Sample Buildings](https://www.fab.com/listings/008fe959-5511-428e-93bd-f99b1179f6d5) indica Free, UE-Only y **Allows usage with AI: No**; [City Sample Vehicles](https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f) indica Free, UE-Only y **Allows usage with AI: Yes** al abrirla en la revisión de hoy. Esta última precisión corrige el plan inicial y concuerda con PR22. La ficha de Buildings también lista 2000+ módulos y Vehicles 13 vehículos: **paquetes independientes NO garantizan descarga por pieza**. En ningún caso se confirma propiedad, importación, espacio libre ni derecho a redistribuir sus `.uasset` originales en un repositorio público. Revisar procedencia/derechos de los assets Epic ya presentes en PR18/19 antes de publicar un clon completo; NO agregar nuevos archivos Fab en fuente público.

## 6. Manual exacto de integración cuando haya entregas

**Nunca tocar `main` ni ocultar la ausencia de labs.** La rama de esta auditoría `integration/ib-s2-gates-audit-20261115` arranca **exactamente** en S2, por lo que es un punto de control reversible. Para una futura entrega real:

~~~bash
git fetch origin
git status --short
git switch integration/ib-s2-gates-audit-20261115
python -m unittest discover -s Tools/tests -p 'test_IBAudit_*.py' -v

# Repetir en este ORDEN, una rama por vez.
python Tools/IBAudit_ValidateLabs.py --repo . --base 1d01db54b63250ed0aacb0704a5ac16290324e32 --lab S25 --json --require-all
git diff --name-status 1d01db54b63250ed0aacb0704a5ac16290324e32...origin/prototype/ib-s25-realism-lab
# Solo si el gate reporta STATIC_PASS, el diff es propiedad exclusiva y la licencia es clara:
git merge --no-ff --no-commit origin/prototype/ib-s25-realism-lab
git diff --cached --check
# revisar archivos binarios, archivos permitidos y pruebas; si falla: git merge --abort
# tras aprobación del diff, git commit -m "integrate: S2.5 validated lab"
~~~

Para **S3**, **S35**, **S4** repetir el gate con `--lab S3`, `--lab S35` y `--lab S4`, examinar PR y merge separadamente; ejecutar tests y build luego de cada paso. **No ejecutar los comandos merge automáticamente** si no existe SHA/branch, hay cambios sin commitear, ownership inválido, dependencia incompatible o no hay permisos. Para un hotfix de código compartido, crear commit de adaptador exclusivo en rama integradora bajo revisión; jamás incorporar cambios forzados de dos agentes sobre Character.

**Prueba humana con Windows/UE:** identificar versión exacta de Unreal y SSD libre, abrir `ImpactBeyond/ImpactBeyond.uproject`, compilar Win64/DX12 sin errores, iniciar mapa `/Game/IB_C00/Lvl_IB_C00` en modo visible y audio ON; probar teclado y mando real (con Steam Input cuando corresponda) 10–15min continuos; rutas urbanas/coast ida-vuelta, 5 velocidades + turbo/freno, agarre de vehículo y vuelo rasante sin clipping, cinco capturas A/B, test de impactos y guardado tras integrar regiones. Registrar logs UE, frametime p50/p95/p99, VRAM, audio y fallos. No marcar validación por pruebas fuera de pantalla.

## 7. Veredicto

**No se puede integrar todavía ninguna de las fases S2.5–S7 como gameplay** porque faltan sus ramas/PRs verificables. Sí quedó creada rama de QA **reversible**, con herramienta Git probada y matriz lista para auditar entregas cuando aparezcan. La ausencia de canal de envío Codex y de acceso al Unreal Windows sigue siendo bloqueo operativo. Mantener PR en **draft** apuntando a `prototype/ib-sprint02`. No fusionar a `main`.
