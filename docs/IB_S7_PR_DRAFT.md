# Integrar S1–S7 en una partida con arte reconstruible

Los laboratorios de planeta, interacción, combate, civiles, héroes y amenazas estaban separados. Esta candidata los reúne en un personaje y una sesión comunes, con daño y persistencia compartidos, cuatro ciudades, reloj/civiles, tres héroes, robot/kaiju y rueda de poderes. Conserva las recetas S2.5, los cambios de Robert y las decisiones aceptadas de PR17/22/23.

La rama parte de main y añade fuentes, recetas y documentación. C++: `fa75f7f`; autoría: `730b39f`; configuración y validador de paquete: `17d7f91`. No incorpora historial privado ni Content binario nuevo. La reconstrucción requiere dependencias autorizadas y generación local de `/Game/IB_Playable`.

Validación actual:

- [CI a645030](https://github.com/meliodr/juego-/actions/runs/43813155209) y [auditoría](https://github.com/meliodr/juego-/actions/runs/43813155278) verdes, incluidas 13 suites nativas.
- Game `fa75f7f` y paquete actualizado `s7-package-20261116T060244Z` PASS (37,01 s); autoría PASS con salida limpia y cero dependencias ausentes. Editor aprobado en `6b4631d`; el cambio posterior es QA de captura/GPU y dependencias RHI/RenderCore, sin cambios de lógica jugable.
- Dos perfiles del mismo ejecutable PASS: medias 59,94 / 59,9848 FPS; p95 16,7181 / 16,684 ms y p99 16,9259 / 16,7149 ms. Salida 1920×1080, escala interna 83%, calidad 2, límite 60 FPS, DX12/audio y tiempo real; Fortnite ausente. Mediciones acotadas anteriores a la corrección del agua, no garantía de 60 FPS.
- Gate final: las cuatro suites aprobaron sus aserciones, pero el control de contenido falló por instancing del material de agua. Autoría corregida PASS: once materiales verificados y solo `agua` guardado (`s7-author-20261116T060201Z`), sin cambios C++; recook `8b56a20` aprobado; faltan cuatro funcionales con gate limpio, un perfil confirmatorio y arranque visible desde Steam. El acceso apunta al ejecutable actual y Steam Input está activado y comprobado en UI. DualSense físico y aceptación humana se registran por separado.

[Fuentes y hashes](IB_S7_PUBLIC_EXPORT.json) · [Reproducción](IB_S7_SOURCE_OVERVIEW.md) · [Resultados y límites](IB_S7_CLOSURE_REVIEW.md).

Después de integrar la candidata validada: #26/#27/#28 cubiertos, #25 laboratorio sustituido sin afirmar importación completa, #7/#8 prototipos cubiertos, #17/#22/#23 decisiones consolidadas. Conservar ramas y descripciones históricas. PR21 ya está cerrado como cubierto por PR30. No se propone cerrar #1/#5/#6/#20.
