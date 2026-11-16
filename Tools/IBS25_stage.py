"""Local authorized PBR/tree import, then isolated lab authoring."""
import json
import os
from pathlib import Path
import sys
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
import unreal as ue
import IBS25_pbr as pbr
import IBS25_author as author

root=Path(__file__).resolve().parent
pbr.import_into_unreal()
tree_root=root/'IBS25_sources'/'tree'
gltf=json.loads((tree_root/'island_tree_01_1k.gltf').read_text())
# The glTF's leaves are actual geometry and its RGB JPEG carries no alpha.
# Opaque geometry avoids unnecessary translucency; no silhouette cards are invented.
for material in gltf['materials']:
    material.pop('alphaMode',None)
selected=tree_root/'S25_IslandTree.gltf'
selected.write_text(json.dumps(gltf),encoding='utf-8')
destination='/Game/IB_S25/Vegetation'
paths=ue.EditorAssetLibrary.list_assets(destination,True,False) if ue.EditorAssetLibrary.does_directory_exist(destination) else []
meshes=[ue.load_asset(path) for path in paths if '/StaticMeshes/' in path]
meshes=[m for m in meshes if isinstance(m,ue.StaticMesh)]
if not meshes:
    task=ue.AssetImportTask()
    task.filename=str(selected)
    task.destination_path=destination
    task.automated=True
    task.save=True
    task.replace_existing=False
    ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    meshes=[ue.load_asset(p) for p in task.imported_object_paths]
    meshes=[m for m in meshes if isinstance(m,ue.StaticMesh)]
if len(meshes)!=1:
    raise RuntimeError('Expected one tree mesh, got '+str([m.get_path_name() for m in meshes]))
tree=meshes[0]
# Preserve original LOD0; native reductions will be evaluated in screenshots/performance.
if tree.get_num_lods()==1:
    options=ue.StaticMeshReductionOptions()
    options.auto_compute_lod_screen_size=False
    options.reduction_settings=[ue.StaticMeshReductionSettings(percent_triangles=p,screen_size=s) for p,s in ((1.0,1.0),(0.25,0.5),(0.08,0.2))]
    ue.get_editor_subsystem(ue.StaticMeshEditorSubsystem).set_lods(tree,options)
    ue.EditorAssetLibrary.save_loaded_asset(tree)
config=json.loads((root/'IBS25_sources'/'assets.json').read_text())
config['tree']=tree.get_path_name()
cfg=root/'IBS25_sources'/'runtime_assets.json'
cfg.write_text(json.dumps(config,indent=2),encoding='utf-8')
os.environ['IBS25_ASSETS']=str(cfg)
os.environ['IBS25_GENERATED']=str(root/'IBS25_generated')
os.environ['IBS25_EXECUTE']='1'
author.main()
