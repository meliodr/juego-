# S3.5 — evidencia y pendientes, 2026-11-15

**Resultado: CÓDIGO implementado, sin compilación Unreal ni aceptación jugable.** Se conserva la base `1d01db54b63250ed0aacb0704a5ac16290324e32`, PR #19, rama `prototype/ib-s35-interactions-lab`. No integración global ni sprint 7.5.

El checkout existente no fue modificado. Git no pudo entrar en él (`Permission denied`); se utilizó el fallback autorizado de clone sparse independiente dentro de la carpeta de esta tarea. La red del sandbox tampoco resolvió github.com; las lecturas de Git para el clone/checkout se completaron con escalación autorizada. No se cambiaron permisos, credenciales, seguridad, motor ni configuración global. RD$0, cero instalaciones/descargas de assets y cero subagentes.

Se consultó la rama documental `docs/roadmap-sprints-02-08-20261115` y su `PLAN_PARALELO_S25_S3_S35_S4_20261115.md`. No existía rama S3.5 remota al comprobar refs antes de implementar. Se leyó `.agents/skills`; no se encontró memoria específica de S3.5 en esa carpeta. Catálogo local de modelos: GPT-6.1 Sol admite High. **No se verificó ni cambió el modelo/esfuerzo efectivo de esta sesión.**

## Ejecutado

La primera pasada produjo los 2.595 checks indicados abajo. La revisión local posterior S3.5/S4 amplió la suite a **2.613 aserciones** y ejecutó **280 aserciones comparativas** adicionales. Estado y divergencias en `IB_S35_S4_INTEGRATION_REVIEW.md`; hashes/resultados comparativos en `IB_S35_S4_NATIVE_RESULTS.json`. Ninguna de esas pruebas ejecutó Unreal.

`Tools/IBS35_TestPolicy.ps1` compiló una única unidad C++ con MSVC 14.52.37025, C++17, `/W4 /WX`. Ejecutó el header real `IBS35Policy.h` usado por el runtime:

```text
IBS35_TestPolicy.cpp
PASS IBS35 policy: 2595 assertions; 150 lease cycles, 30 dedup attacks, 20 resets.
This executable does NOT test Unreal collision, physics, animation, DX12 or FPS.
```

Los 150 ciclos son **máquina de estados de propiedad**, no 150 agarres en Chaos. Las 30 secuencias prueban ledger por blanco con ocho contactos repetidos y replay; no son golpes renderizados. Los 20 resets son de estado, no recargas de mapa. También se verifican cancelación, dueño inválido/competidor, carga y velocidad por masa, NaN, límites, historial lleno y un frame largo que cruza toda la ventana de contacto.

`python Tools/IBS35_CheckScope.py` comprueba exclusividad de rutas, ausencia de binarios, sintaxis de los scripts Python y whitespace Git. No acredita APIs Unreal. Se revisaron headers del motor para APIs sensibles; eso tampoco sustituye UHT/C++.

## Preparado, no ejecutado

| Gate | Cobertura preparada | Estado real |
|---|---|---|
| UHT + C++ Unreal | Nuevos tipos/componentes/derivado/AutomationTests dentro del módulo existente | PENDIENTE: no hubo turno exclusivo coordinado |
| Mapa `/Game/IB_S35/Lvl_IB_S35_Interactions` | Generador con 9 objetos, 3 coches, 5 categorías, 2 escalas extra, blanco, 2 muros y carril | SCRIPT; `.umap` y DataAssets todavía NO generados |
| `ImpactBeyond.S35.GripAndReset` | 30 ciclos por clase, disputa de dueño, cancelación, restauración, 20 barridos contra muro de 4 cm y resets | NO EJECUTADO |
| `ImpactBeyond.S35.PunchAndContract` | 30 golpes suelo + 30 aire, rápido/pesado, hit único, frame largo, cancelación, muro entre frames, replay/reset/NaN | NO EJECUTADO |
| `ImpactBeyond.S35.ThrowsPhysical` | 10 lanzamientos y colisiones por Car/Container/Rock, CCD y caja real contra muro de 4 cm | NO EJECUTADO; exige frames reales de Chaos |
| Herramientas | Poste por SweepTool; contenedor/roca/parte por impulso; mensajes comunes | Código sin ejecución/aceptación visual |
| IK y animación | Proveedor de contactos, OnAttackCue y notify preparado; curva temporal de ensayo | Adaptador IK pendiente; clips/poses de puñetazo NO validados |
| Regresiones circuito/vuelo | Suite heredada intacta | NO REEJECUTADA en esta rama |
| DX12 1920×1080 | Capturas de manos/cuerpo, carga, contacto y lanzamiento; p95/p99/VRAM | NO EJECUTADO; sin cifras FPS ni capturas nuevas |
| Mando físico | Inputs de laboratorio separados, cancelación, suelo/aire y recuperación | HUMANO pendiente |

Matriz adicional obligatoria al obtener turno: repetir cargas contra pared frontal/oblicua a velocidades crecientes (incluyendo cambios bruscos, caída de frames, giro de portador y vuelo extremo); 20 resets con/sin ocupación; 5 colisiones por las tres clases a velocidades bajas/altas; capturas de postura de las cinco categorías y comparación de los tres coches. Confirmar ausencia de vibración/colisión fantasma mediante ejecución; no deducirla de código. Comprobar input cancelado por pérdida de foco y ventanas de notify con el clip real.

## Limitaciones conocidas y decisión

- Orientación de carga fija durante transporte. Fuera de alcance se suelta de forma segura, sin impulso; no se promete seguir al héroe a todas las marchas.
- Collider de caja conservador; daños/materiales son contrato de laboratorio. No deformación de coches ni destrucción de muros.
- El circuito legado sigue siendo la fuente IK de H01: la adaptación no aplicada está en `IB_S35_ADAPTATION.md`. Las manos y animación no están terminadas por haber publicado puntos/eventos.
- Historial de 4.096 AttackId por vida, luego rechazo explícito hasta reset. Sin persistencia entre sesiones ni semántica S3/S4 final.
- Scripts de editor y AutomationTests requieren compilación/ejecución antes de aceptarlos. La escena no debe presentarse como demo lista.

Decisión: **CONTINUAR en un turno exclusivo de Unreal**, corregir cualquier fallo de compilación/API/colisión y después revisar DX12/control. Mantener PR draft; no merge ni reemplazo del paquete jugable existente. El mismo agente implementó y revisó el código; no hubo QA independiente.

## Historial y autorización de publicación

La implementación se guardó en el commit local `bfa20140e88b734f78bafbbde1bf0b476677b7ed`. La revisión automática **rechazó antes de ejecutar** el push de `prototype/ib-s35-interactions-lab` a `meliodr/juego-`, indicando autorización insuficientemente explícita para publicar código no público en ese destino. No se reintentó ni se cambió de herramienta para publicar. PR remoto: **NO CREADO**. El cuerpo listo y el motivo exacto están en `IB_S35_PR_DRAFT.md`; se requiere autorización específica para la publicación. Un commit documental posterior registra este bloqueo sin alterar la implementación.

Coordinación final recibida: el siguiente turno de Editor/Cook y compilación pesada está reservado a **S2.5** hasta que lo libere. S3.5 no tiene procesos pesados activos y no inició ninguno. La entrega permanece local y las pruebas Unreal siguen pendientes de ese relevo explícito.

Actualización 2026-11-15, 16:09 UTC: autorización explícita del usuario para publicar las ramas de los cuatro laboratorios en el repositorio público meliodr/juego- y abrir sus PR draft sin merge. Se verificó con el historial de mensajes. Los 30 archivos S3.5 son texto propio; no se incorporan assets binarios, archivos de S4 ni coincidencias con los patrones de secretos revisados. La consulta previa no encontró rama remota ni PR S3.5. Se prepara un nuevo intento autorizado; su éxito y el SHA/PR publicados requieren confirmación remota. No se reejecutó Unreal ni cambiaron los pendientes técnicos.

Resultado confirmado 16:14 UTC: la rama `prototype/ib-s35-interactions-lab` se publicó correctamente, inicialmente en `f833868af42cceadcc9ff194b66389978ec7836b`. [PR #26](https://github.com/meliodr/juego-/pull/26) abierto, draft=true, merged=false, contra `prototype/ib-sprint02` en el snapshot fijo. No hubo force-push, merge ni uso de Editor/Cook. El bloqueo de publicación inicial quedó resuelto por la autorización nueva; la validación Unreal sigue pendiente. El último SHA se consulta en el head del PR, incluyendo este registro documental.
