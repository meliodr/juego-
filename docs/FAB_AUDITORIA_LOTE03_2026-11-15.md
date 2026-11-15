# Auditoría Fab lote 03 — 2026-11-15

Estado: **biblioteca actual COMPLETA; catálogo público PARCIAL; pruebas Unreal NO EJECUTADAS**. 64 UUID únicos documentados:52 originales revisitados,6 productos reclamados ausentes del CSV original y6 nuevas fichas. No son64 productos gratuitos compatibles certificados: incluyen pago,404,variantes condicionadas, versiones antiguas y formatos externos.

## Evidencia y cobertura

Método: navegador autenticado del usuario, lectura directa de ficha/tabla de formatos/versiones/licencia y selector de variantes. Sin compras,reclamos,descargas,instalaciones,aceptación de términos ni edición de Unreal. URLs canónicas,fecha y observaciones normalizadas en CSV; no se publican identificadores de cuenta,capturas privadas,credenciales ni texto completo de vendedores.

[Inventario](FAB_INVENTARIO_PERSONAL.md):7/7 en https://www.fab.com/library. 52/52 UUID originales revisitados; uno devuelve404. Cada nueva ficha se abrió. El catálogo no mide universo completo de Fab.

Consulta adicional: https://www.fab.com/search/channels/unreal-engine?is_free=1&q=terrain&asset_formats=unreal-engine . Se aplicaron Free y formato Unreal Engine. Se observaron72 tarjetas cargadas frente a contador783; el desplazamiento devolvió timeout aunque la lista creció. No hay prueba de haber recorrido783. No se encontró selector5.8 en More filters; compatibilidad se comprobó por ficha. Las tarjetas relacionadas son descubrimiento, no fichas auditadas. El grueso de las demás categorías se cubrió mediante los52 enlaces existentes, no paginación exhaustiva de nuevas búsquedas.

| Necesidad | Evidencia útil auditada | Límite pendiente |
|---|---|---|
| Ciudad/edificios/calles | CartoonCity80meshes; CitySample y Buildings | No comparar todo el catálogo público |
| Coches/objetos | VehicleVariety,PortYard,CityVehicles | Colisión/masa/destrucción no probadas |
| Caminar/correr/saltar/vuelo/combate | GASP,FreeAnimationSet,Motifect,bundle propio | Cotejar clips y retarget; combate no completo |
| Humanos/civiles | Adventure propio,Crowds,MetaHumans | Ropa/rig/VRAM y clips por verificar |
| Robots/mechas/desarme | Scout,Robot13,3DLAKOUR,StackOBot | Partes y unión funcional solo ensayo futuro |
| Kaiju/monstruos | Bull propio,MonsterSurvival,Kraken | Escala no crea colisiones ni desmembramiento |
| VFX/teleportación/energía | NiagaraExamples,SplineVFX,EnergyMaterials | Migración5.7 y costeFluids; lógica propia mínima |
| Impactos/audio | METALLIC,Sampler | Loudness/loops/prueba auditiva pendiente |
| Chaos/destrucción | DemoUE4,plugin5.7,ContentExamples | Chaos nativo primero; ninguno certifica destrucción universal |
| Terrenos | CoastalSample,LandscapesPack,Orb propio | No cargar mapas64km² por defecto |
| Planetas/espacio | PlanetProject,CosmoKit,Asteroids,SpaceStarter | No prueban superficie continua visitable |

## Correcciones frente a los52

- Niagara Explosion Pack_1 (UUID a48b3fa2-2ebf-42c2-8892-fa20a1eff289): precio observado USD11.99–20.99. Se conserva nombre/observación original en copia histórica; DESCARTAR por presupuesto.
- Robot styloo (3a36bb9e-9b8b-4091-b7e4-e1f86b546285):404; precio/licencia no verificables.
- Scout Bot: PersonalFree,ProfessionalUSD0.99; Planet Project: PersonalFree,ProfessionalUSD4.99. No usar DesdeGratis como precio cero para cualquier usuario. Elegibilidad Personal no comprobada.
- ChaosDemoUE4 deja selección; NiagaraExamples5.7 pasa a PROBAR. RealtimeDestructibleMesh5.7 no se declara plugin5.8.
- CitySample,Vehicles,Crowds muestran Allows usage with AI Yes actualmente; corregido sin autorizar ningún entrenamiento. Paragon no tiene veto general sobre usar el arte: restricción de marcas/publicidad y descarte artístico son cosas distintas.
- KillerDoll no incluye animaciones; DemonicSpider gratis solo idle; CoastalSample no incluye el entorno completo. Bull excluyeaudio/FX. UnityContainer no es paqueteUE. Filtrar Free no sustituye leer variante.

## Promociones y licencias

Página comprobada https://www.fab.com/limited-time-free : UndergroundTunnel,SciFiBeast06Bull,3DPlatformerBundle; termina **17nov2026 9:59AM ET** (10:59AM SantoDomingo en esa fecha). Los tres ya reclamados. No se reclamó nada. Precio unitario para un comprador nuevo no se volvió a comprobar en sesión sin propiedad; se documenta promoción oficial.

Gratis actual no significa permanencia: solo VehicleVariety (Free for Life) y RuralCabins (Free Forever) se etiquetan permanentes por declaración del autor, no garantía contractual. Resto Free observado sin plazo garantizado. No se convierten precios pagados a RD$; quedan fuera de RD$0.

Fuente oficial leída: https://www.fab.com/eula (texto indica actualización1oct2024). Standard permite incorporar/modificar para proyectos privados; no redistribuir assets standalone públicamente. Personal/Professional comparten alcance de derechos, pero elegibilidad Personal depende de ingresos brutos comerciales en contenido digital y fondos pertinentes de últimos12meses (incluye entidades relacionadas), no solo monetización de este juego. No aceptar términos durante auditoría. CC-BY requiere conservar autor/licencia/enlace y señalar modificaciones cuando corresponda; enlace/version exacta de atribución se verificará antes de distribuir. Restricciones UE-only indicadas en ciertos samples se conservan; no resolver automáticamente discrepancias entre etiqueta actual y licencia original de adquisición. NoAI de ficha registra estado actual, no etiqueta histórica de la transacción. Regla del proyecto: cero entrenamiento, cero IA generativa runtime y cero envío de assets NoAI a generadores. Etiqueta GeneratedWithAI no demuestra que nuestro proyecto entreneIA ni garantiza procedencia del autor.

## Punto exacto de reanudación

1. Reabrir biblioteca sin filtros y comparar7UUID por si cambió desde15nov.
2. Terrenos: consulta anterior,72 tarjetas cargadas,contador783; continuar desde el último elemento visible tras validar lista nueva. Revisar fichas aún no auditadas: InstantTerra638feee7-83c7-4af2-9759-3f16898b5875 (comprobar software externo pagado),StampIT11388bc3-c13a-4c2f-b7d0-578022969a60,LandscapeMeshBuilder73eb89d1-18be-476f-8303-e60b2467fdc2,MountainTops8cdfadfb-5fa6-4b4e-99db-1a02b01295c2,VTOLDust03152868-7c84-4363-bafc-322031e95513,Namaqualand295ed897-fe78-4711-a583-d9b80fde554a. Son pistas, no recomendaciones gratis verificadas.
3. Ejecutar nuevas consultas filtradas por cada categoría restante; registrar contador/lotes/últimoUUID. La cobertura temática del cuadro NO equivale a completar paginación pública.
4. Resolver candidatos antiguos pendientes: Freeflowa51dbdb2-eb68-4d92-a5c7-f2509e6e162e;ModularCitycf8f6692-36e0-4305-9589-cbd567652b1b;LowPolyCityLocations5c944115-e098-4329-855a-bd373efbf42b;NY09484226-f51b-4a63-a4fc-eb7c88a15e76;EnergyWeapon1d3c8d00-473b-4eb8-b0d4-471337285c20;Shockwavedb0d837f-3c94-4a67-86a8-5653ebddce12;DynamicFalling68aca290-3b17-4acc-93f7-719d980f17f9;AnimationLibrary481ef75b-892b-424f-a213-f1cc058c9c19;PlanetMoondf4899d6-1427-42e9-a2a0-50d2f5ebc104. No añadir como gratuitos hasta verificar.
5. Antes de integrar: autorización de descarga/reclamo, edición elegible, versión licencia y tamaños reales. Probar enUE5.8Windows aislado según selección. No hay bloqueos de login ahora; límites son búsqueda pública restante y pruebas no autorizadas.
