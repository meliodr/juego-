"""Apply the reviewed R2 recipe to this lab only. Requires the coordinated Editor.
Never calls the old author rebuild, touches donors, main map or Config.
Reruns set stable owned actors/instances; new geometry is versioned and hash guarded.
"""
import importlib
import json
import math
from pathlib import Path
import unreal as ue
import IBS25_recipe as base
import IBS25_author as author
import IBS25_refine_recipe
import IBS25_soil
r=importlib.reload(IBS25_refine_recipe)
a=importlib.reload(author)
actors=ue.get_editor_subsystem(ue.EditorActorSubsystem)
levels=ue.get_editor_subsystem(ue.LevelEditorSubsystem)
world=ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name().split('.')[0]==base.MAP, 'Wrong world'
assert not levels.is_in_play_in_editor(), 'Leave PIE first'
lookup={o.get_actor_label():o for o in actors.get_all_level_actors()}
for name,o in lookup.items():
    if name.startswith('IBS25_'): assert base.OWNER in [str(t) for t in o.tags], name
    else: assert o.get_class().get_name() in ('WorldSettings','Brush'), 'Foreign actor '+name
data=r.scene()
fingerprint=base.digest(data)
oldmarker=lookup['IBS25_MANIFEST']
assert any(str(t).startswith('IBS25Recipe_276cfa49') or str(t)=='IBS25Revision_R2_'+fingerprint for t in oldmarker.tags)
out=Path(__file__).with_name('IBS25_refined_generated')
for name,mesh in r.geometry().items(): assert (out/(name+'.obj')).read_text()==mesh.obj()

def actor(name,cls,xyz=(0,0,0)):
    label='IBS25_'+name
    obj=lookup.get(label)
    if obj is None:
        obj=actors.spawn_actor_from_class(cls,a.point(xyz))
        obj.set_actor_label(label)
        obj.tags=[ue.Name(base.OWNER)]
        obj.set_folder_path('IB_S25/Refinement_R2')
        lookup[label]=obj
    assert isinstance(obj,cls)
    obj.set_actor_location(a.point(xyz),False,True)
    obj.set_actor_rotation(ue.Rotator(),True)
    return obj

def pbr(name,color,rough=.65,metal=0,plaster=False):
    path=a.ROOT+'/Materials/M_R2v3_'+name
    signature=base.digest(dict(color=color,rough=rough,metal=metal,plaster=plaster,shader_revision=3))
    if a.assets.does_asset_exist(path):
        mat=ue.load_asset(path)
        assert a.owned(mat)
        previous=a.assets.get_metadata_tag(mat,'IBS25R2Parameters')
        assert not previous or previous==signature,'Material parameters changed; version the asset'
        if not previous:
            a.assets.set_metadata_tag(mat,'IBS25R2Parameters',signature);a.stamp(mat)
        return mat
    mat=ue.AssetToolsHelpers.get_asset_tools().create_asset('M_R2v3_'+name,a.ROOT+'/Materials',ue.Material,ue.MaterialFactoryNew())
    ed=ue.MaterialEditingLibrary
    def node(cls): return ed.create_material_expression(mat,cls,-300,0)
    color_node=node(ue.MaterialExpressionConstant3Vector)
    color_node.constant=ue.LinearColor(*color,1)
    basecolor=color_node
    assert not plaster, 'Use the proven Substrate facade material instance'
    n=node(ue.MaterialExpressionConstant);n.r=rough
    assert ed.connect_material_property(n,'',ue.MaterialProperty.MP_ROUGHNESS)
    assert ed.connect_material_property(basecolor,'',ue.MaterialProperty.MP_BASE_COLOR)
    n=node(ue.MaterialExpressionConstant);n.r=metal
    assert ed.connect_material_property(n,'',ue.MaterialProperty.MP_METALLIC)
    ed.set_base_material_usage(mat,ue.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES,True)
    ed.recompile_material(mat)
    a.assets.set_metadata_tag(mat,'IBS25R2Parameters',signature);a.stamp(mat)
    return mat

def facade_material(name,color):
    # A/B at the fixed facade camera showed the new legacy shader producing false
    # dark surface patterns. Preserve the donor's proven Substrate glTF graph/UVs.
    # Only the owned material instance's BaseColorFactor changes; no donor mutation.
    dest=a.ROOT+'/Materials/MI_R2v2_'+name
    source='/Game/IB_Donors/UrbanFacades/ImpactApartments/Materials/modular_urban_apartments_facade_plaster'
    signature=base.digest(dict(source=source,color=color))
    if a.assets.does_asset_exist(dest):
        mat=ue.load_asset(dest)
        assert a.owned(mat) and a.assets.get_metadata_tag(mat,'IBS25R2Parameters')==signature
        return mat
    mat=a.assets.duplicate_asset(source,dest)
    assert isinstance(mat,ue.MaterialInstanceConstant)
    ed=ue.MaterialEditingLibrary
    assert 'BaseColorFactor' in [str(n) for n in ed.get_vector_parameter_names(mat)]
    ed.set_material_instance_vector_parameter_value(mat,'BaseColorFactor',ue.LinearColor(*color,1))
    observed=ed.get_material_instance_vector_parameter_value(mat,'BaseColorFactor')
    assert all(abs(v-w)<1e-5 for v,w in zip((observed.r,observed.g,observed.b),color))
    ed.update_material_instance(mat)
    a.assets.set_metadata_tag(mat,'IBS25R2Parameters',signature);a.stamp(mat)
    return mat

def camera_state():
    result={}
    for view in data['views']:
        c=lookup['IBS25_'+view['id']];p=c.get_actor_location();q=c.get_actor_rotation()
        result[view['id']]=[p.x,p.y,p.z,q.pitch,q.yaw,q.roll,c.camera_component.field_of_view]
    return result
camera_before=camera_state()
soil=a.material('R2_Soil',IBS25_soil.imported())
ground_path=a.ROOT+'/Materials/M_R2_GroundScale100'
if a.assets.does_asset_exist(ground_path):
    ground=ue.load_asset(ground_path);assert a.owned(ground)
else:
    ground=a.assets.duplicate_asset(soil.get_path_name(),ground_path)
    ed=ue.MaterialEditingLibrary
    uv=ed.create_material_expression(ground,ue.MaterialExpressionTextureCoordinate,-800,0)
    scale=ed.create_material_expression(ground,ue.MaterialExpressionMultiply,-600,0)
    scale.set_editor_property('const_b',100.0)
    assert ed.connect_material_expressions(uv,'',scale,'A')
    for n in ed.get_material_expressions(ground):
        if isinstance(n,ue.MaterialExpressionTextureSample):
            assert ed.connect_material_expressions(scale,'',n,'UVs')
    ed.recompile_material(ground);a.stamp(ground)
styles={name:facade_material(name,color) for name,color in
        [('Stone',(1.2,1.9,3.0)),('Blue',(.12,.8,2.5)),('Clay',(1.15,.6,.5))]}
materials={'Paving':ue.load_asset(a.ROOT+'/Materials/M_Paving'),
           'Markings':ue.load_asset(a.ROOT+'/Materials/M_Markings'),
           'Roof':pbr('Roof',(.20,.22,.23),.85), 'Ground':ground,'Soil':soil,
           'Curb':pbr('Curb',(.42,.43,.40),.82),
           'Metal':pbr('Metal',(.07,.09,.10),.4,.75),
           'RoofMetal':pbr('RoofMetal',(.39,.43,.45),.48,.65),
           'StoneDetail':pbr('StoneDetail',(.5,.49,.44),.75),
           'BlueDetail':pbr('BlueDetail',(.08,.16,.20),.4,.5),
           'ClayDetail':pbr('ClayDetail',(.38,.16,.075),.7),
           'GlassSign':pbr('GlassSign',(.26,.42,.43),.22,.35)}
for name in r.geometry():
    obj=actor(name,ue.StaticMeshActor)
    obj.set_actor_scale3d(ue.Vector(100,-100,1) if name=='R2_Ground' else ue.Vector(1,-1,1))
    c=obj.static_mesh_component
    c.set_static_mesh(a.imported_mesh(name,out))
    c.set_material(0,materials[name[3:]])
    c.set_collision_profile_name('NoCollision' if name.endswith(('Ground','Markings','GlassSign')) else 'BlockAll')
    c.set_cast_shadow(name not in ('R2_Ground','R2_Soil','R2_Markings'))
    if name=='R2_Ground':
        # Distant support only: no added city, gameplay collision or giant shadow caster.
        c.set_editor_property('affect_distance_field_lighting',False)
# Delete only exact superseded owned actors after all replacement meshes/materials exist.
superseded=[o for name,o in lookup.items() if name in ('IBS25_Paving','IBS25_Markings','IBS25_Roof')
            or name.startswith(('IBS25_B0_-1_','IBS25_B0_+1_'))]
for obj in superseded:
    lookup.pop(obj.get_actor_label());assert actors.destroy_actor(obj)

for b in data['buildings']:
    if not b['id'].startswith('R2_'): continue
    batches={}
    for panel in base.facade_panels(b):
        # Keep donor entry and cornice meshes; styles differ in detail/rhythm too.
        module=panel['module']
        if module in ('BayWide','BayDouble'):
            module='BayDouble' if b['style']=='Blue' else 'BayWide'
        mesh=ue.load_asset(base.FACADE+module);bounds=mesh.get_bounds()
        wide_x=bounds.box_extent.x>bounds.box_extent.y
        width=bounds.box_extent.x if wide_x else bounds.box_extent.y
        sx,sy=(panel['width_m']*50/width,1) if wide_x else (1,panel['width_m']*50/width)
        sz=panel['height_m']*50/bounds.box_extent.z
        yaw=math.radians(panel['yaw']-(0 if wide_x else 90))
        ox,oy=bounds.origin.x*sx,bounds.origin.y*sy
        t=ue.Transform();t.translation=a.point(panel['center_m'])-ue.Vector(ox*math.cos(yaw)-oy*math.sin(yaw),ox*math.sin(yaw)+oy*math.cos(yaw),bounds.origin.z*sz)
        t.rotation=ue.Quat(0,0,math.sin(yaw/2),math.cos(yaw/2));t.scale3d=ue.Vector(sx,sy,sz)
        batches.setdefault(module,[]).append(t)
    for module,transforms in batches.items():
        obj=actor(b['id']+'_'+module,ue.ImpactFacade)
        c=obj.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
        mesh=ue.load_asset(base.FACADE+module);c.set_static_mesh(mesh)
        for i,slot in enumerate(mesh.static_materials):
            source=slot.material_interface
            c.set_material(i,styles[b['style']] if source and 'plaster' in source.get_name() else source)
        c.clear_instances();c.add_instances(transforms,False,True,False)
        c.set_collision_profile_name('NoCollision');c.set_cast_shadow(True)
        assert c.get_instance_count()==len(transforms)
    obj=actor(b['id']+'_Collision',ue.StaticMeshActor,(*b['center_m'],b['base_m']+b['stories']*b['floor_m']/2))
    c=obj.static_mesh_component;c.set_static_mesh(ue.load_asset('/Engine/BasicShapes/Cube'))
    c.set_visibility(False);c.set_hidden_in_game(True);c.set_cast_shadow(False);c.set_collision_profile_name('BlockAll')
    obj.set_actor_scale3d(ue.Vector(b['width_m']-.2,b['depth_m']-.2,b['stories']*b['floor_m']))

restored=0
for obj in actors.get_all_level_actors():
    if not obj.get_actor_label().startswith('IBS25_'): continue
    c=obj.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
    if c:
        for i,slot in enumerate(c.static_mesh.static_materials):
            current=c.get_material(i)
            if current and current.get_name()=='M_MVP_WindowInteriors':
                c.set_material(i,slot.material_interface);restored+=1
# Mild indirect fill, fixed exposure and sun; no fog or camera changes to mask defects.
lookup['IBS25_Sky'].light_component.set_editor_property('intensity',1.25)
assert camera_state()==camera_before,'Camera changed'
oldmarker.tags=[ue.Name(base.OWNER),ue.Name('IBS25Revision_R2_'+fingerprint)]
assert levels.save_current_level()
report=dict(revision='R2',recipe_sha256=fingerprint,cameras_before=camera_before,cameras_after=camera_state(),
            restored_interior_slots=restored,sky_intensity=1.25,sun=data['sun'],exposure_ev100=14,
            actors=len(actors.get_all_level_actors()),visual_acceptance='PENDING',performance='NOT_MEASURED',gameplay='NOT_TESTED')
Path(__file__).with_name('IBS25_jobs').joinpath('refinement.json').write_text(json.dumps(report,indent=2))
ue.log('IBS25_R2_APPLIED '+json.dumps(report))
