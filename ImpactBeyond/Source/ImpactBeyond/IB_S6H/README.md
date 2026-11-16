# Hero runtime integration

Attach one `UIBS6HBridge` to the existing `ACharacter`. Set `bAllowNonPlanetLab`
only for a deliberately nonstreamed flat map. A present S3 world still supplies
readiness. Root input and the existing character remain the host's responsibility.

## Input and presentation

- `OpenSelector`, `SelectorInput`, `CloseSelector` drive the UMG selector.
  Controls 0/1 navigate, 2 confirms, 3 cancels, and 4/5/6 choose a card.
  Feed pressed and released edges. The widget supplies mouse buttons.
- The input host uses `IsSelectorOpen` to set its own cursor and input context.
  It must route navigation exclusively while the selector is visible.
- `GetAbilities` returns six descriptors for the current hero. `EquipAbility`
  selects, `ActivateEquipped` activates, and `ReleaseEquipped` releases held
  construct input and forwards release of host abilities.
- `OnHeroChanged` refreshes the wheel and model. `Energy`, `EquippedAbility`,
  `LastResult` and ability cooldowns supply the HUD.
- H01 slots are 101 quick, 102 heavy, 103 uppercut, 104 laser, 105 telekinesis,
  106 air dash. H02 slots are 201 shield, 202 hand, 203 hammer, 101, 104, 105.
  H03 slots are 301 blink, 302 counterblink, 303 reposition, 101, 102, 106.
- Bind `HostAbility(int32)` and `HostReleaseAbility(int32)` to existing laser,
  telekinesis and swept air dash for IDs 104–106. Missing providers report
  unavailable. The host resolves successful execution before returning true.
- Shield, hand and hammer use engine cube meshes with real blocking boxes.
  Supply a material exposing Color/BaseColor/EmissiveColor for art direction.
  ActivationSound is optional. The fallback is a short generated audio cue.

## Damage and movement

Bind `CanonicalStrike(const FIBS35DamageMessage&, const FHitResult&)` to the
integration owner's ImpactContract dispatch. Messages carry an attack GUID,
instigator, contact and exact component. The owner resolves TargetId from its
receiver life. The bridge never calls a legacy health receiver.

After canonical dedup and before HP application, call
`NotifyIncomingHit(FVector Source)` and `Absorb(FGuid Attack, float Damage)`.
Absorb returns the absorbed amount. Apply the remaining amount once.

Blink travels up to 30 m. Counterblink travels 25 m away from a hit received
within three seconds, otherwise backward. Reposition travels 25 m along the
last movement input, otherwise local right. They share native energy and cooldown
and preserve movement velocity. Full capsule sweeps, entry/exit overlaps,
grounded support and S3 path readiness apply to every candidate. Carrying blocks
blink. Shield/construct durations and cooldowns use the existing native model.

Call `ResetAbilities` with the host's reset operation. EndPlay destroys owned
effects, removes the widget and unbinds delegates.

## Verification

`ImpactBeyond.S6.Runtime` is an Unreal Automation suite for the flat city map.
It tests selector/equipment agreement, visible shield geometry and cleanup,
absorption replay/capacity, canonical strike dispatch, cooldown, blink readiness,
thin wall/occupied destination/ground support, velocity retention and directions.
The strike test uses a counting dispatch seam. End-to-end S4/S7 health and physical
controller acceptance are integration tests owned by the complete game runner.
