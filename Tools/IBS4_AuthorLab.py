"""Execute inside the exclusive UE 5.8 Editor session AFTER compiling S4.

Creates only /Game/IB_S4/Lvl_IB_S4_Response. Refuses an existing destination.
Does not enable plugins, import packs, touch shared maps, or save all dirty assets.
Optional IBS4_TANK_ASSET is an already approved and imported mesh object path.
"""
import os
import unreal as ue

MAP = "/Game/IB_S4/Lvl_IB_S4_Response"


def main():
    assets = ue.EditorAssetLibrary
    if assets.does_asset_exist(MAP):
        raise RuntimeError(f"Destination already exists; inspect it instead of overwriting: {MAP}")
    scene_class = ue.load_class(None, "/Script/ImpactBeyond.IBS4LabScene")
    game_class = ue.load_class(None, "/Script/ImpactBeyond.IBS4LabGameMode")
    if not scene_class or not game_class:
        raise RuntimeError("S4 C++ classes are not compiled/loaded")
    # Require actual asset load. A Git filename or LFS pointer is not asset evidence.
    human = ue.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")
    anim = ue.load_class(None, "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C")
    if human is None or anim is None:
        raise RuntimeError("Existing Manny mesh/AnimBP bytes missing. No new character is fabricated.")
    tank_path = os.environ.get("IBS4_TANK_ASSET", "")
    tank = ue.load_asset(tank_path) if tank_path else None
    if tank_path and not isinstance(tank, ue.StaticMesh):
        raise RuntimeError("IBS4_TANK_ASSET must be an already imported authorized StaticMesh")
    levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
    actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
    # Refuse to replace a dirty user map; no hidden save/discard of other sessions.
    dirty = ue.EditorLoadingAndSavingUtils.get_dirty_map_packages()
    if dirty:
        raise RuntimeError("Save or close the current dirty map before authoring the isolated lab")
    if not levels.new_level(MAP):
        raise RuntimeError("Could not create lab level")
    scene = actors.spawn_actor_from_class(scene_class, ue.Vector())
    scene.set_actor_label("S4 isolated response fixtures - PROXY ART")
    scene.set_editor_property("human_mesh", human)
    scene.set_editor_property("human_anim", anim)
    if tank:
        scene.set_editor_property("tank_mesh", tank)
    else:
        ue.log_warning("S4 tank mesh unavailable: labelled box launcher only, tank asset gate remains BLOCKED")
    start = actors.spawn_actor_from_class(ue.PlayerStart, ue.Vector(-300, -700, 240), ue.Rotator(yaw=25))
    start.set_actor_label("S4 player start")
    light = actors.spawn_actor_from_class(ue.DirectionalLight, ue.Vector(0, 0, 1000), ue.Rotator(pitch=-55, yaw=-25))
    light.light_component.set_editor_property("intensity", 4.0)
    actors.spawn_actor_from_class(ue.SkyLight, ue.Vector(0, 0, 1000))
    world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", game_class)
    if not levels.save_current_level():
        raise RuntimeError("Lab level save failed")
    ue.log(f"IBS4_AUTHORED {MAP}; human=Manny provisional; tank={'approved local mesh' if tank else 'BOX PROXY'}")


if __name__ == "__main__":
    main()
