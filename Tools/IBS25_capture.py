"""Five sequential, warm-start editor screenshots. NOT a performance benchmark.

Open the lab in a coordinated visible DX12 editor first. Execute with Python.
IBS25_RUN_ID must name a new directory (letters/digits/_/-) under Tools/IBS25_evidence.
Checks PNG bytes and records camera/sun metadata; does not judge realism.
"""
import datetime
import json
import os
from pathlib import Path
import re
import struct
import sys
import time

sys.dont_write_bytecode = True
sys.path.insert(0,str(Path(__file__).resolve().parent))
import IBS25_recipe as recipe
import unreal as ue


class Capture:
    def __init__(self):
        run = os.environ.get("IBS25_RUN_ID", "")
        if not re.fullmatch(r"[A-Za-z0-9_-]{1,64}",run):
            raise ValueError("Set a fresh IBS25_RUN_ID")
        self.levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
        current = self.levels.get_current_level().get_outer().get_path_name()
        if current.split(".")[0] != recipe.MAP or self.levels.is_in_play_in_editor():
            raise RuntimeError("Open the S2.5 lab, outside PIE. No map will be auto-loaded.")
        self.cameras = {a.get_actor_label():a for a in ue.get_editor_subsystem(ue.EditorActorSubsystem).get_all_level_actors()}
        for view in recipe.VIEWS:
            camera = self.cameras.get("IBS25_"+view["id"])
            if not isinstance(camera,ue.CameraActor):
                raise RuntimeError("Missing camera " + view["id"])
        self.output = Path(__file__).resolve().parent/"IBS25_evidence"/run
        self.output.mkdir(parents=True,exist_ok=False)
        self.old_pilot = self.levels.get_pilot_level_actor()
        self.old_info = self.levels.get_level_viewport_camera_info(self.levels.get_active_viewport_config_key())
        self.rows = []
        self.index, self.task, self.handle = 0, None, None
        self.started = time.monotonic()
        self.begin_view()
        self.handle = ue.register_slate_post_tick_callback(self.tick)

    def begin_view(self):
        self.view = recipe.VIEWS[self.index]
        self.camera = self.cameras["IBS25_"+self.view["id"]]
        self.levels.pilot_level_actor(self.camera)
        self.deadline = time.monotonic()+10
        self.warm_ticks = 0
        self.task = None

    def tick(self, delta):
        try:
            if time.monotonic()-self.started > 300:
                raise TimeoutError("Capture did not complete in 300 seconds")
            if self.task is None:
                # Background throttling can consume wall time without rendering.
                # Require viewport/Slate progress too before taking the screenshot.
                self.warm_ticks += 1
                if time.monotonic() < self.deadline or self.warm_ticks < 240:
                    return
                self.path = self.output/(self.view["id"]+".png")
                self.task = ue.AutomationLibrary.take_high_res_screenshot(1920,1080,str(self.path),camera=self.camera)
                if not self.task or not self.task.is_valid_task():
                    raise RuntimeError("Screenshot was not queued")
                return
            if not self.task.is_task_done() or not self.path.exists():
                return
            image = self.path.read_bytes()
            if image[:8] != b"\x89PNG\r\n\x1a\n" or struct.unpack(">II",image[16:24]) != (1920,1080):
                raise RuntimeError("Invalid screenshot dimensions/format")
            # Record observed transforms in addition to planned transforms.
            location = self.camera.get_actor_location()
            rotation = self.camera.get_actor_rotation()
            self.rows.append({**self.view,"recipe_position_m":self.view["position_m"],
                              "recipe_rotation_deg":self.view["rotation_deg"],"recipe_fov":self.view["fov"],
                              "position_m":[location.x/100,location.y/100,location.z/100],
                              "rotation_deg":[rotation.pitch,rotation.yaw,rotation.roll],
                              "fov":self.camera.camera_component.field_of_view,
                              "observed_position_cm":[location.x,location.y,location.z],
                              "observed_rotation_deg":[rotation.pitch,rotation.yaw,rotation.roll],
                              "observed_fov":self.camera.camera_component.field_of_view,
                              "file":self.path.name,"bytes":len(image),"visual_review":"PENDING"})
            self.index += 1
            if self.index == len(recipe.VIEWS):
                self.finish("CAPTURED_NOT_VISUALLY_ACCEPTED")
            else:
                self.begin_view()
        except Exception as error:
            self.finish("FAILED: " + str(error))

    def finish(self,status):
        if self.handle is not None:
            ue.unregister_slate_post_tick_callback(self.handle)
            self.handle = None
        self.levels.eject_pilot_level_actor()
        if self.old_pilot:
            self.levels.pilot_level_actor(self.old_pilot)
        elif self.old_info:
            self.levels.set_level_viewport_camera_info(*self.old_info,self.levels.get_active_viewport_config_key())
        payload = {"status":status,"utc":datetime.datetime.now(datetime.timezone.utc).isoformat(),
                   "map":recipe.MAP,"recipe_sha256":recipe.digest(recipe.scene()),
                   "sun_planned":recipe.scene()["sun"],"exposure_ev100_planned":14,
                   "engine":ue.SystemLibrary.get_engine_version(),"warmup_seconds_per_view":10,
                   "minimum_warmup_slate_ticks":240,
                   "resolution":[1920,1080],"benchmark":False,"views":self.rows}
        # R2 retains camera poses but changes the geometry recipe. Record observed
        # lighting and revision instead of suggesting the base hash describes R2.
        marker=self.cameras.get('IBS25_MANIFEST')
        payload['map_revision_tags']=[str(t) for t in marker.tags] if marker else []
        for tag in payload['map_revision_tags']:
            if tag.startswith('IBS25Revision_R2_'):
                payload['base_recipe_sha256']=payload['recipe_sha256']
                payload['recipe_sha256']=tag.removeprefix('IBS25Revision_R2_')
        sun=self.cameras.get('IBS25_Sun'); sky=self.cameras.get('IBS25_Sky')
        exposure=self.cameras.get('IBS25_Exposure')
        if sun:
            rotation=sun.get_actor_rotation()
            payload['sun_observed']={'pitch':rotation.pitch,'yaw':rotation.yaw,
                                     'lux':sun.light_component.intensity}
        if sky: payload['sky_intensity_observed']=sky.light_component.intensity
        if exposure:
            settings=exposure.get_editor_property('settings')
            payload['exposure_observed']={'min':settings.auto_exposure_min_brightness,
                                          'max':settings.auto_exposure_max_brightness}
        (self.output/"capture.json").write_text(json.dumps(payload,indent=2),encoding="utf-8")
        ue.log("IBS25 " + status)


# Keep callback owner alive across editor ticks; no blocking sleep on the game thread.
if __name__ == "__main__":
    IBS25_CAPTURE = Capture()
