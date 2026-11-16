# Fachadas modulares CC0: candidato acotado

Verificado documentalmente 2026-11-16 UTC. **No descargado, importado ni probado.** No cambia la selección activa ni interfiere con integración.

## Hallazgo
[Modular Urban Apartments Facade, Poly Haven](https://polyhaven.com/a/modular_urban_apartments_facade): modelo gratuito, coste de descarga RD$0. La ficha ofrece módulos residenciales, ventanas, puertas y esquinas, con FBX, glTF, USD y Blend; resoluciones 1K a 8K. La selección visible 4K/Blend mostraba 663,38 MB; no extrapolar ese tamaño a los demás formatos.

**Valor propuesto:** ensayo futuro de una sola fachada para reducir repetición y aumentar detalle arquitectónico, no reemplazar la ciudad completa. Poly Haven ya figuraba como fuente general; este modelo concreto no apareció en PR #1 ni en Robert/RECURSOS_GRATUITOS.md, CODIGO_Y_SISTEMAS.md, FUENTES_ADICIONALES.md, docs/REUSE_CATALOG.md, CITY_SOLAR_TELEPORT_V04.md o FREE_RESOURCE_AUDIT_V05.md revisados.

## Licencia y edición
[Licencia oficial](https://polyhaven.com/license): los assets del catálogo son CC0; permite uso personal/comercial, modificación y redistribución, sin atribución obligatoria. No se identificó restricción NoAI para este asset CC0. Esto no licencia logotipos, textos o renders promocionales del sitio. Registrar el paquete exacto y su licencia cuando se descargue; esta revisión no verificó el contenido del ZIP ni fijó un hash del archivo.

## Compatibilidad y límites
La ficha ofrece formatos de intercambio, **no una certificación UE5.8/Windows**. Importación, escala, material, colisión, LOD, memoria y empaquetado están pendientes. Tampoco acredita interiores, geometría fracturable, colisiones transitables o rendimiento a máxima velocidad. No es un plugin de destrucción.

## Decisión
RESERVA PARA ENSAYO AISLADO, no dependencia aprobada. Cuando termine la integración activa, probar 1–3 módulos a resolución mínima útil, conservar evidencia antes/después con cámara y luz equivalentes, medir memoria/draw calls y verificar colisión. Preparar destrucción por separado si se acepta. Rechazar si mejora solo capturas pero perjudica traversal o presupuesto. No se subieron archivos de terceros; este PR contiene únicamente investigación documental.
