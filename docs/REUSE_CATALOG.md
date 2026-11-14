# Catálogo de reutilización — IMPACT: BEYOND

Fecha de revisión: **2026-11-14**. Alcance: **28 candidatos y 3 índices**, no cientos de proyectos auditados. No se han descargado paquetes completos, compilado proyectos ni ejecutado juegos en Unreal durante esta revisión.

## Cómo leer el catálogo

- **Probar:** candidato concreto para un experimento aislado. No significa integrado o compatible.
- **Reserva:** potencial útil, pero no bloquea la primera versión.
- **Bloqueado:** faltan permisos, contenido o acceso verificable.
- **Referencia:** útil para observar una idea; no se autoriza copiar su código o sus recursos.

Las prestaciones descritas son declaraciones del autor salvo cuando se indique inspección de código. Una licencia del repositorio no acredita automáticamente los derechos sobre todos los personajes, marcas, sonidos o assets externos. En Fab, precio gratuito no equivale a permiso para redistribuir el paquete fuente en un GitHub público.

## A. Candidatos prioritarios y reservas con utilidad directa

### R01 — Ephemera / 0xWerz

Fuente: https://github.com/0xWerz/ephemera

**Reserva orbital.** El autor describe un sandbox ambiental de caminar, jetpack y vuelo desde un planeta a su luna, desarrollado con Claude Code. Licencia MIT inspeccionada. El archivo `unreal/Ephemera.uproject` declara UE 5.8; existen Source, Config y herramientas de preparación de contenido. No es un juego de combate. Reutilización propuesta: estudiar coordenadas, transición superficie-espacio y vuelo, sin importar todo el proyecto a la base urbana.

Evidencia adicional: árbol inspeccionado en `e9e0a38448deb70907563d47d4e0648e9f79356f`; plugins declarados incluyen ProceduralMeshComponent, Niagara, herramientas de editor, Reflex y PixelStreaming2; DLSS aparece como opcional. No se ha verificado una construcción reproducible local.

### R02 — Modular Mechanics Toolkit / REPIRK

Fuente: https://github.com/REPIRK/Modular-Mechanics-Toolkit-UE5

**Probar primero para agarres.** MIT inspeccionada; README orientado a UE 5.7/Win64. Se inspeccionó `Public/PhysicsGrabberComponent.h`: agarrar/soltar, lanzar, cargar lanzamiento, rotar objeto y eventos de impacto. El árbol en `55209ef932ceb1c45ce23f1682b13aa4b64debb5` contiene un módulo C++ con Public/Private y Build.cs, pero no `.uplugin`, `.uproject` ni Content. Los Blueprints y recursos comerciales anunciados no deben darse por incluidos. Extraer únicamente el componente y adaptar sus dependencias.

### R03 — Realtime Destructible Mesh / LazyDevelopers

Fuente: https://www.fab.com/listings/46f4e8fc-c95c-4e1c-be61-17439bcbbd1b

Documentación: https://lazydevelopers24.github.io/RealtimeDestructibleMesh/

**Probar con condiciones.** Fab muestra Free y etiquetas del autor de uso/generación con IA. Anuncia cortes de mallas mediante operaciones booleanas y trabajo en segundo plano. Candidato para perforar paredes; no demuestra destrucción urbana a velocidad supersónica. Falta confirmar versión binaria compatible y licencia de adquisición, no visibles en el texto recuperado. No copiar el paquete al repositorio público. Comparar una pared con Chaos nativo antes de elegir.

### R04 — Niagara Destruction Driver / eanticev

Fuente: https://github.com/eanticev/niagara-destruction-driver

**Reserva de rendimiento visual.** MIT inspeccionada. Convierte geometría prefracturada en representación de destrucción con Niagara/GPU; documenta pruebas en UE 5.5.4. Tiene limitaciones declaradas, entre ellas fracturación de un nivel y tareas pendientes para estructuras/movimiento. Aprovechar solo para escombros cosméticos, no asumir cuerpos rígidos agarrables o eliminación automática de la colisión intacta. No introducirlo hasta medir un problema real.

### R05 — Dismemberment / Thyke

Fuente: https://github.com/Thyke/Dismemberment

**Probar después del combate básico.** MIT inspeccionada. Plugin C++ con configuración de partes mediante `ULimbData`, ocultación de huesos y creación de actores de extremidad. Propuesta: piezas de armadura o separación estilizada sin sangre explícita. Requiere preparar mallas separadas, tapas y físicas apropiadas. No es corte arbitrario de cualquier personaje. Versión exacta compatible con nuestra instalación pendiente.

### R06 — SkeletalMeshDestruction / Lim-Young

Fuente: https://github.com/Lim-Young/SkeletalMeshDestruction

**Reserva alternativa a R05.** El repositorio publica MIT y presenta daño de malla esquelética, marcas/roturas y desmembramiento. Antes de incorporar: inspeccionar archivo de licencia, fuente y versión concreta. No activar dos sistemas de daño corporal superpuestos. Usar solo si resuelve mejor una prueba reproducible con el mismo personaje.

### R07 — Target System Plugin / mklabs

Fuente: https://github.com/mklabs/ue4-targetsystemplugin

**Reserva para fijación de enemigos.** MIT publicada. Ofrece selección y seguimiento de objetivos; el README enumera UE 5.1–5.3. Puede ahorrar selección de blancos y comportamiento de cámara, pero requiere adaptación para combate aéreo y validación en la versión elegida. No garantiza que dos combatientes a gran velocidad puedan interceptarse correctamente.

### R08 — Cosmic Architect / narratech

Fuente: https://github.com/narratech/TFG-Bravo-Perez-Rubio

**Reserva planetaria alternativa.** Proyecto de generación planetaria procedural, MIT publicada, con referencia a UE 5.7 y dependencia FastNoiseLite. Evaluar terreno/LOD en un proyecto separado. No combinar su arquitectura planetaria con Ephemera sin justificarlo; seleccionar un enfoque tras una prueba de ascenso y regreso.

### R09 — RealtimeMeshComponent Core / TriAxis-Games

Fuente: https://github.com/TriAxis-Games/RealtimeMeshComponent

**Reserva técnica.** El núcleo es MIT; el README anuncia UE 5.5–5.8. Sirve para geometría dinámica, no es un juego ni un sistema completo de destrucción. Las funciones comerciales Pro no deben considerarse parte de la opción gratuita. Incorporar solo si una necesidad de mallas en ejecución no queda resuelta con los componentes actuales.

### R10 — Procedural Surface Terrain Generation / ShotaKhakhishvili

Fuente: https://github.com/ShotaKhakhishvili/Procedural-Surface-Terrain-Generation

**Reserva de terreno.** MIT visible; proyecto con Source/Config/Content y descripción de generación por bloques, carga asíncrona y niveles de detalle. La versión exacta y el recorrido de construcción siguen pendientes. No confundir terreno generado con un planeta completo ni asumir documentación suficiente para una integración rápida.

### R11 — Universal Base Characters, edición Standard / Quaternius

Fuente: https://quaternius.com/packs/universalbasecharacters.html

Descarga del autor: https://quaternius.itch.io/universal-base-characters

**Probar para el aspecto del héroe.** El autor ofrece Standard gratuitamente y declara CC0. El paquete comercial completo incluye contenido adicional: no asumir que todos los personajes, variantes y proyectos de motor anunciados pertenecen al ZIP gratuito. Revisar el manifiesto de Standard y elegir una base de proporciones superheroicas. Sin trajes, logos o identidades copiados de franquicias.

### R12 — Universal Animation Library, edición Standard / Quaternius

Fuente: https://quaternius.com/packs/universalanimationlibrary.html

Descarga: https://quaternius.itch.io/universal-animation-library

**Probar las animaciones realmente incluidas.** CC0 declarada. El catálogo completo anuncia más animaciones que la edición gratuita: no afirmar que toda la biblioteca es gratis. Importar Standard, listar clips, seleccionar movimientos útiles y probar retargeting. El vuelo y los agarres pueden requerir poses y ajustes adicionales.

### R13 — Universal Animation Library 2, edición Standard / Quaternius

Fuente: https://quaternius.com/packs/universalanimationlibrary2.html

Descarga: https://quaternius.itch.io/universal-animation-library-2

**Reserva complementaria de animación.** CC0 declarada y descarga Standard gratuita; Source completo es una opción distinta de pago. Seleccionar solo clips que falten, evitando importar ambas bibliotecas por acumulación. No asumir que todo el catálogo de combos y parkour anunciado está incluido gratis.

### R14 — Shockwave Distortion / Sandbox Studio

Fuente: https://www.fab.com/listings/db0d837f-3c94-4a67-86a8-5653ebddce12

**Probar como efecto visual.** El autor lo describe como gratuito: onda de distorsión Niagara para impactos/explosiones/energía. Verificar precio final, versión y licencia al añadir a la biblioteca. Es presentación visual; no calcula daño, impulso ni destrucción. No publicar el paquete fuente en este repositorio sin permiso comprobado.

### R15 — Psychokinesis Force / Master Dinochan

Fuente: https://www.fab.com/listings/6a5e0c10-ddfe-4aa5-8fa9-11a049aaaf53

**Reserva para comparar telequinesis.** La ficha consultada aparece gratis. Ofrece agarrar, colocar y lanzar objetos/personajes. Es un recurso antiguo orientado a UE4; compatibilidad UE5 y licencia exacta pendientes. No sustituirá R02 por una migración larga. Puede servir al forjador de energía si pasa una prueba rápida sin dependencias pagas.

## B. Atractivos, pero no aprobados para copiar

### R16 — ManOfSteel / ProdiG66

Fuente: https://github.com/ProdiG66/ManOfSteel

**Bloqueado como base completa.** MIT publicada y UE 5.3, con vuelo, aterrizaje y combate según el autor. El README advierte que la cuota Git LFS impide descargar correctamente los assets. El enlace alternativo no se ha auditado. Código concreto podría evaluarse con su licencia; personajes/marcas y archivos artísticos requieren revisión independiente. No asumir proyecto descargable y listo.

### R17 — Colossus Combat / MythwellDev

Fuente: https://github.com/MythwellDev/Colossus-Combat

**Referencia de gran interés, permiso pendiente.** Presenta UE 5.8, agarres de personajes vivos/ragdoll, lanzamiento, combate, cámara y `UBrawlerGrabComponent`. Hay estructura de proyecto, pero no se encontró licencia explícita en los elementos revisados. No copiar mientras no haya autorización. Que el README diga modular/reutilizable no equivale a una licencia.

### R18 — SecretIdentity / ShikenNuggets

Fuente: https://github.com/ShikenNuggets/SecretIdentity

**No utilizar como base.** Proyecto UE5 superheroico archivado; el autor indica que no incluye todos los assets. No se encontró licencia explícita en la revisión. Queda como referencia de diseño, sin copiar código ni recursos.

### R19 — UE5 Superhero Flight System / kn-404

Fuente: https://github.com/kn-404/UE5_Superhero-Flight-System

**Bloqueado.** Describe vuelo Blueprint relativo a cámara y mezcla direccional. Código/paquete enlazado externamente; no se verificó una licencia explícita ni la integridad de esa descarga. No es una dependencia aprobada solo porque muestre vuelo convincente.

### R20 — Superhero Mechanics, VFX-Driven Powers / KishorePaul0205

Fuente localizada: https://github.com/KishorePaul0205/Superhero-Mechanics---VFX-Driven-Powers-in-Unreal-Engine-5

**Descubrimiento, no auditoría.** Apareció en búsqueda con descripción de poderes en UE5. La inspección directa no se completó; disponibilidad, licencia y archivos permanecen sin verificar. No recomendar su incorporación con la evidencia actual.

### R21 — PhysicalRagdoll / Vaei

Fuente: https://github.com/Vaei/PhysicalRagdoll

**Reserva bloqueada por revisión de licencia.** Describe mezcla de animación física, ragdoll y recuperación. No se confirmó licencia explícita en el material examinado. No marcarlo MIT por asociación con otros proyectos del autor. Comprobar permisos y API antes de plantear una prueba.

### R22 — VFX / gabrielecossutta

Fuente: https://github.com/gabrielecossutta/VFX

**Referencia visual.** Ejemplos Niagara de energía, rayos y escudos; licencia explícita no confirmada. Útil para identificar el tipo de efecto deseado, no para copiar assets sin permiso.

### R23 — Telekinesis-2022 / a-lubecki

Fuente: https://github.com/a-lubecki/Telekinesis-2022

**Referencia, no incorporación.** Experimento UE4 inspirado en telequinesis. Sin licencia explícita encontrada y con riesgo de migración. Preferir el componente MIT de R02 o una alternativa con términos claros.

### R24 — Superman Style Flight Experience / TJATOMICA

Fuente: https://tjatomica.itch.io/superman-ue5

**Demo de sensación de vuelo.** Descarga jugable gratuita, basada en una ciudad exigente. No se verificó publicación de fuente reutilizable. Podemos estudiar la sensación de velocidad/cámara; no extraer assets del ejecutable ni tomar gratuidad de juego como autorización de reutilización.

### R25 — Living Superweapon / taskmasterpeace

Referencia indexada: https://github.com/taskmasterpeace/living-superweapon

**No disponible en la comprobación.** El buscador devolvió una descripción, pero la consulta directa y el conector de GitHub respondieron 404. No tratar el resultado indexado como proyecto accesible, licencia confirmada ni contenido descargable.

## C. Otros motores o legado: no importar a Unreal directamente

### R26 — ScamScatter / danbystrom

Fuente: https://github.com/danbystrom/ScamScatter

**Referencia de algoritmo.** Fractura procedural en Unity, MIT publicada. No es un plugin Unreal: portarlo requiere trabajo. No cambiar de motor ni reescribirlo para el primer sandbox cuando ya hay opciones nativas.

### R27 — Gliderman / tinmarr

Fuente: https://github.com/tinmarr/Gliderman

**Referencia de vuelo.** Proyecto Unity con vuelo y terreno procedural, MIT publicada. Puede orientar experiencias de desplazamiento, pero no reutilizar sus componentes como si fueran C++/Blueprint de Unreal.

### R28 — UE4 Custom Gravity Plugin / HoussineMehnik

Fuente: https://github.com/HoussineMehnik/UE4-CustomGravityPlugin

**Legado no seleccionado.** Ejemplos de gravedad personalizada en UE4; existe LICENSE.md, cuyas condiciones deben verificarse antes de incorporar. No se ha validado compatibilidad con nuestra versión UE5. No añadirlo automáticamente a la arquitectura planetaria.

## D. Tres índices para ampliar la búsqueda

Estos índices no son paquetes reutilizables ni garantizan que todos sus enlaces tengan la misma licencia.

1. **Juegos creados con IA:** https://github.com/theolundqvist/frontier-games — descubrimiento de demos/proyectos; verificar fuente y licencia individualmente.
2. **Juegos Unreal abiertos:** https://github.com/SeeleAI/awesome-unreal-open-source-games — buscar componentes y proyectos de referencia nativos del motor.
3. **Juegos de distintos motores:** https://github.com/michelpereira/awesome-open-source-games — útil para encontrar alternativas, sin asumir compatibilidad Unreal.

## Fuentes de criterio y próximos controles

GitHub explica la diferencia entre un repositorio público y una licencia de reutilización: https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository

Antes de incorporar cualquier candidato: fijar commit o versión; leer licencia completa; separar permisos del código y del arte; verificar archivos reales y Git LFS; inspeccionar scripts antes de ejecutarlos; comprobar dependencias gratuitas; compilar en un proyecto aislado; registrar pruebas y costo de adaptación. Ver [plan](REUSE_PLAN.md).
