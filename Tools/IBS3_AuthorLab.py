"""Author only /Game/IB_S3 in a coordinated UE 5.8 editor slot after native build.
No Fab bytes, plugins downloaded, main map or project/config edits. Generated geometry is original.
Run from Unreal Python: exec(open(r'<repo>/Tools/IBS3_AuthorLab.py').read()).
Idempotent: refuses to overwrite an existing lab map or generated asset.
"""
import json
import math
from pathlib import Path
import unreal

ROOT = "/Game/IB_S3"
MAP = ROOT + "/Lvl_IB_S3_PlanetLab"


def write_obj(path, vertices, faces):
    with path.open("w", encoding="ascii", newline="\n") as f:
        for x, y, z in vertices:
            f.write(f"v {x:.10f} {y:.10f} {z:.10f}\n")
        for a, b, c in faces:
            f.write(f"f {a+1} {b+1} {c+1}\n")


def sphere(path):
    # 50 cm normalized source radius; runtime scales from manifest radius.
    lon, lat = 256, 128
    vertices = [(0, 0, 50)]
    for j in range(1, lat):
        a = math.pi*j/lat
        for i in range(lon):
            b = math.tau*i/lon
            vertices.append((50*math.sin(a)*math.cos(b), 50*math.sin(a)*math.sin(b), 50*math.cos(a)))
    south = len(vertices)
    vertices.append((0, 0, -50))
    faces = []
    for i in range(lon):
        nxt = (i+1) % lon
        faces.append((0, 1+i, 1+nxt))
        for j in range(lat-2):
            a, b = 1+j*lon+i, 1+j*lon+nxt
            faces.extend(((a, a+lon, b+lon), (a, b+lon, b)))
        a, b = 1+(lat-2)*lon+i, 1+(lat-2)*lon+nxt
        faces.append((a, south, b))
    write_obj(path, vertices, faces)


def cap(path):
    # Curved +/-0.12-radian near field, 3 km half-width at R=25km; <=~9cm facet error.
    count, half = 128, 0.12
    vertices, faces = [], []
    for y in range(count+1):
        for x in range(count+1):
            a, b = (x/count*2-1)*half, (y/count*2-1)*half
            norm = math.sqrt(1+a*a+b*b)
            vertices.append((50*a/norm, 50*b/norm, 50/norm))
    for y in range(count):
        for x in range(count):
            a = y*(count+1)+x
            faces.extend(((a, a+1, a+count+2), (a, a+count+2, a+count+1)))
    write_obj(path, vertices, faces)


def import_mesh(path, name):
    dest = ROOT + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        raise RuntimeError("Refusing to replace existing asset: " + dest)
    opts = unreal.FbxImportUI()
    opts.set_editor_property("import_mesh", True)
    opts.set_editor_property("import_as_skeletal", False)
    opts.set_editor_property("import_materials", False)
    opts.set_editor_property("import_textures", False)
    opts.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    opts.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
    task = unreal.AssetImportTask()
    for key, value in {"filename": str(path), "destination_path": ROOT, "destination_name": name,
                       "automated": True, "replace_existing": False, "save": True, "options": opts}.items():
        task.set_editor_property(key, value)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(dest)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError("Static mesh import failed: " + dest)
    return mesh


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(MAP):
        raise RuntimeError("Existing S3 map preserved; inspect it before reauthoring")
    planet_class = unreal.load_class(None, "/Script/ImpactBeyond.IBS3PlanetWorld")
    game_class = unreal.load_class(None, "/Script/ImpactBeyond.IBS3GameMode")
    data_class = unreal.load_class(None, "/Script/ImpactBeyond.IBS3WorldManifest")
    if not all((planet_class, game_class, data_class)):
        raise RuntimeError("Compile the IB_S3 C++ module first")
    unreal.EditorAssetLibrary.make_directory(ROOT)
    # Generated source files stay under the user's project Saved, never engine/global directories.
    stage = Path(unreal.Paths.project_saved_dir()) / "IB_S3_Generated"
    stage.mkdir(parents=True, exist_ok=True)
    sphere(stage/"SM_S3Planet.obj")
    cap(stage/"SM_S3NearSurface.obj")
    globe = import_mesh(stage/"SM_S3Planet.obj", "SM_S3Planet")
    near = import_mesh(stage/"SM_S3NearSurface.obj", "SM_S3NearSurface")
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", data_class)
    data = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_S3World", ROOT, data_class, factory)
    if not data:
        raise RuntimeError("WorldManifest asset creation failed")
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.new_level(MAP):
        raise RuntimeError("Could not create isolated S3 map")
    planet = actors.spawn_actor_from_class(planet_class, unreal.Vector(0, 0, 0))
    planet.set_editor_property("manifest", data)
    planet.get_editor_property("planet_proxy").set_static_mesh(globe)
    planet.get_editor_property("near_surface").set_static_mesh(near)
    radius = data.get_editor_property("radius_cm")
    # Central city at (lat=0,lon=0). +Z of the capsule is world +X; +X facing geographic north (+Z).
    orientation = unreal.MathLibrary.make_rot_from_xz(unreal.Vector(0, 0, 1), unreal.Vector(1, 0, 0))
    actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(radius+350, 0, 0), orientation)
    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 0), unreal.Rotator(-35, -25, 0))
    light = sun.get_component_by_class(unreal.DirectionalLightComponent)
    light.set_editor_property("atmosphere_sun_light", True)
    light.set_editor_property("intensity", 10.0)
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 0))
    sky.get_component_by_class(unreal.SkyLightComponent).set_editor_property("real_time_capture", True)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", game_class)
    if not levels.save_current_level():
        raise RuntimeError("S3 map save failed")
    unreal.EditorAssetLibrary.save_asset(data.get_path_name())
    report = {"map": MAP, "world_manifest": data.get_path_name(), "radius_cm": radius,
              "art_status": "placeholder tecnico", "collision": "native analytic sphere + curved hub streets",
              "runtime_tested": False, "assets": [globe.get_path_name(), near.get_path_name()]}
    (stage/"author-report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log("IB_S3 authoring complete; PIE/DX12/packaged tests are separate gates")


if __name__ == "__main__":
    main()
