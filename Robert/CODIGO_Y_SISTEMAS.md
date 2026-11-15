# Código y sistemas reutilizables

Corte 2026-11-15. **Investigado, no integrado ni compilado.** Los candidatos no cambian la selección activa.

| Candidato | Parte aprovechable | Evidencia y licencia | Límite / decisión |
|---|---|---|---|
| [PBCharacterMovement](https://github.com/ProjectBorealis/PBCharacterMovement) | Aceleración, conservación de impulso, strafe y rampas para estudiar superspeed | [LICENSE MIT](https://raw.githubusercontent.com/ProjectBorealis/PBCharacterMovement/main/LICENSE); [descriptor](https://raw.githubusercontent.com/ProjectBorealis/PBCharacterMovement/main/PBCharacterMovement.uplugin) Runtime v3.1.0, CanContainContent=false | Referencia FPS, no controlador completo de tercera persona. Autor declara Win64 UE5.5 y recompilación para otras versiones. Sprint/escaleras tienen partes específicas no incluidas. No licencia personajes ni arte Half-Life. Apto para ensayo aislado de código, UE5.8 sin validar. |
| [ALS Refactored](https://github.com/Sixze/ALS-Refactored) | Locomoción, cámara, pies y mantling | [MIT](https://raw.githubusercontent.com/Sixze/ALS-Refactored/main/LICENSE.md). Tabla del autor vincula ALS 4.13–4.17 con UE5.3–5.7; pruebas principalmente Windows | Alternativa deliberada, no sumar ALS y GASP completos. Sin vuelo/destrucción. UE5.8 pendiente. |
| [ManOfSteel](https://github.com/ProdiG66/ManOfSteel) | Subconjunto de vuelo, cámara e inclinación | Código declarado MIT, prototipo UE5.3; consultar auditoría existente por archivo | El README advierte assets incompletos por cuota Git LFS. Un puntero no es un archivo disponible. No copiar arte de Superman ni asumir permisos de clips. No base lista. |

## Soluciones del motor que evitan dependencias innecesarias
- [Physics Handle y Physical Animation](https://dev.epicgames.com/documentation/unreal-engine/physics-components-in-unreal-engine): estudiar agarres físicos de objetos y mezcla de animación/simulación. Hay que probar colisiones de la carga y restaurar estados al soltar.
- [Chaos / Geometry Collections](https://dev.epicgames.com/documentation/unreal-engine/destruction-quick-start): preparar una pared, fracturar y comprobar que el hueco es transitable. Un kit urbano no llega automáticamente destruible.
- Niagara puede presentar polvo, energía y escombros cosméticos; no debe ser la fuente de verdad de daño ni sustituir colisión interactiva.

## Reutilización ya auditada en docs
REPIRK, PlotBuilder, FastestManAlive y Ephemera ya están inventariados. Consultar sus fichas existentes antes de abrir otra búsqueda. Extraer un componente mínimo permitido, no importar un framework entero.

## Puerta de aceptación
Fijar commit, conservar avisos MIT, comprobar bytes y dependencias, construir mapa pequeño y empaquetar Windows/UE real. Ningún candidato está marcado como funcional en el equipo del usuario.
