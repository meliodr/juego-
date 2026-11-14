# AGENTS.md — IMPACT: BEYOND

## Misión

Crear un sandbox superheroico 3D divertido mediante reutilización gratuita. Priorizar vuelo frenético, agarres, lanzamientos y destrucción. No convertir la primera entrega en un proyecto de planeta completo.

Leer README.md, docs/GAMEPLAY_SPEC.md, docs/SANDBOX_DESIGN.md, docs/COOP_PLAN.md, docs/LINUX_BASELINE.md, docs/REUSE_CATALOG.md y docs/REUSE_PLAN.md antes de modificar arquitectura o importar dependencias.

Precedencia: LINUX_BASELINE para plataforma; GAMEPLAY_SPEC para controles y núcleo individual; SANDBOX_DESIGN para identidad, personajes, enemigos y alcance creativo; COOP_PLAN para dos jugadores, autoridad, pausa online y pruebas tempranas. Las exclusiones antiguas de multijugador se refieren a la primera entrega, no a una arquitectura permanentemente individual. El catálogo conserva candidatos y reservas, no aprobaciones de compatibilidad.

## Estado inicial

Este repositorio contiene documentación. No contiene todavía un proyecto Unreal. Ningún candidato del catálogo se ha compilado o ejecutado durante la investigación inicial. No describir documentos, personajes propuestos ni matrices de pruebas como un juego implementado.

## Plataforma y jugabilidad — actualización 2026-11-14

- Decisión del usuario: Linux. Desarrollar y entregar el primer ejecutable nativo Linux, con Vulkan. No depender de Wine/Proton, DLL de Windows ni servicios gráficos exclusivos de un fabricante.
- No imponer distribución ni versión del motor sin inspeccionar el entorno real. Usar toolchain y documentación correspondientes a la versión efectivamente elegida. No confundir un contenedor de chat con el equipo del usuario.
- Todo candidato requiere inspección de fuentes/descriptores y prueba de compilación, empaquetado y ejecución en Linux. El README, una licencia MIT o un binario Windows no acreditan esa compatibilidad.
- R02/REPIRK condicionado a portabilidad del agarre; R03 en reserva hasta confirmar Linux, licencia y acceso. La pared Chaos modular es el respaldo inicial. R01/Ephemera es laboratorio separado, no base obligatoria.
- Orden de entrega: L00 base Linux -> D0 vuelo/control/cámara -> D1 carro y pared -> D2 rival y sandbox -> D3 forjador/variedad -> cooperativo regional -> expansión orbital. N0/N1 se prueban durante D0/D1, no después de todo el contenido. No construir lobby/Internet antes de la primera diversión individual.
- Vuelo libre sin combustible; interacciones compartidas; no autofrenadas bruscas invisibles, menús para cada agarre ni generación de objetos con IA durante gameplay.
- Dos héroes, nombres provisionales VECTOR y PRISMA. Ambos deben funcionar solos y poder repetirse en cooperativo. PRISMA empieza con escudo, puño, martillo y pinza.
- Primer enemigo: rival volador. Avispa y Ancla amplían el sandbox; Interdictor y Bastión son posteriores. No crear cinco clases/rigs completos antes de probar el rival.
- Identidad de cómic 3D estilizado, siluetas legibles y materiales consistentes. Preferir reutilización autorizada, modelos provisionales y piezas rígidas antes que capas/ropa complejas. Daño corporal estilizado no bloquea el bucle.
- Compilar no demuestra diversión. Obtener feedback real de control, velocidad y claridad, además de pruebas técnicas.

## Dos jugadores y límites

- El usuario pidió dos jugadores a futuro. Cooperativo LAN y luego Internet, con anfitrión jugador y cámara individual, es la propuesta actual. No afirmar que escogió explícitamente online en vez de pantalla dividida.
- Preparar autoridad y contratos desde el inicio; pruebas con dos conexiones de movimiento, carro y pared antes de consolidar donantes. No presentar 'activar Replicates' como solución completa.
- No usar jugador 0 como propietario global de cámara, poderes, inventario o streaming. Entradas/cámara/UI son locales; daño, agarres y cambios estructurales tienen autoridad definida.
- Un solo portador por objeto. Validar solicitudes por el actor poseído; limpiar propiedad en muerte, desconexión, destrucción y reinicio. No dos clientes simulando daño definitivo sobre el mismo impacto.
- Replicar estado jugable de estructuras y pocos objetos importantes; escombros cosméticos sin daño/collisión de gameplay son locales. No asumir Chaos determinista entre máquinas ni usar solo un evento transitorio para daños persistentes.
- Movimiento personalizado integrado con predicción/replicación de CMC cuando aplique. No mover por Tick con SetActorLocation y declarar red resuelta.
- Individual puede pausar; online abrir menú/perder foco libera entradas pero no detiene el mundo. No pausa global por impacto ni cámara lenta unilateral que altere simulación.
- Fuego amigo desactivado por defecto. Agarre de aliado requiere aceptación individual y posibilidad de soltarse; no lo sustituye una opción del anfitrión.
- Primera región cooperativa acotada y anunciada; no prometer simultáneamente órbita y superficie, migración de anfitrión, PvP, pantalla dividida o conexión universal sin configurar la red.
- No contratar servicios, abrir puertos, instalar VPN, conectar proveedores ni modificar redes del usuario sin autorización específica. Ninguna IA externa es necesaria en runtime.

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
4. Ejecutar pruebas del plan, Linux y puertas cooperativas pertinentes. Si falla, registrar evidencia y valorar complejidad cualitativamente antes de continuar.
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
- En red, identificar anfitrión/cliente, procesos/equipos, RTT medido, pérdida/jitter y configuración usada. PIE limpio no sustituye dos paquetes Linux ni una prueba de latencia.
- Limitaciones y cambios propios. No inventar porcentajes de implementación.

No se fija un parche de motor aquí: comprobar instalación y experimentos antes de fijarlo. Una página de documentación que cambie al selector más nuevo no autoriza migrar a una Preview.

## Primera tarea técnica pendiente de ejecución

Inspeccionar Linux y preparar la base mínima de Unreal. Obtener un paquete nativo ejecutable y después un circuito de vuelo con aceleración, frenado, recuperación y cámara. Probar N0 en un mapa mínimo; a continuación evaluar R02 y la pared nativa según D1/N1. No descargar todo el catálogo ni iniciar una migración planetaria.

No declarar la tarea terminada sin abrir, ejecutar y empaquetar donde exista una instalación funcional del motor. Cuando el entorno no permita ejecutar Unreal, completar solo la inspección/documentación útil e indicar exactamente qué prueba no pudo realizarse. No crear archivos Blueprint ficticios ni presentar código no compilado como validado.
