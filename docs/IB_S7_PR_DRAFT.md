# Integrar S1–S7 en una partida jugable con arte reconstruible

Los laboratorios de planeta, interacción, combate, civiles, héroes y amenazas estaban separados. Esta entrega los reúne en un personaje y una sesión comunes: daño y persistencia compartidos, cuatro ciudades, reloj/civiles, tres héroes, robot/kaiju y rueda de poderes. Conserva las recetas S2.5, los cambios de Robert y las decisiones aceptadas de PR17/22/23.

La rama parte de main y publica fuentes, recetas y documentación, sin historial privado ni Content binario nuevo. C++ `fa75f7f`, autoría `730b39f`, configuración/validador `17d7f91`; el arte autorizado y `/Game/IB_Playable` se reconstruyen localmente.

Validación:

- [CI dce329e](https://github.com/meliodr/juego-/actions/runs/43814696403) y [auditoría](https://github.com/meliodr/juego-/actions/runs/43814696452) PASS, incluidas 13 suites nativas. Game, autoría y paquete final `s7-package-20261116T060244Z` PASS.
- Doce suites funcionales sobre el ejecutable exacto y cuatro suites del paquete en DX12 PASS. El gate final confirma cero recursos ausentes y cero materiales de reserva (`s7-tests-20261116T060433Z`).
- Perfil final `s7-tests-20261116T060613Z`: 90,011 s, 5.395 fotogramas, 59,9253 FPS medios; p95 16,6848 ms, p99 16,7421 ms, máximo 93,0082 ms y un fotograma >50 ms. DX12/audio y reloj real; salida 1920×1080, escala interna 83%, calidad alta/2, límite 60 FPS, Fortnite ausente. Es una medición local acotada, no garantía de rendimiento.
- Steam Input está activado y el acceso preparado; arranque visible pendiente de cerrar un aviso Windows de QA. No se concedieron permisos de red ni cambiaron reglas del firewall. La evaluación humana del control y DualSense físico permanece separada de la entrega validada.

[Fuentes y hashes](IB_S7_PUBLIC_EXPORT.json) · [Reproducción y arranque](IB_S7_SOURCE_OVERVIEW.md) · [Resultados y límites](IB_S7_CLOSURE_REVIEW.md).

Tras integrar: #26/#27/#28 cubiertos; #25 laboratorio sustituido sin importación completa; #7/#8 prototipos cubiertos; #17/#22/#23 decisiones consolidadas. Conservar ramas y descripciones históricas. PR21 ya está cerrado como cubierto por PR30. No se propone cerrar #1/#5/#6/#20.
