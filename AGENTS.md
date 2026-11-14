# AGENTS.md — IMPACT: BEYOND

## Misión

Crear un sandbox superheroico 3D divertido mediante reutilización gratuita. Priorizar vuelo frenético, agarres, lanzamientos y destrucción. No convertir la primera entrega en un proyecto de planeta completo. Leer README.md, docs/REUSE_CATALOG.md y docs/REUSE_PLAN.md antes de modificar arquitectura o importar dependencias.

## Estado inicial

Este repositorio comienza con documentación. No contiene todavía un proyecto Unreal. Ningún candidato del catálogo se ha compilado o ejecutado durante la investigación inicial. No describir esta documentación como un juego implementado.

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
4. Ejecutar las pruebas del plan. Si falla, registrar evidencia y estimar complejidad de adaptación cualitativamente antes de continuar.
5. Conservar una única solución principal por sistema. No sumar frameworks de agarre, movimiento o destrucción incompatibles.
6. Dejar de buscar cuando una opción gratuita cumple la prueba y permite avanzar al sandbox.

## Seguridad de ejecución

Inspeccionar scripts de terceros antes de ejecutarlos. No ejecutar instaladores desconocidos ni desactivar controles de seguridad para superar advertencias. No instalar plugins globalmente o modificar la instalación del motor sin necesidad y autorización. No ejecutar acciones destructivas de Git, sobrescribir trabajo existente ni tocar otros repositorios.

## Evidencia y estados

Usar estados: DESCUBIERTO, DOCUMENTADO, FUENTE_INSPECCIONADA, COMPILADO, EJECUTADO, ACEPTADO, BLOQUEADO. No saltar de un enlace a ACEPTADO. Una compilación sin errores no demuestra jugabilidad, rendimiento ni compatibilidad visual.

Para cerrar una tarea, registrar:

- Fuentes y commit/versión upstream, licencia y archivos importados.
- Versión real de Unreal, compilador y hardware donde se probó.
- Comandos o pasos reproducibles, mapa de prueba y resultado esperado/observado.
- Logs de compilación y empaquetado, pruebas ejecutadas y fallidas.
- Mediciones con resolución/calidad/hardware; no FPS sin contexto.
- Limitaciones y cambios propios. No inventar porcentajes de implementación.

No se ha fijado en este archivo una versión de parche del motor: comprobar la instalación y la compatibilidad de los experimentos antes de fijarla en el proyecto.

## Primera tarea técnica autorizada por el plan, pendiente de ejecución

Inspeccionar el entorno disponible y preparar la base mínima de Unreal. Después evaluar el componente de agarre R02 y una pared de destrucción nativa. No descargar todos los candidatos ni iniciar una migración planetaria. No declarar la tarea terminada sin abrir, ejecutar y empaquetar donde exista una instalación funcional del motor.

Cuando el entorno no permita ejecutar Unreal, completar solo la inspección/documentación útil e indicar exactamente qué prueba no pudo realizarse. No crear archivos Blueprint ficticios ni presentar código no compilado como validado.
