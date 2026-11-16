# IB-VIS01 — Vuelo y contacto con el carro

**Estado: EJECUTADO, pendiente de revisión visual y aceptación del usuario.** Referencias: [issue #10](https://github.com/meliodr/juego-/issues/10), [PR #9](https://github.com/meliodr/juego-/pull/9).

## Reutilización

Se inspeccionaron el maniquí, el esqueleto de la plantilla, sus secuencias Idle/Jump/Drop y el flujo ABP_Unarmed y rigs CR_Mannequin_Body/FootIK/Procedural. El ABP conserva suelo y salto. Las tres poses aéreas reutilizan una muestra neutra de MM_Idle, el esqueleto y el contenedor de secuencia ya usado en el proyecto. Los cambios de brazos, piernas, torso, cuello y respiración cubren siluetas de vuelo y transporte para las que la plantilla no trae clips específicos. Los assets del motor se usan según su licencia de ejemplos. Catálogo Fab consultado como referencia, sin importar recursos.

Two Bone IK nativo se ejecuta al finalizar la pose y lee el chasis real, tanto con el ABP como con reproducción aérea. El código propio conecta una posición barrida con dos objetivos de mano y el instante de liberación. Como alternativa, Control Rig o un Post Process AnimBP pueden aplicar la misma solución. Slots y Layered Blend Per Bone ofrecen una ruta de edición de montajes dentro de un AnimBP compatible, pero esta build usa reproducción de secuencia directa en vuelo. Se evita duplicar capas o perder los estados nativos de suelo.

## Implementación

- Suspensión neutra, crucero con brazo adelantado y compensación de cabeza, y variante aérea de carga erguida con rodillas flexionadas. Cambio de pose interpolado durante 0,18 s, sin root motion.
- Levantamiento de 0,55 s, objetivo sobre el cuerpo independiente del yaw de cámara, barrido completo del proxy, dos objetivos bajo su superficie y soltado por obstrucción fuera de alcance. La cámara retrocede suavemente mientras hay carga.
- Preparación de lanzamiento con empuje y liberación a 0,25 s. El seguimiento de los brazos se desvanece después de liberar. El tiro conserva la dirección de la cámara. Se mantienen la rotura, abertura y recarga previas.

## Evidencia

Win64 Development compiló y se empaquetó. Steam abrió el paquete actualizado con Steam Input activado y el ejecutable renderizó la manzana en DX12 SM6. Se inspeccionó la suspensión desde atrás y en ángulo y se guardaron capturas de esos encuadres. No se obtuvo una comparación antes/después equivalente ni vídeo.

- `ImpactBeyond.Flight.Headless` PASS, topes 9/18/32 m/s, frenada y colisión de cápsula.
- `ImpactBeyond.Circuit.Headless` PASS, levantamiento, caída, preparación y liberación, apertura, paso y reset.
- `ImpactBeyond.Circuit.TransportCycles` PASS, 20 agarres, transporte aéreo con las tres velocidades y 20 recargas completas.

Pendientes: verificar orientación de las palmas y calidad del contacto durante giro y frenada, bloqueo de la carga contra fachadas, continuidad visual del lanzamiento y vistas frontal, lateral y de tres cuartos de crucero. Los ensayos sin render verifican objetivos de agarre y física, no la silueta de las manos. Para la evaluación del usuario, jugar desde Steam con el acceso ImpactBeyond, agarrar con Círculo, volar con R3, cambiar de velocidad con R1/L1, frenar con Cuadrado, lanzar con Triángulo y reiniciar con cruceta abajo.
