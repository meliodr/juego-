"""Lightweight contract tests. These do NOT execute Unreal or validate appearance."""
import copy
import csv
import hashlib
import json
import math
from pathlib import Path
import tempfile
import unittest
import sys

sys.dont_write_bytecode = True
import IBS25_recipe as recipe
import IBS25_metrics as metrics
import IBS25_pbr as pbr

TEST_ROOT = Path(__file__).resolve().parent / "IBS25_test_work"
TEST_ROOT.mkdir(exist_ok=True)


class RecipeTests(unittest.TestCase):
    def test_reproducible_export_and_foreign_file_guard(self):
        with tempfile.TemporaryDirectory(dir=TEST_ROOT) as temp:
            target = Path(temp)/"IBS25_generated"
            first = recipe.export(target)
            snapshot = {p.name:p.read_bytes() for p in target.iterdir()}
            self.assertEqual(first,recipe.export(target))
            self.assertEqual(snapshot,{p.name:p.read_bytes() for p in target.iterdir()})
            (target/"Paving.obj").write_text("manual work")
            with self.assertRaises(FileExistsError):
                recipe.export(target)
            self.assertEqual((target/"Paving.obj").read_text(),"manual work")
            self.assertEqual((target/"scene.json").read_bytes(),snapshot["scene.json"])

    def test_scope_guard(self):
        data = recipe.scene()
        data["map"] = "/Game/IB_C00/Lvl_IB_C00"
        with self.assertRaises(ValueError):
            recipe.validate_recipe(data)
        with self.assertRaises(ValueError):
            recipe.export(Path(tempfile.gettempdir())/"NotTheOutput")

    def test_corridor_and_landing_reject_obstructions(self):
        data = recipe.validate_recipe(recipe.scene())
        obstruction = copy.deepcopy(data)
        obstruction["buildings"][0]["center_m"][1] = 0
        with self.assertRaises(ValueError):
            recipe.validate_recipe(obstruction)
        obstruction = copy.deepcopy(data)
        obstruction["trees"][0]["position_m"] = [54,39,0.18]
        with self.assertRaises(ValueError):
            recipe.validate_recipe(obstruction)

    def test_facade_is_closed_on_four_sides_with_real_entries(self):
        data = recipe.scene()
        interior_count = 0
        for building in data["buildings"]:
            panels = list(recipe.facade_panels(building))
            self.assertEqual({p["yaw"] for p in panels},{0,90,180,-90})
            self.assertEqual(sum(p["module"] == "Entry" for p in panels),4)
            self.assertEqual(len({p["id"] for p in panels}),len(panels))
            for panel in panels:
                self.assertGreater(panel["width_m"],0)
                self.assertGreater(panel["height_m"],0)
                self.assertTrue(all(math.isfinite(v) for v in panel["center_m"]))
            interior_count += sum(p["interior"] for p in panels)
        self.assertLessEqual(interior_count,72)
        # The facade camera is actually ten metres from the north facade.
        camera = recipe.VIEWS[2]["position_m"]
        building = data["buildings"][0]
        self.assertEqual(camera[1]-(building["center_m"][1]+building["depth_m"]/2),10)

    def test_bevel_winding_outwards_and_nondegenerate(self):
        mesh = recipe.Mesh()
        mesh.slab(0,0,4,3,0.18)
        centre = (0,0,0.09)
        for face in mesh.faces:
            a,b,c = [mesh.vertices[i-1] for i in face]
            u,v = [b[i]-a[i] for i in range(3)],[c[i]-a[i] for i in range(3)]
            n = (u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0])
            middle = [(a[i]+b[i]+c[i])/3-centre[i] for i in range(3)]
            self.assertGreater(sum(n[i]*middle[i] for i in range(3)),0)

    def test_uv_world_scale_and_obj_units(self):
        mesh = recipe.Mesh()
        mesh.surface(0,0,3,3,0,3)
        self.assertEqual(mesh.uvs,[(0,0),(1,0),(1,1),(0,1)])
        self.assertIn("v 300.000000 300.000000 0.000000",mesh.obj())
        self.assertEqual(len(mesh.faces),2)


class MetricsTests(unittest.TestCase):
    def capture(self,rows):
        temp = tempfile.TemporaryDirectory(dir=TEST_ROOT)
        self.addCleanup(temp.cleanup)
        path = Path(temp.name)/"synthetic.csv"
        with path.open("w",newline="") as stream:
            writer = csv.writer(stream)
            writer.writerow(["FrameTime","GPUTime"])
            writer.writerows(rows)
        return path

    def test_known_quantiles_reject_metadata_and_nonfinite(self):
        path = self.capture([(n,n/2) for n in range(1,101)]+[("[metadata]",""),("nan",1),(-1,1)])
        result = metrics.summarize(path)
        self.assertEqual(result["timing"]["FrameTime"]["p95_ms"],95)
        self.assertEqual(result["timing"]["FrameTime"]["p99_ms"],99)
        self.assertEqual(result["timing"]["FrameTime"]["mean_ms"],50.5)
        self.assertEqual(result["rejected_rows"],3)
        self.assertIn("NOT_INFERABLE",result["visual_audio_dx12"])

    def test_warmup_uses_elapsed_time_and_preserves_post_warmup(self):
        result = metrics.summarize(self.capture([(100,50)]*10+[(10,5)]*10),1)
        self.assertEqual(result["trimmed_frames"],10)
        self.assertEqual(result["timing"]["FrameTime"]["mean_ms"],10)
        result = metrics.summarize(self.capture([(100,50)]*10+[(10,5)]*10),0.999)
        self.assertEqual(result["trimmed_frames"],9)
        self.assertEqual(result["timing"]["FrameTime"]["samples"],11)

    def test_missing_samples_are_error_not_pass(self):
        with self.assertRaises(ValueError):
            metrics.summarize(self.capture([("nan",1),(0,2)]))
        with self.assertRaises(ValueError):
            metrics.summarize(self.capture([(10,5)]),2)


class SourceTests(unittest.TestCase):
    def test_six_explicit_cc0_2k_sources(self):
        sources = list(pbr.sources())
        self.assertEqual(len(sources),6)
        self.assertEqual(len({s["url"] for s in sources}),6)
        self.assertTrue(all(s["url"].startswith("https://dl.polyhaven.org/") for s in sources))
        with self.assertRaises(ValueError):
            pbr.dimensions(b"<html>not an image</html>")

    @unittest.skipUnless((pbr.ROOT/"manifest.json").exists(),"Downloaded CC0 bytes not available")
    def test_downloaded_sources_match_receipt_and_resolution(self):
        rows = json.loads((pbr.ROOT/"manifest.json").read_text())
        self.assertEqual(len(rows),6)
        for row in rows:
            data = (pbr.ROOT/row["filename"]).read_bytes()
            self.assertEqual(len(data),row["bytes"])
            self.assertEqual(hashlib.sha256(data).hexdigest(),row["sha256"])
            self.assertEqual(pbr.dimensions(data),(2048,2048))


if __name__ == "__main__":
    unittest.main(verbosity=2)
