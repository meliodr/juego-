"""Read-only material diagnostics, no donor edits."""
import json
from pathlib import Path
import unreal as ue
rows=[]
root='/Game/IB_Donors/UrbanFacades/ImpactApartments/'
for path in [root+'Materials/modular_urban_apartments_facade_plaster',
             '/Game/IB_S25/Materials/M_R2v3_Stone']:
    mat=ue.load_asset(path)
    row={'path':path,'class':mat.get_class().get_name()}
    if isinstance(mat,ue.MaterialInstanceConstant):
        row['parent']=mat.parent.get_path_name() if mat.parent else None
        row['vectors']={str(n):str(ue.MaterialEditingLibrary.get_material_instance_vector_parameter_value(mat,n)) for n in ue.MaterialEditingLibrary.get_vector_parameter_names(mat)}
        row['textures']={str(n):str(ue.MaterialEditingLibrary.get_material_instance_texture_parameter_value(mat,n)) for n in ue.MaterialEditingLibrary.get_texture_parameter_names(mat)}
    if isinstance(mat,ue.Material):
        row['two_sided']=mat.get_editor_property('two_sided');row['tangent_space_normal']=mat.get_editor_property('tangent_space_normal')
        row['nodes']=[]
        for n in ue.MaterialEditingLibrary.get_material_expressions(mat):
            item={'class':n.get_class().get_name()}
            if isinstance(n,ue.MaterialExpressionTextureSample):
                item.update(texture=n.texture.get_path_name() if n.texture else None,sampler=str(n.sampler_type))
            row['nodes'].append(item)
    rows.append(row)
for path in [root+'Textures/modular_urban_apartments_facade_plaster_nor_gl',
             '/Game/IB_S25/Textures/T_R2_PlasterNormalDX']:
    t=ue.load_asset(path)
    rows.append(dict(path=path,srgb=t.srgb,flip_green_channel=t.flip_green_channel,compression=str(t.compression_settings)))
Path(__file__).with_name('IBS25_jobs').joinpath('surfaceinspect.json').write_text(json.dumps(rows,indent=2))
