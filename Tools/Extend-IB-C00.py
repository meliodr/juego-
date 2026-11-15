"""Extend the C00 circuit with a second, reusable blockout for flight."""
import unreal as ue


levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.load_level("/Game/IB_C00/Lvl_IB_C00")

cube = ue.load_asset("/Engine/BasicShapes/Cube")
material_path = "/Game/LevelPrototyping/Materials/"
light = ue.load_asset(material_path + "MI_PrototypeGrid_Gray")
dark = ue.load_asset(material_path + "MI_PrototypeGrid_TopDark")
trim = ue.load_asset(material_path + "MI_PrototypeGrid_Gray_02")
assert all((cube, light, dark, trim))

existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}


def volume(name, center, size, material):
    label = f"Distrito - {name}"
    actor = existing.get(label)
    location = ue.Vector(*(meters * 100 for meters in center))
    if actor is None:
        actor = actors.spawn_actor_from_class(ue.StaticMeshActor, location)
    else:
        actor.set_actor_location(location, False, True)
    actor.set_actor_label(label)
    actor.set_folder_path("Distrito de vuelo")
    component = actor.static_mesh_component
    component.set_static_mesh(cube)
    component.set_material(0, material)
    component.set_collision_profile_name("BlockAll")
    actor.set_actor_scale3d(ue.Vector(*size))


# Continue the original avenue east with a second avenue running north-south.
volume("suelo este", (420, 55, -0.25), (400, 410, 0.5), light)
volume("suelo norte", (100, 212.5, -0.25), (240, 95, 0.5), light)
volume("suelo sur", (100, -90, -0.25), (240, 120, 0.5), light)
volume("avenida este", (416, 0, 0.015), (392, 26, 0.03), dark)
volume("avenida transversal", (403, 60, 0.015), (30, 390, 0.03), dark)
volume("plaza elevada", (510, 123, 0.4), (104, 96, 0.8), trim)

for i, (x, y, width, depth, height) in enumerate((
    (260, 49, 44, 50, 22), (322, 55, 49, 52, 41), (366, 117, 35, 46, 58),
    (265, -48, 39, 46, 27), (329, -57, 60, 59, 52), (360, -114, 50, 43, 72),
    (467, -55, 59, 67, 38), (535, -51, 48, 50, 61), (590, -84, 39, 103, 29),
    (458, 57, 46, 43, 50), (553, 51, 53, 44, 56), (582, 190, 49, 44, 78),
    (318, 191, 70, 49, 34), (455, 208, 60, 49, 46), (190, 204, 39, 57, 25),
), 1):
    volume(f"edificio {i:02}", (x, y, height / 2), (width, depth, height), light if i % 2 else trim)
    volume(f"azotea {i:02}", (x, y, height + 0.35), (width + 1, depth + 1, 0.7), dark)

volume("puente sobre la transversal", (414, 52, 33), (64, 8, 2.4), trim)
volume("puente sobre la plaza", (506, 52, 38), (55, 9, 2.4), trim)
assert levels.save_current_level()
ue.log("IB-C00 DISTRICT SAVED")
