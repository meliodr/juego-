"""Load only the saved lab and capture it in an already coordinated editor."""
from pathlib import Path
import runpy
import unreal as ue
import IBS25_recipe as recipe
levels=ue.get_editor_subsystem(ue.LevelEditorSubsystem)
assert not ue.EditorLoadingAndSavingUtils.get_dirty_map_packages(), 'Unsaved map open'
assert levels.load_level(recipe.MAP)
runpy.run_path(str(Path(__file__).with_name('IBS25_review.py')),run_name='__main__')
