# Primer incremento verificado hacia main

Peticion vigente: integrar incrementalmente cambios comprobados en main. No implica
aceptacion jugada, actualizacion de Steam ni publicacion indiscriminada de laboratorios.

## Alcance y procedencia

Base main: `aaad4491ca3610ba22d12f764d1ed0e35ca0356f`.
Se reutilizan sin modificaciones estos tres archivos de PR #24, commit
`65322aabc943ab22f430b3e6af1adf35fa069230`:

- `Tools/IBAudit_ValidateLabs.py`
- `Tools/tests/test_IBAudit_ValidateLabs.py`
- `Tools/tests/test_IBAudit_IntegrationGates.py`

Se incorpora solamente ese gate Git, sus tests, este registro y CI de tests.
No se fusiona PR #24 completo: su base S2 arrastraria el prototipo y sus dependencias.
Los PR originales conservan sus ramas, historia y estado.
CI tiene permiso contents:read, no usa secretos ni Unreal y no instala dependencias.
Comprueba fixtures de Git, no compilacion de los laboratorios.

## Evidencia reproducida

- Windows, Git 2.55.0.windows.3, Python 3.13.15: 8/8 unit tests PASS,
  `python -m unittest discover -s Tools/tests -p 'test_IBAudit*.py' -v`.
- Primer intento: ERROR por acceso al temporal del sandbox antes de crear los repos
  fixture. Repeticion con TEMP/TMP/TMPDIR en un temporal propio: PASS (5.327 s).
- `python Tools/IBAudit_ValidateLabs.py --repo . --require-all`: STATIC_PASS en
  S2.5 (79 rutas), S3 (30), S3.5 (30) y S4 (27). Esto solo verifica ancestro y rutas.
- Revision y pruebas por el mismo integrador; no se presenta como QA independiente.
- No Editor, cook, GPU, assets nuevos, instalaciones ni procesos terminados.

## Inventario y bloqueo del siguiente incremento

main no contiene el proyecto Unreal. S2 `1d01db54b63250ed0aacb0704a5ac16290324e32`
depende de la cadena de prototipos anterior, no solo de PR #19. Este ultimo conserva
pendiente el barrido de carga a velocidad extrema y la aceptacion con mando fisico.
Las ramas locales se inspeccionaron en solo lectura y coincidian con estos heads:

| Frente | Head | Condicion antes de integrar gameplay |
| --- | --- | --- |
| S2.5 / #25 | 7018708 | Reconstruir dependencias de assets excluidos y verificar clon portable |
| S3 / #27 | 549b9f9 | Validar base integrada; evidencia Unreal aislada no cubre main |
| S3.5 / #26 | 1ffe100 | UHT/UBT y pruebas Unreal pendientes |
| S4 / #28 | 7755e36 | UHT/UBT y pruebas Unreal pendientes |
| S6 local | 623a90d | Solo mocks/nativo; no publicar automaticamente |
| S7 local | 2f46cc3 | Solo mocks/nativo; no publicar automaticamente |

PR #29 no es el candidato actual de S3: contiene una copia anterior a su validacion.
No conectar S3.5/S4 hasta probar adaptador de enums, signo de dano, GUID/int64,
impulso kg.cm/s frente a delta-velocidad cm/s, clocks y deduplicacion.
No se alteraron los checkouts de autores ni su trabajo sin seguimiento.
La consulta de procesos no mostro Unreal/Shader/MSBuild/dotnet activos; eso no reserva
un turno. Antes de pruebas pesadas hay que coordinar propietario y turno.

Siguiente paso tecnico: preparar la cadena base jugable en otro checkout propio,
auditar assets/licencias y dependencias desde un clon limpio, coordinar un turno
Unreal y ejecutar UHT/UBT y regresiones de esa base exacta. Despues incorporar S3
actualizado como incremento independiente. No sustituir esas pruebas por este CI.

## Reversion

Revertir el commit de merge de este incremento con `git revert -m 1 <merge-sha>`.
Las fuentes originales permanecen en PR #24; no hay cambio de paquete instalado.
El resultado remoto y CI del hash final se verifican despues del merge y se reportan
en la entrega, sin afirmar anticipadamente su exito aqui.
