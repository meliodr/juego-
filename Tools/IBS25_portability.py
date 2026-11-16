"""Inventory, stage and verify R2 dependencies without Unreal or target writes.

Only stage() writes binary payload, under this checkout's ignored staging folder.
Target mode is ALWAYS read-only. Exact paths and SHA-256 guard local handoff.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import sys

sys.dont_write_bytecode = True
from IBS25_package_refs import references

ROOT = Path(__file__).resolve().parents[1]
CONTENT = 'ImpactBeyond/Content/'
LAB = '/Game/IB_S25/Lvl_IB_S25_RealismLab'
SCOPES = ('IB_S25', 'Car', 'IB_Donors/UrbanFacades/ImpactApartments')
MANIFEST = ROOT / 'docs/IB_S25_PORTABILITY_MANIFEST.json'
STAGING = ROOT / 'Tools/IBS25_portability_staging/package'
ENGINE = Path(r'C:\Program Files\Epic Games\UE_5.8')
EXTENSIONS = {'.uasset', '.umap', '.uexp', '.ubulk', '.uptnl'}
TEMPLATE = 'Templates/TemplateResources/Standard/ConceptCar/Content/Car'
ART_COMMIT = '70187086042669e8ec178b3b268c57730da8492f'
POLY = 'https://polyhaven.com/license'
EULA = 'https://www.unrealengine.com/eula/unreal'


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def safe_path(root, relative):
    p = PurePosixPath(relative)
    if not relative or '\\' in relative or ':' in relative or p.is_absolute() or any(x in ('..', '.') for x in relative.split('/')):
        raise ValueError('unsafe relative path: ' + relative)
    root = root.resolve()
    result = root.joinpath(*p.parts)
    current = root
    for part in p.parts:
        current = current / part
        if current.is_symlink() or (hasattr(current, 'is_junction') and current.is_junction()):
            raise ValueError('link/junction not permitted: ' + str(current))
    if not result.resolve().is_relative_to(root):
        raise ValueError('path escapes root')
    return result


def allowed_payload(path):
    if not path.startswith(CONTENT) or Path(path).suffix not in EXTENSIONS:
        raise ValueError('not an allowed content payload: ' + path)
    tail = path[len(CONTENT):]
    if not any(tail.startswith(s + '/') for s in SCOPES + ('ConceptCar/Car',)):
        raise ValueError('payload outside donor/lab scopes: ' + path)


def json_write(path, value):
    path.write_text(json.dumps(value, indent=2, ensure_ascii=True) + '\n', encoding='utf-8')


def provenance(package):
    if package.startswith('/Game/IB_S25/Geometry/'):
        return 'authored'
    if package.startswith(('/Game/Car/', '/Game/ConceptCar/Car/')) or package.rsplit('/',1)[-1].startswith(('MI_CarPaint', 'M_CarPaint_Local')):
        return 'epic_conceptcar'
    if '/UrbanFacades/' in package or '/MI_R2v2_' in package or 'PlasterNormalDX' in package:
        return 'poly_facade'
    if '/Vegetation/' in package:
        return 'poly_tree'
    if 'Soil' in package or 'GroundScale100' in package:
        return 'poly_soil'
    if '/Textures/T_asphalt_' in package or package.endswith('/M_Asphalt'):
        return 'poly_asphalt'
    if '/Textures/T_paving_' in package or package.endswith('/M_Paving'):
        return 'poly_paving'
    return 'authored'


def licenses():
    result = {}
    for key, asset in [('facade','modular_urban_apartments_facade'), ('tree','island_tree_01'),
                       ('soil','brown_mud_leaves_01'), ('asphalt','asphalt_02'), ('paving','concrete_pavement')]:
        result['poly_' + key] = {'source': 'https://polyhaven.com/a/' + asset,
            'license': 'CC0-1.0', 'license_url': POLY,
            'redistribution': 'CC0 source assets allow redistribution; engine dependencies retain their own terms.',
            'publication_this_task': False}
    result['epic_conceptcar'] = {'source': TEMPLATE, 'license': 'Unreal Engine EULA: Examples',
        'license_url': EULA, 'redistribution': 'Installed Templates content qualifies as Examples (section 1); section 5(b) permits distribution of Examples, including modifications. Not CC0. This task authorizes local copies only.',
        'publication_this_task': False}
    result['authored'] = {'source': 'Own procedural scripts and R2 layout at ' + ART_COMMIT,
        'license': 'Project-authored; no new open-source license granted',
        'redistribution': 'Local staging authorized; no publication in this task.', 'publication_this_task': False}
    return result


def closure(graph, start=LAB):
    todo, seen, missing, native = [start], set(), {}, set()
    while todo:
        key = todo.pop()
        if key in seen:
            continue
        if key not in graph:
            raise ValueError('missing root package: ' + key)
        seen.add(key)
        refs = graph[key]['imports'] + graph[key]['soft_packages']
        for ref in refs:
            if ref.startswith('/Game/'):
                if ref in graph:
                    todo.append(ref)
                else:
                    missing.setdefault(ref, set()).add(key)
            else:
                native.add(ref)
    return seen, {k: sorted(v) for k,v in sorted(missing.items())}, sorted(native)


def native_file(package, engine):
    if package.startswith('/Engine/'):
        return engine / ('Engine/Content/' + package[len('/Engine/'):] + '.uasset')
    if package.startswith('/InterchangeAssets/'):
        return engine / ('Engine/Plugins/Interchange/Assets/Content/' + package[len('/InterchangeAssets/'):] + '.uasset')
    return None


def inventory(engine=ENGINE):
    graph, entries = {}, {}
    for scope in SCOPES:
        folder = safe_path(ROOT, CONTENT + scope)
        if not folder.is_dir():
            raise ValueError('missing source directory: ' + scope)
        for p in sorted(folder.rglob('*')):
            if p.suffix not in EXTENSIONS:
                continue
            relative = p.relative_to(ROOT).as_posix()
            p = safe_path(ROOT, relative)
            if p.suffix not in ('.uasset', '.umap'):
                raise ValueError('unexpected sidecar requires explicit inventory support: ' + relative)
            package = '/Game/' + p.relative_to(ROOT / CONTENT).with_suffix('').as_posix()
            graph[package] = references(p.read_bytes())
            entries[package] = {'package': package, 'path': relative, 'origin': 'checkout',
                'source_path': relative, 'bytes': p.stat().st_size, 'sha256': digest(p),
                'format': p.suffix[1:], 'provenance': provenance(package)}
    before, missing_before, _ = closure(graph)
    template_proof = []
    # The installed template bytes are identical to the donor copy; restoring the
    # original mount-relative folder closes its serialized references, no resave.
    for p in sorted((ROOT / CONTENT / 'Car').rglob('*.uasset')):
        relative = p.relative_to(ROOT / CONTENT / 'Car').as_posix()
        src = safe_path(engine, TEMPLATE + '/' + relative)
        if not src.is_file() or digest(p) != digest(src):
            raise ValueError('ConceptCar template provenance mismatch: ' + relative)
        package = '/Game/ConceptCar/Car/' + str(PurePosixPath(relative).with_suffix(''))
        graph[package] = references(src.read_bytes())
        entries[package] = {'package': package, 'path': CONTENT + 'ConceptCar/Car/' + relative,
            'origin': 'engine_template', 'source_path': TEMPLATE + '/' + relative,
            'bytes': src.stat().st_size, 'sha256': digest(src), 'format': 'uasset',
            'provenance': 'epic_conceptcar', 'purpose': 'restore serialized original ConceptCar package path'}
        template_proof.append({'relative': relative, 'sha256': digest(src), 'matches_checkout': True})
    reached, missing, native = closure(graph)
    native_inventory = []
    for package in native:
        p = native_file(package, engine)
        item = {'package': package, 'bundled': False}
        if p:
            if not p.is_file():
                raise ValueError('native asset absent: ' + package)
            item.update({'path': p.relative_to(engine).as_posix(), 'bytes': p.stat().st_size, 'sha256': digest(p)})
        else:
            item['requirement'] = 'native script module; load/cook classification requires integrator UE validation'
        native_inventory.append(item)
    payload = [entries[k] for k in sorted(reached)]
    for entry in payload:
        allowed_payload(entry['path'])
    code_paths = ['ImpactBeyond/Source/ImpactBeyond/ImpactFacade.h', 'ImpactBeyond/Source/ImpactBeyond/ImpactFacade.cpp']
    receipts = ['Tools/IBS25_sources/manifest.json', 'Tools/IBS25_sources/tree/receipt.json', 'Tools/IBS25_sources/soil/receipt.json']
    raw_sources = []
    for receipt in receipts:
        rp = ROOT / receipt
        for item in json.loads(rp.read_text(encoding='utf-8')):
            name = item.get('filename', item.get('path'))
            file = safe_path(rp.parent, name)
            if not file.is_file() or digest(file) != item['sha256'].lower() or file.stat().st_size != item['bytes']:
                raise ValueError('raw source receipt mismatch: ' + str(file))
            raw_sources.append({'path': file.relative_to(ROOT).as_posix(), 'bytes': item['bytes'],
                'sha256': item['sha256'].lower(), 'source': item['source'], 'url': item['url'],
                'receipt': receipt, 'verified': True, 'bundled': False})
    plugin = engine / 'Engine/Plugins/Interchange/Assets/InterchangeAssets.uplugin'
    plugin_info = json.loads(plugin.read_text(encoding='utf-8-sig'))
    result = {'schema': 1, 'art_commit': ART_COMMIT, 'map': LAB, 'local_only': True,
        'engine_build': json.loads((engine / 'Engine/Build/Build.version').read_text(encoding='utf-8-sig')),
        'method': 'Offline version-checked FName/import/soft-package table closure. Not cooked/runtime reachability; does not deserialize arbitrary export data or code-built string loads. All reachable /Game packages included, native engine/plugin packages supplied by installed UE.',
        'license_checked_on': '2026-11-16', 'licenses': licenses(),
        'source_inventory': [dict(entries[k], selected=k in reached) for k in sorted(entries) if entries[k]['origin']=='checkout'],
        'template_provenance': template_proof, 'game_missing_before': missing_before,
        'game_missing_after': missing, 'package_graph': {k: {'imports': graph[k]['imports'], 'soft_packages': graph[k]['soft_packages'], 'ue4': graph[k]['ue4'], 'ue5': graph[k]['ue5']} for k in sorted(graph)},
        'payload': payload, 'payload_count': len(payload), 'payload_bytes': sum(e['bytes'] for e in payload),
        'native_prerequisites': native_inventory,
        'interchange_plugin': {'path': plugin.relative_to(engine).as_posix(), 'sha256': digest(plugin),
            'enabled_by_default': plugin_info['EnabledByDefault'], 'modules': plugin_info['Modules'],
            'plugin_dependencies': plugin_info['Plugins'], 'bundled': False},
        'native_class_contract': {'class': '/Script/ImpactBeyond.ImpactFacade', 'component': 'Facade', 'type': 'UHierarchicalInstancedStaticMeshComponent', 'files': [{'path': p, 'sha256': digest(ROOT / p)} for p in code_paths], 'copied': False},
        'source_receipts': [{'path': p, 'sha256': digest(ROOT / p)} for p in receipts],
        'verified_raw_sources': raw_sources,
        'not_copied': ['Config', 'Binaries', 'Intermediate', 'Saved', 'raw downloads', 'engine/plugin content', 'IB_C00', 'unused intermediate lab assets'],
        'validation_pending': ['Unreal load/AssetRegistry validation in candidate', 'Substrate/Interchange compatibility', 'cook/package', 'gameplay/collision', 'visible DX12 benchmark', 'art acceptance']}
    return result


def validate_manifest(m):
    if m.get('schema') != 1 or m.get('map') != LAB or m.get('game_missing_after'):
        raise ValueError('invalid/unresolved manifest')
    seen = set()
    for e in m['payload']:
        allowed_payload(e['path'])
        safe_path(ROOT, e['path'])
        if e['path'].casefold() in seen:
            raise ValueError('duplicate destination')
        seen.add(e['path'].casefold())
        if e['bytes'] <= 0 or len(e['sha256']) != 64 or any(c not in '0123456789abcdef' for c in e['sha256']):
            raise ValueError('invalid file size/hash')
        if e['origin'] == 'checkout':
            if e['source_path'] != e['path']:
                raise ValueError('checkout source mismatch')
        elif e['origin'] == 'engine_template':
            if not e['source_path'].startswith(TEMPLATE + '/') or e['path'] != CONTENT + 'ConceptCar/Car/' + e['source_path'][len(TEMPLATE)+1:]:
                raise ValueError('invalid template mapping')
        else:
            raise ValueError('unknown origin')
    if m['payload_count'] != len(m['payload']) or m['payload_bytes'] != sum(e['bytes'] for e in m['payload']):
        raise ValueError('payload totals mismatch')


def matches(path, entry):
    return path.is_file() and path.stat().st_size == entry['bytes'] and digest(path) == entry['sha256']


def compare(m, target):
    """Read-only target inspection; never creates even a report on the target."""
    validate_manifest(m)
    if not target.is_dir():
        raise ValueError('target root does not exist')
    result = {'identical': [], 'missing': [], 'conflicts': []}
    for e in m['payload']:
        p = safe_path(target, e['path'])
        key = 'missing' if not p.exists() else ('identical' if matches(p,e) else 'conflicts')
        result[key].append(e['path'])
    result['safe_to_copy_missing'] = not result['conflicts']
    return result


def copy_missing(m, sources, target):
    """Internal staging copier: preflight everything, exclusive create, no overwrite."""
    validate_manifest(m)
    pending = []
    for e in m['payload']:
        source = safe_path(sources[e['origin']], e['source_path'])
        if not matches(source, e):
            raise ValueError('source hash mismatch/absent: ' + e['source_path'])
        dest = safe_path(target, e['path'])
        if dest.exists():
            if not matches(dest,e):
                raise ValueError('staging conflict: ' + e['path'])
        else:
            pending.append((source,dest,e))
    ancestor = target
    while not ancestor.exists():
        ancestor = ancestor.parent
    required = sum(e['bytes'] for _,_,e in pending)
    free = shutil.disk_usage(ancestor).free
    if free < required + 2 * 1024**3:
        raise ValueError('insufficient free space (2 GiB reserve)')
    for source,dest,e in pending:
        dest.parent.mkdir(parents=True, exist_ok=True)
        with source.open('rb') as src, dest.open('xb') as out:
            shutil.copyfileobj(src,out)
        if not matches(dest,e):
            raise ValueError('copy hash mismatch; preserve failed file for diagnosis: ' + str(dest))
    return {'copied': len(pending), 'already_identical': len(m['payload'])-len(pending),
            'payload_bytes': m['payload_bytes'], 'free_bytes_before': free,
            'free_bytes_after': shutil.disk_usage(ancestor).free}


def stage(m, engine=ENGINE):
    # CLI has no destination override. Cannot write to integrator/candidate.
    safe_path(ROOT, 'Tools/IBS25_portability_staging/package')
    return copy_missing(m, {'checkout': ROOT, 'engine_template': engine}, STAGING)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=['inventory','stage','verify','target'])
    parser.add_argument('--engine', type=Path, default=ENGINE)
    parser.add_argument('--target', type=Path)
    args = parser.parse_args()
    if args.mode == 'inventory':
        m = inventory(args.engine)
        validate_manifest(m)
        json_write(MANIFEST,m)
        print(json.dumps({'manifest': str(MANIFEST), 'files': m['payload_count'], 'bytes': m['payload_bytes'], 'missing_after': m['game_missing_after']}))
    else:
        m = json.loads(MANIFEST.read_text(encoding='utf-8'))
        if args.mode == 'stage':
            result = stage(m,args.engine)
            print(json.dumps(result))
        else:
            if args.mode == 'target' and not args.target:
                parser.error('--target required')
            result = compare(m,args.target if args.mode=='target' else STAGING)
            print(json.dumps(result,indent=2))
            if result['conflicts'] or (args.mode=='verify' and result['missing']):
                raise SystemExit(2)


if __name__ == '__main__':
    main()
