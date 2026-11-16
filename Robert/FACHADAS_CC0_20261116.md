# Fachadas modulares CC0: recurso ya utilizado, no novedad

Corrección documental 2026-11-16 UTC. La revisión inicial lo presentó como candidato nuevo tras buscar solo parte del catálogo. **Eso era incompleto: PR #16 ya incorporó este mismo recurso y S2 hereda ese commit.** Esta auditoría no descargó, importó ni ejecutó assets; no debe confundirse con ausencia de importaciones anteriores.

## Recurso y licencia documentada
[Modular Urban Apartments Facade, Poly Haven](https://polyhaven.com/a/modular_urban_apartments_facade): recurso gratuito RD$0, identificado en la revisión inicial mediante ficha oficial y [licencia CC0](https://polyhaven.com/license). CC0 permite uso personal/comercial, modificación y redistribución; no se identificó restricción NoAI para el asset. No cubre logotipos, textos o renders promocionales del sitio. Esta auditoría de repositorio no verificó de nuevo el ZIP original ni su hash.

## Evidencia del uso previo
- [PR #16](https://github.com/meliodr/juego-/pull/16), abierto y borrador, head `32001f504fcfa4681c5f2ba8a1c46754573eddd4`, describe cuatro módulos para 28 edificios.
- [Documentación en ese commit](https://github.com/meliodr/juego-/blob/32001f504fcfa4681c5f2ba8a1c46754573eddd4/docs/IB_C00_VALIDACION.md), sección «Fachadas urbanas»: glTF 1K, cuatro grupos de instancias jerárquicas, colisiones conservadas en los volúmenes del circuito y cristales convertidos a opacos oscuros.
- Los archivos del PR incluyen BayWide, BayDouble, Entry y Cornice, materiales/texturas en `ImpactBeyond/Content/IB_Donors/UrbanFacades/ImpactApartments/`, el mapa y `Tools/Apply-RealisticFacades.py`. No es solo una propuesta escrita.
- [Comparación PR16 → S2](https://github.com/meliodr/juego-/compare/32001f504fcfa4681c5f2ba8a1c46754573eddd4...1d01db54b63250ed0aacb0704a5ac16290324e32): ahead 4, behind 0, merge-base igual al head de PR16. Por tanto el head de [PR #19](https://github.com/meliodr/juego-/pull/19) hereda PR16. La comparación muestra además modificaciones posteriores de sus cinco materiales.
- [Script conservado en S2](https://github.com/meliodr/juego-/blob/1d01db54b63250ed0aacb0704a5ac16290324e32/Tools/Apply-RealisticFacades.py): reutiliza assets guardados si existen; solo pide fuente glTF mediante IB_POLY_FACADE si no existen.

## Qué está y qué no está demostrado
El cuerpo de PR16 declara capturas DX12, paquete Win64 abierto por Steam y regresión de 20 ciclos del carro aprobada. Son resultados reportados por su autor, no reproducidos por esta auditoría. Su documento mantiene aceptación jugable pendiente y otros apartados de validación pendientes; esa diferencia debe reconciliarse antes de emitir un aprobado global.

PR16 y PR19 seguían abiertos, sin fusionar, durante esta revisión. La comparación PR16 → main es divergente (ahead 2 / behind 24), por lo que no acredita integración de PR16 en main. Herencia Git en S2 no demuestra que la build instalada de Steam esté actualizada, ni valida destrucción, recorrido continuo, colisión de fachada detallada o carga a máxima velocidad. No se inspeccionó el checkout local del integrador ni se ocupó Unreal.

## Decisión corregida
**REUTILIZAR Y AUDITAR LO EXISTENTE. NO DESCARGAR NI IMPORTAR DE NUEVO POR ESTE HALLAZGO.** Retirar la etiqueta de novedad y el ensayo de importación duplicado. Coordinar con S2 sobre sus assets ya presentes, reconciliar evidencia y comprobar su estado en la candidata integrada. Cualquier mejora de variedad, materiales, colisiones o destrucción requiere un encargo separado y pruebas comparables; no es un plugin de destrucción.

Este PR solo corrige investigación documental en una rama propia. No modifica ramas ajenas, main, assets, mapa, permisos ni trabajos activos.
