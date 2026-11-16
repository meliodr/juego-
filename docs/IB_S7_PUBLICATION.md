# Publicación de fuentes S7

La rama nace de la base pública `6c2d903` y contiene una exportación de fuentes, sin historial privado de arte. Snapshot C++ `fa75f7f`, autoría `730b39f`, configuración y validador de paquete `17d7f91`. [Archivos y hashes exactos](IB_S7_PUBLIC_EXPORT.json).

Estado actual: Game, autoría, cook y dos perfiles DX12 aprobados; CI de `a645030` verde. El gate final de contenido falló por instancing del agua aunque las cuatro suites aprobaron sus aserciones. Autoría corregida PASS (once materiales, solo agua guardado); recook `8b56a20` aprobado en 37,01 s; cuatro funcionales con gate limpio, perfil confirmatorio y arranque visible Steam pendientes. [Evidencia y límites](IB_S7_CLOSURE_REVIEW.md). PR33 continúa en borrador hasta que el integrador valide el cierre.

## Contrato de exportación

- Leer los archivos por commit fijado; no copiar un árbol mutable ni mergear/cherry-pickear la rama privada o commits de assets.
- Conservar `Robert/FACHADAS_CC0_20261116.md`, recetas S2.5, fixture actualizada `Tools/IBS35_CompareS4.cpp`, plan/cola vigentes y exclusiones de `.gitignore`.
- Exportar solo Source, configuración textual, herramientas seleccionadas y documentación revisada. No añadir Content binario, packs Fab, derivados privados, Binaries, Intermediate, Saved, DDC, fuentes descargadas, frames ni capturas.
- Los assets ya presentes en la base pública permanecen intactos. Los flags locales de ocho materiales del mundo y tres del árbol se reconstruyen mediante la [receta](IB_S25_SOURCE_REBUILD.md); no se publican sus `.uasset` modificados.
- Revisar datos y rutas personales en documentos y recibos. El gate inspecciona cada commit introducido: añadir y después borrar un asset no lo elimina de la auditoría.
- La versión final debe registrar commit, archivos y pruebas exactas. `final_source_commit` permanece sin fijar mientras se valida la entrega; no convertir un snapshot intermedio en aceptación por cambiar una etiqueta.

Un clone de fuentes requiere provisión legal local y autoría del mapa. Compilar C++ no suministra el arte ni garantiza el arranque de `/Game/IB_Playable/Lvl_IB_Playable`.

## CI y comprobaciones

[Integrated source checks](https://github.com/meliodr/juego-/actions/runs/43813155209) y [Lab audit tests](https://github.com/meliodr/juego-/actions/runs/43813155278) aprobaron `a645030`. Incluyen política de publicación, sintaxis Python sin importar Unreal, recetas S2.5, geometría S3 y 13 suites C++17/MSVC. Una fixture requerida ausente provoca fallo.

`IBCI_Native.py` escribe fuera del checkout, verifica estabilidad de hashes y no ejecuta Unreal ni instala herramientas. Las pruebas nativas no certifican física CMC/Chaos, render, audio o control humano. La comparación S35/S4 conserva casos de unidades/categorías/inmunidad y rechazo de replay; el contrato canónico tiene sus propias pruebas. [Recibo nativo inicial](IB_S7_NATIVE_VALIDATION.json) y resultados remotos quedan identificados por revisión.

## Reconciliación tras integrar PR33

| PR | Cierre previsto |
|---|---|
| #26 / #27 / #28 | Fuentes adaptadas cubiertas por la integración |
| #25 | Laboratorio sustituido; fuentes seleccionadas recuperadas, no importación completa |
| #7 / #8 | Prototipos cubiertos por la partida vigente |
| #17 / #22 / #23 | Decisiones aceptadas consolidadas; prompts/investigación históricos conservados |

Conservar ramas y anteponer el cierre sin borrar las descripciones originales. PR21 ya fue cerrado por el integrador como cubierto por PR30. #1/#5/#6/#20 no se etiquetan integrados ni se cierran por esta reconciliación. Este frente prepara las fuentes y evidencia; el integrador publica, integra y cierra después de revisar el paquete exacto.
