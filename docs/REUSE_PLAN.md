# Plan de reutilización e integración

Fecha: 2026-11-14. Plan propuesto; ninguna prueba de Unreal de este documento está completada.

## Decisión de arquitectura

Mantener una única base ligera de Unreal. No fusionar Ephemera, Colossus Combat, Lyra y una plantilla de naves. Se reutilizan componentes pequeños o recursos compatibles, no varios frameworks enteros. El primer bucle es vuelo + carro + pared + enemigo. El planeta completo, daño corporal y segundo personaje no bloquean esa entrega.

Los candidatos y fuentes están en [REUSE_CATALOG.md](REUSE_CATALOG.md). Los identificadores R01–R28 se refieren a ese archivo.

## Requisitos antes de importar

- Comprobar versión instalada del motor, compilador, GPU, RAM, VRAM y almacenamiento. No inventar especificaciones ni cambiar de versión para seguir un README.
- Crear proyecto de prueba y obtener compilación y paquete ejecutable base.
- Para cada incorporación, registrar URL, commit/versión, fecha, licencia completa, archivos seleccionados y dependencias.
- Separar código con licencia permisiva de arte comercial, marcas y paquetes de tiendas.
- Inspeccionar scripts de construcción/importación antes de ejecutarlos. No desactivar medidas de seguridad para abrir instaladores de terceros.
- El precio final debe ser cero. No comprar, contratar API ni introducir dependencia Pro/Source de pago.

## Experimentos acotados

### E00 — Base y medición

Preparar un proyecto ligero con personaje temporal, entrada, cámara y una escena de cubos. Registrar hardware, resolución y calidad. Medir tiempo de fotograma y física en una ruta reproducible, sin declarar una cifra objetivo cumplida antes de medir.

**Aceptación:** compila, se empaqueta, arranca fuera del editor y vuelve a producirse a partir de instrucciones documentadas. Guardar logs y cambios mínimos.

### E01 — Reutilizar el agarre, no el toolkit entero

Candidato R02: incorporar únicamente `PhysicsGrabberComponent` y sus dependencias necesarias, manteniendo licencia y avisos. El upstream no trae descriptor de plugin ni todos los Blueprints anunciados: crear el envoltorio mínimo en nuestro proyecto. Revisar implementación, no solo encabezado, antes de importar. No incorporar sistema de construcción, inventario o guardado que no necesitemos.

Pruebas: cubo liviano, objeto pesado y vehículo provisional; agarrar, girar, soltar, cargar y lanzar. Añadir transporte controlado durante vuelo si el resorte físico oscila o se separa. Comprobar también el volumen del objeto sostenido, no solo la cápsula del jugador.

**Aceptación:** 30 ciclos por objeto sin referencias inválidas, duplicación, gravedad/collisión incorrectas al soltar ni inestabilidad persistente. Probar a velocidad baja y alta, y con variación de tasa de fotogramas. No declarar éxito supersónico por funcionar estando quieto.

**Salida alternativa:** si el donante necesita reconstruir media arquitectura, conservar las ideas y usar Physics Handle/movimiento controlado nativos con implementación mínima.

### E02 — Elegir destrucción jugable con una pared

Comparar dos escenas equivalentes: A) una pared prefracturada con Chaos; B) R03, solo después de confirmar compatibilidad y licencia de adquisición gratuitas. No integrar ambas soluciones en toda la ciudad.

Pruebas: carro lanzado, puñetazo, impacto repetido en el mismo borde, perforaciones cercanas, fragmento grande agarrable y reinicio. Comprobar cuándo se actualiza la colisión después de una operación asíncrona. Diferenciar desaparición visual de paso realmente transitable.

**Aceptación:** 50 impactos reproducibles; agujero transitable; no colisión fantasma; reinicio restaura visuales y físicas; tiempos de fotograma/física y memoria registrados antes y después. Ningún bloque de Unreal puede convertirse en un tirón inaceptable sin quedar documentado. Elegir la alternativa que resuelva el bucle con menor integración y costo medido.

**R04 no sustituye esta prueba:** un fragmento cosmético Niagara no equivale a un objeto físico que pueda agarrarse. Evaluarlo más adelante solo para reducir gasto visual.

### E03 — Personaje y animación gratuitos

Descargar solo Standard de R11/R12, conservar licencias y generar una lista real de modelos/clips. Elegir un personaje original/provisional. Probar esqueleto, escala, retargeting, manos y poses de vuelo. Usar R13 solo si aporta un clip necesario no cubierto.

**Aceptación:** movimiento, vuelo, golpe y agarre no cambian la posición de la cápsula de forma inesperada; manos y objeto se alinean razonablemente; no se requiere edición Source de pago. Si el retargeting se vuelve un bloqueo, continuar con el maniquí temporal.

### E04 — Combate físico y daño estilizado

Primero un enemigo: perseguir, golpear, ser agarrado, lanzar, ragdoll y recuperación o estado final. Añadir fijación de objetivo de R07 solamente si ahorra trabajo verificable y funciona en el aire.

Después evaluar R05 con piezas preparadas —armadura/robot o separación estilizada— sin efectos anatómicos explícitos. R06 es alternativa, no dependencia simultánea. Los sistemas sin licencia confirmada R17/R21/R23 no se copian.

**Aceptación:** 20 agarres/lanzamientos seguidos y 20 separaciones/reinicios sin piezas duplicadas, enlaces de huesos inválidos ni capacidades activas que requieran una extremidad perdida. El daño corporal no debe bloquear la entrega del sandbox base.

### E05 — Sensación de poder y segundo personaje

Evaluar R14 para onda de choque visual. El daño y el impulso pertenecen al sistema de gameplay, no al efecto. Crear cuatro construcciones prefabricadas: escudo, puño, martillo y pinza. Reutilizar agarre y daño del primer personaje. R15 solo se prueba como alternativa si funciona sin migración extensa.

**Aceptación:** el escudo detiene un objeto; la pinza lo sujeta y devuelve; puño/martillo usan las mismas paredes y objetivos. Ninguna llamada a un generador externo de IA es necesaria durante la partida. Mantener lectura visual con varios efectos simultáneos.

### E06 — Laboratorio de superficie a espacio

Probar R01 por separado, desde fuente inspeccionada. Verificar scripts, contenido generado y dependencias; revisar necesidad de plugins opcionales antes de activarlos. Solo después extraer conceptos o módulos puntuales a nuestra arquitectura. R08 es alternativa de terreno/planeta, no una segunda arquitectura superpuesta; R09/R10 se usan únicamente ante una necesidad concreta.

**Aceptación:** despegar y regresar a una región reconocible; orientación y posición estables; una prueba con objeto transportado; carga y consumo de memoria registrados. No confundir planeta visible con planeta detallado o destruible en toda su superficie. No añadir combate y ciudades al laboratorio antes de validar el recorrido.

## Orden de entregas

| Entrega | Incluye | Excluye |
|---|---|---|
| D1: bucle físico | E00 + E01 + E02, vuelo provisional | Campaña, planeta, multijugador |
| D2: sandbox divertido | Distrito pequeño, enemigo, sonidos, cámara, reinicio; parte de E03/E04 | Ciudad viva compleja, daño universal |
| D3: poderes distintos | E05, mejoras visuales y daño estilizado opcional | Generación arbitraria de objetos con IA en runtime |
| D4: exploración extendida | E06 y conexión con región detallada | Simulación física de todo el planeta |

## Evidencia exigida para cerrar una tarea

Commit propio, revisión upstream utilizada, licencia preservada, lista de archivos importados/modificados, instrucciones de reproducción, comando de compilación usado, resultado del paquete, logs, escena de pruebas, mediciones y limitaciones. Capturas o vídeo complementan la evidencia, no reemplazan ejecutar la prueba.

Estados permitidos: DESCUBIERTO, DOCUMENTADO, FUENTE_INSPECCIONADA, COMPILADO, EJECUTADO, ACEPTADO o BLOQUEADO. Los candidatos actuales no tienen estado COMPILADO/EJECUTADO/ACEPTADO en este entorno.

## Regla de parada

Dejar de buscar alternativas para un subsistema cuando una opción gratuita supera su prueba y permite avanzar al bucle jugable. No añadir dependencias por acumular funciones o porque un vídeo resulte impresionante. Si falla, registrar el motivo antes de evaluar la siguiente opción. Ningún calendario o porcentaje sustituye estos resultados.
