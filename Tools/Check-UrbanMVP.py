"""Verify the saved urban MVP and its original IB-C00 flight circuit in Unreal Editor."""

import math
from pathlib import Path
import runpy

import unreal as ue


circuit = runpy.run_path(str(Path(__file__).with_name('Check-IB-C00.py')))
actors = circuit['actors'].get_all_level_actors()
by_label = circuit['by_label']
bounds = circuit['bounds']
trace = circuit['trace']

bounds('Distrito - avenida este', ((220, 612), (-13, 13), (0, 0.03)))
bounds('Distrito - avenida transversal', ((388, 418), (-135, 255), (0, 0.03)))
bounds('Distrito - plaza elevada', ((458, 562), (75, 171), (0, 0.8)))
for step in range(1, 5):
    bounds(f'MVP - Impacto - Acceso plaza {step}',
           ((453 + step, 454 + step), (127.1, 132.9), (0, 0.2 * step)))

target_layout = {
    'MVP - Impacto - Bidon este': (522.6, 130, 1.35),
    'MVP - Impacto - Bidon oeste': (517.4, 130, 1.35),
    'MVP - Impacto - Caja sur': (520, 126.5, 1.26),
    'MVP - Impacto - Caja norte': (522.4, 133.5, 1.29),
}
panel_label = 'MVP - Impacto - Panel ligero'

for tag, expected in (('ImpactLandingTarget', sorted(target_layout)), ('ImpactLandingPanel', [panel_label])):
    observed = sorted(actor.get_actor_label() for actor in actors if actor.actor_has_tag(ue.Name(tag)))
    assert observed == expected, (tag, observed, expected)

for label, center in target_layout.items():
    actor = by_label[label]
    position = actor.get_actor_location()
    assert all(math.isclose(getattr(position, axis), offset * 100, abs_tol=0.5)
               for axis, offset in zip('xyz', center)), (label, position, center)
    component = actor.static_mesh_component
    assert component.get_editor_property('mobility') == ue.ComponentMobility.MOVABLE, label
    assert component.get_editor_property('body_instance').get_editor_property('simulate_physics'), label
    assert component.get_collision_response_to_channel(ue.CollisionChannel.ECC_VISIBILITY) == ue.CollisionResponseType.ECR_BLOCK, label

bounds(panel_label, ((517.5, 522.5), (136.16, 136.44), (0.8, 3.3)))
assert by_label[panel_label].static_mesh_component.get_collision_response_to_channel(
    ue.CollisionChannel.ECC_VISIBILITY) == ue.CollisionResponseType.ECR_BLOCK, panel_label

decoration_groups = {}
for label, actor in by_label.items():
    if label.startswith('MVP - Detalle '):
        component = actor.get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
        assert component is not None, label
        count = component.get_instance_count()
        assert count > 0, label
        assert component.get_collision_enabled() == ue.CollisionEnabled.NO_COLLISION, label
        decoration_groups[label] = count

road_label = 'MVP - Detalle Cube road 0m 0'
road_mesh = by_label[road_label].get_component_by_class(ue.HierarchicalInstancedStaticMeshComponent)
road_layout = set()
for index in range(road_mesh.get_instance_count()):
    transform = road_mesh.get_instance_transform(index, True)
    center = transform.translation
    scale = transform.scale3d
    road_layout.add((round(center.x / 100, 1), round(center.y / 100, 1), round(scale.x), round(scale.y)))
expected_roads = {(319, 0, 602, 20), (180, 64.5, 20, 109), (403, 54, 26, 366), (289.5, 52, 11, 75)}
assert expected_roads <= road_layout, expected_roads - road_layout

# The approach stops before the steps. The open plaza center is the landing target.
trace((403, 130, 2), (452, 130, 2))
trace((520, 130, 36), (520, 130, 2))
trace((520, 130, 2), (520, 130, 1.5), 'Distrito - plaza elevada')
trace((519, 134.8, 1.8), (519, 138, 1.8), panel_label)

ue.log(f'IB URBAN MVP CHECK PASS: {len(target_layout)} physics targets, 1 independent panel, 4 access steps, '
       f'{sum(decoration_groups.values())} decoration instances in {len(decoration_groups)} groups; '
       'original circuit, street widths, plaza bounds and capsule sweeps verified')
