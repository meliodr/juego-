# Integrar S1–S7 en una partida con fuentes y arte reconstruible

Los laboratorios de planeta, carga, combate, civiles, héroes y amenazas estaban separados. Esta candidata los reúne en un personaje y una sesión comunes, con daño y persistencia compartidos, cuatro ciudades, reloj/civiles, tres héroes, robot/kaiju y rueda. La publicación parte de main y conserva recetas S2.5, los cambios de Robert y las decisiones aceptadas de PR17/22/23.

Fuentes C++: `fa75f7f` (base jugable `6b4631d` y corrección QA de captura/GPU); autoría `742ff49`; configuración/validador de paquete `17d7f91`. No se incorpora historial privado ni Content binario nuevo. El autor debe proveer las dependencias autorizadas y generar `/Game/IB_Playable`: el clone público no incluye ese arte ni mapa.

Validación observada:

- CI de `534f0d4`: [Integrated source checks](https://github.com/meliodr/juego-/actions/runs/43812082671) y [Lab audit tests](https://github.com/meliodr/juego-/actions/runs/43812082685) aprobados, incluidas 13 suites nativas. La actualización `fa75f7f` aún necesita su CI.
- Editor + Game de `6b4631d` PASS; Game de `fa75f7f` PASS en `s7-build-20261116T054825Z` (114,26 s). La corrección afecta captura QA y módulos RHI/RenderCore, sin cambios de lógica jugable; no se atribuye una nueva compilación Editor.
- Autoría `b341476` PASS, exit 0 y cero dependencias ausentes (`s7-author-20261116T053232Z`). Los siete materiales S25 con flags de instancing permanecen locales; su receta es pública.
- Paquete `17d7f91` PASS, cero errores y 807 paquetes (`s7-package-20261116T054101Z`). Sus cuatro pruebas DX12 —PlayableSession, RealInput, S3 Contacts y S7 PreparedEncounter— pasaron en `s7-tests-20261116T054558Z`, sin recursos ausentes ni materiales de reserva. Se conservan resultados y límites por mapa, incluida FlightReentry de diez regresos en S3Lab.
- El perfil previo falló en la lectura QA de imagen (`InRHITexture null`) y no produjo CSV. `fa75f7f` corrige esa captura y registra la GPU real de RHI. Faltan recook/actualización del ejecutable, repetición del perfil y revisión de presentación/audio. La captura definitiva medirá el equipo como está, con contexto de carga registrado; Fortnite fue cerrado por el usuario antes de esta repetición, mientras las cuatro pruebas DX12 previas sí lo tuvieron activo. Input simulado, DualSense físico y aceptación humana permanecen separados; no se promete 60 FPS ni se declara S7 aceptado.

[Archivos y hashes](IB_S7_PUBLIC_EXPORT.json) · [Reproducción](IB_S7_SOURCE_OVERVIEW.md) · [Evidencia y límites de cierre](IB_S7_CLOSURE_REVIEW.md).

Tras integrar la candidata validada, #26/#27/#28 podrán cerrarse como cubiertos; #25 como laboratorio sustituido, conservando su rama y sin afirmar importación completa. #7/#8 quedan como prototipos cubiertos y PR17/22/23 como decisiones consolidadas. PR21 ya fue cerrado por el integrador como cubierto por PR30. No se propone cerrar #1/#5/#6/#20 ni integrar investigación ausente.
