# Sprint 6 — H02 y H03 construidos a partir de personajes y assets EXISTENTES

**Fecha:** 2026-11-15. **Estado:** PLANIFICACIÓN FUTURA, cero importaciones / pruebas realizadas desde este encargo. **Decisión explícita del usuario:** en el **mismo sprint 6** se entregan **dos héroes**, H02 y H03, con modelos preexistentes de Fab o CC0 **editados lo mínimo**, no cuerpos/trajes/cabello modelados desde cero. Sprint 1 sigue activo. Presupuesto RD$0, UE 5.8.4 Win64 DX12, un jugador offline. No descargar ni reclamar recursos Fab, habilitar plugins, instalar software o modificar sistema sin permiso.

## 1. Fuente real de biblioteca y reglas

[Inventario personal de Fab del 2026-11-15](FAB_INVENTARIO_PERSONAL.md), actualmente en rama `docs/fab-auditoria-lote03-20261115`/PR #6: **7 fichas reclamadas**, incluyendo [Adventure Characters (Pack)](https://www.fab.com/listings/db81464d-fcb5-43a8-b309-5555c392be3a), que anuncia cinco personajes y pruebas con esqueleto UE4/UE5. Biblioteca verificada en sesión; **descarga local, edición exacta de licencia y prueba del paquete en UE5.8.4 siguen sin verificar**. La ficha pública puede mostrar Buy now incluso cuando la cuenta ya lo tiene reclamado. No interpretar precio actual como pérdida de derechos adquiridos.

Otros recursos obtenidos públicamente mediante la **ficha Fab oficial** el 2026-11-15 (Free en ese momento; NO constan reclamados ni importados, formato/licencia real por paquete pendiente): [CoreCharacters Starter Kit](https://www.fab.com/listings/37eceaec-46df-4b44-8cb5-5e7ca92c7de8) (male/female rig Epic Skeleton, ropa modular, materiales/color, NoAI: No), [Mage Collection Samples](https://www.fab.com/listings/78f85a52-4132-4ce1-bb87-0c9e3a45a42a) (10 clips de magia, UE4/UE5 Mannequin, Allows AI: Yes), [Niagara Examples Pack](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600) (VFX y ejemplos de teleport, NoAI: No), [Elf - Male Wizard Fantasy Collection](https://www.fab.com/listings/919a44b8-3cd5-4fd0-ab68-61c6f48b42c3) (free, **add-on** para kits modulares elfos, disponibilidad autónoma sin verificar), y [Ninja Number 2 - Animated](https://www.fab.com/listings/67e00b63-d660-46dd-861c-6b144faaea4c) (masculino, 19 tiras, glb, Free, rig a validar). **No ofrecer** Hero Modular SciFi Character Pack, Modular Sci-fi Characters 03 o Sci Fi Soldier Male como gratuitos: sus fichas muestran Buy now/selector sin precio gratis acreditado.

**Respaldo fuera de Fab, CC0 ya identificado en auditoría:** [Quaternius Universal Base Characters Standard](https://quaternius.com/packs/universalbasecharacters.html), 6 humanoides, proporción superhero y male, rig humanoide, 13k tris promedio; **la edición Source de pago NO se utiliza**. Compatibilidad/parches reales pendiente.

La etiqueta NoAI restringe el envío de bytes del recurso a herramientas de IA generativa; Codex puede trabajar con **metadatos y nuestro propio C++** y usar el editor/operaciones deterministas autorizadas, sin entregar meshes/texturas NoAI como entrada a modelos. Fab Standard no autoriza publicar el paquete suelto en GitHub público; almacenar bajo la política correcta de distribución del juego.

## 2. Selección por héroe (único principal + respaldo)

| Héroe | Modelo principal | Respaldo | Edición propia MÍNIMA |
|---|---|---|---|
| **H02** genio cósmico masculino esmeralda | **CoreCharacters Starter Kit: male rigged + outfit modular** (Fab Free observado, todavía NO poseído) | Quaternius Standard CC0 male Superhero; o un masculino elegible del pack Adventure ya reclamado si encaja mejor visualmente | Cambiar prendas/módulos existentes, piel/cabello/materiales, hombros plateados, detalles dorados sencillos y cristal geométrico propio en el pecho; sin anillo, sin silueta de Linterna Verde ni modelado completo |
| **H03** teletransporte masculino índigo/negro | **Adventure Characters (Pack): elegir UNO de los cinco personajes reclamados**, retirar armas innecesarias, comparar rig con Manny y retarget | CoreCharacters male variante del mismo kit (si finalmente autorizado/importado); prueba alternativa Fab [Ninja Number 2](https://www.fab.com/listings/67e00b63-d660-46dd-861c-6b144faaea4c) Free si un personaje ágil encaja tras rehacer materiales | Silueta atlética, paleta negro/índigo con trazos azul-violeta, rostro/accesorios propios, sin parecido directo a Nightcrawler ni símbolos prestados |

**Corte creativo obligatorio:** no decidir solo mirando miniaturas. Crear **tres vistas por candidato** (frontal, tres cuartos, vuelo) con el rig de producción, escala de ciudad y materiales reales. Deben distinguirse por silueta, forma de moverse y poderes a distancia, incluso sin VFX. La identidad H02 y H03 prevalece sobre el acabado que trae el recurso.

**Fallback si no existe asset legal/accesible:** mantener **maniquí Manny de Third Person ya instalado** con materiales provisionales y animaciones diferentes, sin declarar H02/H03 artísticamente terminados. No crear cuerpos completos desde cero ni reclamar nada automáticamente.

## 3. VFX, animaciones y poderes: reutilizar antes de crear

- **Animación H02:** primera prueba con clips del mannequin/rig de Unreal más 1–3 clips de [Mage Collection Samples (Fab Free)](https://www.fab.com/listings/78f85a52-4132-4ce1-bb87-0c9e3a45a42a) cuando el usuario autorice acceso/descarga; retarget IK Rig de Unreal, nunca montar 200 clips ni un sistema paralelo. Reutilizar poses de manos para conjurar, lanzar y concentrar.
- **VFX H02/H03:** seleccionar 1–2 efectos de [Niagara Examples Pack (Fab Free)](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600) si autorizado, incluido ejemplo de teleportation pads, trails, hit dissolve y dat channels. Paleta esmeralda H02; violet/blue H03. Alternativa: Niagara y materiales del motor/proyecto. Evitar [Free Spline VFX](https://www.fab.com/listings/2b923e61-b02d-4cc9-bd0b-b067c9e6056e) como dependencia inicial, ya que el autor pide Niagara Fluids y distance fields; alto coste de setup.
- **Construcciones H02:** **usar meshes simples o existentes y un mismo material instanciado** para escudo, mano/pinza y mazo/puño. Son formas de energía, NO se modelan personajes/escudos complejos nuevos. El daño y agarre vienen de componentes existentes del juego y la geometría tiene colisión real donde importe. No confundir VFX con objeto físico.
- **Teletransporte H03:** reaprovechar cápsula/movimiento del héroe, pruebas de suelo/volumen, `TeleportTo` / sweep/overlap cuando proceda y la validación del destino (no dentro de sólidos, bajo mapa, fuera de región cargada o con carga incompatible). Efecto Niagara de entrada/salida solo comunica un movimiento ya validado. Para cadenas de blink, estado/cooldown configurable y seguro.
- **Arquitectura:** heredar/componer mínimo sobre H01: un controlador de cámara/input compartido, una interfaz de acciones con DataAssets/GameplayTags si resulta útil, un único Damage/Impact, un único Grab/Carry/Throw, un único reset/estado por región. No forzar GAS ni copiar todo Lyra a menos que un ensayo demuestre ahorros frente a componentes simples.
- **Animación / rig:** retarget al esqueleto de producción Manny mediante IK Rig/IK Retargeter nativos, ajustar manos y huesos especiales con Control Rig/TwoBoneIK cuando corresponda. Nunca presumir rig compatible porque la ficha solo pone UE5.

## 4. Objetivo jugable del sprint 6

**H02** debe poder moverse y explorar por sí mismo (su identidad de movimiento no es H01 con material verde):
- 1. Escudo esmeralda de energía con colisión/absorción real contra un ataque de robot/carro.
- 2. Mano/pinza construida que agarra y desplaza el **mismo carro u objeto** de H01, con límites y volumen válidos; no inventar una segunda física.
- 3. Mazo/puño prefabricado de energía que golpea muro o rival y usa Damage/Impact existentes.
- 4. Efecto de invocación legible y animaciones de gesto; no dependencia de generador de IA en runtime.

**H03** debe explorar solo con estilo de movimiento propio:
- 1. Blink corto direccional en suelo y aire a destino seguro; objetivo sugerido de ensayo 25–40 m según control y carga disponible.
- 2. Contra-blink evasivo breve que no concede invulnerabilidad infinita.
- 3. Golpe de reposicionamiento: se mueve primero a destino válido y después ataca con las reglas del juego, nunca daño desde un VFX.
- 4. Feedback de entrada/salida, animación de recuperación y regulación de cooldown en modo desafío; modo creativo configurable.

**Selector**: en pausa/spawn permite elegir H01/H02/H03; las tres opciones conservan identificación, input DualSense/teclado, mapa/minimapa, región y reglas de colisión. H02/H03 no bloquean el vuelo de H01 ni requieren cooperativo.

## 5. Plan de ejecución con hasta 5 agentes, una sola integración

A1 integra personaje/selector/compilación y tiene exclusivo el `Character` C++ y el mapa de juego. A2 audita/reutiliza arte H02 en laboratorio; A3 audita/reutiliza arte H03 en laboratorio; A4 construye las **habilidades compartidas y checks de destino** en componentes aislados; A5 QA de rig/animación, desempeño y licencias. Los auditores de activos NoAI operan sin enviar archivos protegidos a un modelo generativo.

**No iniciar nada mientras sprint 1 esté en ejecución.** Al comenzar sprint 6: inventario local/edición/licencia → aprobación de reclamación/descarga de **solo** los recursos elegidos → ensayos aislados Manny/retarget/material/coste → dos héroes con sus poderes sobre el mismo auto/robot/muro → integración única → Win64 DX12 → pruebas DualSense y visibilidad de poderes → valoración humana.

## 6. Gates reales (no completados hoy)

- H02 y H03 tienen **modelos preexistentes con procedencia, licencia exacta, versión y archivos reales** registrables; no se hizo modelo corporal entero. Si no se autorizó obtención, conservar marcador provisional explícito.
- 20 transiciones locomoción/salto/vuelo por personaje, IK y materiales sin deformaciones de rig, cloth ni manos; captura frontal, perfil y tres cuartos en DX12.
- 30 escudos, 30 construcciones de agarre, 30 golpes de energía H02 con hitbox real, misma pieza carro/muro/robot y sin daño duplicado.
- 50 blinks H03 (válidos e inválidos), incluyendo suelo, aire, ciudad/bioma, frontera de celda cargada, región no cargada, y contra-blink; nada de quedarse en sólido o hacerse invisible para siempre.
- Reinicio con habilidad activa, bloqueo en menú y cambio de personaje; sin actores duplicados ni pérdida de estado en otra ciudad.
- Sesión humana independiente de 10–15 minutos por héroe, puntúa **control/identidad/potencia/ganas de repetir** (1–5, meta orientativa ≥4 en cada eje). Registrar resultados reales, no afirmar aceptación por screenshots de Fab.
- Perfil con ambos assets a 1080p en RTX 4070 Laptop 8 GB: GPU/CPU, frame time p95/p99, VRAM, efectos sobrepuestos y streaming.

**Fuera de sprint 6:** modelado desde cero de trajes completos, creación de un rig humanoide nuevo, editores de personajes, cientos de habilidades, combate multijugador, H02 como copia de Linterna Verde, H03 como copia de Nightcrawler, Kaiju del sprint 7 y espacio del sprint 8.
