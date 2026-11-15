"""Release only the coordinated Python-driven S25 editor after saving its lab."""
import unreal as ue
import IBS25_recipe as recipe
world=ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split('.')[0]!=recipe.MAP:
    raise RuntimeError('Refusing to close an unrelated world')
dirty=ue.EditorLoadingAndSavingUtils.get_dirty_map_packages()
if any(p.get_name()!=recipe.MAP for p in dirty):
    raise RuntimeError('Unrelated dirty maps must be handled by their owner')
if not ue.get_editor_subsystem(ue.LevelEditorSubsystem).save_current_level():
    raise RuntimeError('Lab save failed; editor remains open')
ue.EditorPythonScripting.set_keep_python_script_alive(False)
