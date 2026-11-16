"""Repeat lab authoring and compare actor/instance state, not binary save timestamps."""
import hashlib
import importlib
import json
import runpy
from pathlib import Path
import unreal as ue
import IBS25_author

def transform_values(t):
    return [round(v,6) for v in (t.translation.x,t.translation.y,t.translation.z,
                                t.rotation.x,t.rotation.y,t.rotation.z,t.rotation.w,
                                t.scale3d.x,t.scale3d.y,t.scale3d.z)]

def snapshot():
    rows=[]
    for a in ue.get_editor_subsystem(ue.EditorActorSubsystem).get_all_level_actors():
        if not a.get_actor_label().startswith('IBS25_'):
            continue
        t=a.get_actor_transform()
        row={'actor':a.get_actor_label(),'class':a.get_class().get_name(),
             'transform':transform_values(t)}
        c=a.get_component_by_class(ue.StaticMeshComponent)
        if c and c.static_mesh:
            row['mesh']=c.static_mesh.get_path_name()
            row['materials']=[c.get_material(i).get_path_name() if c.get_material(i) else None for i in range(c.get_num_materials())]
            row['forced_lod_model']=c.get_editor_property('forced_lod_model')
        if isinstance(a,ue.CameraActor):
            row['fov']=a.camera_component.field_of_view
        h=a.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
        if h:
            row['instances']=h.get_instance_count()
            row['instance_transforms']=[transform_values(h.get_instance_transform(i,True)) for i in range(h.get_instance_count())]
        rows.append(row)
    return sorted(rows,key=lambda r:r['actor'])

before=snapshot()
runpy.run_path(str(Path(__file__).with_name('IBS25_refine.py')),run_name='__main__')
after=snapshot()
digest=lambda rows:hashlib.sha256(json.dumps(rows,sort_keys=True).encode()).hexdigest()
report={'status':'PASS' if before==after else 'FAIL_STATE_CHANGED',
        'before_actor_count':len(before),'after_actor_count':len(after),
        'before_instances':sum(r.get('instances',0) for r in before),
        'after_instances':sum(r.get('instances',0) for r in after),
        'before_sha256':digest(before),'after_sha256':digest(after),
        'hash_scope':'R2 refinement: actor class/label/transforms, FOV, forced LOD, material/mesh references, HISM transforms/counts; numeric transforms rounded to 1e-6, no transient struct addresses',
        'changed_actors':[a['actor'] for a,b in zip(before,after) if a!=b],
        'collision_playtest':'PENDING','performance_benchmark':'PENDING'}
Path(__file__).resolve().parents[1].joinpath('docs/IB_S25_R2_VALIDATION.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
if before!=after:
    raise RuntimeError('Repeated authoring changed the observed actor state')
