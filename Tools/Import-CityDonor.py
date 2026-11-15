"""Import CC0 buildings and place them on vacant lots in the flight district."""
import os
import unreal as ue


source = os.environ["IB_CITY_DONOR"]
destination = "/Game/IB_Donors/CityKit"
registry = ue.EditorAssetLibrary
asset_tools = ue.AssetToolsHelpers.get_asset_tools()
levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.load_level("/Game/IB_C00/Lvl_IB_C00")
collision_cube = ue.load_asset("/Engine/BasicShapes/Cube")

buildings = {}
for category, name in (("comercial", "escritorio"), ("comercial", "hotel"),
                       ("residencial", "predio_alto"), ("residencial", "predio_apartamentos")):
    assets = registry.list_assets(f"{destination}/{name}", True, False)
    if not assets:
        task = ue.AssetImportTask()
        task.filename = os.path.join(source, "models", category, name + ".glb")
        task.destination_path = f"{destination}/{name}"
        task.automated = True
        task.save = True
        asset_tools.import_asset_tasks([task])
        assets = task.imported_object_paths
    matches = [asset for path in assets if isinstance(asset := registry.load_asset(path), ue.StaticMesh)]
    assert len(matches) == 1, (name, assets)
    buildings[name] = matches[0]

existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
for index, (name, x, y, width, depth, height) in enumerate((
    ("hotel", 55, 210, 30, 29, 37),
    ("predio_apartamentos", 109, 208, 38, 29, 32),
    ("escritorio", 159, 214, 24, 29, 49),
    ("predio_alto", 250, 182, 27, 36, 62),
    ("escritorio", 492, -123, 46, 24, 50),
    ("hotel", 543, -126, 36, 24, 41),
    ("predio_alto", 595, 113, 24, 34, 60),
), 1):
    label = f"City Kit CC0 - {index:02} - {name}"
    mesh = buildings[name]
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    scale = ue.Vector(width * 50 / extent.x, depth * 50 / extent.y, height * 50 / extent.z)
    origin = bounds.origin
    location = ue.Vector(x * 100 - origin.x * scale.x, y * 100 - origin.y * scale.y,
                         (extent.z - origin.z) * scale.z)
    actor = existing.get(label) or actors.spawn_actor_from_class(ue.StaticMeshActor, location)
    actor.set_actor_label(label)
    actor.set_folder_path("Distrito de vuelo/Kit externo CC0")
    actor.static_mesh_component.set_static_mesh(mesh)
    actor.static_mesh_component.set_collision_profile_name("NoCollision")
    actor.set_actor_location(location, False, True)
    actor.set_actor_scale3d(scale)
    collision_label = f"{label} - colision"
    center = ue.Vector(x * 100, y * 100, height * 50)
    blocker = existing.get(collision_label) or actors.spawn_actor_from_class(ue.StaticMeshActor, center)
    blocker.set_actor_label(collision_label)
    blocker.set_folder_path("Distrito de vuelo/Kit externo CC0")
    blocker.set_actor_location(center, False, True)
    blocker.set_actor_scale3d(ue.Vector(width, depth, height))
    blocker.static_mesh_component.set_static_mesh(collision_cube)
    blocker.static_mesh_component.set_collision_profile_name("BlockAll")
    blocker.static_mesh_component.set_visibility(False)

assert levels.save_current_level()
ue.log("IB CITY DONOR IMPORTED AND PLACED")
