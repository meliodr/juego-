"""Import the authorized City Kit CC0 vehicle used on the circuit."""
import os
import unreal as ue

destination = "/Game/IB_Donors/Carro"
asset = "/Game/IB_Donors/Carro/carro/StaticMeshes/carro"
if not ue.EditorAssetLibrary.does_asset_exist(asset):
    task = ue.AssetImportTask()
    task.filename = os.path.join(os.environ["IB_CITY_DONOR"], "models", "veiculos", "carro.glb")
    task.destination_path = destination
    task.automated = True
    task.save = True
    ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    ue.log(f"IB MVP imported car paths: {list(task.imported_object_paths)}")
mesh = ue.load_asset(asset)
assert isinstance(mesh, ue.StaticMesh), asset
ue.log(f"IB MVP car mesh bounds: {mesh.get_bounds()}")
