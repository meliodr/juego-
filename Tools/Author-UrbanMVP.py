"""Dress Lvl_IB_C00 using project meshes without altering the tested street geometry."""

from collections import defaultdict
import math

import unreal as ue


MAP = '/Game/IB_C00/Lvl_IB_C00'
PREFIX = 'MVP - '
FOLDER = 'Distrito de vuelo/MVP urbano'
CITY_MATERIALS = '/Game/IB_Donors/CityKit/predio_apartamentos/predio_apartamentos/Materials/'
TRIM = '/Game/LevelPrototyping/Materials/'


def asset(path):
    loaded = ue.load_asset(path)
    assert loaded is not None, path
    return loaded


levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.load_level(MAP)
existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
assert all(label in existing for label in ('Carro de ensayo estatico', 'Pared panel central', 'Distrito - plaza elevada'))

meshes = {name: asset('/Engine/BasicShapes/' + name) for name in ('Cube', 'Cylinder', 'Sphere')}
materials = {
    'road': asset(CITY_MATERIALS + 'concreto_escuro'),
    'paving': asset(CITY_MATERIALS + 'concreto'),
    'ivory': asset(CITY_MATERIALS + 'creme'),
    'yellow': asset(CITY_MATERIALS + 'amarelo'),
    'metal': asset(CITY_MATERIALS + 'metal_escuro'),
    'wood': asset(CITY_MATERIALS + 'madeira'),
    'green': asset(CITY_MATERIALS + 'verde'),
    'rust': asset('/Game/IB_Donors/CityKit/hotel/hotel/Materials/laranja'),
    'skyline': asset(TRIM + 'MI_PrototypeGrid_TopDark'),
}
for material in materials.values():
    if isinstance(material, ue.MaterialInstanceConstant):
        ue.MaterialEditingLibrary.set_material_usage_override(
            material, ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES, True, True)
        assert ue.EditorAssetLibrary.save_loaded_asset(material)
batches = defaultdict(list)


def point(meters):
    return ue.Vector(*(coordinate * 100 for coordinate in meters))


def stage_actor(name, actor_class, center, folder, yaw=0.0):
    label = PREFIX + name
    rotation = ue.Rotator(yaw=yaw)
    actor = existing.get(label) or actors.spawn_actor_from_class(actor_class, point(center), rotation)
    assert actor is not None, label
    actor.set_actor_location(point(center), False, True)
    actor.set_actor_rotation(rotation, True)
    actor.set_actor_label(label)
    actor.set_folder_path(FOLDER + '/' + folder)
    return actor


def detail(material, center, size, mesh='Cube', yaw=0.0, cull=180, shadows=False):
    """Queue a decorative instance. Stock basic shapes measure one meter per axis."""
    transform = ue.Transform()
    transform.translation = point(center)
    radians = math.radians(yaw) / 2
    transform.rotation = ue.Quat(0, 0, math.sin(radians), math.cos(radians))
    transform.scale3d = ue.Vector(*size)
    batches[(mesh, material, cull, shadows)].append(transform)


def solid(name, center, size, material, mesh='Cube', tag=None, mass=None):
    actor = stage_actor('Impacto - ' + name, ue.StaticMeshActor, center, 'Centro de Impacto')
    component = actor.static_mesh_component
    component.set_simulate_physics(False)
    component.set_mobility(ue.ComponentMobility.MOVABLE if mass else ue.ComponentMobility.STATIC)
    component.set_static_mesh(meshes[mesh])
    component.set_material(0, materials[material])
    actor.set_actor_scale3d(ue.Vector(*size))
    component.set_collision_profile_name('PhysicsActor' if mass else 'BlockAll')
    # World traces and impact line-of-sight must observe the actual target.
    component.set_collision_response_to_channel(ue.CollisionChannel.ECC_VISIBILITY, ue.CollisionResponseType.ECR_BLOCK)
    if tag:
        actor.set_editor_property('tags', [ue.Name(tag)])
    if mass:
        component.set_mass_override_in_kg(ue.Name('None'), mass, True)
        component.set_simulate_physics(True)


def sign(name, text, center, yaw, size=66):
    """A single two-sided sign, independent of camera or player inputs."""
    for side in (0, 180):
        angle = math.radians(yaw + side)
        loc = (center[0] + math.cos(angle) * 0.13, center[1] + math.sin(angle) * 0.13, center[2])
        actor = stage_actor(f'Senal - {name} {side}', ue.TextRenderActor, loc, 'Señalización', yaw + side)
        component = actor.get_component_by_class(ue.TextRenderComponent)
        component.set_text(text)
        component.set_horizontal_alignment(ue.HorizTextAligment.EHTA_CENTER)
        component.set_vertical_alignment(ue.VerticalTextAligment.EVRTA_TEXT_CENTER)
        component.set_text_render_color(ue.Color(240, 241, 226, 255))
        component.set_world_size(size)
        component.set_collision_profile_name('NoCollision')
        component.set_cast_shadow(False)
        component.set_cull_distance(16000.0)


def board(center, width, yaw):
    x, y, z = center
    detail('metal', center, (0.18, width, 1.0), yaw=yaw)
    detail('metal', (x, y, z / 2 - 0.18), (0.14, 0.14, z), cull=160)


def planter(center, base):
    x, y = center
    detail('paving', (x, y, base + 0.29), (2.1, 1.55, 0.58), cull=125, shadows=True)
    for offset in (-0.55, 0.0, 0.55):
        detail('green', (x + offset, y, base + 0.66), (0.8, 1.12, 0.72), mesh='Sphere', cull=125, shadows=True)


def bench(center, base, yaw):
    x, y = center
    angle = math.radians(yaw)
    detail('wood', (x, y, base + 0.50), (2.2, 0.54, 0.12), yaw=yaw, cull=125, shadows=True)
    for offset in (-0.74, 0.74):
        detail('metal', (x + math.cos(angle) * offset, y + math.sin(angle) * offset, base + 0.23),
               (0.12, 0.46, 0.46), yaw=yaw, cull=125)


def lamp(x, y, side):
    detail('metal', (x, y, 0.12), (0.44, 0.44, 0.24), mesh='Cylinder')
    detail('metal', (x, y, 2.72), (0.16, 0.16, 5.42), mesh='Cylinder', shadows=True)
    detail('metal', (x, y + side * 0.63, 5.36), (0.14, 1.39, 0.15))
    detail('ivory', (x, y + side * 1.28, 5.24), (0.45, 0.56, 0.13))


# Original floors remain the only collision surfaces along the circuit. Offset the
# cosmetic road layer above the existing east avenue to avoid coplanar flicker.
detail('road', (319, 0, 0.041), (602, 20, 0.038), cull=0)
detail('road', (180, 64.5, 0.042), (20, 109, 0.038), cull=0)
detail('road', (403, 54, 0.044), (26, 366, 0.038), cull=0)
for side in (-1, 1):
    for x, length in ((88, 135), (291, 193), (512, 183)):
        detail('paving', (x, side * 11.2, 0.10), (length, 2.4, 0.16), cull=0)
        detail('ivory', (x, side * 10.03, 0.145), (length, 0.18, 0.22), cull=0)
    for y in (23, 51, 95):
        detail('paving', (180 + side * 11.2, y, 0.10), (2.4, 18, 0.16), cull=0)
    for x in (33, 74, 116, 153, 214, 251, 294, 343, 382, 434, 480, 534, 585, 611):
        lamp(x, side * 10.8, -side)

for x in range(27, 614, 10):
    if abs(x - 180) > 15 and abs(x - 403) > 19:
        detail('yellow', (x, 0, 0.071), (4.7, 0.14, 0.018), cull=195)
for y in range(16, 117, 9):
    detail('yellow', (180, y, 0.072), (0.14, 4.6, 0.018), cull=175)
for y in range(21, 157, 11):
    detail('yellow', (403, y, 0.073), (0.16, 4.6, 0.018), cull=195)
for x in (163, 195, 385, 421):
    for band in range(-7, 8):
        detail('ivory', (x, band * 1.2, 0.077), (1.65, 0.55, 0.020), cull=185)

# The existing x=10 PlayerStart reads as a small civic plaza opening to the east.
detail('paving', (8.0, 0, 0.023), (29.0, 21, 0.035), cull=0)
for y in (-8.1, -6.7, 6.7, 8.1):
    detail('ivory', (8.0, y, 0.048), (27.6, 0.08, 0.018), cull=120)
for x in (-1.7, 5.3, 13.3):
    for side in (-1, 1):
        planter((x, side * 8.9), 0.02)
        if x == 5.3:
            bench((x, side * 7.5), 0.02, 0)
board((19.1, -8.9, 3.2), 6.7, 0)
sign('plaza', 'PLAZA DE INICIO', (19.1, -8.9, 3.2), 0, 52)

board((170.7, -8.0, 3.65), 6.0, -30)
sign('sector coches', 'SECTOR COCHES', (170.7, -8.0, 3.65), -30, 49)
board((424.4, 9.3, 4.3), 9.0, -20)
sign('sector impacto', 'CENTRO DE IMPACTO', (424.4, 9.3, 4.3), -20, 67)

# A recognizable service lane between the first two modern buildings.
detail('road', (289.5, 52.0, 0.042), (11.0, 75, 0.036), cull=0)
for y in (25, 58, 84):
    detail('metal', (283.2, y, 0.64), (0.8, 1.55, 1.28), cull=135, shadows=True)
    detail('green', (283.2, y, 1.29), (0.83, 1.60, 0.12), cull=135)
board((289.5, 25.2, 3.5), 6.3, 90)
sign('callejon', 'CALLEJON DE SERVICIO', (289.5, 25.2, 3.5), 90, 44)

# Roof volume remains the real landing surface. Put equipment near the edges
# rather than at the landing center or at the established QA test coordinates.
for x, y, roof_z in ((248, 60, 22.7), (336, 65, 41.7), (354, 126, 58.7),
                      (481, -68, 38.7), (545, -61, 61.7), (565, 179, 78.7)):
    for offset in (-2.5, 2.5):
        detail('metal', (x + offset, y, roof_z + 0.76), (2.3, 1.42, 1.52), cull=220, shadows=True)
        detail('ivory', (x + offset, y, roof_z + 1.53), (2.37, 1.48, 0.13), cull=220)
    detail('metal', (x, y + 4.8, roof_z + 1.58), (2.2, 2.2, 3.16), mesh='Cylinder', cull=245, shadows=True)

# Raised impact plaza. Leave the middle and all prop trajectories unobstructed.
detail('paving', (435, 130, 0.054), (38, 5.8, 0.026), cull=0)
for x in (422, 433, 444):
    detail('yellow', (x, 129.55, 0.077), (1.3, 0.15, 0.018), yaw=45, cull=150)
    detail('yellow', (x, 130.45, 0.077), (1.3, 0.15, 0.018), yaw=-45, cull=150)
for step in range(1, 5):
    height = 0.2 * step
    solid(f'Acceso plaza {step}', (453.5 + step, 130, height / 2), (1.0, 5.8, height), 'paving')
for angle in range(0, 360, 18):
    radians = math.radians(angle)
    detail('yellow', (520 + 8.0 * math.cos(radians), 130 + 8.0 * math.sin(radians), 0.824),
           (1.6, 0.22, 0.027), yaw=angle + 90, cull=180)
for x, y in ((503, 118), (538, 118), (503, 143), (538, 143)):
    planter((x, y), 0.80)
    bench((x, y + 2.2), 0.80, 0)
board((520, 117.8, 4.4), 8.0, 90)
sign('impacto', 'CENTRO DE IMPACTO', (520, 117.8, 4.4), 90, 60)
solid('Bidon este', (522.6, 130, 1.35), (0.65, 0.65, 1.1), 'rust', 'Cylinder', 'ImpactLandingTarget', 35.0)
solid('Bidon oeste', (517.4, 130, 1.35), (0.65, 0.65, 1.1), 'yellow', 'Cylinder', 'ImpactLandingTarget', 45.0)
solid('Caja sur', (520, 126.5, 1.26), (1.03, 0.78, 0.92), 'wood', tag='ImpactLandingTarget', mass=48.0)
solid('Caja norte', (522.4, 133.5, 1.29), (0.88, 1.1, 0.98), 'wood', tag='ImpactLandingTarget', mass=58.0)
solid('Panel ligero', (520, 136.3, 2.05), (5.0, 0.28, 2.50), 'paving', tag='ImpactLandingPanel')

# Distant, noncolliding skyline. Chopped setbacks add variety at silhouette cost.
for index, (x, y, width, depth, height) in enumerate((
    (-97, 120, 48, 45, 46), (-90, -30, 56, 47, 79), (10, -264, 57, 46, 43),
    (97, -267, 49, 50, 99), (182, -252, 58, 55, 66), (275, -260, 55, 43, 118),
    (370, -266, 59, 45, 64), (475, -261, 70, 49, 89), (580, -262, 57, 45, 49),
    (700, -145, 58, 63, 91), (701, -49, 62, 59, 64), (724, 46, 52, 56, 134),
    (739, 142, 70, 55, 96), (707, 239, 53, 69, 62), (630, 340, 72, 58, 109),
    (517, 343, 54, 63, 70), (418, 337, 64, 55, 100), (317, 356, 51, 57, 51),
    (202, 346, 52, 54, 88), (94, 357, 65, 60, 67), (-15, 324, 56, 54, 103),
)):
    finish = ('skyline', 'road', 'metal')[index % 3]
    detail(finish, (x, y, height / 2), (width, depth, height), cull=0)
    if index % 3 != 0:
        detail(finish, (x + width * 0.06, y, height * 1.055), (width * 0.70, depth * 0.66, height * 0.11), cull=0)

for (mesh, material, cull, shadows), transforms in sorted(batches.items()):
    group_name = f'Detalle {mesh} {material} {cull}m {int(shadows)}'
    group = stage_actor(group_name, ue.ImpactFacade, (0, 0, 0), 'Instancias')
    component = group.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
    component.set_static_mesh(meshes[mesh])
    component.set_material(0, materials[material])
    component.set_collision_profile_name('NoCollision')
    component.set_cast_shadow(shadows)
    component.set_cull_distances(cull * 85, cull * 100)
    component.clear_instances()
    component.add_instances(transforms, False, True, False)
    assert component.get_instance_count() == len(transforms), group_name

assert levels.save_current_level()
ue.log(f'IB URBAN MVP SAVED: {sum(map(len, batches.values()))} instances in {len(batches)} groups, '
       'four targets, one impact panel, four access steps')
