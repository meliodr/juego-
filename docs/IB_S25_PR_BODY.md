El laboratorio aislado ahora concentra el detalle en una manzana y su parque: tres fachadas distinguibles y plantas bajas, bordillos/cuatro pasos peatonales/mobiliario, parapetos y equipos de cubierta, tierra/caminos/alcorques y soporte de horizonte. No amplía la ciudad ni modifica el mapa principal.

**[Ver las mismas cinco cámaras R1/R2 y una toma cenital de apoyo](https://github.com/meliodr/juego-/blob/prototype/ib-s25-realism-lab/docs/IB_S25_VISUAL_REVIEW.md).** Los A/B localizan las diagonales de ventanas en el material aplicado y las manchas de fachada en el shader intermedio. Se conservan cristal y grafo Substrate donantes mediante componentes/instancias propias. Cámaras/FOV, EV14 y sol iguales; SkyLight de 1 a 1,25 declarado.

Validado en UE 5.8.4/DX12 visible: imports, guardado, cinco PNG 1920×1080 inspeccionados y repetición del refinador sin cambios de estado (86 actores, 1.438 instancias). 16 pruebas ligeras PASS. Sesión propia cerrada con código0 y turno libre; Config autogenerada respaldada y restaurada.

**Draft; realismo NO aceptado.** Siguen pendientes QA humano, densidad/coste de follaje lejano, interior mapping, paisaje fuera del lab, jugabilidad/colisiones y benchmark A/B FPS/p95/p99/VRAM. No hay cook, paquete ni integración global.

Solo rutas S2.5: mapa, scripts, OBJ propios, documentación y renders. Fuentes CC0/uassets importados, logs y respaldos siguen locales; un clone requiere reconstruir dependencias. Tierra nueva: tres mapas Poly Haven CC0 1K, 7.959.346 bytes; RD$0. Base congelada `1d01db54b63250ed0aacb0704a5ac16290324e32`, destino `prototype/ib-sprint02`. Sin merge.

[Evidencia](https://github.com/meliodr/juego-/blob/prototype/ib-s25-realism-lab/docs/IB_S25_EVIDENCE.json) · [Reproducción/licencias/limitaciones](https://github.com/meliodr/juego-/blob/prototype/ib-s25-realism-lab/docs/IB_S25_REALISM_LAB.md).
