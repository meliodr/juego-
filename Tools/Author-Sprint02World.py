"""Extend IB-C00 east to the coast and stage the Sprint 02 vehicle sandbox."""

import math
import os
from pathlib import Path
import random

import unreal as ue


MAP = '/Game/IB_C00/Lvl_IB_C00'
PREFIX = 'S02 - '
FOLDER = 'Distrito de vuelo/Costa este'
DONOR_PACKAGE = '/Game/IB_Donors/SprintVehicles'
CITY_KIT = '/Game/IB_Donors/CityKit/'
APARTMENTS = CITY_KIT + 'predio_apartamentos/predio_apartamentos/Materials/'
HOTEL = CITY_KIT + 'hotel/hotel/Materials/'


def required_asset(path):
    asset = ue.load_asset(path)
    assert asset is not None, path
    return asset


def import_vehicle(name, fallback):
    assets = ue.EditorAssetLibrary
    package = f'{DONOR_PACKAGE}/{name}/StaticMeshes/{name}'
    if assets.does_asset_exist(package):
        return required_asset(package)

    donor = os.environ.get('IB_CITY_DONOR')
    source = Path(donor) if donor else Path(__file__).resolve().parents[2] / 'donor-city'
    glb = source / 'models' / 'veiculos' / f'{name}.glb'
    if not glb.is_file():
        ue.log_warning(f'IB SPRINT02: missing optional {glb}; using {fallback.get_name()} for {name}')
        return fallback
    task = ue.AssetImportTask()
    task.filename = str(glb)
    task.destination_path = DONOR_PACKAGE
    task.automated = True
    task.save = True
    ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    assert assets.does_asset_exist(package), (name, list(task.imported_object_paths))
    mesh = required_asset(package)
    assert isinstance(mesh, ue.StaticMesh), package
    ue.log(f'IB SPRINT02 VEHICLE IMPORTED: {mesh.get_path_name()}')
    return mesh


def point(meters):
    return ue.Vector(*(coordinate * 100.0 for coordinate in meters))


levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.load_level(MAP)
existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
assert all(label in existing for label in ('Distrito - avenida este', 'Carro de ensayo estatico', 'Pared panel central'))
shapes = {name: required_asset('/Engine/BasicShapes/' + name)
          for name in ('Cube', 'Cylinder', 'Cone', 'Plane', 'Sphere')}
materials = {name: required_asset(path) for name, path in {
    'road': APARTMENTS + 'concreto_escuro',
    'shoulder': APARTMENTS + 'concreto',
    'stripe': APARTMENTS + 'amarelo',
    'white': APARTMENTS + 'creme',
    'grass': APARTMENTS + 'verde',
    'hill': CITY_KIT + 'predio_alto/predio_alto/Materials/verde_escuro',
    'sand': HOTEL + 'creme',
    'water': HOTEL + 'agua',
    'wood': APARTMENTS + 'madeira',
}.items()}
ue.MaterialEditingLibrary.set_material_usage_override(
    materials['hill'], ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES, True, True)
assert ue.EditorAssetLibrary.save_loaded_asset(materials['hill'])


def stage_actor(name, actor_class, center, folder, yaw=0.0):
    label = PREFIX + name
    rotation = ue.Rotator(yaw=yaw)
    actor = existing.get(label) or actors.spawn_actor_from_class(actor_class, point(center), rotation)
    assert actor is not None, label
    actor.set_actor_label(label)
    actor.set_folder_path(FOLDER + '/' + folder)
    if isinstance(actor, ue.StaticMeshActor):
        actor.static_mesh_component.set_simulate_physics(False)
    actor.set_actor_location(point(center), False, True)
    actor.set_actor_rotation(rotation, True)
    return actor


def surface(name, center, size, shape, material, folder='Paisaje', collision=True):
    actor = stage_actor(name, ue.StaticMeshActor, center, folder)
    component = actor.static_mesh_component
    component.set_mobility(ue.ComponentMobility.STATIC)
    component.set_static_mesh(shapes[shape])
    component.set_material(0, materials[material])
    component.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    component.set_cast_shadow(collision)
    actor.set_actor_scale3d(ue.Vector(*size))


batches = {}


def detail(shape, material, center, size, cull=0):
    transform = ue.Transform()
    transform.translation = point(center)
    transform.scale3d = ue.Vector(*size)
    batches.setdefault((shape, material, cull), []).append(transform)


# These volumes begin where the old district ground ends at x=620 m. Leave
# y=0 clear all the way to the coastal turnout beyond the asphalt's end.
surface('Terreno costero', (1215, 39, -0.25), (1190, 362, 0.5), 'Cube', 'sand')
surface('Autovia este', (1200, 0, 0.0275), (1160, 20, 0.055), 'Cube', 'road', 'Autovia')
for side in (-1, 1):
    surface(f'Arcén {side}', (1200, side * 12.1, 0.0175), (1160, 4.2, 0.035),
            'Cube', 'shoulder', 'Autovia')
    detail('Cube', 'white', (1200, side * 9.65, 0.063), (1160, 0.14, 0.012))
for x in range(628, 1758, 10):
    detail('Cube', 'stripe', (x, 0, 0.066), (4.8, 0.16, 0.016), cull=245)
surface('Mirador del cabo', (1783, 0, 0.024), (46, 44, 0.048), 'Cylinder', 'shoulder', 'Autovia')

surface('Agua', (3620, -2200, -0.60), (6000, 5200, 1), 'Plane', 'water', collision=False)
for index, (x, y, width, depth) in enumerate(((710, -148, 140, 63), (963, -151, 165, 54),
                                              (1192, -139, 210, 80), (1460, -150, 130, 68),
                                              (1690, -143, 180, 73), (1785, -62, 102, 120)), 1):
    surface(f'Playa {index:02}', (x, y, -0.36), (width, depth, 0.5), 'Cylinder', 'sand')
for index, (x, y, width, depth, relief) in enumerate(((795, 113, 206, 142, 51), (1013, 143, 255, 163, 73),
                                                     (1260, 159, 244, 214, 86), (1480, 144, 229, 192, 58),
                                                     (1674, 107, 193, 143, 52), (1011, -89, 123, 75, 22),
                                                     (1535, -90, 151, 76, 31)), 1):
    surface(f'Colina {index:02}', (x, y, -2.5), (width, depth, relief), 'Sphere',
            'hill' if index % 2 else 'grass')

# Without a tree in the local CC0 kit, instance simple conifers from Engine
# meshes. Keep them on the low coastal verge and clear of the landing strip.
rng = random.Random(2715)
for anchor in (662, 812, 1130, 1360, 1660, 1738):
    for index in range(9):
        x = anchor + rng.uniform(-35, 35)
        y = rng.uniform(-62, -31)
        height = rng.uniform(5.0, 9.5)
        detail('Cylinder', 'wood', (x, y, height * 0.20), (0.35, 0.35, height * 0.40), cull=280)
        detail('Cone', 'hill', (x, y, height * 0.55), (height * 0.56, height * 0.56, height * 0.68), cull=280)
        detail('Cone', 'grass', (x, y, height * 0.78), (height * 0.39, height * 0.39, height * 0.56), cull=280)


cars = {'carro': required_asset('/Game/IB_Donors/Carro/carro/StaticMeshes/carro')}
for vehicle in ('taxi', 'carro_eletrico'):
    cars[vehicle] = import_vehicle(vehicle, cars['carro'])

vehicle_tags = {'taxi': 'ImpactVehicleTaxi', 'carro_eletrico': 'ImpactVehicleElectric'}
HULL_SIZE = (2.0, 4.5, 1.5)
PULLOUT_Z = 0.065
pullouts = (
    (684, 18.5, 'carro', ('taxi', 'carro_eletrico')),
    (731, -18.5, 'taxi', ('carro_eletrico', 'carro')),
    (778, 18.5, 'carro_eletrico', ('carro', 'taxi')),
    (1248, -18.5, 'taxi', ('carro_eletrico',)),
    (1502, 18.5, 'carro_eletrico', ('taxi',)),
)


def place_parked(name, mesh, x, y, hull_yaw):
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    assert min(extent.x, extent.y, extent.z) > 0.0, name
    sideways = extent.x > extent.y
    width, length, height = HULL_SIZE
    size_x, size_y = (length, width) if sideways else (width, length)
    scale = ue.Vector(size_x * 50.0 / extent.x, size_y * 50.0 / extent.y, height * 50.0 / extent.z)
    yaw = hull_yaw + (90.0 if sideways else 0.0)
    angle = math.radians(yaw)
    tx = bounds.origin.x * scale.x / 100.0
    ty = bounds.origin.y * scale.y / 100.0
    actor_center = (x - tx * math.cos(angle) + ty * math.sin(angle),
                    y - tx * math.sin(angle) - ty * math.cos(angle),
                    PULLOUT_Z + height / 2.0 - bounds.origin.z * scale.z / 100.0)
    actor = stage_actor(name, ue.StaticMeshActor, actor_center, 'Vehiculos', yaw)
    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    component.set_collision_profile_name('NoCollision')
    actor.set_actor_scale3d(scale)


for index, (x, y, vehicle, parked) in enumerate(pullouts, 1):
    yaw = -90.0 if y > 0.0 else 90.0
    surface(f'Apartadero {index}', (x, y, PULLOUT_Z / 2.0), (34, 11.6, PULLOUT_Z),
            'Cube', 'shoulder', 'Autovia')
    for offset in (-12, -4, 4, 12):
        detail('Cube', 'white', (x + offset, y, 0.073), (0.11, 3.0, 0.012), cull=160)

    label = f'Vehiculo interactivo {index} - {vehicle}'
    car = stage_actor(label, ue.StaticMeshActor, (x, y, PULLOUT_Z + HULL_SIZE[2] / 2.0), 'Vehiculos', yaw)
    tags = ['ImpactSprintVehicle']
    if vehicle in vehicle_tags:
        tags.append(vehicle_tags[vehicle])
    car.set_editor_property('tags', [ue.Name(tag) for tag in tags])
    chassis = car.static_mesh_component
    chassis.set_mobility(ue.ComponentMobility.MOVABLE)
    chassis.set_static_mesh(shapes['Cube'])
    car.set_actor_scale3d(ue.Vector(*HULL_SIZE))
    chassis.set_collision_profile_name('PhysicsActor')
    chassis.set_collision_response_to_channel(ue.CollisionChannel.ECC_VISIBILITY, ue.CollisionResponseType.ECR_BLOCK)
    chassis.set_mass_override_in_kg(ue.Name('None'), 1200.0, True)
    chassis.set_simulate_physics(True)

    for ordinal, (offset, variant) in enumerate(zip((-8, 8), parked), 1):
        place_parked(f'Auto estacionado {index}.{ordinal} - {variant}', cars[variant], x + offset, y, yaw)


for (shape, material, cull), transforms in sorted(batches.items()):
    group = stage_actor(f'Instancias {shape} {material} {cull}', ue.ImpactFacade, (0, 0, 0), 'Instancias')
    component = group.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
    component.set_static_mesh(shapes[shape])
    component.set_material(0, materials[material])
    component.set_collision_profile_name('NoCollision')
    component.set_cast_shadow(False)
    component.set_cull_distances(cull * 85, cull * 100)
    component.clear_instances()
    component.add_instances(transforms, False, True, False)
    assert component.get_instance_count() == len(transforms)

assert levels.save_current_level()
ue.log(f'IB SPRINT02 WORLD SAVED: 1.16 km of highway, 5 physics vehicles, 8 parked cars, '
       f'{sum(map(len, batches.values()))} decorative instances in {len(batches)} batches')
