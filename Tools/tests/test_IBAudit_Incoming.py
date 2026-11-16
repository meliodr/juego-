import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

spec=importlib.util.spec_from_file_location('incoming',Path(__file__).parents[1]/'IBAudit_Incoming.py')
incoming=importlib.util.module_from_spec(spec);spec.loader.exec_module(incoming)

class IncomingTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.repo=Path(self.temp.name)
        self.git('init','-q');self.git('config','user.name','Fixture');self.git('config','user.email','fixture@example.invalid')
        (self.repo/'README').write_text('fixture');self.git('add','.');self.git('commit','-qm','base')
        self.base=self.git('rev-parse','HEAD')
    def tearDown(self):self.temp.cleanup()
    def git(self,*args):return subprocess.check_output(['git','-C',str(self.repo),*args],stderr=subprocess.PIPE).decode().strip()
    def change(self,path):
        p=self.repo/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('fixture')
        self.git('add','.');self.git('commit','-qm','change');return self.git('rev-parse','HEAD')
    def test_shared_adapter_has_only_s35_author(self):
        head=self.change('ImpactBeyond/Source/ImpactBeyond/IB_Integration/ImpactContract/Adapter.h')
        self.assertEqual(incoming.inspect(self.repo,'S35',self.base,head)['status'],'STATIC_PASS')
        self.assertEqual(incoming.inspect(self.repo,'S4',self.base,head)['reason'],'OUTSIDE_OWNER_SCOPE')
    def test_canonical_character_is_integrator_only(self):
        head=self.change('ImpactBeyond/Source/ImpactBeyond/ImpactBeyondCharacter.cpp')
        for owner in incoming.SCOPES:
            self.assertEqual(incoming.inspect(self.repo,owner,self.base,head)['status'],'BLOCKED')
    def test_no_delivery_and_mutable_ref_fail_closed(self):
        self.assertEqual(incoming.inspect(self.repo,'S6',self.base,None)['reason'],'AWAITING_DELIVERY')
        self.assertEqual(incoming.inspect(self.repo,'S7',self.base,'HEAD')['reason'],'EXACT_COMMIT_REQUIRED')
    def test_unavailable_object_and_empty_commit_fail(self):
        self.assertEqual(incoming.inspect(self.repo,'S25',self.base,'0'*40)['status'],'BLOCKED')
        self.assertEqual(incoming.inspect(self.repo,'S25',self.base,self.base)['reason'],'EMPTY_DELIVERY')
    def test_renaming_shared_file_into_owned_path_still_fails(self):
        p=self.repo/'ImpactBeyond/Source/ImpactBeyond/IB_S7T/README';p.parent.mkdir(parents=True)
        self.git('mv','README',str(p));self.git('commit','-qm','rename')
        r=incoming.inspect(self.repo,'S7',self.base,self.git('rev-parse','HEAD'))
        self.assertIn('README',r['violations'])

if __name__=='__main__':unittest.main()
