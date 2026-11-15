"""Apply the verified S3 lighting fix to this lab's existing authored map only."""
from pathlib import Path
import runpy
import unreal

api = runpy.run_path(str(Path(__file__).with_name("IBS3_AuthorLab.py")))
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.load_level(api["MAP"]):
    raise RuntimeError("Cannot load isolated S3 map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
suns = [a for a in actors if isinstance(a, unreal.DirectionalLight)]
skies = [a for a in actors if isinstance(a, unreal.SkyLight)]
if len(suns) != 1 or len(skies) != 1:
    raise RuntimeError("Unexpected S3 lighting; preserve map for inspection")
api["configure_lighting"](suns[0], skies[0])
if not levels.save_current_level():
    raise RuntimeError("S3 lighting save failed")
unreal.log("IB_S3 lighting repair complete")
