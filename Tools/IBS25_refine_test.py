"""Light regression checks for the requested scope and immutable capture setup."""
import unittest
import IBS25_recipe as base
import IBS25_refine_recipe as r

class RefinementTests(unittest.TestCase):
    def test_background_blocks_unchanged(self):
        old=[b for b in base.scene()['buildings'] if not b['id'].startswith('B0_')]
        new=[b for b in r.scene()['buildings'] if not b['id'].startswith('R2_')]
        self.assertEqual(old,new)
        self.assertEqual(base.scene()['blocks'],r.scene()['blocks'])
    def test_three_facades_inside_existing_block(self):
        buildings=[b for b in r.scene()['buildings'] if b['id'].startswith('R2_')]
        self.assertEqual(len(buildings),3)
        self.assertEqual(len({b['style'] for b in buildings}),3)
        for b in buildings:
            self.assertGreaterEqual(b['center_m'][0]-b['width_m']/2,-94)
            self.assertLessEqual(b['center_m'][0]+b['width_m']/2,-14)
            self.assertEqual(b['center_m'][1],-39)
    def test_capture_setup_is_reviewed_baseline(self):
        expected=[dict(v) for v in base.VIEWS]
        expected[1].update(position_m=[-18,1,1.7],rotation_deg=[3,-138,0],fov=65)
        self.assertEqual(expected,r.scene()['views'])
        self.assertEqual(base.scene()['sun'],r.scene()['sun'])
        self.assertEqual(14,r.scene()['exposure_ev100'])
    def test_flight_route_and_landing_stay_clear(self):
        d=r.scene()
        self.assertEqual(base.scene()['landing_plaza'],d['landing_plaza'])
        self.assertEqual(base.scene()['flight_corridor'],d['flight_corridor'])
        base.validate_recipe(d)
        for name in ('R2_Metal','R2_RoofMetal','R2_StoneDetail','R2_BlueDetail','R2_ClayDetail'):
            for x,y,z in r.geometry()[name].vertices:
                self.assertFalse(-5<y<5 and z>=3,name)
                self.assertFalse(40<x<68 and 27<y<51 and z>.3,name)
    def test_geometry_deterministic_and_valid_indices(self):
        first=r.geometry();second=r.geometry()
        self.assertEqual({n:m.obj() for n,m in first.items()},{n:m.obj() for n,m in second.items()})
        for m in first.values():
            self.assertTrue(m.faces)
            self.assertEqual(len(m.vertices),len(m.uvs))
            self.assertTrue(all(1<=i<=len(m.vertices) for face in m.faces for i in face))

if __name__=='__main__': unittest.main()
