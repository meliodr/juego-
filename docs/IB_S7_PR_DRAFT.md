# Integrar fuentes S1–S7 en una partida y conservar reconstrucción privada del arte

Los laboratorios de planeta, carga, combate, civiles, héroes y amenazas estaban separados y sus PR no ofrecían una entrada común de juego. Esta candidata reúne sus fuentes alrededor de Playable: un host de personaje/sesión, daño y persistencia compartidos, ciudades, reloj/civiles, tres héroes, robot/kaiju y rueda. El alcance sigue en validación; este PR no declara todavía una versión S7 jugable aceptada.

Se exporta C++ y herramientas `6b4631d`, con autoría desde `742ff49` y configuración/validador de paquete desde `17d7f91` desde una rama limpia basada en `6c2d903`. No se incorpora el historial local de assets ni Content binario nuevo. Se conservan recetas S2.5, recibos de procedencia, los cambios públicos de Robert y las decisiones de PR17/22/23 en el plan vigente. La partida requiere provisión local autorizada y autoría de `/Game/IB_Playable`; un clone de fuentes no incluye ese arte ni el mapa generado.

La CI añade política de publicación sobre todos los commits nuevos, sintaxis/recetas y 13 suites nativas. La comparación S35/S4 exige rechazar el replay con timestamp renovado, manteniendo pruebas de unidades, signo, categorías e inmunidad.

Validación observada:

- CI remota de `43b52a5`: [Integrated source checks](https://github.com/meliodr/juego-/actions/runs/43811267932) y [Lab audit tests](https://github.com/meliodr/juego-/actions/runs/43811267857) aprobados, incluidas las 13 suites nativas. Las evidencias documentales de paquete permanecen locales hasta completar su validación.

- Exportación: **17 tests de auditoría/publicación PASS**, 10 tests de receta S2.5 y 5 de refinamiento PASS; 1 test de bytes locales omitido explícitamente.
- Nativos sobre la rama pública exportada: **13 suites requeridas PASS** con MSVC C++17, incluidas 282 assertions de comparación S35/S4. Hashes estables sobre `9ca77e1`; las políticas y fixtures nativas no cambian hasta `6b4631d` (la nueva regresión de ticks es una prueba Unreal). No se ejecutó Unreal desde este frente. Geometría S3 PASS.
- Unreal: `4f95872` compiló y **`9ca77e1` compiló Editor + Game**, según el integrador: run `s7-build-20261116T045837Z`, 224,52 s y 210,82 s. El Editor de `acf4156` también pasó según el integrador. Editor y Game de `6b4631d` pasaron en `s7-build-20261116T052603Z` (Editor: 181,87 s).
- Automatización reportada por el integrador: 12 funcionales PASS en `acf4156`; tres contratos S4 PASS en ThirdPerson con override S4; PlayableSession sin render e input simulado DX12 PASS en `761a2b5`. [Runs y alcance](IB_S7_CLOSURE_REVIEW.md). El input pasó con Fortnite activo: no sirve como medición de rendimiento ni prueba de DualSense físico.
- Autoría final `b341476`: **PASS, exit 0 y cero dependencias ausentes**, run `s7-author-20261116T053232Z`. La limpieza de wrappers resolvió el exit 3 anterior. El script exacto `742ff49` aplica flags de instancing a siete materiales S25 locales; sus `.uasset` no se publican. La regresión de producción `s7-tests-20261116T053308Z` aprobó PlayableSession, S3 Contacts y S7 PreparedEncounter; cero warnings de ciclos. FlightReentry falló inicialmente por fixtures del laboratorio ausentes en producción; su repetición en S3Lab pasó diez regresos físicos (`s7-tests-20261116T053404Z`). El batch inicial conserva su FAIL; los resultados posteriores identifican su mapa.
- Primer paquete `a91b163`, run `s7-package-20261116T053439Z`: cook PASS; **candidato retirado** tras fallo DX12 por cinco recursos S3 ausentes y viewport 888×500 por DPI. `17d7f91` corrige AlwaysCook/High DPI/ForceRes y añade rechazo explícito de contenido ausente/materiales de reserva. Recook en curso, C++ idéntico a `6b4631d`. Pendientes: nuevo ejecutable exacto, recorrido DX12/audio, rendimiento y aceptación. Input simulado y DualSense físico se registran por separado.

[Archivos y hashes exportados](IB_S7_PUBLIC_EXPORT.json) · [Reproducción y dependencias](IB_S7_SOURCE_OVERVIEW.md) · [Alcance y gates](PLAN_ACTIVO.md).

Tras el merge y validación correspondientes, #26/#27/#28 podrán cerrarse como cubiertos por la integración, y #25 como laboratorio sustituido preservando su rama y las fuentes recuperadas. PR21 ya fue cerrado por el integrador como cubierto por PR30. Esta propuesta no cierra otros PR ni afirma integración de investigación/documentos todavía ausentes.
