"""Author the isolated S7 playable selection in an already-built UE 5.8 editor.

Run locally: exec(open(r'<repo>/Tools/IBPlay_AuthorProduction.py').read()).
Required input: set IB_PLAY_DONOR_CONTENT to the absolute Content directory of
the user's installed donor project before launching the editor/commandlet, or
set os.environ["IB_PLAY_DONOR_CONTENT"] in that editor's Python session first.
There is no machine-specific default. Referenced package files must already be
staged in the destination project, preserving their original /Game paths.
Requires Python, EditorScriptingUtilities and GeometryScripting (editor only).
Root coordinates the editor slot and first stages licensed packs at their original
/Game paths. This script never downloads, uploads, exports, or logs asset payloads.
Only paths, bone names, counts, parameter names and dependencies enter its report.
Existing source assets/maps are never modified. Outputs stay under /Game/IB_Playable.
Reruns reuse outputs bearing this script's metadata tag; unrelated assets fail closed.
"""
import json
import math
import os
import re
from pathlib import Path
from types import SimpleNamespace
import unreal

ROOT = "/Game/IB_Playable"
MAP = ROOT + "/Lvl_IB_Playable"
SOURCE_MAP = "/Game/IB_S3/Lvl_IB_S3_PlanetLab"
TAG = "IBPlayAuthorVersion"
VERSION = "1"
DONOR_CONTENT = None
REPORT = {"schema": 1, "status": "started", "warnings": [], "created": [], "parts": {},
          "runtime_tested": False, "source_assets_modified": False}
ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MAT = unreal.MaterialEditingLibrary
GEOMETRY = {}
# UE's camel-case iterator exports plural IDs as i_ds in some Python builds.
# Resolve the exact installed callable once, before touching any content assets.
GEOMETRY_METHODS = {
    "AssetUtils": ("copy_mesh_from_skeletal_mesh", "get_lod_material_list_from_skeletal_mesh"),
    "BoneWeights": ("get_all_bones_info", "get_vertex_bone_weights"),
    "MeshQueries": ("get_all_vertex_ids", "get_all_triangle_ids", "get_triangle_indices",
                    "get_mesh_bounding_box", "get_vertex_position"),
    "List": ("convert_index_list_to_array",),
    "MeshSelection": ("convert_index_array_to_mesh_selection",),
    "MeshDecomposition": ("copy_mesh_selection_to_mesh",),
    "MeshTransforms": ("inverse_transform_mesh", "translate_mesh", "scale_mesh", "rotate_mesh"),
    "NewAssetUtils": ("create_new_static_mesh_asset_from_mesh",),
    "Primitives": ("append_box", "append_cylinder", "append_cone"),
    "Materials": ("enable_material_ids",),
    "MeshEdits": ("append_mesh",),
}
CUSTOM_PROPERTIES = {
    "IBPlayArt": ("Heroes", "CitizenMesh", "HeroAccents", "EnergyMaterial", "Robot", "Kaiju",
        "RobotAnimations", "KaijuAnimations", "RobotParts", "KaijuParts", "RobotPartBones", "KaijuPartBones",
        "Props", "Tank", "Drone", "SmallRobot", "Cannon", "Concrete", "Ground", "Glass", "Metal"),
    "IBS3PlanetWorld": ("bProductionVisuals", "SaveSlot"),
    "IBPlaySession": ("SaveSlot", "Art"),
}

HERO = "/Game/Adventures_Pack/Mesh/Adventure_05/Mesh_UE5/Full/SK_UE5_Adventure_05_Full_01"
MANNY = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
ROBOT = "/Game/robot13/skeletal_mesh/SK_Robot_13"
KAIJU = "/Game/SciFi_Beast06/Mesh/SK_SciFi_Beast06_Skin1"
ROBOT_CLIPS = ["/Game/robot13/animation/" + name for name in (
    "A_idle_root_robot_13", "A_walk_root_robot_13", "A_attack_1_robot_13")]
KAIJU_CLIPS = ["/Game/SciFi_Beast06/Animations/" + name for name in (
    "Anim_SciFi_Beast06_Idle1", "Anim_SciFi_Beast06_WalkF", "Anim_SciFi_Beast06_Attack1")]


def require(value, message):
    if not value:
        raise RuntimeError(message)
    return value


def configured_donor_content():
    value = os.environ.get("IB_PLAY_DONOR_CONTENT", "").strip()
    require(value, "Set IB_PLAY_DONOR_CONTENT to the absolute donor project's Content directory before authoring")
    directory = Path(value).expanduser()
    require(directory.is_absolute() and directory.is_dir(),
            "IB_PLAY_DONOR_CONTENT must name an existing absolute Content directory")
    return directory.resolve()


def transform(location, rotation=(0, 0, 0), scale=(1, 1, 1)):
    # UE 5.8 exposes NativeMakeFunc as the struct constructor, not as
    # MathLibrary.make_transform. This is covered by Engine's Python tests.
    # Name Rotator arguments explicitly: native make order is roll,pitch,yaw.
    return unreal.Transform(unreal.Vector(*location),
        unreal.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]), unreal.Vector(*scale))


def geometry_preflight():
    failures, metadata = [], {}
    for suffix, methods in GEOMETRY_METHODS.items():
        library_name = "GeometryScript_" + suffix
        library = getattr(unreal, library_name, None)
        if library is None:
            failures.append(library_name + " missing: enable GeometryScripting in the editor")
            continue
        bindings = {}
        for requested in methods:
            candidates = [name for name in dir(library)
                          if name.replace("_", "").casefold() == requested.replace("_", "").casefold()
                          and callable(getattr(library, name, None))]
            actual = requested if requested in candidates else candidates[0] if len(candidates) == 1 else None
            if actual is None:
                failures.append(library_name + "." + requested + ": no unambiguous installed callable")
                continue
            function = getattr(library, actual)
            bindings[requested] = function
            metadata[library_name + "." + requested] = {"actual": actual, "signature": function.__doc__ or ""}
        GEOMETRY[suffix] = SimpleNamespace(**bindings)
    # Check every GeometryScript structure/property/enum we create or read. This
    # operates only on transient metadata structures, never on source asset data.
    structures = {
        "GeometryScriptCopyMeshFromAssetOptions": (),
        "GeometryScriptMeshReadLOD": (),
        "GeometryScriptCreateNewStaticMeshAssetOptions": ("enable_collision",),
        "GeometryScriptPrimitiveOptions": ("material_id",),
        "GeometryScriptBoneInfo": ("index", "name", "parent_index", "world_transform"),
        "GeometryScriptBoneWeight": ("bone_index", "weight"),
    }
    for name, properties in structures.items():
        structure_type = getattr(unreal, name, None)
        if structure_type is None:
            failures.append(name + " missing")
            continue
        metadata[name] = {"signature": structure_type.__doc__ or "", "properties": list(properties)}
        try:
            instance = structure_type()
            for prop in properties:
                instance.get_editor_property(prop)
        except Exception as error:
            failures.append(name + ": " + str(error))
    for name, value in (("GeometryScriptOutcomePins", "SUCCESS"),
                        ("GeometryScriptMeshSelectionType", "TRIANGLES"),
                        ("GeometryScriptPrimitiveOriginMode", "CENTER")):
        if not hasattr(getattr(unreal, name, None), value):
            failures.append(name + "." + value + " missing")
    try:
        probe = transform((11, 12, 13), (23, 47, 19), (2, 3, 4))
        require((probe.translation.x, probe.translation.y, probe.translation.z) == (11,12,13),
                "Transform constructor did not retain location")
        require((probe.scale3d.x, probe.scale3d.y, probe.scale3d.z) == (2,3,4),
                "Transform constructor did not retain scale")
        metadata["Transform"] = {"signature": unreal.Transform.__doc__ or "", "constructor_probe_passed": True}
    except Exception as error:
        failures.append("Transform constructor: " + str(error))
    REPORT["python_api"] = metadata
    REPORT["python_api_failures"] = failures
    require(not failures, "Geometry API preflight failed: " + "; ".join(failures))


def native_property_preflight():
    # Project classes without generated Python wrappers use the generic UObject
    # wrapper. ResolvePropertyName then passes names through to FindPropertyByName;
    # native UPROPERTY names work whereas snake_case aliases need not exist.
    failures = []
    for name, properties in CUSTOM_PROPERTIES.items():
        native_class = unreal.load_class(None, "/Script/ImpactBeyond." + name)
        if native_class is None:
            failures.append(name + ": compiled class missing")
            continue
        default = unreal.get_default_object(native_class)
        for prop in properties:
            try:
                default.get_editor_property(prop)
            except Exception as error:
                failures.append(name + "." + prop + ": " + str(error))
    REPORT["native_properties"] = {name: list(properties) for name, properties in CUSTOM_PROPERTIES.items()}
    REPORT["native_property_failures"] = failures
    require(not failures, "Native property preflight failed: " + "; ".join(failures))


def load(path, cls):
    return require(unreal.load_asset(path) if ASSETS.does_asset_exist(path) else None,
                   "Missing staged asset: " + path) if cls is None else typed(path, cls)


def typed(path, cls):
    value = unreal.load_asset(path) if ASSETS.does_asset_exist(path) else None
    require(isinstance(value, cls), "Missing/wrong asset class: " + path)
    return value


def save(asset):
    require(asset.get_path_name().startswith(ROOT + "/"), "Refusing source mutation")
    ASSETS.set_metadata_tag(asset, TAG, VERSION)
    require(ASSETS.save_loaded_asset(asset, only_if_is_dirty=False), "Save failed: " + asset.get_path_name())
    return asset


def existing(path, cls):
    if not ASSETS.does_asset_exist(path):
        return None
    asset = typed(path, cls)
    require(ASSETS.get_metadata_tag(asset, TAG) == VERSION,
            "Refusing unrelated existing output: " + path)
    return asset


def create(path, cls, factory):
    asset = existing(path, cls)
    if asset:
        return asset, False
    folder, name = path.rsplit("/", 1)
    ASSETS.make_directory(folder)
    asset = require(TOOLS.create_asset(name, folder, cls, factory), "Creation failed: " + path)
    ASSETS.set_metadata_tag(asset, TAG, VERSION)
    REPORT["created"].append(path)
    return asset, True


def duplicate(source, path, cls):
    asset = existing(path, cls)
    if asset:
        return asset, False
    ASSETS.make_directory(path.rsplit("/", 1)[0])
    asset = require(ASSETS.duplicate_asset(source.get_path_name(), path), "Duplicate failed: " + path)
    ASSETS.set_metadata_tag(asset, TAG, VERSION)
    REPORT["created"].append(path)
    return asset, True


def bone_map(mesh):
    # Engine exposes reference skeleton names without reading raw package payloads.
    component = unreal.SkeletalMeshComponent()
    component.set_skeletal_mesh_asset(mesh)
    return {str(component.get_bone_name(i)): str(component.get_parent_bone(component.get_bone_name(i)))
            for i in range(component.get_num_bones())}


def validate_leader(hero, manny):
    follower, leader = bone_map(hero), bone_map(manny)
    critical = ["root", "pelvis", "spine_01", "spine_02", "spine_03", "head"]
    critical += [part + side for side in ("_l", "_r")
                 for part in ("clavicle", "upperarm", "lowerarm", "hand", "thigh", "calf", "foot")]
    missing = [name for name in critical if name not in follower or name not in leader]
    mismatched = [name for name in follower if name in leader and follower[name] != leader[name]]
    extras = sorted(set(follower) - set(leader))
    REPORT["leader_pose"] = {"leader": manny.get_path_name(), "follower": hero.get_path_name(),
        "shared_bones": len(set(follower) & set(leader)), "missing_required": missing,
        "parent_mismatches": mismatched, "follower_extra_bones": extras}
    require(not missing and not mismatched, "Adventure05/Manny bone hierarchy mismatch; see metadata report")
    if extras:
        REPORT["warnings"].append("Follower extra bones use reference pose under leader pose; inspect accessories in PIE")


def vector(material, name, rgba, x, y):
    node = MAT.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", unreal.LinearColor(*rgba))
    return node


def energy_material():
    material, fresh = create(ROOT + "/Materials/M_ConstructEnergy", unreal.Material, unreal.MaterialFactoryNew())
    if fresh:
        material.set_editor_property("two_sided", True)
        color = vector(material, "Color", (0.06, 0.9, 0.3, 1), -550, 0)
        base = vector(material, "BaseColor", (0.06, 0.9, 0.3, 1), -550, 180)
        glow = vector(material, "EmissiveColor", (0.06, 0.9, 0.3, 1), -550, 360)
        blend = MAT.create_material_expression(material, unreal.MaterialExpressionLinearInterpolate, -250, 30)
        blend.set_editor_property("const_alpha", 0.5)
        require(MAT.connect_material_expressions(color, "RGB", blend, "A"), "Color connection failed")
        require(MAT.connect_material_expressions(base, "RGB", blend, "B"), "BaseColor connection failed")
        require(MAT.connect_material_property(blend, "", unreal.MaterialProperty.MP_BASE_COLOR), "Base color output failed")
        strength = MAT.create_material_expression(material, unreal.MaterialExpressionMultiply, -250, 300)
        strength.set_editor_property("const_b", 2.4)
        require(MAT.connect_material_expressions(glow, "RGB", strength, "A"), "Emissive connection failed")
        require(MAT.connect_material_property(strength, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR), "Emissive output failed")
        rough = MAT.create_material_expression(material, unreal.MaterialExpressionConstant, -200, 470)
        rough.set_editor_property("r", 0.27)
        require(MAT.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS), "Roughness output failed")
        errors = MAT.recompile_material(material)
        require(not errors, "Energy material compile errors: " + str(errors))
    required = {"Color", "BaseColor", "EmissiveColor"}
    names = {str(name) for name in MAT.get_vector_parameter_names(material)}
    if not required.issubset(names):
        # Recover a tagged asset saved before its expression metadata was refreshed.
        # Preserve its graph and references; no assets or expressions are deleted.
        errors = MAT.recompile_material(material)
        require(not errors, "Energy material compile errors: " + str(errors))
        names = {str(name) for name in MAT.get_vector_parameter_names(material)}
    REPORT["energy_material"] = {"path": material.get_path_name(), "vector_parameters": sorted(names)}
    require(required.issubset(names), "Energy material missing vector parameters: " + str(sorted(required-names)))
    return save(material)


def set_verified_vector(instance, parameter, rgba):
    names = {str(name) for name in MAT.get_vector_parameter_names(instance)}
    require(parameter in names, "Material instance missing vector parameter: " + parameter)
    # UE 5.8 MaterialEditingLibrary.cpp sets/updates the value but returns an
    # unchanged false bResult. Verify the actual override and readback instead.
    MAT.set_material_instance_vector_parameter_value(instance, parameter, unreal.LinearColor(*rgba))
    actual = MAT.get_material_instance_vector_parameter_value(instance, parameter)
    values = (actual.r, actual.g, actual.b, actual.a)
    require(all(abs(value-wanted) <= 0.00001 for value, wanted in zip(values, rgba)),
            "Material vector readback mismatch: " + parameter + " " + str(values))
    require(MAT.is_material_instance_parameter_overridden(instance, parameter),
            "Material vector override not retained: " + parameter)


def author_heroes(source):
    # Preserve the authored atlas, skin and normal/roughness maps. Original energy
    # accent materials belong on constructs/accessories, never across the skin atlas.
    tints = [(1.0, 0.64, 0.13, 1), (0.08, 1.0, 0.35, 1), (0.29, 0.23, 1.0, 1)]
    heroes, accents = [], []
    original = list(source.get_editor_property("materials"))
    body_slots = [i for i, slot in enumerate(original)
                  if slot.get_editor_property("material_interface") and
                  re.search(r"Adventure_05_(?:Without_Tatoo_)?\d+$",
                            slot.get_editor_property("material_interface").get_name(), re.I)]
    require(body_slots, "Cannot identify Adventure05 body atlas by metadata")
    energy = energy_material()
    for index, tint in enumerate(tints):
        mesh, fresh = duplicate(source, ROOT + "/Heroes/SK_Hero" + str(index + 1), unreal.SkeletalMesh)
        # UStruct wrappers may reference their owner's array: copy each slot
        # before changing it so the source asset is never edited in memory.
        slots = [slot.copy() for slot in source.get_editor_property("materials")]
        outfit = typed("/Game/Adventures_Pack/Materials/Adventure_05/Adventure_05/"
                       + "M_Adventure_05_Without_Tatoo_0" + str(index + 1), unreal.MaterialInterface)
        for slot in body_slots:
            slots[slot].set_editor_property("material_interface", outfit)
        mesh.set_editor_property("materials", slots)
        save(mesh)
        accent, _ = create(ROOT + "/Materials/MI_HeroAccent" + str(index + 1),
                           unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        MAT.set_material_instance_parent(accent, energy)
        MAT.update_material_instance(accent)
        for parameter in ("Color", "BaseColor", "EmissiveColor"):
            set_verified_vector(accent, parameter, tint)
        MAT.update_material_instance(accent)
        heroes.append(mesh)
        accents.append(save(accent))
    REPORT["heroes"] = {"source": source.get_path_name(), "outfit_slots": body_slots,
                         "outfit_variants": [1, 2, 3], "atlas_tint_includes_skin": False,
                         "accents": "gold / emerald / indigo: constructs and accessories only"}
    return heroes, accents


def validate_clips(mesh, paths):
    skeleton = mesh.get_editor_property("skeleton")
    clips = [typed(path, unreal.AnimSequence) for path in paths]
    for clip in clips:
        require(clip.get_editor_property("skeleton") == skeleton, "Animation skeleton mismatch: " + clip.get_path_name())
    return clips


def pick_bone(names, patterns, used):
    for pattern in patterns:
        candidates = sorted(name for name in names if name not in used and re.search(pattern, name, re.I))
        if candidates:
            return candidates[0]
    return None


def author_parts(mesh, label, patterns):
    require(hasattr(unreal, "GeometryScript_AssetUtils"), "Enable GeometryScripting for this editor authoring run")
    source = unreal.DynamicMesh()
    _, result = GEOMETRY["AssetUtils"].copy_mesh_from_skeletal_mesh(mesh, source,
        unreal.GeometryScriptCopyMeshFromAssetOptions(), unreal.GeometryScriptMeshReadLOD())
    require(result == unreal.GeometryScriptOutcomePins.SUCCESS, "Cannot read skeletal source model for " + label)
    _, infos = GEOMETRY["BoneWeights"].get_all_bones_info(source)
    info = {str(b.name): b for b in infos}
    by_index = {b.index: b for b in infos}
    _, vertex_ids, _ = GEOMETRY["MeshQueries"].get_all_vertex_ids(source)
    vertices = GEOMETRY["List"].convert_index_list_to_array(vertex_ids)
    weights = {}
    missing_weights = 0
    for vertex_id in vertices:
        _, entries, valid = GEOMETRY["BoneWeights"].get_vertex_bone_weights(source, vertex_id)
        if not valid:
            missing_weights += 1
        weights[vertex_id] = [(w.bone_index, w.weight) for w in entries] if valid else []
    _, triangle_ids, _ = GEOMETRY["MeshQueries"].get_all_triangle_ids(source)
    triangles = []
    for triangle_id in GEOMETRY["List"].convert_index_list_to_array(triangle_ids):
        vertex, valid = GEOMETRY["MeshQueries"].get_triangle_indices(source, triangle_id)
        if valid:
            triangles.append((triangle_id, (vertex.x, vertex.y, vertex.z)))
    require(missing_weights == 0, "Missing skin weights in " + label + "; cannot author reliable detachable parts")
    materials, _, _, material_result = GEOMETRY["AssetUtils"].get_lod_material_list_from_skeletal_mesh(
        mesh, unreal.GeometryScriptMeshReadLOD())
    require(material_result == unreal.GeometryScriptOutcomePins.SUCCESS, "Could not resolve skeletal section materials")
    parts, bones, used = [], [], set()
    REPORT["parts"][label] = {"source": mesh.get_path_name(), "unweighted_vertices": missing_weights,
                               "bones": sorted(info), "slots": []}
    for index, (part_name, expressions) in enumerate(patterns):
        bone = pick_bone(info, expressions, used)
        if bone is None:
            part = original_part_module(label, part_name, materials)
            REPORT["parts"][label]["slots"].append({"part": part_name, "status": "original detachable armor module",
                "bone": None, "asset": part.get_path_name(), "pivot": "module local space",
                "reason": "Source rig has no independent semantic bone; preserve all original skeletal geometry",
                "runtime": "NAME_None: visible static module at prepared part template; never hide a skeletal bone"})
            parts.append(part)
            bones.append(unreal.Name("None"))
            REPORT["warnings"].append(label + " " + part_name + ": original armor module supplements the source rig")
            continue
        used.add(bone)
        subtree = set()
        for candidate in infos:
            ancestor = candidate
            seen = set()
            while ancestor and ancestor.index not in seen:
                seen.add(ancestor.index)
                if ancestor.index == info[bone].index:
                    subtree.add(candidate.index)
                    break
                ancestor = by_index.get(ancestor.parent_index)
        influence = {vertex: sum(weight for idx, weight in values if idx in subtree)
                     for vertex, values in weights.items()}
        # Retain boundary triangles with any meaningful subtree influence. Report
        # low-weight omissions; never silently pretend dominant-bone slicing is exact.
        selected = [tid for tid, ids in triangles if max(influence[v] for v in ids) >= 0.01]
        omitted = sum(1 for _, ids in triangles if 0 < max(influence[v] for v in ids) < 0.01)
        require(len(selected) >= 4, label + " " + bone + " has no useful weighted geometry")
        _, selection = GEOMETRY["MeshSelection"].convert_index_array_to_mesh_selection(
            source, selected, unreal.GeometryScriptMeshSelectionType.TRIANGLES)
        local = unreal.DynamicMesh()
        GEOMETRY["MeshDecomposition"].copy_mesh_selection_to_mesh(source, local, selection)
        GEOMETRY["MeshTransforms"].inverse_transform_mesh(local, info[bone].world_transform)
        path = ROOT + "/Parts/SM_" + label + "_" + part_name
        part = save_static(local, path, materials)
        parts.append(part)
        bones.append(unreal.Name(bone))
        REPORT["parts"][label]["slots"].append({"part": part_name, "bone": bone, "subtree_bones": len(subtree),
            "triangles": len(selected), "source_triangles": len(triangles), "omitted_subthreshold_triangles": omitted,
            "pivot": "bone reference space", "asset": path})
    return parts, bones


def box_geometry(vertices, faces, center, size):
    offset = len(vertices)
    x, y, z = center
    a, b, c = [v * 0.5 for v in size]
    vertices.extend([(x-a,y-b,z-c),(x+a,y-b,z-c),(x+a,y+b,z-c),(x-a,y+b,z-c),
                     (x-a,y-b,z+c),(x+a,y-b,z+c),(x+a,y+b,z+c),(x-a,y+b,z+c)])
    faces.extend(tuple(offset + i for i in face) for face in
                 ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)))


def mesh_bounds(mesh):
    bounds = GEOMETRY["MeshQueries"].get_mesh_bounding_box(mesh)
    low, high = bounds.min, bounds.max
    center = unreal.Vector((low.x+high.x)*0.5, (low.y+high.y)*0.5, (low.z+high.z)*0.5)
    size = unreal.Vector(high.x-low.x, high.y-low.y, high.z-low.z)
    return center, size


def centered_mesh(mesh, height):
    center, size = mesh_bounds(mesh)
    require(size.z > 0.001, "Degenerate authored geometry")
    GEOMETRY["MeshTransforms"].translate_mesh(mesh, unreal.Vector(-center.x, -center.y, -center.z))
    factor = height / size.z
    GEOMETRY["MeshTransforms"].scale_mesh(mesh, unreal.Vector(factor, factor, factor))
    return mesh


def select_geometry(source, triangle_ids):
    require(len(triangle_ids) >= 4, "Selected robot assembly has no useful geometry")
    _, selection = GEOMETRY["MeshSelection"].convert_index_array_to_mesh_selection(
        source, triangle_ids, unreal.GeometryScriptMeshSelectionType.TRIANGLES)
    result = unreal.DynamicMesh()
    GEOMETRY["MeshDecomposition"].copy_mesh_selection_to_mesh(source, result, selection)
    return result


def save_static(mesh, path, materials):
    asset = existing(path, unreal.StaticMesh)
    if asset is None:
        ASSETS.make_directory(path.rsplit("/", 1)[0])
        options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
        options.set_editor_property("enable_collision", False)  # Runtime prepared bodies own collision.
        asset, result = GEOMETRY["NewAssetUtils"].create_new_static_mesh_asset_from_mesh(mesh, path, options)
        require(result == unreal.GeometryScriptOutcomePins.SUCCESS and asset, "Static assembly creation failed: " + path)
        ASSETS.set_metadata_tag(asset, TAG, VERSION)
        REPORT["created"].append(path)
    # Also refresh references on a partial rerun; geometry itself is reused.
    for slot, material in enumerate(materials):
        if material:
            asset.set_material(slot, material)
    return save(asset)


def primitive(mesh, material_id, kind, location, dimensions, rotation=(0, 0, 0)):
    options = unreal.GeometryScriptPrimitiveOptions()
    options.set_editor_property("material_id", material_id)
    pose = transform(location, rotation)
    if kind == "box":
        GEOMETRY["Primitives"].append_box(mesh, options, pose, *dimensions,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER)
    elif kind == "cone":
        GEOMETRY["Primitives"].append_cone(mesh, options, pose, dimensions[0], dimensions[1], dimensions[2],
            radial_steps=16, height_steps=1, origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER)
    else:
        GEOMETRY["Primitives"].append_cylinder(mesh, options, pose, dimensions[0], dimensions[1],
            radial_steps=20, origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER)


def original_part_module(label, part_name, source_materials):
    """Additive original armor where the licensed rig has no detachable subtree.

    These modules are visible while attached and use NAME_None in PartBones. They
    must not hide a head, torso, or any unrelated bone on detachment.
    """
    require((label, part_name) in (("Robot", "Claw"), ("Kaiju", "Horn"), ("Kaiju", "Tail")),
            "No approved original module recipe for " + label + " " + part_name)
    mesh = unreal.DynamicMesh()
    GEOMETRY["Materials"].enable_material_ids(mesh)
    if part_name == "Claw":
        # Wrist collar, armored palm and three hooked, tapered mechanical fingers.
        primitive(mesh, 0, "cylinder", (-38,0,0), (20,25), (90,0,0))
        primitive(mesh, 0, "box", (-12,0,0), (34,68,24))
        for y in (-25,0,25):
            primitive(mesh, 0, "cylinder", (6,y,0), (9,12), (0,0,90))
            primitive(mesh, 0, "cone", (25,y,5), (8,5,39), (75,0,0))
            primitive(mesh, 0, "cone", (47,y,-8), (5,0.8,29), (130,0,0))
    elif part_name == "Horn":
        # One removable crown plate carries a pair of upward armor horns.
        primitive(mesh, 0, "box", (0,0,-12), (63,87,18))
        primitive(mesh, 0, "box", (18,0,-7), (20,93,14), (16,0,0))
        for y, roll in ((-29,18),(29,-18)):
            primitive(mesh, 0, "cone", (2,y,21), (14,7,57), (12,0,roll))
            primitive(mesh, 0, "cone", (13,y*1.35,59), (7,0.7,35), (22,0,roll))
    else:
        # Seven articulated armor segments form a visible, removable rear appendage.
        primitive(mesh, 0, "cylinder", (18,0,0), (27,22), (90,0,0))
        for index in range(7):
            x = -index*24
            z = index*index*1.7
            radius = 23-index*2.6
            primitive(mesh, 0, "cone", (x,0,z), (radius, max(2,radius-3), 29), (-90+index*5,0,0))
            primitive(mesh, 0, "cylinder", (x+12,0,z), (radius+2,5), (-90+index*5,0,0))
        primitive(mesh, 0, "cone", (-167,0,89), (6,0.5,35), (-50,0,0))
    center, _ = mesh_bounds(mesh)
    GEOMETRY["MeshTransforms"].translate_mesh(mesh, unreal.Vector(-center.x,-center.y,-center.z))
    # Keep an already-authorized source PBR material; no new texture files.
    material = next((item for item in source_materials if item), None)
    require(material, "Source has no authorized material for original armor module")
    return save_static(mesh, ROOT + "/Parts/SM_" + label + "_" + part_name, [material])


def author_s4(robot):
    """Local deterministic derivatives, retaining source UVs and materials.

    No source mesh vertices, textures or other payloads leave Unreal/Python. The
    report records only selection rules/counts and bone metadata for review.
    """
    source = unreal.DynamicMesh()
    _, outcome = GEOMETRY["AssetUtils"].copy_mesh_from_skeletal_mesh(robot, source,
        unreal.GeometryScriptCopyMeshFromAssetOptions(), unreal.GeometryScriptMeshReadLOD())
    require(outcome == unreal.GeometryScriptOutcomePins.SUCCESS, "Robot source geometry unavailable")
    source_materials, _, _, outcome = GEOMETRY["AssetUtils"].get_lod_material_list_from_skeletal_mesh(
        robot, unreal.GeometryScriptMeshReadLOD())
    require(outcome == unreal.GeometryScriptOutcomePins.SUCCESS, "Robot section materials unavailable")
    metal = typed("/Game/IB_S25/Materials/M_R2v3_Metal", unreal.MaterialInterface)
    rubber = typed("/Game/Car/Assets/M_TireRubber", unreal.MaterialInterface)
    energy = energy_material()
    materials = list(source_materials) + [metal, rubber, energy]
    metal_id, rubber_id, energy_id = len(materials)-3, len(materials)-2, len(materials)-1
    _, bones = GEOMETRY["BoneWeights"].get_all_bones_info(source)
    by_index = {bone.index: bone for bone in bones}

    def descendants(pattern):
        found = {bone.index for bone in bones if re.search(pattern, str(bone.name), re.I)}
        expanded = set(found)
        for bone in bones:
            cursor = bone
            visited = set()
            while cursor and cursor.index not in visited:
                visited.add(cursor.index)
                if cursor.index in found:
                    expanded.add(bone.index)
                    break
                cursor = by_index.get(cursor.parent_index)
        return expanded

    legs = descendants(r"thigh|calf|(?:upper|lower)?leg|foot|toe")
    arms = descendants(r"(?:upper|lower)?arm|clavicle|hand|finger")
    weapons = descendants(r"cannon|barrel|gun|launcher")
    _, vertex_ids, _ = GEOMETRY["MeshQueries"].get_all_vertex_ids(source)
    influences, positions = {}, {}
    for vertex in GEOMETRY["List"].convert_index_list_to_array(vertex_ids):
        _, weights, valid = GEOMETRY["BoneWeights"].get_vertex_bone_weights(source, vertex)
        require(valid, "Robot assembly has unweighted source vertices")
        influences[vertex] = {
            "limb": sum(weight.weight for weight in weights if weight.bone_index in legs | arms),
            "weapon": sum(weight.weight for weight in weights if weight.bone_index in weapons)}
        position, valid = GEOMETRY["MeshQueries"].get_vertex_position(source, vertex)
        require(valid, "Robot source vertex missing")
        positions[vertex] = position
    _, triangle_ids, _ = GEOMETRY["MeshQueries"].get_all_triangle_ids(source)
    triangles = []
    for triangle in GEOMETRY["List"].convert_index_list_to_array(triangle_ids):
        ids, valid = GEOMETRY["MeshQueries"].get_triangle_indices(source, triangle)
        if valid:
            triangles.append((triangle, (ids.x, ids.y, ids.z)))
    all_ids = [triangle for triangle, _ in triangles]
    weapon_ids = [triangle for triangle, ids in triangles if max(influences[v]["weapon"] for v in ids) >= 0.01]
    body_ids = [triangle for triangle, ids in triangles if max(influences[v]["weapon"] for v in ids) < 0.01]
    core_ids = [triangle for triangle, ids in triangles
                if max(influences[v]["limb"] for v in ids) < 0.01 and
                max(influences[v]["weapon"] for v in ids) < 0.01]
    core_rule = "skin weights exclude arm and leg subtrees"
    if not legs or not arms or len(core_ids) < 20:
        # Unknown rig naming: upper central geometry, with no lower half/feet.
        # This is an explicit geometry rule, not a guessed anatomical bone name.
        center, size = mesh_bounds(source)
        core_ids = [triangle for triangle, ids in triangles
                    if all(positions[v].z >= center.z and
                           abs(positions[v].x-center.x) <= size.x*0.24 and
                           abs(positions[v].y-center.y) <= size.y*0.35 for v in ids)]
        core_rule = "upper central bounds: z>=center, |x|<=24% width, |y|<=35% depth"
    require(len(core_ids) >= 20, "Could not isolate a legless central robot assembly")

    small = centered_mesh(select_geometry(source, body_ids if weapon_ids else all_ids), 200)
    small_asset = save_static(small, ROOT + "/Units/SM_SmallRobot", source_materials)

    # Robot central assembly plus four original propulsion nacelles and armor fins.
    drone = centered_mesh(select_geometry(source, core_ids), 95)
    for x in (-75, 75):
        for y in (-70, 70):
            primitive(drone, metal_id, "box", (x*0.5,y*0.5,-5), (90,16,12), (0,0,0))
            primitive(drone, metal_id, "cylinder", (x,y,-20), (28,38))
            primitive(drone, rubber_id, "cylinder", (x,y,-39), (23,5))
            primitive(drone, energy_id, "cylinder", (x,y,-42), (16,3))
    drone_asset = save_static(drone, ROOT + "/Units/SM_Drone", materials)

    # A separate cannon remains available even if this pack names its weapons as
    # generic arm bones. The fallback is original geometry with existing PBR metal.
    if len(weapon_ids) >= 20:
        cannon = select_geometry(source, weapon_ids)
        center, size = mesh_bounds(cannon)
        GEOMETRY["MeshTransforms"].translate_mesh(cannon, unreal.Vector(-center.x,-center.y,-center.z))
        factor = 180 / max(size.x, size.y, size.z, 0.001)
        GEOMETRY["MeshTransforms"].scale_mesh(cannon, unreal.Vector(factor,factor,factor))
        if size.z > max(size.x, size.y):
            GEOMETRY["MeshTransforms"].rotate_mesh(cannon, unreal.Rotator(pitch=90, yaw=0, roll=0))
        elif size.y > size.x:
            GEOMETRY["MeshTransforms"].rotate_mesh(cannon, unreal.Rotator(pitch=0, yaw=90, roll=0))
        cannon_rule = "actual robot weapon bone subtree"
    else:
        cannon = unreal.DynamicMesh()
        GEOMETRY["Materials"].enable_material_ids(cannon)
        primitive(cannon, metal_id, "cylinder", (35,0,0), (13,150), (90,0,0))
        primitive(cannon, metal_id, "box", (-45,0,-8), (70,46,44))
        primitive(cannon, metal_id, "box", (-45,0,-33), (32,58,16))
        for x in (-10,10,30,50,70,90):
            primitive(cannon, metal_id, "cylinder", (x,0,0), (17,5), (90,0,0))
        primitive(cannon, rubber_id, "cylinder", (111,0,0), (10,1), (90,0,0))
        cannon_rule = "original cannon barrel, breech, cooling collars; existing metal/rubber materials"
    cannon_asset = save_static(cannon, ROOT + "/Units/SM_Cannon", materials)

    # Original tracked chassis and turret geometry surrounds the textured robot
    # core. No generic engine cube is used as a visible production asset.
    tank = centered_mesh(select_geometry(source, core_ids), 120)
    GEOMETRY["MeshTransforms"].translate_mesh(tank, unreal.Vector(-30,0,155))
    primitive(tank, metal_id, "box", (-15,0,65), (390,235,80))
    primitive(tank, metal_id, "box", (145,0,90), (90,230,32), (18,0,0))
    primitive(tank, metal_id, "cylinder", (-35,0,124), (92,28))
    primitive(tank, metal_id, "box", (-35,0,155), (155,160,65))
    for y in (-145,145):
        primitive(tank, rubber_id, "box", (0,y,49), (365,65,80))
        for x in (-145,-75,0,75,145):
            primitive(tank, metal_id, "cylinder", (x,y,50), (39,70), (0,0,90))
            primitive(tank, rubber_id, "cylinder", (x,y,50), (43,64), (0,0,90))
        for x in range(-180,181,24):
            for z in (8,90):
                primitive(tank, metal_id, "box", (x,y,z), (17,78,8))
        for x in (-194,194):
            for z in (27,50,73):
                primitive(tank, metal_id, "box", (x,y,z), (9,78,17))
    # Append already prepared cannon along +X at turret height, preserving material IDs.
    GEOMETRY["MeshEdits"].append_mesh(tank, cannon,
        transform((85,0,166), scale=(1.4,1.4,1.4)))
    tank_asset = save_static(tank, ROOT + "/Units/SM_Tank", materials)
    REPORT["s4_units"] = {"source": robot.get_path_name(), "source_triangles": len(triangles),
        "core_triangles": len(core_ids), "core_selection": core_rule, "weapon_triangles": len(weapon_ids),
        "cannon_selection": cannon_rule, "pose": "source reference pose, geometry processed locally",
        "derived_assets": [asset.get_path_name() for asset in (small_asset, drone_asset, tank_asset, cannon_asset)],
        "original_details": "drone nacelles; tank hull, tracks, wheels, turret; fallback cannon"}
    return {"SmallRobot": small_asset, "Drone": drone_asset, "Tank": tank_asset, "Cannon": cannon_asset,
        "Concrete": typed("/Game/IB_S25/Materials/M_R2v3_StoneDetail", unreal.MaterialInterface),
        "Ground": typed("/Game/IB_S25/Materials/M_Paving", unreal.MaterialInterface),
        "Glass": typed("/Game/Car/Assets/M_Glass", unreal.MaterialInterface), "Metal": metal}


def import_original(name, vertices, faces, material, stage):
    path = ROOT + "/Props/" + name
    mesh = existing(path, unreal.StaticMesh)
    if mesh:
        mesh.set_material(0, material)
        return save(mesh)
    obj = stage / (name + ".obj")
    # Original authored geometry only; licensed assets never exported to OBJ.
    lines = ["v %.7f %.7f %.7f" % vertex for vertex in vertices]
    uv = [(0,0),(1,0),(1,1),(0,1)]
    lines += ["vt %f %f" % item for item in uv]
    for face in faces:
        lines.append("f " + " ".join(str(vertex+1) + "/" + str(i+1) for i, vertex in enumerate(face)))
    obj.write_text("\n".join(lines) + "\n", encoding="ascii")
    options = unreal.FbxImportUI()
    for key, value in {"import_mesh": True, "import_as_skeletal": False,
                       "import_materials": False, "import_textures": False,
                       "mesh_type_to_import": unreal.FBXImportType.FBXIT_STATIC_MESH}.items():
        options.set_editor_property(key, value)
    options.static_mesh_import_data.set_editor_property("auto_generate_collision", True)
    task = unreal.AssetImportTask()
    for key, value in {"filename": str(obj), "destination_path": ROOT + "/Props", "destination_name": name,
                       "automated": True, "replace_existing": False, "save": False, "options": options}.items():
        task.set_editor_property(key, value)
    TOOLS.import_asset_tasks([task])
    mesh = typed(path, unreal.StaticMesh)
    mesh.set_material(0, material)
    REPORT["created"].append(path)
    return save(mesh)


def author_props(stage):
    car = typed("/Game/Car/SM_AutomotiveTP_Car", unreal.StaticMesh)
    metal = typed("/Game/IB_S25/Materials/M_R2v3_RoofMetal", unreal.MaterialInterface)
    stone = typed("/Game/IB_S25/Materials/M_R2v3_StoneDetail", unreal.MaterialInterface)
    vertices, faces = [], []
    box_geometry(vertices, faces, (0,0,0), (600,240,240))
    for x in range(-280, 281, 28):
        for y in (-122,122):
            box_geometry(vertices, faces, (x,y,0), (5,5,230))
    for x in (-302,302):
        box_geometry(vertices, faces, (x,0,0), (6,5,230))
        for y in (-62,62):
            box_geometry(vertices, faces, (x,y,0), (8,4,210))
    for x in (-290,290):
        for y in (-115,115):
            box_geometry(vertices, faces, (x,y,0), (14,14,250))
    container = import_original("SM_CorrugatedContainer", vertices, faces, metal, stage)
    vertices = [(0,0,-70)]
    for ring in range(1, 5):
        latitude = -math.pi/2 + math.pi*ring/5
        for i in range(12):
            angle = math.tau*i/12
            jitter = 1 + 0.13*math.sin(i*2.77+ring*1.81)
            vertices.append((math.cos(angle)*math.cos(latitude)*110*jitter,
                             math.sin(angle)*math.cos(latitude)*85*jitter, math.sin(latitude)*80*jitter))
    top = len(vertices)
    vertices.append((5,-3,84))
    faces = []
    for i in range(12):
        j = (i+1)%12
        faces.extend(((0,1+j,1+i),(top,37+i,37+j)))
        for ring in range(3):
            a, b = 1+ring*12+i, 1+ring*12+j
            faces.extend(((a,b,b+12),(a,b+12,a+12)))
    rock = import_original("SM_FieldRock", vertices, faces, stone, stage)
    return [car, container, rock]


def author_map(art):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    game_mode = require(unreal.load_class(None, "/Script/ImpactBeyond.IBPlayGameMode"), "Compile IBPlayGameMode first")
    planet_class = require(unreal.load_class(None, "/Script/ImpactBeyond.IBS3PlanetWorld"), "Compile S3 first")
    session_class = require(unreal.load_class(None, "/Script/ImpactBeyond.IBPlaySession"), "Compile IBPlaySession first")
    world = existing(MAP, unreal.World)
    if world is None:
        world, _ = duplicate(typed(SOURCE_MAP, unreal.World), MAP, unreal.World)
        ASSETS.set_metadata_tag(world, TAG, VERSION)
        require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "Could not save duplicated playable map")
    require(levels.load_level(MAP), "Could not open isolated playable map")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    planets = [actor for actor in actors.get_all_level_actors() if actor.get_class() == planet_class]
    require(len(planets) == 1, "Playable map requires exactly one S3 planet")
    planets[0].set_editor_property("bProductionVisuals", True)
    planets[0].set_editor_property("SaveSlot", "IB_Playable_S7_v1_planet")
    sessions = [actor for actor in actors.get_all_level_actors() if actor.get_class() == session_class]
    require(len(sessions) <= 1, "Duplicate playable sessions in map")
    session = sessions[0] if sessions else actors.spawn_actor_from_class(session_class, unreal.Vector())
    require(session, "Could not create playable session actor")
    session.set_editor_property("SaveSlot", "IB_Playable_S7_v1")
    # LoadObject string literals alone do not guarantee cooking. This serialized
    # actor reference roots the data asset and its selected art in the entry map.
    session.set_editor_property("Art", art)
    world.get_world_settings().set_editor_property("default_game_mode", game_mode)
    world.get_world_settings().set_editor_property("kill_z", -1000000000.0)
    ASSETS.set_metadata_tag(world, TAG, VERSION)
    require(levels.save_current_level(), "Playable map save failed")
    REPORT["map"] = {"path": MAP, "source": SOURCE_MAP, "mode": game_mode.get_path_name(),
                     "planet_slot": "IB_Playable_S7_v1_planet", "session_slot": "IB_Playable_S7_v1",
                     "serialized_art_reference": art.get_path_name()}


def dependency_report(seeds):
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous([ROOT], force_rescan=True)
    options = unreal.AssetRegistryDependencyOptions(include_soft_package_references=True,
        include_hard_package_references=True, include_game_package_references=True,
        include_editor_only_package_references=False, include_searchable_names=False,
        include_soft_management_references=False, include_hard_management_references=False)
    def closure(roots, dependency_options):
        pending, found = list(roots), set()
        while pending:
            package = pending.pop().split(".")[0]
            if package in found or not package.startswith("/Game/"):
                continue
            found.add(package)
            pending.extend(str(dependency) for dependency in (registry.get_dependencies(package, dependency_options) or []))
        return found
    packages = closure(seeds, options)
    hard_options = unreal.AssetRegistryDependencyOptions(include_soft_package_references=False,
        include_hard_package_references=True, include_game_package_references=True,
        include_editor_only_package_references=False, include_searchable_names=False,
        include_soft_management_references=False, include_hard_management_references=False)
    hard_map_packages = closure([MAP], hard_options)
    art_package = ROOT + "/DA_PlayArt"
    REPORT["cook"] = {"entry_map": MAP, "art_root": art_package,
        "art_is_hard_map_dependency": art_package in hard_map_packages,
        "hard_map_packages": sorted(hard_map_packages),
        "instruction": "Cook the entry map; its serialized Session.Art roots the selected art. Validate the packaged game separately."}
    require(art_package in hard_map_packages, "Entry map does not retain a hard package reference to DA_PlayArt")
    donor = []
    missing = []
    local_content = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()))
    for package in sorted(packages):
        relative = package[len("/Game/"):]
        local_files = [str(Path(relative + suffix)).replace("\\", "/") for suffix in
                       (".uasset", ".umap", ".uexp", ".ubulk", ".uptnl") if (local_content / (relative + suffix)).is_file()]
        if not local_files:
            missing.append(package)
        if package.startswith(ROOT + "/"):
            continue
        donor_files = [str(Path(relative + suffix)).replace("\\", "/") for suffix in
                       (".uasset", ".umap", ".uexp", ".ubulk", ".uptnl") if (DONOR_CONTENT / (relative + suffix)).is_file()]
        if donor_files:
            donor.append({"package": package, "files": donor_files})
    REPORT["dependencies"] = {"roots": sorted(set(seeds)), "game_packages": sorted(packages),
        "donor_content": str(DONOR_CONTENT), "donor_packages_to_copy": donor,
        "missing_local_packages": missing, "editor_only_dependencies_excluded": True,
        "copy_policy": "Copy exact relative package files, preserving /Game paths; cook only referenced packages"}
    require(not missing, "Dependency closure contains missing local packages; see report")


def main():
    global DONOR_CONTENT
    stage = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir())) / "IB_Playable_Authoring"
    stage.mkdir(parents=True, exist_ok=True)
    api_only = os.environ.get("IB_PLAY_API_ONLY", "") == "1"
    try:
        geometry_preflight()
        native_property_preflight()
        if api_only:
            REPORT["status"] = "api_preflight_passed"
            return
        # Validate portable input before creating or modifying any content asset.
        DONOR_CONTENT = configured_donor_content()
        ASSETS.make_directory(ROOT)
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        registry.scan_paths_synchronous(["/Game/Adventures_Pack", "/Game/robot13", "/Game/SciFi_Beast06", "/Game/Car"])
        hero, manny = typed(HERO, unreal.SkeletalMesh), typed(MANNY, unreal.SkeletalMesh)
        validate_leader(hero, manny)
        robot, kaiju = typed(ROBOT, unreal.SkeletalMesh), typed(KAIJU, unreal.SkeletalMesh)
        heroes, accents = author_heroes(hero)
        robot_parts, robot_bones = author_parts(robot, "Robot", [
            ("Hammer", [r"hammer", r"mace", r"weapon.*r", r"hand[_ .-]?r$", r"r[_ .-]?hand$"]),
            ("Claw", [r"claw", r"pincer", r"hand[_ .-]?l$", r"l[_ .-]?hand$"])])
        kaiju_parts, kaiju_bones = author_parts(kaiju, "Kaiju", [
            ("Horn", [r"horn.*(?:root|01|1)$", r"horn"]),
            ("Tail", [r"tail[_ .-]?(?:root|01|1)$", r"tail"]),
            ("Shoulder", [r"shoulder.*r", r"clavicle[_ .-]?r$", r"upperarm[_ .-]?r$"])])
        data_class = require(unreal.load_class(None, "/Script/ImpactBeyond.IBPlayArt"), "Compile UIBPlayArt first")
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", data_class)
        # Reflected native data class is a UClass, not a Python isinstance type.
        if ASSETS.does_asset_exist(ROOT + "/DA_PlayArt"):
            art = load(ROOT + "/DA_PlayArt", None)
            require(art.get_class() == data_class and ASSETS.get_metadata_tag(art, TAG) == VERSION,
                    "Refusing unrelated DA_PlayArt")
        else:
            art = require(TOOLS.create_asset("DA_PlayArt", ROOT, data_class, factory), "Data asset creation failed")
            ASSETS.set_metadata_tag(art, TAG, VERSION)
        values = {"Heroes": heroes, "CitizenMesh": hero, "HeroAccents": accents,
            "EnergyMaterial": energy_material(), "Robot": robot, "Kaiju": kaiju,
            "RobotAnimations": validate_clips(robot, ROBOT_CLIPS),
            "KaijuAnimations": validate_clips(kaiju, KAIJU_CLIPS),
            "RobotParts": robot_parts, "RobotPartBones": robot_bones,
            "KaijuParts": kaiju_parts, "KaijuPartBones": kaiju_bones,
            "Props": author_props(stage)}
        values.update(author_s4(robot))
        for key, value in values.items():
            art.set_editor_property(key, value)
        save(art)
        author_map(art)
        # Explicit C++ runtime asset paths are additional cook roots, because a
        # LoadObject string alone is not necessarily recorded by map serialization.
        runtime = [MANNY] + ["/Game/IB_Flight/" + name for name in (
            "A_FlightCruise", "A_FlightSpeed2", "A_FlightSpeed3", "A_FlightSpeed4", "A_FlightSpeed5",
            "A_FlightCarry", "A_FlightBrake", "A_FlightThrow")]
        runtime += ["/Game/Characters/Mannequins/Anims/Unarmed/" + name for name in (
            "MM_Idle", "Walk/MF_Unarmed_Walk_Fwd", "Jog/MF_Unarmed_Jog_Fwd")]
        dependency_report([MAP, ROOT + "/DA_PlayArt"] + runtime)
        REPORT["status"] = "authored_requires_runtime_validation"
        unreal.log("IB playable authoring finished; metadata report: " + str(stage / "author-report.json"))
    except Exception as error:
        REPORT["status"] = "failed"
        REPORT["error"] = str(error)
        raise
    finally:
        report_name = "api-report.json" if api_only else "author-report.json"
        (stage / report_name).write_text(json.dumps(REPORT, indent=2), encoding="utf-8")


main()
