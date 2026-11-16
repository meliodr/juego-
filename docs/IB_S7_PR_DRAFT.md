# Integrar fuentes S1–S7 en una partida y conservar reconstrucción privada del arte

Los laboratorios de planeta, carga, combate, civiles, héroes y amenazas estaban separados y sus PR no ofrecían una entrada común de juego. Esta candidata reúne sus fuentes alrededor de Playable: un host de personaje/sesión, daño y persistencia compartidos, ciudades, reloj/civiles, tres héroes, robot/kaiju y rueda. El alcance sigue en validación; este PR no declara todavía una versión S7 jugable aceptada.

Se exporta el snapshot C++ y herramientas `acf4156` desde una rama limpia basada en `6c2d903`. No se incorpora el historial local de assets ni Content binario nuevo. Se conservan recetas S2.5, recibos de procedencia, los cambios públicos de Robert y las decisiones de PR17/22/23 en el plan vigente. La partida requiere provisión local autorizada y autoría de `/Game/IB_Playable`; un clone de fuentes no incluye ese arte ni el mapa generado.

La CI añade política de publicación sobre todos los commits nuevos, sintaxis/recetas y 13 suites nativas. La comparación S35/S4 exige rechazar el replay con timestamp renovado, manteniendo pruebas de unidades, signo, categorías e inmunidad.

Validación observada:

- CI remota de `52e8da3`: [Integrated source checks](https://github.com/meliodr/juego-/actions/runs/43808923815) y [Lab audit tests](https://github.com/meliodr/juego-/actions/runs/43808923825) aprobados. La exportación `acf4156` todavía no se ha publicado ni ejecutado remotamente.

- Exportación: **17 tests de auditoría/publicación PASS**, 10 tests de receta S2.5 y 5 de refinamiento PASS; 1 test de bytes locales omitido explícitamente.
- Nativos sobre la rama pública exportada: **13 suites requeridas PASS** con MSVC C++17, incluidas 282 assertions de comparación S35/S4. Hashes estables sobre `9ca77e1`; las políticas y fixtures nativas no cambian en `acf4156`. No se ejecutó Unreal desde este frente. Geometría S3 PASS.
- Unreal: `4f95872` compiló y **`9ca77e1` compiló Editor + Game**, según el integrador: run `s7-build-20261116T045837Z`, 224,52 s y 210,82 s. El Editor de `acf4156` también pasó según el integrador; Game y QA finales de la nueva revisión siguen pendientes.
- Pendientes: actualización final de autoría y ajustes de Source posteriores a este snapshot, validación Unreal de esa versión final, cook/paquete, partida DX12/audio, rendimiento y aceptación. Input simulado y DualSense físico se registran por separado.

[Archivos y hashes exportados](IB_S7_PUBLIC_EXPORT.json) · [Reproducción y dependencias](IB_S7_SOURCE_OVERVIEW.md) · [Alcance y gates](PLAN_ACTIVO.md).

Tras el merge y validación correspondientes, #26/#27/#28 podrán cerrarse como cubiertos por la integración, y #25 como laboratorio sustituido preservando su rama y las fuentes recuperadas. PR21 ya fue cerrado por el integrador como cubierto por PR30. Esta propuesta no cierra otros PR ni afirma integración de investigación/documentos todavía ausentes.
