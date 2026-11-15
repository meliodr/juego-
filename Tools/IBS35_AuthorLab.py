"""Run manually in an exclusively reserved UE editor, after compiling S3.5.
Creates ONLY /Game/IB_S35 assets; refuses to replace an existing map.
No imports, downloads, plugin activation, production-map edits or automatic PIE.
"""
import unreal as ue

ROOT = '/Game/IB_S35'
MAP = ROOT + '/Lvl_IB_S35_Interactions'
assets = ue.EditorAssetLibrary
assert not assets.does_asset_exist(MAP), 'Existing lab: inspect it; do not overwrite automatically.'
for name in ('IBS35LabHero', 'IBS35Object', 'IBS35LabTarget', 'IBS35ObjectProfileAsset'):
    assert hasattr(ue, name), 'Compile S3.5 before authoring: ' + name

def required(path):
    result = assets.load_asset(path)
    assert result, 'Missing installed dependency: ' + path
    return result

# References only to installed baseline assets. Missing vehicles block the comparison,
# rather than silently substituting an invented model or importing a licensed pack.
models = {
    'Sport': required('/Game/Car/SM_AutomotiveTP_Car'),
    'Sedan': required('/Game/IB_Donors/SprintVehicles/taxi/StaticMeshes/taxi'),
    'Electric': required('/Game/IB_Donors/SprintVehicles/carro_eletrico/StaticMeshes/carro_eletrico'),
}
cube = required('/Engine/BasicShapes/Cube')
original_hero = '/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter'
original_mode = '/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode'
required(original_hero)
required(original_mode)

def duplicate_once(source, target):
    assert target.startswith(ROOT + '/')
    if assets.does_asset_exist(target):
        return required(target)
    result = assets.duplicate_asset(source, target)
    assert result, target
    return result

hero_bp = duplicate_once(original_hero, ROOT + '/BP_S35_H01')
ue.BlueprintEditorLibrary.reparent_blueprint(hero_bp, ue.IBS35LabHero)
ue.BlueprintEditorLibrary.compile_blueprint(hero_bp)
assets.save_loaded_asset(hero_bp)
hero_class = assets.load_blueprint_class(ROOT + '/BP_S35_H01')
assert hero_class
mode_bp = duplicate_once(original_mode, ROOT + '/BP_S35_GameMode')
ue.BlueprintEditorLibrary.compile_blueprint(mode_bp)
mode_class = assets.load_blueprint_class(ROOT + '/BP_S35_GameMode')
ue.get_default_object(mode_class).set_editor_property('default_pawn_class', hero_class)
assets.save_loaded_asset(mode_bp)

levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.new_level(MAP)
ue.EditorLevelLibrary.get_editor_world().get_world_settings().set_editor_property('default_game_mode', mode_class)

def spawn(cls, label, xyz):
    actor = actors.spawn_actor_from_class(cls, ue.Vector(*xyz))
    assert actor, label
    actor.set_actor_label(label)
    return actor

def prop(name, kind, mass, extent, xyz, model=None):
    profile = ue.IBS35ObjectProfile()
    profile.set_editor_property('object_class', kind)
    profile.set_editor_property('variant', name)
    profile.set_editor_property('mass_kg', mass)
    profile.set_editor_property('half_extent', ue.Vector(*extent))
    width = min(30, extent[1] * 0.75)
    profile.set_editor_property('left_contact', ue.Vector(-10, -width, -extent[2]))
    profile.set_editor_property('right_contact', ue.Vector(-10, width, -extent[2]))
    profile.set_editor_property('lift_seconds', min(0.9, 0.35 + mass / 6000))
    profile.set_editor_property('melee_tool', kind == ue.IBS35ObjectClass.POLE)
    asset_path = ROOT + '/Profiles/DA_' + name
    if assets.does_asset_exist(asset_path):
        data = required(asset_path)
    else:
        factory = ue.DataAssetFactory()
        factory.set_editor_property('data_asset_class', ue.IBS35ObjectProfileAsset)
        data = ue.AssetToolsHelpers.get_asset_tools().create_asset('DA_' + name, ROOT + '/Profiles', ue.IBS35ObjectProfileAsset, factory)
    assert data
    data.set_editor_property('profile', profile)
    assets.save_loaded_asset(data)
    actor = spawn(ue.IBS35Object, name, xyz)
    actor.set_editor_property('profile_asset', data)
    actor.get_editor_property('appearance').set_static_mesh(model or cube)
    actor.refresh_profile()
    return actor

floor = spawn(ue.StaticMeshActor, 'S35 laboratory floor', (2200, 0, -25))
floor.static_mesh_component.set_static_mesh(cube)
floor.set_actor_scale3d(ue.Vector(65, 38, 0.5))
floor.static_mesh_component.set_collision_profile_name('BlockAll')
spawn(ue.PlayerStart, 'S35 start', (-500, 0, 110))
light = spawn(ue.DirectionalLight, 'S35 daylight', (0, 0, 1000))
light.set_actor_rotation(ue.Rotator(-45, -30, 0), False)
spawn(ue.SkyLight, 'S35 skylight', (0, 0, 1000))

kind = ue.IBS35ObjectClass
prop('Sport', kind.CAR, 1200, (210,90,65), (0,-1100,85), models['Sport'])
prop('Sedan', kind.CAR, 1450, (225,95,75), (600,-1100,95), models['Sedan'])
prop('Electric', kind.CAR, 1650, (200,92,72), (1200,-1100,92), models['Electric'])
prop('Container', kind.CONTAINER, 2200, (150,100,110), (0,-450,130))
prop('Pole', kind.POLE, 85, (180,12,12), (600,-450,32))
prop('Rock', kind.ROCK, 350, (65,55,50), (1200,-450,70))
prop('LoosePart', kind.LOOSE_PART, 45, (40,30,20), (1600,-450,40))
prop('RockSmall', kind.ROCK, 43.75, (32.5,27.5,25), (1800,-450,45))
prop('LoosePartLarge', kind.LOOSE_PART, 151.875, (60,45,30), (2000,-450,50))

for name, xyz, extent in (
    ('Prepared target', (500,450,110), (35,80,100)),
    ('Thin wall 4cm', (2500,-650,500), (2,650,500)),
    ('Flight lane end wall', (5000,800,700), (25,600,700)),
):
    actor = spawn(ue.IBS35LabTarget, name, xyz)
    actor.set_editor_property('half_extent', ue.Vector(*extent))
    actor.refresh_geometry()

assert levels.save_current_level()
ue.log('IBS35 AUTHORED: ' + MAP + '; 9 profile objects, target, two walls, clear flight lane y=800. NOT playtested.')
