# S3 planet lab content

`Tools/IBS3_AuthorLab.py` creates `Lvl_IB_S3_PlanetLab`, `DA_S3World`,
`SM_S3Planet` and `SM_S3NearSurface` here during an exclusively coordinated editor slot.
The script succeeded on 2026-11-15 in UE 5.8.4. These four generated assets are now present.
The map lighting fix references the installed Engine DaylightAmbientCubemap; no engine asset bytes are copied here.
Execution evidence and remaining gates: `docs/IB_S3_UNREAL_VALIDATION.md`.

The HUD uses the native UMG `UIBS3MapWidget`; any optional Widget Blueprint child must also live here.
The runtime uses original generated sphere/cap geometry and references built-in Engine basic shapes.
All hubs are **placeholder técnico**, not accepted realistic cities. No Fab/third-party asset bytes are included.
