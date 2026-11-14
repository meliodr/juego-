# AGENTS.md — IMPACT: BEYOND

## Misión

Crear un sandbox superheroico 3D divertido mediante reutilización gratuita. Priorizar vuelo frenético, agarres, lanzamientos y destrucción. No convertir la primera entrega en un proyecto de planeta completo.

Leer README.md, docs/GAMEPLAY_SPEC.md, docs/LINUX_BASELINE.md, docs/REUSE_CATALOG.md y docs/REUSE_PLAN.md antes de modificar arquitectura o importar dependencias. Para plataforma, controles y orden de entregas prevalecen GAMEPLAY_SPEC.md y LINUX_BASELINE.md; el catálogo conserva los candidatos y sus reservas.

## Estado inicial

Este repositorio comienza con documentación. No contiene todavía un proyecto Unreal. Ningún candidato del catálogo se ha compilado o ejecutado durante la investigación inicial. No describir esta documentación como un juego implementado.

## Plataforma y jugabilidad — actualización 2026-11-14

- Decisión del usuario: Linux. Desarrollar y entregar el primer ejecutable nativo Linux, con Vulkan. No depender de Wine/Proton, DLL de Windows ni servicios gráficos exclusivos de un fabricante.
- No imponer distribución ni versión del motor sin inspeccionar el entorno real. Usar toolchain y documentación correspondientes a la versión efectivamente elegida. No confundir un contenedor de chat con el equipo del usuario.
- Todo candidato requiere inspección de fuentes/descriptores y prueba de compilación, empaquetado y ejecución en Linux. El README, una licencia MIT o un binario Windows no acreditan esa compatibilidad.
- R02/REPIRK queda condicionado a portabilidad del componente de agarre; R03 queda en reserva hasta confirmar Linux, licencia y acceso. La pared Chaos nativa es el primer respaldo. R01/Ephemera es laboratorio separado, no base obligatoria del sandbox.
- Orden obligatorio: L00 base Linux -> D0 vuelo/control/cámara -> D1 carro y pared -> D2 rival y sandbox -> D3 forjador -> D4 exploración orbital. El daño corporal estilizado no bloquea D0–D2.
- Vuelo libre sin combustible; interacciones compartidas; no auto-frenadas bruscas invisibles, menús para cada agarre ni generación de objetos con IA durante gameplay.
- Compilar no demuestra diversión. Obtener feedback real de control, velocidad y claridad, además de pruebas técnicas.

## Presupuesto y permisos

- Compras de assets/plugins: RD$0. No usar ediciones Pro/Source de pago, pruebas temporales o dependencias que después requieran comprar.
- No asumir acceso ilimitado a servicios de IA ni contratar APIs para runtime.
- No copiar código de repositorios sin licencia explícita. No inferir licencias por estar públicos o por proyectos vecinos del mismo autor.
- Al incorporar código permitido, conservar LICENSE, copyright y avisos requeridos; documentar cambios propios.
- Revisar permisos del arte por separado del código. Mantener personajes y marcas originales.
- El repositorio es público: no publicar credenciales, rutas personales, datos privados ni paquetes de Fab u otros recursos cuya redistribución fuente no esté permitida.
- No enviar solicitudes a autores, publicar mensajes ni cambiar visibilidad/permisos sin autorización adicional.

## Reutilización

1. Seleccionar un subsistema y una prueba pequeña. Consultar el catálogo antes de escribir su implementación completa.
2. Inspeccionar archivos reales, licencia y dependencias; fijar commit o versión. No usar README como prueba de que los assets o Blueprints existen.
3. Importar lo mínimo en un experimento aislado. Un framework completo requiere justificación adicional.
4. Ejecutar las pruebas del plan y la base Linux. Si falla, registrar evidencia y estimar complejidad de adaptación cualitativamente antes de continuar.
5. Conservar una única solución principal por sistema. No sumar frameworks de agarre, movimiento o destrucción incompatibles.
6. Dejar de buscar cuando una opción gratuita cumple la prueba y permite avanzar al sandbox.

## Seguridad de ejecución

Inspeccionar scripts de terceros antes de ejecutarlos. No ejecutar instaladores desconocidos ni desactivar controles de seguridad para superar advertencias. No instalar plugins globalmente o modificar la instalación del motor sin necesidad y autorización. No ejecutar acciones destructivas de Git, sobrescribir trabajo existente ni tocar otros repositorios.

MCP es opcional y de editor; verificar su disponibilidad en el motor Linux antes de prometer automatización. No exponerlo a Internet. No incluirlo como dependencia del ejecutable ni modificar configuraciones existentes a ciegas.

## Evidencia y estados

Usar estados: DESCUBIERTO, DOCUMENTADO, FUENTE_INSPECCIONADA, COMPILADO, EJECUTADO, ACEPTADO, BLOQUEADO. No saltar de un enlace a ACEPTADO. Una compilación sin errores no demuestra jugabilidad, rendimiento ni compatibilidad visual.

Para cerrar una tarea, registrar:

- Fuentes y commit/versión upstream, licencia y archivos importados.
- Versión real de Unreal, distribución Linux, compilador, GPU/driver y hardware donde se probó.
- Comandos o pasos reproducibles, mapa de prueba y resultado esperado/observado.
- Logs de compilación y empaquetado, pruebas ejecutadas y fallidas.
- Mediciones con resolución/calidad/hardware; no FPS sin contexto. Separar primera ejecución y recorridos repetidos.
- Limitaciones y cambios propios. No inventar porcentajes de implementación.

No se ha fijado en este archivo una versión de parche del motor: comprobar la instalación y la compatibilidad de los experimentos antes de fijarla en el proyecto.

## Primera tarea técnica pendiente de ejecución

Inspeccionar el entorno Linux disponible y preparar la base mínima de Unreal. Obtener un paquete nativo ejecutable y después un circuito de vuelo con aceleración, frenado, recuperación y cámara. A continuación evaluar R02 y una pared de destrucción nativa según D1. No descargar todos los candidatos ni iniciar una migración planetaria.

No declarar la tarea terminada sin abrir, ejecutar y empaquetar donde exista una instalación funcional del motor. Cuando el entorno no permita ejecutar Unreal, completar solo la inspección/documentación útil e indicar exactamente qué prueba no pudo realizarse. No crear archivos Blueprint ficticios ni presentar código no compilado como validado.
