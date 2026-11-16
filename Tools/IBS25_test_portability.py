"""Lightweight portability checks. No UE, GPU, network or candidate writes."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
import sys

sys.dont_write_bytecode = True
from IBS25_package_refs import references
import IBS25_portability as p


class PortabilityTests(unittest.TestCase):
    def setUp(self):
        scratch = p.ROOT / 'Tools/IBS25_portability_staging/tests'
        scratch.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix='IBS25-portability-', dir=scratch)
        if not Path(self.temp.name).resolve().is_relative_to(scratch.resolve()):
            raise ValueError('test cleanup outside scratch directory')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.src, self.dst = self.root/'source', self.root/'stage'
        self.src.mkdir()
        self.dst.mkdir()
        path = p.CONTENT + 'IB_S25/Materials/Test.uasset'
        f = self.src / path
        f.parent.mkdir(parents=True)
        f.write_bytes(b'fixture content')
        self.entry = {'path': path, 'source_path': path, 'origin': 'checkout',
                      'bytes': f.stat().st_size, 'sha256': p.digest(f)}
        self.m = {'schema':1, 'map':p.LAB, 'game_missing_after':{},
                  'payload':[self.entry], 'payload_count':1, 'payload_bytes':f.stat().st_size}

    def test_copy_idempotent_and_verified(self):
        first = p.copy_missing(self.m, {'checkout':self.src},self.dst)
        f = self.dst/self.entry['path']
        stamp = f.stat().st_mtime_ns
        second = p.copy_missing(self.m, {'checkout':self.src},self.dst)
        self.assertEqual((first['copied'],second['copied']), (1,0))
        self.assertEqual(stamp,f.stat().st_mtime_ns)
        self.assertTrue(p.matches(f,self.entry))

    def test_changed_source_blocks_all_copy(self):
        (self.src/self.entry['path']).write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError,'source hash'):
            p.copy_missing(self.m, {'checkout':self.src},self.dst)
        self.assertEqual(list(self.dst.rglob('*')),[])

    def test_conflict_preserved(self):
        f=self.dst/self.entry['path']; f.parent.mkdir(parents=True); f.write_bytes(b'other owner')
        with self.assertRaisesRegex(ValueError,'conflict'):
            p.copy_missing(self.m, {'checkout':self.src},self.dst)
        self.assertEqual(f.read_bytes(),b'other owner')

    def test_target_is_read_only(self):
        before=list(self.dst.rglob('*'))
        result=p.compare(self.m,self.dst)
        self.assertEqual(result['missing'],[self.entry['path']])
        self.assertEqual(list(self.dst.rglob('*')),before)

    def test_path_escape_and_main_map_rejected(self):
        for path in ('../other','C:/other','/absolute','a\\b','a/../../b','a:stream','a/./b'):
            with self.subTest(path=path), self.assertRaises(ValueError): p.safe_path(self.dst,path)
        with self.assertRaises(ValueError): p.allowed_payload(p.CONTENT+'IB_C00/Lvl_IB_C00.umap')

    def test_duplicate_case_insensitive_destination_rejected(self):
        e=copy.deepcopy(self.entry); e['path']=e['path'].replace('Test','test'); e['source_path']=e['path']
        self.m['payload'].append(e)
        with self.assertRaisesRegex(ValueError,'duplicate'): p.validate_manifest(self.m)

    def test_external_source_mapping_rejected(self):
        self.entry['origin']='engine_template'; self.entry['source_path']='Engine/Config/Base.ini'
        with self.assertRaisesRegex(ValueError,'template mapping'): p.validate_manifest(self.m)

    def test_unreal_reader_rejects_pointer_and_truncation(self):
        with self.assertRaises(ValueError): references(b'version https://git-lfs.github.com/spec/v1')
        with self.assertRaises(ValueError): references(b'\xc1\x83\x2a\x9e')

    def test_missing_closure_report_and_cycles(self):
        graph={p.LAB:{'imports':['/Game/missing','/Script/Engine'], 'soft_packages':[p.LAB]}}
        reached,missing,native=p.closure(graph)
        self.assertEqual(reached,{p.LAB})
        self.assertEqual(missing,{'/Game/missing':[p.LAB]})
        self.assertEqual(native,['/Script/Engine'])

    def test_real_map_numbered_names_and_no_main_map_dependency(self):
        a=references((p.ROOT/(p.CONTENT+'IB_S25/Lvl_IB_S25_RealismLab.umap')).read_bytes())
        for i in range(3): self.assertIn('/Game/IB_S25/Materials/MI_CarPaint_'+str(i),a['imports'])
        self.assertNotIn('/Game/IB_C00/Lvl_IB_C00',a['imports']+a['soft_packages'])

    def test_real_manifest_closure_and_class_contract(self):
        m=json.loads(p.MANIFEST.read_text(encoding='utf-8'))
        p.validate_manifest(m)
        reached,missing,_=p.closure(m['package_graph'])
        self.assertFalse(missing)
        self.assertEqual(set(e['package'] for e in m['payload']),reached)
        self.assertTrue(m['game_missing_before'])
        self.assertEqual(len(m['template_provenance']),8)
        self.assertTrue(all(e['matches_checkout'] for e in m['template_provenance']))
        for e in m['native_class_contract']['files']:
            self.assertEqual(p.digest(p.ROOT/e['path']),e['sha256'])


if __name__=='__main__':
    unittest.main(verbosity=2)
