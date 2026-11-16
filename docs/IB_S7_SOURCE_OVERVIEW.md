# Candidata integrada S1–S7: fuentes y reproducción

La fuente C++ y las herramientas se exportan del snapshot `acf4156` a una rama pública limpia. **Es una candidata en validación, no una declaración de S7 jugable aceptado.** El integrador reportó Editor aprobado para `acf4156` y Game aprobado para `9ca77e1`. Los cambios posteriores de autoría/QA y el paquete se siguen por separado; esta exportación aún es intermedia. El [manifiesto de exportación](IB_S7_PUBLIC_EXPORT.json) fija los archivos y hashes; el [plan activo](PLAN_ACTIVO.md) fija lo que falta demostrar.

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
4. Configurar `IB_PLAY_DONOR_CONTENT` con el Content del proyecto donante autorizado. El script no tiene fallback personal. Ejecutar `Tools/IBPlay_AuthorProduction.py` en Unreal ya compilado; produce derivados y el mapa bajo `/Game/IB_Playable`, ignorados por Git.
5. Validar referencias, colisión, animación, materiales y cocinar el paquete. `DefaultEngine.ini` apunta a `/Game/IB_Playable/Lvl_IB_Playable`: **no arrancará esa partida en un clone que todavía carece de ese mapa generado**. El arte no queda implícitamente disponible por compilar el C++.

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

Un PASS nativo cubre políticas y matemáticas. No valida UObject/CMC, controles de Unreal, DX12/audio, rendimiento, paquete ni sensación. El informe final debe distinguir la compilación anterior `4f95872`, la candidata `acf4156`, sus diferencias respecto a `9ca77e1` y los cambios posteriores de autoría y la versión exacta del paquete probado.
