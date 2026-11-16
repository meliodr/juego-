# Publicación de fuentes S7

Estado actual, 16 de noviembre de 2026: **integración técnica S1–S7 publicada en PR33; presentación rechazada por el usuario, 2/10. No está aceptada como juego.** Está en curso la [corrección de presentación](IB_S7_PRESENTATION_REWORK.md), regida por el [plan activo](PLAN_ACTIVO.md). Su validación y nueva valoración humana siguen pendientes; S8 permanece aplazado.

## Entrega anterior: evidencia histórica

La exportación de PR33 nació de la base pública `6c2d903`, sin historial privado de arte. Snapshot C++ `fa75f7f`, autoría `730b39f`, configuración y validador de paquete `17d7f91`. Su [manifiesto de archivos y hashes](IB_S7_PUBLIC_EXPORT.json) identifica esa entrega, no la candidata de corrección actual.

El paquete `s7-package-20261116T060244Z` aprobó build, autoría, doce suites funcionales del ejecutable, cuatro suites DX12 del paquete, gate de contenido y perfil confirmatorio. CI de `8acc379` verde. El arranque visible desde Steam verificó ese ejecutable con Steam Input y DX12/audio, sin conceder permisos de red. [Evidencia y límites](IB_S7_CLOSURE_REVIEW.md). Estos resultados técnicos se conservan; no equivalen a aceptación visual ni se trasladan a nuevas revisiones.

## Contrato de exportación

- Leer los archivos por commit fijado; no copiar un árbol mutable ni mergear/cherry-pickear la rama privada o commits de assets.
- Conservar `Robert/FACHADAS_CC0_20261116.md`, recetas S2.5, fixture actualizada `Tools/IBS35_CompareS4.cpp`, plan/cola vigentes y exclusiones de `.gitignore`.
- Exportar solo Source, configuración textual, herramientas seleccionadas y documentación revisada. No añadir Content binario, packs Fab, derivados privados, Binaries, Intermediate, Saved, DDC, fuentes descargadas, frames ni capturas.
- Los assets ya presentes en la base pública permanecen intactos. Los flags locales de ocho materiales del mundo y tres del árbol se reconstruyen mediante la [receta](IB_S25_SOURCE_REBUILD.md); no se publican sus `.uasset` modificados.
- Revisar datos y rutas personales en documentos y recibos. El gate inspecciona cada commit introducido: añadir y después borrar un asset no lo elimina de la auditoría.
- Cada entrega debe registrar commit, archivos y pruebas exactas. En el manifiesto de PR33, `final_source_commit` fija el C++ `fa75f7f`; la autoría y contenido local tienen revisiones separadas. La corrección en curso necesita su propia evidencia. La aceptación humana no se infiere de esa etiqueta.

Un clone de fuentes requiere provisión legal local y autoría del mapa. Compilar C++ no suministra el arte ni garantiza el arranque de `/Game/IB_Playable/Lvl_IB_Playable`.

## CI y comprobaciones de PR33

[Integrated source checks](https://github.com/meliodr/juego-/actions/runs/43815842275) y [Lab audit tests](https://github.com/meliodr/juego-/actions/runs/43815842311) aprobaron `8acc379`. Incluyen política de publicación, sintaxis Python sin importar Unreal, recetas S2.5, geometría S3 y 13 suites C++17/MSVC. Una fixture requerida ausente provoca fallo.

`IBCI_Native.py` escribe fuera del checkout, verifica estabilidad de hashes y no ejecuta Unreal ni instala herramientas. Las pruebas nativas no certifican física CMC/Chaos, render, audio o control humano. La comparación S35/S4 conserva casos de unidades/categorías/inmunidad y rechazo de replay; el contrato canónico tiene sus propias pruebas. [Recibo nativo inicial](IB_S7_NATIVE_VALIDATION.json) y resultados remotos quedan identificados por revisión.

## Reconciliación tras integrar PR33

| PR | Cobertura registrada al integrar PR33 |
|---|---|
| #26 / #27 / #28 | Fuentes adaptadas cubiertas por la integración |
| #25 | Laboratorio sustituido; fuentes seleccionadas recuperadas, no importación completa |
| #7 / #8 | Prototipos cubiertos por la partida vigente |
| #17 / #22 / #23 | Decisiones aceptadas consolidadas; prompts/investigación históricos conservados |

Las nueve PR de la tabla quedaron cerradas como cubiertas o sustituidas por PR33; se conservan sus descripciones e historia. PR21 fue cerrado por el integrador como cubierto por PR30. #1/#5/#6/#20 no se etiquetan integrados ni se cierran por esta reconciliación. La publicación de una nueva corrección requiere revisar su paquete y evidencia exactos; este registro no concede aceptación humana.
