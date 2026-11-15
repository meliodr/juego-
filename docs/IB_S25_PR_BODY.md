El primer plano del Sprint 2 sigue usando proxies de carretera, árboles y coches. Este laboratorio prepara una autoría aislada que reutiliza los módulos Poly Haven, ConceptCar e interior mapping existentes, añade superficies biseladas/UV a escala física y exige mallas vegetales reales en vez de sustituirlas por primitivas.

Incluye receta determinista de 6 edificios/3 manzanas más plaza y parque, generador Unreal con preflight y protección de autoría, seis texturas CC0 2K descargadas y verificadas (solo manifiesto en Git), cinco cámaras, captura secuencial y protocolo/launcher A/B DX12 con analizador de métricas. El mapa principal conserva exactamente el blob de la base congelada. Todos los cambios están en `Tools/IBS25_*` y `docs/IB_S25_*`.

**Draft deliberado: CÓDIGO, no realismo entregado.** No hay `.umap`, screenshots ni benchmark de Unreal en esta pasada. Faltan turno exclusivo coordinado de editor, árbol autorizado, imports y validación de la API/colisiones. No se abrió editor, cook ni build pesada; no se publicaron assets Fab/UE-Only nuevos ni se cambió Config/proyecto/código compartido. RD$0.

Validación ejecutada: 11 pruebas Python PASS (idempotencia/rechazo de sobrescritura, geometría/UV, pasillo/plaza, cuantiles y hashes/resolución de bytes reales), sintaxis de seis Python y PowerShell PASS, launcher en modo comando sin arrancar Unreal. Estas pruebas no certifican rendimiento ni calidad visual. Evidencia y reproducción: `docs/IB_S25_REALISM_LAB.md`, `docs/IB_S25_EVIDENCE.json`.

Base: `1d01db54b63250ed0aacb0704a5ac16290324e32`; destino `prototype/ib-sprint02`. Sin merge automático ni integración global.
