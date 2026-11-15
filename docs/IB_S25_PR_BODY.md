El primer plano del Sprint 2 utiliza proxies. Este laboratorio prepara una escena aislada de seis edificios, tres manzanas, plaza y parque reutilizando fachadas, ConceptCar e interior mapping existentes. Añade receta determinista, geometría propia con UV a escala física, materiales PBR, cinco cámaras y scripts de captura y benchmark A/B.

**Draft: implementación parcial, sin integración jugable ni realismo validado.** Todavía no existe el mapa `.umap`, ni hay screenshots o métricas del laboratorio. La autoría/importación, colisiones, idempotencia en Unreal, reducción de vegetación y aceptación visual siguen pendientes.

Validación real: 11 pruebas Python aprobadas; sintaxis Python/PowerShell y geometría/hashes comprobados. UE 5.8.4 arrancó con DX12 en RTX 4070 Laptop, audio y cola Python. `ExecutePythonScript` cerró el Editor al terminar; se preparó la corrección nativa de keep-alive, aún sin ejecutar. La autorización de acceso visual expiró. El turno y el mutex quedaron libres.

Solo se añaden scripts, OBJ propios y documentación bajo `Tools/IBS25_*` y `docs/IB_S25_*`. Las seis texturas CC0 2K (44.243.757 bytes) y el árbol CC0 (66.337.268 bytes) permanecen locales; no se publican bytes nuevos Fab/UE-Only, logs ni respaldos. Un clone no basta para generar el mapa: requiere las dependencias locales documentadas y completar la validación. El mapa principal, Config y código compartido conservan la base. RD$0.

Base congelada: `1d01db54b63250ed0aacb0704a5ac16290324e32`; destino `prototype/ib-sprint02`. Sin merge automático. Reproducción, inventario y límites: `docs/IB_S25_REALISM_LAB.md` y `docs/IB_S25_EVIDENCE.json`.
