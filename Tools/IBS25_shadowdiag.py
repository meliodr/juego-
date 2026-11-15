"""Read local facade geometry/materials; capture an original-glass control.
Temporary component overrides are restored after screenshot, never saved.
"""
import json
from pathlib import Path
import time
import unreal as ue
import IBS25_recipe as recipe

levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
if ue.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError('Save unrelated edits before diagnosis')
if not levels.load_level(recipe.MAP):
    raise RuntimeError('Lab unavailable')
world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
for cmd in ('r.EyeAdaptation.CachedLightingPreExposure 8','r.Shadow.Virtual.Enable 0',
            'r.Streaming.PoolSize 2500','r.AntiAliasingMethod 4','r.ScreenPercentage 100'):
    ue.SystemLibrary.execute_console_command(world,cmd)
out = Path(__file__).with_name('IBS25_evidence')/'shadow_diagnosis_r2'
out.mkdir(exist_ok=False)
rows, restored = [], []
camera = None
for actor in ue.get_editor_subsystem(ue.EditorActorSubsystem).get_all_level_actors():
    if actor.get_actor_label() == 'IBS25_03_facade_10m':
        camera = actor
    c = actor.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
    if not c or not actor.get_actor_label().startswith('IBS25_'):
        continue
    mesh = c.static_mesh
    row = {'actor':actor.get_actor_label(),'mesh':mesh.get_path_name(),
           'instances':c.get_instance_count(),'bounds':str(mesh.get_bounds()),'slots':[]}
    for i,slot in enumerate(mesh.static_materials):
        current, original = c.get_material(i), slot.material_interface
        row['slots'].append({'slot':i,'current':current.get_path_name() if current else None,
                             'original':original.get_path_name() if original else None})
        if current and current.get_name().startswith('M_R2v3_') and original and 'plaster' in original.get_name():
            restored.append((c,i,current))
            c.set_material(i,original)
    rows.append(row)
(out/'diagnosis.json').write_text(json.dumps(rows,indent=2))
assert camera and restored
levels.pilot_level_actor(camera)
deadline, task, handle = time.monotonic()+15,None,None
def tick(delta):
    global task,handle
    if time.monotonic()<deadline:
        return
    if task is None:
        task=ue.AutomationLibrary.take_high_res_screenshot(1920,1080,str(out/'03_original_plaster.png'),camera=camera)
        return
    if not task.is_task_done():
        return
    for c,i,m in restored:
        c.set_material(i,m)
    ue.unregister_slate_post_tick_callback(handle)
    ue.log('IBS25_SHADOW_DIAGNOSIS_CAPTURED_OVERRIDES_RESTORED')
handle=ue.register_slate_post_tick_callback(tick)
