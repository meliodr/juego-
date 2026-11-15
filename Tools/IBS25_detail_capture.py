"""Supplemental intersection evidence, not one of the five comparison cameras.
Temporary camera is removed after capture. No persistent camera or scene edits.
"""
import datetime
import json
from pathlib import Path
import time
import unreal as ue
import IBS25_recipe as recipe

levels=ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors=ue.get_editor_subsystem(ue.EditorActorSubsystem)
assert levels.get_current_level().get_outer().get_path_name().split('.')[0]==recipe.MAP
assert not levels.is_in_play_in_editor()
out=Path(__file__).with_name('IBS25_evidence')/('intersection_'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S'))
out.mkdir(exist_ok=False)
old_pilot=levels.get_pilot_level_actor()
old_info=levels.get_level_viewport_camera_info(levels.get_active_viewport_config_key())
before=len(actors.get_all_level_actors())
camera=actors.spawn_actor_from_class(ue.CameraActor,ue.Vector(0,0,7500),ue.Rotator(pitch=-90,yaw=0,roll=0))
camera.set_actor_label('IBS25_QA_TEMP_Intersection')
camera.tags=[ue.Name(recipe.OWNER)]
camera.camera_component.field_of_view=65
camera.camera_component.aspect_ratio=16/9
levels.pilot_level_actor(camera)
deadline=time.monotonic()+10
started=time.monotonic()
ticks=0
task=None
handle=None
def finish(status):
    global handle
    ue.unregister_slate_post_tick_callback(handle);handle=None
    levels.eject_pilot_level_actor()
    assert actors.destroy_actor(camera)
    if old_pilot: levels.pilot_level_actor(old_pilot)
    elif old_info: levels.set_level_viewport_camera_info(*old_info,levels.get_active_viewport_config_key())
    assert len(actors.get_all_level_actors())==before
    (out/'capture.json').write_text(json.dumps(dict(status=status,position_m=[0,0,75],rotation_deg=[-90,0,0],fov=65,
         resolution=[1920,1080],supplemental=True,comparison_camera=False,temporary_camera_removed=True,
         benchmark=False),indent=2))
def tick(delta):
    global ticks,task
    ticks+=1
    if time.monotonic()-started>120:
        finish('TIMEOUT');return
    if time.monotonic()<deadline or ticks<240: return
    if task is None:
        task=ue.AutomationLibrary.take_high_res_screenshot(1920,1080,str(out/'06_intersection_detail.png'),camera=camera)
        return
    if task.is_task_done() and (out/'06_intersection_detail.png').exists(): finish('CAPTURED_NOT_ACCEPTED')
handle=ue.register_slate_post_tick_callback(tick)
