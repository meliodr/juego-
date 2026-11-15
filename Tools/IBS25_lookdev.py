"""Apply reviewed component-only corrections to the existing owned lab."""
import json
from pathlib import Path
import unreal as ue
import IBS25_recipe as recipe
import importlib
import IBS25_author
author=importlib.reload(IBS25_author)

levels=ue.get_editor_subsystem(ue.LevelEditorSubsystem)
world=ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split('.')[0] != recipe.MAP:
    raise RuntimeError('Wrong world')
rows=[]
materials=[ue.load_asset('/Game/Car/Assets/'+n) for n in
           ('M_TireRubber','MI_CarPaint','M_Glass','M_Metal','M_TireRubber')]
if not all(isinstance(m,ue.MaterialInterface) for m in materials):
    raise RuntimeError('Missing donor car material')
for a in ue.get_editor_subsystem(ue.EditorActorSubsystem).get_all_level_actors():
    label=a.get_actor_label()
    if not label.startswith('IBS25_'):
        continue
    if recipe.OWNER not in [str(t) for t in a.tags]:
        raise RuntimeError('Foreign actor: '+label)
    if label in ('IBS25_Asphalt','IBS25_Paving','IBS25_Markings','IBS25_Roof'):
        a.set_actor_scale3d(ue.Vector(1,-1,1))
        rows.append({'actor':label,'scale':[1,-1,1]})
    if label.startswith('IBS25_Car_'):
        c=a.static_mesh_component
        if c.get_num_materials()!=len(materials):
            raise RuntimeError('Car material slots changed')
        for i,m in enumerate(materials):
            c.set_material(i,author.car_paint(int(label.split('_')[-1])) if i==1 else m)
        rows.append({'actor':label,'materials':[m.get_path_name() for m in materials]})
    if label.startswith('IBS25_Tree_'):
        a.static_mesh_component.set_editor_property('forced_lod_model',1)
        rows.append({'actor':label,'forced_lod_model':1,'performance_acceptance':'PENDING'})
    if label=='IBS25_02_corner_cars':
        # The original corner frame missed every car; keep the new measured pose explicit.
        a.set_actor_location(ue.Vector(-1800,100,170),False,True)
        a.set_actor_rotation(ue.Rotator(pitch=3,yaw=-138,roll=0),True)
        a.camera_component.set_editor_property('field_of_view',65)
        rows.append({'actor':label,'position_cm':[-1800,100,170],'rotation_deg':[3,-138,0],'fov':65})
# The installed UE5.8 source documents value 8 as covering exposure EV[-4,16].
ue.SystemLibrary.execute_console_command(world,'r.EyeAdaptation.CachedLightingPreExposure 8')
if not levels.save_current_level():
    raise RuntimeError('Lab save failed')
Path(__file__).with_name('IBS25_jobs').joinpath('lookdev.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
