# S4 — Inventario y procedencia

2026-11-15. No se descargó, reclamó, compró ni instaló ningún asset/plugin. No se enviaron bytes de arte a herramientas generativas. El PR contiene código y texto propio; no nuevos paquetes de terceros.

| Recurso | Evidencia inspeccionada | Uso / decisión |
|---|---|---|
| CMC, PhysicsHandle, ProjectileMovement, Enhanced Input, colisión/Chaos de UE | Headers locales de UE5.8.4 y [API PhysicsHandle de Epic](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UPhysicsHandleComponent) | Reutilizados por referencia desde módulos ya presentes. No se redistribuye fuente del motor |
| Manny Simple / ABP_Unarmed / montage de pistola de plantilla | En un checkout preexistente se verificaron nombres/tamaños: 15.825.101 bytes, 385.911 bytes y 10.119 bytes respectivamente; sin cargar ni mostrar contenido al modelo | Donantes existentes UE, humano provisional. El autor del mapa exige cargar Manny/AnimBP; no afirma uniforme, aim/fire clips integrados ni licencia nueva. No copia binarios al PR |
| City Sample Crowds | Selección/catálogo del repo, sin descarga ni validación de bytes del paquete | HOLD; no se deduce propiedad/ausencia general de la biblioteca. Manny es fallback explícito |
| Tanque T-72 de Nekis5_3D | [Ficha original consultada](https://nekis5-3d.itch.io/t-72-3d-model-free): autor permite uso personal/comercial y señala ausencia de texturas | No descargado. Ese permiso no demuestra autorización de redistribuir FBX fuente en repo público. HOLD para malla final; caja rotulada de launcher en laboratorio |
| Scout Bot / Sci Fi Robot 13 | [Scout](https://www.fab.com/listings/9c57ad32-3d3e-45ed-91e8-b32fc3af2ac0) / [Robot 13](https://www.fab.com/listings/2b68ee14-cec8-4347-a5fa-5ba870160467), catálogo auditado del repo | Bytes/licencia en biblioteca/compatibilidad no verificados en esta sesión. HOLD; fixture de cuerpo/cañón con cubos del motor, explícitamente provisional |
| Niagara Examples Pack / Free Magic Niagara | [Epic](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600) / [alternativa](https://www.fab.com/listings/d0fe50c4-6ebe-40d5-b78a-56960832f49e), documentación del encargo | No adquiridos/cargados. Hooks Blueprint sin dependencia C++ nueva; beam/impactos geométricos provisionales. Respetar NoAI al revisar/importar |
| Formas básicas de Engine | Referencias `/Engine/BasicShapes/Cube`, Sphere, Cylinder | Fixtures técnicos, no modelado de tanque/policía final. Bytes existentes del motor, no nuevos binarios en Git |
| Audio procedural | Generado en código original con envolvente corta, ruido determinista y tono por material | Sin sample externo ni API/paquete. Implementado sin escucha; no se presenta como sonido final |

La búsqueda por nombres Tank/T-72/Scout/Robot/Niagara en el Content del checkout local inspeccionado no encontró candidatos. Es una búsqueda acotada, **no** una auditoría completa de Mi biblioteca/VaultCache/discos. No se afirma que el usuario no posea esos assets. Ninguna malla/no-AI fue enviada al modelo para análisis.

Para sustituir un proxy: documentar paquete/URL, edición/licencia exacta, bytes locales, versión, escala/collider/rig, ruta importada permitida y prueba. Mantener assets restringidos fuera de Git público. Que un modelo sea gratuito no valida redistribución fuente, partes desmontables, animación ni fotorealismo.
