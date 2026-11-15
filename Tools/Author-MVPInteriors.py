"""Author procedural opaque interior mapping and trial it on a few nearby facade bays."""

import unreal as ue


LEVEL = "/Game/IB_C00/Lvl_IB_C00"
DESTINATION = "/Game/IB_C00/Interiors"
MATERIAL_NAME = "M_MVP_WindowInteriors"
INTERIOR_FUNCTION = "/Engine/Functions/Engine_MaterialFunctions01/Cubemaps/InteriorCubemap.InteriorCubemap"
MAX_BAYS_PER_FACADE = 36

# UVW is the intersection direction from UE's tangent-space interior cubemap function.
# Divide by its dominant axis to evaluate a procedural room at that cube face.
ROOM_CODE = """
float3 roomHit = UVW / max(max(max(abs(UVW.x), abs(UVW.y)), abs(UVW.z)), 0.001);
float3 face = abs(roomHit);
float2 rear = saturate(roomHit.xy * 0.5 + 0.5);
float horizontalFace = step(max(face.x, face.z), face.y);
float sideFace = step(max(face.y, face.z), face.x);
float floorFace = horizontalFace * step(roomHit.y, 0.0);
float ceilingFace = horizontalFace * (1.0 - floorFace);
float powered = step(0.37, Seed);
float warm = step(0.53, Seed);
float3 tint = lerp(float3(0.49, 0.62, 0.76), float3(1.0, 0.64, 0.37), warm);
float3 wall = lerp(float3(0.070, 0.078, 0.091), float3(0.66, 0.61, 0.53), powered);
float stripe = 0.92 + 0.08 * cos(rear.x * 47.0 + Seed * 13.0);
float floorBoards = 0.68 + 0.32 * smoothstep(0.02, 0.10, frac(roomHit.x * 7.0));
float3 room = wall * stripe * (1.0 - sideFace * 0.27);
room = lerp(room, wall * float3(0.58, 0.39, 0.25) * floorBoards, floorFace);
room = lerp(room, wall * 0.70, ceilingFace);
float curtainEdge = min(rear.x, 1.0 - rear.x);
float curtain = (1.0 - step(lerp(0.13, 0.27, Seed), curtainEdge))
    * (1.0 - max(horizontalFace, sideFace));
room = lerp(room, wall * tint * (0.42 + 0.13 * cos(rear.x * 126.0)), curtain);
float lamp = 1.0 - smoothstep(0.08, 0.24, length((rear - float2(0.50, 0.71)) * float2(1.0, 2.4)));
room += tint * lamp * powered * (0.36 + ceilingFace * 0.22);
float2 pane = frac(PaneUV);
float edge = min(min(pane.x, 1.0-pane.x), min(pane.y, 1.0-pane.y));
float glass = lerp(0.55, 0.90, smoothstep(0.0, 0.17, edge));
return room * tint * glass + float3(0.024, 0.041, 0.059);
"""


def author_material():
    assets = ue.EditorAssetLibrary
    editing = ue.MaterialEditingLibrary
    function = ue.load_asset(INTERIOR_FUNCTION)
    if function is None:
        raise RuntimeError("UE InteriorCubemap unavailable. Keep the original opaque glass.")

    path = f"{DESTINATION}/{MATERIAL_NAME}"
    if assets.does_asset_exist(path):
        material = assets.load_asset(path)
        editing.delete_all_material_expressions(material)
    else:
        assets.make_directory(DESTINATION)
        material = ue.AssetToolsHelpers.get_asset_tools().create_asset(
            MATERIAL_NAME, DESTINATION, ue.Material, ue.MaterialFactoryNew()
        )
    if material is None:
        raise RuntimeError(f"Could not author {path}")
    material.set_editor_property("blend_mode", ue.BlendMode.BLEND_OPAQUE)
    for usage in (ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES, ue.MaterialUsage.MATUSAGE_NANITE):
        editing.set_base_material_usage(material, usage, True)

    def node(kind, x, y):
        return editing.create_material_expression(material, kind, x, y)

    def connect(source, output, target, input_name):
        if not editing.connect_material_expressions(source, output, target, input_name):
            raise RuntimeError(f"Could not connect '{output}' to '{input_name}'")

    def bind(source, prop):
        if not editing.connect_material_property(source, "", prop):
            raise RuntimeError(f"Could not assign {prop}")

    uv = node(ue.MaterialExpressionTextureCoordinate, -970, -200)
    tiling = node(ue.MaterialExpressionConstant2Vector, -970, 0)
    tiling.set_editor_property("r", 1.0)
    tiling.set_editor_property("g", 1.0)
    rotate_room = node(ue.MaterialExpressionStaticBool, -970, 100)
    rotate_room.set_editor_property("value", False)
    interior = node(ue.MaterialExpressionMaterialFunctionCall, -690, -200)
    if not interior.set_material_function(function):
        raise RuntimeError("Could not bind the engine's InteriorCubemap material function")
    connect(uv, "", interior, "UVs")
    connect(tiling, "", interior, "Tiling")
    connect(rotate_room, "", interior, "Randomize Rotation")

    seed = node(ue.MaterialExpressionPerInstanceRandom, -690, 140)
    room = node(ue.MaterialExpressionCustom, -380, -160)
    room.set_editor_property("description", "Procedural room surfaces, lamps and curtains")
    room.set_editor_property("output_type", ue.CustomMaterialOutputType.CMOT_FLOAT3)
    room.set_editor_property("code", ROOM_CODE)
    inputs = []
    for name in ("UVW", "PaneUV", "Seed"):
        item = ue.CustomInput()
        item.set_editor_property("input_name", name)
        inputs.append(item)
    room.set_editor_property("inputs", inputs)
    connect(interior, "UVW", room, "UVW")
    connect(uv, "", room, "PaneUV")
    connect(seed, "", room, "Seed")
    bind(room, ue.MaterialProperty.MP_BASE_COLOR)

    glow = node(ue.MaterialExpressionMultiply, -110, 25)
    glow.set_editor_property("const_b", 0.32)
    connect(room, "", glow, "A")
    bind(glow, ue.MaterialProperty.MP_EMISSIVE_COLOR)

    roughness = node(ue.MaterialExpressionConstant, -120, 200)
    roughness.set_editor_property("r", 0.33)
    bind(roughness, ue.MaterialProperty.MP_ROUGHNESS)
    editing.recompile_material(material)
    if not assets.save_loaded_asset(material):
        raise RuntimeError(f"Could not save {path}")
    return material


def component(actor):
    return actor.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)


def transforms(hism):
    for index in range(hism.get_instance_count()):
        transform = hism.get_instance_transform(index, True)
        if isinstance(transform, tuple):
            transform = next(value for value in transform if isinstance(value, ue.Transform))
        if transform is None:
            raise RuntimeError(f"Could not read instance {index} from {hism.get_name()}")
        yield transform


def transform_key(transform):
    p, q, s = transform.translation, transform.rotation, transform.scale3d
    return tuple(round(value, 4) for value in (p.x, p.y, p.z, q.x, q.y, q.z, q.w, s.x, s.y, s.z))


def near_trial(transform):
    p = transform.translation
    return -1300.0 <= p.x <= 18000.0 and abs(p.y) <= 8000.0 and p.z <= 650.0


def trial_material(actors, existing, name, material):
    label = "Fachadas Poly Haven - " + name
    base = existing.get(label)
    if base is None:
        ue.log_warning(f"IB MVP INTERIORS: {label} is absent")
        return 0
    base_hism = component(base)
    mesh = base_hism.get_editor_property("static_mesh")
    glass_slots = [slot for slot in range(base_hism.get_num_materials())
                   if base_hism.get_material(slot) and base_hism.get_material(slot).get_name().lower().endswith("_glass")]
    if not glass_slots:
        ue.log_warning(f"IB MVP INTERIORS: {label} has no _glass slot; retaining original materials")
        return 0

    test_label = label + " - MVP interiores"
    test = existing.get(test_label)
    # Recombine prior trial transforms when rerun, including after rebuilding the original facades.
    all_transforms = {transform_key(t): t for t in transforms(base_hism)}
    if test:
        for transform in transforms(component(test)):
            all_transforms.setdefault(transform_key(transform), transform)
    eligible = [key for key, transform in all_transforms.items() if near_trial(transform)]
    eligible.sort(key=lambda key: (key[0] ** 2 + key[1] ** 2, key[2], key))
    selected = set(eligible[:MAX_BAYS_PER_FACADE])
    if not selected:
        ue.log_warning(f"IB MVP INTERIORS: no nearby bay candidates for {label}")
        return 0

    test = test or actors.spawn_actor_from_class(ue.ImpactFacade, ue.Vector())
    test.set_actor_label(test_label)
    test.set_folder_path("Distrito de vuelo/Fachadas Poly Haven")
    test_hism = component(test)
    test_hism.set_static_mesh(mesh)
    for slot in range(base_hism.get_num_materials()):
        test_hism.set_material(slot, material if slot in glass_slots else base_hism.get_material(slot))

    opaque = [transform for key, transform in sorted(all_transforms.items()) if key not in selected]
    interior = [transform for key, transform in sorted(all_transforms.items()) if key in selected]
    test_hism.clear_instances()
    test_hism.add_instances(interior, False, True, False)
    if test_hism.get_instance_count() != len(interior):
        raise RuntimeError(f"{test_label} instance count differs from selected placements")
    base_hism.clear_instances()
    base_hism.add_instances(opaque, False, True, False)
    if base_hism.get_instance_count() + test_hism.get_instance_count() != len(all_transforms):
        raise RuntimeError(f"{label} transform count changed; review before saving")
    ue.log(f"IB MVP INTERIORS: {label}: {len(interior)} trial bays, {len(opaque)} original opaque bays")
    return len(interior)


def main():
    levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
    actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
    assert levels.load_level(LEVEL), LEVEL
    facade_materials = "/Game/IB_Donors/UrbanFacades/ImpactApartments/Materials"
    for path in ue.EditorAssetLibrary.list_assets(facade_materials, False, False):
        instance = ue.load_asset(path)
        if isinstance(instance, ue.MaterialInstanceConstant):
            for usage in (ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES, ue.MaterialUsage.MATUSAGE_NANITE):
                ue.MaterialEditingLibrary.set_material_usage_override(instance, usage, True, True)
            assert ue.EditorAssetLibrary.save_loaded_asset(instance)
    material = author_material()
    existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
    count = sum(trial_material(actors, existing, name, material) for name in ("BayWide", "BayDouble"))
    if not count:
        raise RuntimeError("No facade trial was applied. Original opaque glass remains in use.")
    assert levels.save_current_level(), LEVEL
    ue.log(f"IB MVP INTERIORS SAVED: {count} nearby trial facade bays")


main()
