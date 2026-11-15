"""Run in a coordinated UE 5.8 editor session. Default: read-only preflight.

IBS25_ASSETS points to a JSON object overriding IBS25_recipe.DEFAULT_ASSETS.
IBS25_EXECUTE=1 enables authoring ONLY after all required assets pass preflight.
IBS25_GENERATED points to the output of IBS25_recipe.py (IBS25_generated).
No main-map load, plugin activation, donor mutation or automatic editor launch.
"""
import json
import math
import os
from pathlib import Path
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import IBS25_recipe as recipe
import unreal as ue

ROOT = "/Game/IB_S25"
assets = ue.EditorAssetLibrary
levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)


def check(condition, message):
    if not condition:
        raise RuntimeError(message)


def point(values):
    return ue.Vector(*(v*100 for v in values))


def owned(asset):
    return assets.get_metadata_tag(asset, "IBS25Owner") == recipe.OWNER


def stamp(asset):
    check(asset.get_path_name().startswith(ROOT + "/"), "Refusing donor mutation")
    assets.set_metadata_tag(asset, "IBS25Owner", recipe.OWNER)
    check(assets.save_loaded_asset(asset), "Could not save " + asset.get_path_name())


def preflight(config):
    loaded, missing = {}, []
    for key, path in config.items():
        if not path or not isinstance(path, str) or not path.startswith("/Game/"):
            missing.append(key + ": explicit local /Game asset required")
            continue
        if any(word in path.lower() for word in ("basicshapes", "citykit", "sprintvehicles")):
            missing.append(key + ": foreground proxy is prohibited")
            continue
        asset = ue.load_asset(path)
        expected = ue.MaterialInterface if key == "interior" else ue.Texture2D if key.startswith(("asphalt_", "paving_")) else ue.StaticMesh
        if not isinstance(asset, expected):
            missing.append(key + ": missing or wrong asset type: " + path)
            continue
        if isinstance(asset, ue.Texture2D):
            # Do not alter donor textures. Import correctly or use owned copies first.
            color = key.endswith("basecolor")
            if bool(asset.get_editor_property("srgb")) != color:
                missing.append(key + ": sRGB must be " + str(color))
            if key.endswith("normal_dx") and asset.get_editor_property("compression_settings") != ue.TextureCompressionSettings.TC_NORMALMAP:
                missing.append(key + ": requires normal-map compression / DirectX +Y convention")
        if isinstance(asset, ue.StaticMesh):
            b = asset.get_bounds()
            if min(b.box_extent.x, b.box_extent.y, b.box_extent.z) <= 0:
                missing.append(key + ": degenerate mesh bounds")
        loaded[key] = asset
    check(hasattr(ue, "ImpactFacade"), "Existing ImpactFacade class unavailable; no automatic build")
    return loaded, missing


def imported_mesh(name, generated):
    path = f"{ROOT}/Geometry/SM_{name}"
    if assets.does_asset_exist(path):
        result = ue.load_asset(path)
        check(owned(result), "Foreign asset occupies " + path)
        check(assets.get_metadata_tag(result, "IBS25Source") == recipe.digest((generated / (name+".obj")).read_text()),
              "Generated geometry changed; use a new version, do not overwrite")
        return result
    task = ue.AssetImportTask()
    task.filename = str(generated / (name + ".obj"))
    task.destination_path = ROOT + "/Geometry"
    task.destination_name = "SM_" + name
    task.automated, task.save, task.replace_existing = True, False, False
    options = ue.FbxImportUI()
    options.import_mesh, options.import_materials, options.import_textures = True, False, False
    options.import_as_skeletal = False
    options.static_mesh_import_data.set_editor_property("combine_meshes", True)
    options.static_mesh_import_data.set_editor_property("convert_scene", False)
    options.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
    task.options = options
    ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    result = ue.load_asset(path)
    check(isinstance(result, ue.StaticMesh), "OBJ import did not produce " + path)
    body = result.get_editor_property("body_setup")
    check(body is not None, "Imported mesh has no BodySetup")
    body.set_editor_property("collision_trace_flag", ue.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    assets.set_metadata_tag(result, "IBS25Source", recipe.digest((generated / (name+".obj")).read_text()))
    stamp(result)
    return result


def material(name, textures=None):
    path = f"{ROOT}/Materials/M_{name}"
    fingerprint = recipe.digest({key:value.get_path_name() for key,value in textures.items()} if textures else {"paint_roughness":0.72})
    if assets.does_asset_exist(path):
        result = ue.load_asset(path)
        check(owned(result), "Foreign material occupies " + path)
        check(assets.get_metadata_tag(result,"IBS25Inputs") == fingerprint, "Material dependencies changed: " + path)
        return result
    editing = ue.MaterialEditingLibrary
    result = ue.AssetToolsHelpers.get_asset_tools().create_asset("M_"+name, ROOT+"/Materials", ue.Material, ue.MaterialFactoryNew())
    check(result is not None, "Material factory failed")
    if textures:
        for key, prop in (("basecolor", ue.MaterialProperty.MP_BASE_COLOR),
                          ("normal_dx", ue.MaterialProperty.MP_NORMAL),
                          ("roughness", ue.MaterialProperty.MP_ROUGHNESS)):
            node = editing.create_material_expression(result, ue.MaterialExpressionTextureSample, -400, 0)
            node.set_editor_property("texture", textures[key])
            node.set_editor_property("sampler_type", ue.MaterialSamplerType.SAMPLERTYPE_NORMAL if key == "normal_dx" else ue.MaterialSamplerType.SAMPLERTYPE_COLOR if key == "basecolor" else ue.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
            check(editing.connect_material_property(node, "R" if key == "roughness" else "RGB", prop), "Material connection failed")
    else:
        color = editing.create_material_expression(result, ue.MaterialExpressionConstant3Vector, -400, 0)
        color.set_editor_property("constant", ue.LinearColor(0.65,0.65,0.58,1))
        editing.connect_material_property(color, "", ue.MaterialProperty.MP_BASE_COLOR)
        rough = editing.create_material_expression(result, ue.MaterialExpressionConstant, -400, 200)
        rough.set_editor_property("r", 0.72)
        editing.connect_material_property(rough, "", ue.MaterialProperty.MP_ROUGHNESS)
    editing.recompile_material(result)
    assets.set_metadata_tag(result,"IBS25Inputs",fingerprint)
    stamp(result)
    return result


def car_paint(index):
    # The existing MI_CarPaint has a missing parent. Reuse its real base material
    # through an owned instance, preserving the donor and its public provenance.
    name = "MI_CarPaint_" + str(index % 3)
    path = ROOT + "/Materials/" + name
    result = ue.load_asset(path) if assets.does_asset_exist(path) else None
    if result:
        check(owned(result), "Foreign paint instance occupies " + path)
    parent_path = ROOT + "/Materials/M_CarPaint_Local"
    parent = ue.load_asset(parent_path) if assets.does_asset_exist(parent_path) else None
    if parent:
        check(owned(parent), "Foreign car paint base occupies " + parent_path)
    else:
        parent = assets.duplicate_asset("/Game/Car/Assets/M_CarPaint", parent_path)
        check(isinstance(parent, ue.Material), "Car paint base unavailable")
        # Original migration left two texture references at /Game/ConceptCar/Car/Assets.
        # Repair the owned copy to existing local assets, not the donor graph.
        missing = [n for n in ue.MaterialEditingLibrary.get_material_expressions(parent)
                   if isinstance(n, ue.MaterialExpressionTextureSample) and not n.texture]
        check(len(missing) == 2, "Unexpected missing car paint texture nodes")
        for node in missing:
            texture_name = "T_CarPaintFlakes_N" if node.sampler_type == ue.MaterialSamplerType.SAMPLERTYPE_NORMAL else "T_Smudges_R"
            texture = ue.load_asset("/Game/Car/Assets/" + texture_name)
            check(isinstance(texture, ue.Texture2D), "Missing car texture " + texture_name)
            node.set_editor_property("texture", texture)
        ue.MaterialEditingLibrary.recompile_material(parent)
        stamp(parent)
    if not result:
        result = ue.AssetToolsHelpers.get_asset_tools().create_asset(
            name, ROOT+"/Materials", ue.MaterialInstanceConstant, ue.MaterialInstanceConstantFactoryNew())
    ue.MaterialEditingLibrary.set_material_instance_parent(result, parent)
    colors = ((0.018,0.065,0.16,1), (0.32,0.018,0.012,1), (0.44,0.46,0.48,1))
    ue.MaterialEditingLibrary.set_material_instance_vector_parameter_value(result, "PaintColor", ue.LinearColor(*colors[index % 3]))
    ue.MaterialEditingLibrary.update_material_instance(result)
    stamp(result)
    return result


def author(data, loaded, generated, config_hash):
    # Reruns only accept this exact recipe/dependency set and exclusively owned actors.
    tag = "IBS25Recipe_" + recipe.digest(data) + "_" + config_hash
    existed = assets.does_asset_exist(recipe.MAP)
    if existed:
        check(levels.load_level(recipe.MAP), "Cannot load lab")
        existing = actors.get_all_level_actors()
        markers = [a for a in existing if a.get_actor_label() == "IBS25_MANIFEST"]
        check(len(markers) == 1 and tag in [str(t) for t in markers[0].tags], "Existing map has different authorship/recipe. Stop.")
        foreign = [a for a in existing if not a.get_actor_label().startswith("IBS25_") and a.get_class().get_name() not in ("WorldSettings", "Brush")]
        check(not foreign, "Lab has manual/foreign actors. Stop rather than overwrite.")
    else:
        check(levels.new_level(recipe.MAP), "Cannot create isolated lab")
    lookup = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

    def actor(name, cls, xyz=(0,0,0), yaw=0):
        label = "IBS25_" + name
        obj = lookup.get(label)
        if obj is not None:
            check(isinstance(obj, cls), "Actor class conflict: " + label)
            check(recipe.OWNER in [str(t) for t in obj.tags], "Foreign actor occupies " + label)
        else:
            obj = actors.spawn_actor_from_class(cls, point(xyz), ue.Rotator(yaw=yaw))
        check(obj is not None, "Spawn failed: " + label)
        obj.set_actor_label(label)
        obj.set_folder_path("IB_S25/RealismLab")
        obj.set_actor_location(point(xyz), False, True)
        obj.set_actor_rotation(ue.Rotator(yaw=yaw), True)
        obj.set_editor_property("tags", [ue.Name(recipe.OWNER)])
        return obj

    marker = actor("MANIFEST", ue.Actor)
    marker.tags = [ue.Name(recipe.OWNER), ue.Name(tag)]
    ground_materials = {name: material(name, {key: loaded[f"{kind}_{key}"] for key in ("basecolor", "normal_dx", "roughness")})
                        for name, kind in (("Asphalt", "asphalt"), ("Paving", "paving"))}
    ground_materials["Markings"] = material("Markings")
    ground_materials["Roof"] = ground_materials["Paving"]
    for name in recipe.geometry(data):
        obj = actor(name, ue.StaticMeshActor)
        # OBJ imports reflect Y into Unreal's handedness even with convert_scene=False.
        # Recipe vertices are already authored in Unreal coordinates: undo that reflection.
        obj.set_actor_scale3d(ue.Vector(1,-1,1))
        component = obj.static_mesh_component
        component.set_static_mesh(imported_mesh(name, generated))
        component.set_material(0, ground_materials[name])
        component.set_collision_profile_name("NoCollision" if name == "Markings" else "BlockAll")
        component.set_cast_shadow(name == "Roof")

    for building in data["buildings"]:
        batches = {}
        for panel in recipe.facade_panels(building):
            mesh = loaded[panel["module"]]
            bounds = mesh.get_bounds()
            wide_x = bounds.box_extent.x > bounds.box_extent.y
            width = bounds.box_extent.x if wide_x else bounds.box_extent.y
            sx, sy = (panel["width_m"]*50/width, 1) if wide_x else (1, panel["width_m"]*50/width)
            sz = panel["height_m"]*50/bounds.box_extent.z
            yaw = math.radians(panel["yaw"] - (0 if wide_x else 90))
            ox, oy = bounds.origin.x*sx, bounds.origin.y*sy
            t = ue.Transform()
            t.translation = point(panel["center_m"]) - ue.Vector(ox*math.cos(yaw)-oy*math.sin(yaw), ox*math.sin(yaw)+oy*math.cos(yaw), bounds.origin.z*sz)
            t.rotation = ue.Quat(0,0,math.sin(yaw/2),math.cos(yaw/2))
            t.scale3d = ue.Vector(sx,sy,sz)
            batches.setdefault((panel["module"], panel["interior"]), []).append(t)
        for (module, interior), transforms in batches.items():
            obj = actor(f"{building['id']}_{module}_{interior}", ue.ImpactFacade)
            c = obj.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
            c.set_static_mesh(loaded[module])
            c.set_collision_profile_name("NoCollision")
            c.set_cast_shadow(True)
            if interior:
                matches = 0
                for index, slot in enumerate(loaded[module].static_materials):
                    source = slot.material_interface
                    if source and "glass" in source.get_name().lower():
                        c.set_material(index, loaded["interior"])
                        matches += 1
                check(matches > 0, "No glass slot found; refusing whole-facade interior override")
            c.clear_instances()
            c.add_instances(transforms, False, True, False)
            check(c.get_instance_count() == len(transforms), "Instance count mismatch")
        # Invisible collision only; no primitive is used as an artistic building.
        obj = actor(building["id"]+"_Collision", ue.StaticMeshActor,
                    (*building["center_m"], building["base_m"]+building["stories"]*building["floor_m"]/2))
        c = obj.static_mesh_component
        c.set_static_mesh(ue.load_asset("/Engine/BasicShapes/Cube"))
        c.set_visibility(False)
        c.set_hidden_in_game(True)
        c.set_cast_shadow(False)
        c.set_collision_profile_name("BlockAll")
        obj.set_actor_scale3d(ue.Vector(building["width_m"]-0.2,building["depth_m"]-0.2,building["stories"]*building["floor_m"]))

    for kind, entries in (("car",data["cars"]), ("tree",data["trees"])):
        mesh = loaded[kind]
        b = mesh.get_bounds()
        for entry in entries:
            # Uniform scale preserves the recognizable silhouette and physical proportions.
            scale = entry["height_m"]*50/b.box_extent.z if kind == "tree" else 450/(2*max(b.box_extent.x,b.box_extent.y))
            yaw = entry["yaw"] + (90 if kind == "car" and b.box_extent.y > b.box_extent.x else 0)
            angle = math.radians(yaw)
            origin = ue.Vector((b.origin.x*math.cos(angle)-b.origin.y*math.sin(angle))*scale,
                               (b.origin.x*math.sin(angle)+b.origin.y*math.cos(angle))*scale,
                               (b.origin.z-b.box_extent.z)*scale)
            obj = actor(entry["id"], ue.StaticMeshActor, entry["position_m"], yaw)
            obj.set_actor_location(point(entry["position_m"])-origin,False,True)
            obj.set_actor_scale3d(ue.Vector(scale,scale,scale))
            c = obj.static_mesh_component
            c.set_static_mesh(mesh)
            if kind == "tree":
                # Tested automatic reductions erase leaf silhouettes at distance.
                # Keep source LOD for this visual lab; performance acceptance is pending.
                c.set_editor_property("forced_lod_model", 1)
            if kind == "car":
                # The existing ConceptCar mesh has five unassigned material slots.
                # Override the lab component only; never change the donor mesh/materials.
                names = ("M_TireRubber", "MI_CarPaint", "M_Glass", "M_Metal", "M_TireRubber")
                check(c.get_num_materials() == len(names), "Unexpected ConceptCar material slots")
                for index, name in enumerate(names):
                    donor = car_paint(int(entry["id"].split("_")[-1])) if index == 1 else ue.load_asset("/Game/Car/Assets/" + name)
                    check(isinstance(donor, ue.MaterialInterface), "Missing car material " + name)
                    c.set_material(index, donor)
            c.set_collision_profile_name("BlockAll" if kind == "car" else "NoCollision")
            c.set_cast_shadow(True)
    sun = actor("Sun", ue.DirectionalLight)
    sun.set_actor_rotation(ue.Rotator(pitch=data["sun"]["pitch"],yaw=data["sun"]["yaw"]),True)
    sun.light_component.set_mobility(ue.ComponentMobility.MOVABLE)
    sun.light_component.set_editor_property("intensity",data["sun"]["lux"])
    sun.light_component.set_editor_property("atmosphere_sun_light",True)
    actor("Atmosphere",ue.SkyAtmosphere)
    sky = actor("Sky",ue.SkyLight)
    sky.light_component.set_mobility(ue.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property("real_time_capture",True)
    pp = actor("Exposure",ue.PostProcessVolume)
    pp.set_editor_property("unbound",True)
    settings = pp.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_min_brightness",True)
    settings.set_editor_property("override_auto_exposure_max_brightness",True)
    settings.set_editor_property("auto_exposure_min_brightness",data["exposure_ev100"])
    settings.set_editor_property("auto_exposure_max_brightness",data["exposure_ev100"])
    pp.set_editor_property("settings",settings)
    actor("PlayerStart",ue.PlayerStart,(54,39,1.3))
    for view in data["views"]:
        camera = actor(view["id"],ue.CameraActor,view["position_m"])
        pitch,yaw,roll = view["rotation_deg"]
        camera.set_actor_rotation(ue.Rotator(pitch=pitch,yaw=yaw,roll=roll),True)
        camera.camera_component.set_editor_property("field_of_view",view["fov"])
        camera.camera_component.set_editor_property("aspect_ratio",16/9)
        ue.log("IBS25 runtime camera " + view["id"] + " = " + camera.get_name())
    check(levels.save_current_level(),"Lab save failed")
    ue.log("IBS25 MAP AUTHORED; visual, collision and DX12 acceptance remain PENDING")


def main():
    config = dict(recipe.DEFAULT_ASSETS)
    if os.environ.get("IBS25_ASSETS"):
        overrides = json.loads(Path(os.environ["IBS25_ASSETS"]).read_text(encoding="utf-8"))
        check(set(overrides).issubset(config), "Unknown asset configuration key")
        config.update(overrides)
    loaded, missing = preflight(config)
    ue.log("IBS25 PREFLIGHT " + json.dumps({"missing":missing,"map_written":False}))
    if os.environ.get("IBS25_EXECUTE") != "1":
        return
    check(not missing, "IBS25 dependencies incomplete: " + "; ".join(missing))
    data = recipe.validate_recipe(recipe.scene())
    check(ue.SystemLibrary.get_console_variable_int_value("r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange") == 1,
          "Lab EV100 settings require the existing extended luminance range; no Config changes are made")
    generated = Path(os.environ["IBS25_GENERATED"]).resolve()
    check(generated.name == "IBS25_generated", "Wrong geometry directory")
    for name,mesh in recipe.geometry(data).items():
        check((generated/(name+".obj")).read_text(encoding="utf-8") == mesh.obj(),"Geometry changed: " + name)
    # Never dismiss or save an unrelated dirty editor world.
    check(not ue.EditorLoadingAndSavingUtils.get_dirty_map_packages(), "Unsaved map open; coordinate and save manually first")
    author(data, loaded, generated, recipe.digest(config))


if __name__ == "__main__":
    main()
