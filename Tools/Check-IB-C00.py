import math
import unreal as ue

levels = ue.get_editor_subsystem(ue.LevelEditorSubsystem)
actors = ue.get_editor_subsystem(ue.EditorActorSubsystem)
world = ue.get_editor_subsystem(ue.UnrealEditorSubsystem)
assert levels.load_level("/Game/IB_C00/Lvl_IB_C00")
by_label = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}

def bounds(label, expected):
    center, extent = by_label[label].get_actor_bounds(False)
    observed = tuple((getattr(center,k)-getattr(extent,k),getattr(center,k)+getattr(extent,k)) for k in "xyz")
    for axis, pair in zip(observed, expected):
        assert all(math.isclose(a,b*100,abs_tol=0.5) for a,b in zip(axis,pair)), (label, observed)

bounds("Suelo 240 x 195 m", ((-20,220),(-30,165),(-0.5,0)))
bounds("Carro de ensayo estatico", ((179,181),(59.75,64.25),(0,1.5)))
bounds("Pared panel central", ((177,183),(119.7,120.3),(0,5)))
bounds("Pared dintel", ((168,192),(119.7,120.3),(5,16)))
assert [round(by_label[label].get_actor_location().x/100) for label in ("Inicio recta","Inicio freno","Centro esquina")] == [20,130,180]

def trace(start,end,expected=None,radius=0.42,half_height=0.96):
    hit = ue.SystemLibrary.capsule_trace_single(
        world.get_editor_world(), ue.Vector(*[n*100 for n in start]),
        ue.Vector(*[n*100 for n in end]), radius*100, half_height*100,
        ue.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], ue.DrawDebugTrace.NONE, False
    )
    assert (hit is not None) == bool(expected), (start, end, expected, hit)

trace((10,0,1),(10,0,30))
for y in (-9.5,0,9.5):
    trace((10,y,1),(180,y,1))
for x in (170.5,180,189.5):
    trace((x,0,3),(x,118.5,3))
trace((180,55,1),(180,66,1),"Carro de ensayo estatico")
trace((180,110,1),(180,122,1),"Pared panel central")
trace((130,0,2),(130,15,2),"Edificio avenida Norte 2")
trace((100,18,30),(100,18,23),"Edificio avenida Norte 2")
for x in (172.5,180,187.5):
    for z in (1,6,11):
        trace((x,121,z),(x,155,z))
for x,y in ((10,0),(130,0),(180,0),(180,40),(180,145)):
    trace((x,y,1.5),(x,y,0.5),"Suelo 240 x 195 m")
trace((10,0,2),(610,0,2))
trace((403,0,2),(403,160,2))
trace((403,0,33),(403,75,33),"Distrito - puente sobre la transversal")
trace((582,190,90),(582,190,76),"Distrito - azotea 12")
ue.log("IB-C00 BOUNDS AND CAPSULE SWEEPS PASS")
