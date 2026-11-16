"""Publication regressions using disposable repositories and synthetic bytes."""
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).parents[1] / "IBCI_PublicSource.py"
SPEC = importlib.util.spec_from_file_location("public_source", SCRIPT)
policy = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(policy)


class PublicSourceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.git("init", "-q")
        self.git("config", "user.email", "qa@example.invalid")
        self.git("config", "user.name", "Source QA")
        self.write("README.md", "Public baseline\n")
        self.commit("baseline")
        self.base = self.git("rev-parse", "HEAD").strip()

    def tearDown(self):
        self.temp.cleanup()

    def git(self, *args):
        return subprocess.check_output(["git", "-C", str(self.root), *args], text=True, stderr=subprocess.PIPE)

    def write(self, relative, text):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def commit(self, title):
        self.git("add", "-A")
        self.git("commit", "-qm", title)

    def test_source_and_metadata_are_permitted(self):
        self.write("ImpactBeyond/Source/ImpactBeyond/NewPolicy.h", "#pragma once\n")
        self.write("Tools/recipe.py", "VALUE = 3\n")
        self.write("docs/receipt.json", '{"source":"licensed local input"}\n')
        self.commit("source")
        self.assertEqual(policy.audit(self.root, self.base)["violations"], [])

    def test_deleted_asset_still_exists_in_history(self):
        path = "ImpactBeyond/Content/PrivatePack/model.uasset"
        self.write(path, "SYNTHETIC TEST, NOT AN ASSET")
        self.commit("accidental asset")
        (self.root / path).unlink()
        self.commit("remove from final tree")
        violations = policy.audit(self.root, self.base)["violations"]
        self.assertEqual(len(violations), 1)
        self.assertEqual(violations[0][1], path)

    def test_personal_paths_are_rejected_without_echoing_contents(self):
        private = "C:" + "/Users/" + "example/Documents/local"
        self.write("docs/report.md", "Local folder " + private + "\n")
        self.commit("personal path")
        violations = policy.audit(self.root, self.base)["violations"]
        self.assertEqual(violations[0][1], "docs/report.md:1")
        self.assertNotIn(private, str(violations))

    def test_extension_and_location_both_matter(self):
        self.assertIsNotNone(policy.check_path("Tools/private.fbx"))
        self.assertIsNotNone(policy.check_path("ImpactBeyond/Content/generated.json"))
        self.assertIsNone(policy.check_path("Tools/sources/.gitignore"))


if __name__ == "__main__":
    unittest.main()
