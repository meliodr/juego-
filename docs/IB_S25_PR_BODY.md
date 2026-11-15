S2.5 ya genera y guarda el mapa aislado `/Game/IB_S25/Lvl_IB_S25_RealismLab`: seis edificios, tres manzanas, plaza/parque, seis coches y dieciséis árboles reales. Reutiliza fachadas, ConceptCar e interior mapping existentes con superficies PBR propias.

**[Ver cinco capturas reales y antes/después](https://github.com/meliodr/juego-/blob/prototype/ib-s25-realism-lab/docs/IB_S25_VISUAL_REVIEW.md).** El antes corresponde a esta misma sesión de laboratorio, no al mapa principal. Se corrigieron azoteas reflejadas por importación OBJ, slots/referencias rotas de pintura de coches, exposición de luz cacheada y el encuadre de esquina.

Validación: imports y guardado ejecutados en UE 5.8.4/DX12/RTX 4070 Laptop; acceso visual operativo. Cinco PNG 1920×1080 con pose/FOV/sol anotados. Autoría + lookdev repetidos: 79 actores y 1.288 instancias de fachada, mismo hash de estado. 11 pruebas ligeras aprobadas. Sesión propia cerrada con código 0 y turno liberado.

**Draft; realismo NO aceptado.** Persisten fachadas repetitivas, parque vacío, horizonte sin tratamiento y LOD vegetal costoso. Pendientes: colisiones/jugabilidad, comparación visual directa con City Sample, A/B a 1080p con FPS/p95/p99/VRAM y aceptación humana. No hay integración global, cook ni paquete.

Solo cambios bajo `ImpactBeyond/Content/IB_S25`, `Tools/IBS25_*` y `docs/IB_S25_*`. Se publica el mapa propio, scripts, geometría propia y renders. Los uassets importados, material de plantilla reparado, texturas, logs y respaldos siguen locales; el clone requiere reconstruir dependencias documentadas. Mapa principal, Config y código compartido conservan la base. RD$0.

Base congelada `1d01db54b63250ed0aacb0704a5ac16290324e32`; destino `prototype/ib-sprint02`. Sin merge. Evidencia detallada: `docs/IB_S25_EVIDENCE.json`, `docs/IB_S25_REALISM_LAB.md` y `docs/IB_S25_VALIDATION.json`.
