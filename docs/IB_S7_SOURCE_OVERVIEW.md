# Candidata integrada S1–S7: fuentes y reproducción

Versión jugable integrada con C++ `fa75f7f`, autoría `730b39f` y configuración/validador `17d7f91`. Game, autoría, paquete final `s7-package-20261116T060244Z`, doce suites funcionales del ejecutable, cuatro del paquete DX12 y perfil confirmatorio aprobados. El [manifiesto](IB_S7_PUBLIC_EXPORT.json) fija revisiones y hashes; [resultados y límites](IB_S7_CLOSURE_REVIEW.md). Solo queda confirmar el arranque visible Steam tras cerrar un aviso Windows de QA; control físico y evaluación humana son comprobaciones separadas.

## Entrada y responsabilidades

| Código | Responsabilidad |
|---|---|
| IB_Integration/Playable/IBPlayCharacter | Host de la partida: movimiento radial, controles, presentación, selector y rueda |
| IB_Integration/Playable/IBPlaySession | Sesión, identidad de receptores, impacto canónico, regiones y persistencia |
| IB_Integration/Playable/IBPlayCombat, IBPlayEncounter | Acciones, encuentros y atribución compartidos |
| IB_S3 | Manifiesto del planeta, ciudades instanciadas, residencia, mapa/minimapa y marco radial |
| IB_S35 | Perfiles de carga, trazas y golpes con timing; lab anterior conservado como opt-in |
| IB_S4 | Respuesta a daño, poderes y objetivos preparados con colisión |
| IB_S5 | Tiempo solar global, civiles, incidente y consecuencias |
| IB_S6/Wave8Heroes + IB_S6H | Políticas y adaptador de H01/H02/H03, selector, energía y habilidades |
| IB_S7/Wave8Threats | Robot/kaiju, partes funcionales, persistencia y rueda de poderes |
| IB_Integration/ImpactContract | Traducción explícita de IDs, unidades, daño y deduplicación |

El host de producción conserva los componentes compartidos; las clases de laboratorio permiten pruebas acotadas. Tener ambas clases en el repositorio no implica que dos personajes o dos receptores de daño estén activos en la partida. Las pruebas UE están incluidas junto a sus sistemas y en Playable; su presencia no equivale a haberlas ejecutado.

## Dependencias locales

No se incorporan assets nuevos de Content ni historial privado de arte. Los recursos que ya estaban en la base pública se mantienen. El autor debe disponer legalmente de los donantes privados y de los outputs de S3/S2.5 antes de generar la partida.

1. Compilar el proyecto con la instalación Windows/UE del proyecto. Solo el integrador ejecuta Unreal/UBT/cook; los CI públicos no redistribuyen ni instalan Unreal.
2. Generar el mapa/manifiesto/mallas originales S3 con `Tools/IBS3_AuthorLab.py` en una sesión exclusiva. Proveer los materiales y árbol S2.5 según [su receta](IB_S25_SOURCE_REBUILD.md), conservando fachadas y donantes existentes.
3. Proveer Adventure Characters, Robot 13 y Bull con sus rutas originales locales. [Procedencia](IB_S7_ASSET_PROVENANCE.md). No publicar estos paquetes ni derivados privados.
4. Configurar `IB_PLAY_DONOR_CONTENT` con el Content del proyecto donante autorizado. El script no tiene fallback personal. Ejecutar `Tools/IBPlay_AuthorProduction.py` en Unreal ya compilado; produce derivados y el mapa bajo `/Game/IB_Playable`, ignorados por Git. También aplica únicamente los flags de instancing a los once materiales propios/CC0 del mundo y árbol enumerados por la [receta de reconstrucción](IB_S25_SOURCE_REBUILD.md). No modifica padres, texturas ni recursos Fab; los `.uasset` locales modificados no se publican.
5. Validar referencias, colisión, animación, materiales y cocinar el paquete. `DefaultEngine.ini` apunta a `/Game/IB_Playable/Lvl_IB_Playable`: **no arrancará esa partida en un clone que todavía carece de ese mapa generado**. El arte no queda implícitamente disponible por compilar el C++. La configuración `17d7f91` incluye explícitamente `/Game/IB_S25/Materials` y `/Game/IB_S25/Vegetation/S25_IslandTree/StaticMeshes` en AlwaysCook porque S3 los carga desde C++; el validador del ejecutable rechaza referencias ausentes o uso de materiales de reserva.

La selección de la plantilla base y las herramientas ya existentes no cambia su licencia. El procesamiento local de donantes no envía mallas, texturas o bytes NoAI al modelo. Los informes públicos incluyen solo código, rutas de paquetes, nombres y recuentos revisados.

## Validación reproducible sin Unreal

```powershell
python -B -m unittest discover -s Tools/tests -p 'test_IB*.py' -v
python -B Tools/IBCI_PublicSource.py --base origin/main
python -B Tools/IBCI_PublicSource.py --syntax
python -B Tools/IBS25_test.py
python -B Tools/IBS25_refine_test.py
python -B Tools/IBS3_TestAuthoring.py ../qa-s3-geometry
python -B Tools/IBCI_Native.py --output ../qa-native
```

El gate de publicación inspecciona todo commit introducido, no solo el diff final. La suite nativa exige 13 fixtures y compila C++17 con MSVC en Windows; los outputs deben estar fuera del checkout. El test de texturas locales S2.5 puede omitirse explícitamente cuando no se han provisto bytes; sus recetas y recibos sí se prueban.

Un PASS nativo cubre políticas y matemáticas. No valida UObject/CMC, controles de Unreal, DX12/audio, rendimiento, paquete ni sensación. El informe final debe distinguir las pruebas anteriores de la candidata `fa75f7f` y de la versión exacta del paquete probado. Los scripts públicos no incluyen bytes de los recursos privados ni certifican su aspecto en la partida.

## Arranque de la entrega local

Usar el acceso ImpactBeyond existente en Steam con **Activar Steam Input**. Argumentos recomendados: `-dx12 -windowed -ResX=1920 -ResY=1080 -UDPMESSAGING_TRANSPORT_ENABLE=0`. El último argumento desactiva el transporte UDP de mensajería del motor, innecesario para esta partida offline; no concede permisos de red ni modifica el firewall. La configuración de Steam Input ya fue comprobada en UI. El primer arranque visible está pendiente de cerrar un aviso Windows de una sesión QA anterior.
