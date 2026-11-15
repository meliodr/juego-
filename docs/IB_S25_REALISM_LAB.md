# S2.5 R2 — laboratorio aislado de arte

**Una manzana y parque refinados; cinco vistas comparables verificadas; realismo NO aceptado.** [Capturas y comparación](IB_S25_VISUAL_REVIEW.md). [Diagnósticos, límites y reproducción R2](IB_S25_R2_REVIEW.md).

Rama `prototype/ib-s25-realism-lab`, [PR draft #25](https://github.com/meliodr/juego-/pull/25), base `prototype/ib-sprint02` congelada en `1d01db54b63250ed0aacb0704a5ac16290324e32`. No hay merge, integración global ni cambios finales en C00, Config, código compartido o proyecto. RD$0.

## Estado comprobado

- Mapa `/Game/IB_S25/Lvl_IB_S25_RealismLab`: 86 actores propios, 1.438 instancias de fachada; siete edificios en las tres manzanas existentes, más plaza. La manzana suroeste pasa de dos edificios repetidos a tres fachadas diferenciadas, sin ampliar el perímetro urbano. Seis coches y dieciséis árboles existentes.
- Parapetos/equipos, plantas bajas con marquesinas, bancos, luminarias, bordillos y cuatro pasos peatonales. Parque con tierra PBR, caminos y alcorques. Soporte de horizonte provisional sin colisión; no es un paisaje terminado.
- Cinco capturas 1920×1080 de las mismas cámaras de R1; pose y rotación verificadas con tolerancia 1e-6 y FOV idéntico. EV14 y sol -32/-35, 65.000 lux. SkyLight declarado de 1 a 1,25. Una vista cenital adicional comprueba los cruces y no reemplaza ninguna cámara comparativa.
- Diagnóstico A/B: las diagonales de ventanas desaparecen al restituir el cristal donante, sin modificar mallas. El interior mapping defectuoso queda suspendido en componentes del lab. Segundo A/B detecta manchas del material nuevo; se conserva el grafo Substrate glTF probado mediante instancias propias.
- 16 pruebas ligeras PASS. Repetición real del refinador en Unreal: mismo estado numérico, 86 actores/1.438 instancias, [hash y resultado](IB_S25_R2_VALIDATION.json).
- Sesión DX12 visible en UE 5.8.4, audio inicializado. PID propio 6672 cerrado con código 0 a 17:28:50 UTC. Chequeo posterior sin procesos UE/build y mutex libre. Config autogenerada conservada localmente y restaurada a HEAD.

## Reproducción y licencias

Los scripts, OBJ propios, mapa y renders son públicos. Los uassets importados/generados, texturas fuente, logs y respaldos siguen locales. **Un clone no basta**: reconstruir primero el laboratorio base en otro checkout aislado de la base congelada con los donantes autorizados, sin copiar el mapa R2; después aplicar el refinador. El autor base rechaza deliberadamente sobrescribir el marcador R2.

Preparación base: [instrucciones R1 conservadas](IB_S25_R1_LAB.md). Refinamiento: `IBS25_soil.py` → `IBS25_refine_recipe.py` → en Editor `IBS25_refine.py` → `IBS25_refine_validate.py` → `IBS25_review.py`. Coordinar siempre proceso/mutex/SSD antes de abrir Unreal. Los diagnósticos A/B corresponden a estados intermedios; sus materiales retirados ya no están aplicados en R2.

| Uso | Principal | Respaldo o límite |
|---|---|---|
| Fachada | Cuatro módulos Urban Apartments de Poly Haven ya importados; instancias Substrate locales | City Sample Buildings no incorporado. Persiste repetición de ventanas. |
| Coches | ConceptCar y materiales existentes, reparados en copias del lab | Un solo modelo; ninguna fuente UE-Only nueva publicada. |
| Árbol | Island Tree 01 CC0, glTF/bin + nueve JPG 1K; 66.337.268 bytes | Follaje lejano escaso y coste sin medir; Shrub 03 no importado. |
| Asfalto/acera | Asphalt 02 y Concrete Pavement CC0, seis mapas 2K; 44.243.757 bytes | Fuentes locales excluidas del Git. |
| Tierra | Brown Mud Leaves 01 CC0, tres mapas 1K; 7.959.346 bytes | [Recibo/hash/licencia](IB_S25_R2_SOIL_SOURCES.json); empaquetado Linux sin probar. |
| Ventanas | Cristal donante original | Interior mapping pendiente de revisar aplicación/UV por paño. |

[Licencia Poly Haven](https://polyhaven.com/license). [Evidencia actual](IB_S25_EVIDENCE.json). [Evidencia R1](IB_S25_R1_EVIDENCE.json).

No se ejecutaron benchmark A/B, cook, paquete, recorrido jugable ni QA de colisiones. No hay cifras de FPS, p95/p99 ni VRAM. La revisión visual del implementador no sustituye la aceptación de Jarry ni acredita el objetivo City Sample.
