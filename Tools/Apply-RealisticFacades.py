"""Dress the flight district with modular Poly Haven facades."""
import json
import math
import os
from pathlib import Path
import unreal as ue


destination = "/Game/IB_Donors/UrbanFacades"
registry = ue.EditorAssetLibrary
levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.load_level("/Game/IB_C00/Lvl_IB_C00")

modules = {
    "BayWide": ("wall_window_centered_large_01", "window_centered_large_01"),
    "BayDouble": ("wall_window_centered_double_01", "window_centered_double_01"),
    "Entry": ("wall_door_centered_large_01", "door_centered_large_01"),
    "Cornice": ("cornice_standard_standard_01",),
}
asset_paths = registry.list_assets(destination, True, False) if registry.does_directory_exist(destination) else []
if not asset_paths:
    source = Path(os.environ["IB_POLY_FACADE"])
    gltf = json.loads(source.read_text(encoding="utf-8"))
    mesh_by_node = {node["name"]: gltf["meshes"][node["mesh"]] for node in gltf["nodes"] if "mesh" in node}
    gltf["meshes"] = [
        {"name": key, "primitives": [p for name in parts for p in mesh_by_node[name]["primitives"]]}
        for key, parts in modules.items()
    ]
    gltf["nodes"] = [{"name": name, "mesh": index} for index, name in enumerate(modules)]
    gltf["scene"] = 0
    gltf["scenes"] = [{"nodes": list(range(len(modules)))}]
    for material in gltf["materials"]:
        if material.get("alphaMode") == "BLEND":
            material["alphaMode"] = "OPAQUE"
            material["pbrMetallicRoughness"]["baseColorFactor"] = [0.035, 0.055, 0.07, 1.0]
    selected = source.with_name("ImpactApartments.gltf")
    selected.write_text(json.dumps(gltf), encoding="utf-8")
    task = ue.AssetImportTask()
    task.filename = str(selected)
    task.destination_path = destination
    task.automated = True
    task.save = True
    ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_paths = task.imported_object_paths

imported = [registry.load_asset(path) for path in asset_paths]
meshes = {a.get_name(): a for a in imported if isinstance(a, ue.StaticMesh)}
assert set(modules).issubset(meshes), list(meshes)
plaster = next(a for a in imported if isinstance(a, ue.MaterialInterface) and a.get_name().endswith("_plaster"))

all_actors = actors.get_all_level_actors()
existing = {actor.get_actor_label(): actor for actor in all_actors}
buildings = []
for actor in all_actors:
    label = actor.get_actor_label()
    if label.startswith("City Kit CC0 - "):
        if label.endswith(" - colision"):
            actor.static_mesh_component.set_visibility(True)
            buildings.append(actor)
        else:
            actor.static_mesh_component.set_visibility(False)
    elif label.startswith(("Edificio ", "Distrito - edificio ")):
        buildings.append(actor)
    elif label.startswith("Distrito - azotea "):
        actor.static_mesh_component.set_material(0, plaster)

instance_batches = {name: [] for name in modules}
module_bounds = {name: meshes[name].get_bounds() for name in modules}


def add_panel(name, center, tangent, width, height):
    bounds = module_bounds[name]
    width_is_x = bounds.box_extent.x > bounds.box_extent.y
    half_width = bounds.box_extent.x if width_is_x else bounds.box_extent.y
    yaw = math.atan2(tangent.y, tangent.x) - (0 if width_is_x else math.pi / 2)
    s_width = width / (2 * half_width)
    s_height = height / (2 * bounds.box_extent.z)
    scale = ue.Vector(s_width, 1.0, s_height) if width_is_x else ue.Vector(1.0, s_width, s_height)
    native_x = bounds.origin.x * scale.x
    native_y = bounds.origin.y * scale.y
    offset = ue.Vector(
        native_x * math.cos(yaw) - native_y * math.sin(yaw),
        native_x * math.sin(yaw) + native_y * math.cos(yaw),
        bounds.origin.z * scale.z,
    )
    transform = ue.Transform()
    transform.translation = center - offset
    transform.rotation = ue.Quat(0, 0, math.sin(yaw / 2), math.cos(yaw / 2))
    transform.scale3d = scale
    instance_batches[name].append(transform)


for index, actor in enumerate(buildings):
    actor.static_mesh_component.set_material(0, plaster)
    center, extent = actor.get_actor_bounds(False)
    stories = max(1, round(2 * extent.z / 340))
    height = 2 * extent.z / stories
    base = center.z - extent.z
    for x, y, tx, ty, full_width in (
        (center.x + extent.x + 17, center.y, 0, 1, 2 * extent.y),
        (center.x - extent.x - 17, center.y, 0, -1, 2 * extent.y),
        (center.x, center.y + extent.y + 17, -1, 0, 2 * extent.x),
        (center.x, center.y - extent.y - 17, 1, 0, 2 * extent.x),
    ):
        bays = max(1, round(full_width / 410))
        width = full_width / bays
        for bay in range(bays):
            along = width * (bay + 0.5 - bays / 2)
            point = ue.Vector(x + tx * along, y + ty * along, base)
            tangent = ue.Vector(tx, ty, 0)
            for story in range(stories):
                module = ("BayWide", "BayDouble")[(index + bay // 3) % 2]
                if story == 0 and bay == bays // 2:
                    module = "Entry"
                floor_center = point + ue.Vector(0, 0, (story + 0.5) * height)
                add_panel(module, floor_center, tangent, width, height)
            add_panel("Cornice", point + ue.Vector(0, 0, stories * height - 12), tangent, width, 24)

assert len(buildings) >= 20
for name, transforms in instance_batches.items():
    label = "Fachadas Poly Haven - " + name
    actor = existing.get(label) or actors.spawn_actor_from_class(ue.ImpactFacade, ue.Vector())
    actor.set_actor_label(label)
    actor.set_folder_path("Distrito de vuelo/Fachadas Poly Haven")
    component = actor.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
    component.set_static_mesh(meshes[name])
    component.clear_instances()
    component.add_instances(transforms, False, True, False)
    assert component.get_instance_count() == len(transforms)
assert levels.save_current_level()
ue.log(f"IB REALISTIC FACADES SAVED: {len(buildings)} buildings, "
       f"{sum(len(batch) for batch in instance_batches.values())} instances")
