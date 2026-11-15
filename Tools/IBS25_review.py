"""Capture the existing isolated lab; run through the authorized editor queue."""
import os
import datetime
from pathlib import Path
import unreal as ue
import IBS25_capture
import importlib
importlib.reload(IBS25_capture)

world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split('.')[0] != '/Game/IB_S25/Lvl_IB_S25_RealismLab':
    raise RuntimeError('Open the isolated S25 lab before capture')
for command in ('r.EyeAdaptation.CachedLightingPreExposure 8',
                'r.Shadow.Virtual.Enable 0', 'r.Streaming.PoolSize 2500',
                'r.AntiAliasingMethod 4', 'r.ScreenPercentage 100'):
    ue.SystemLibrary.execute_console_command(world, command)
os.environ['IBS25_RUN_ID'] = os.environ.pop('IBS25_NEXT_RUN_ID',None) or ('visual_'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
IBS25_CAPTURE = IBS25_capture.Capture()
