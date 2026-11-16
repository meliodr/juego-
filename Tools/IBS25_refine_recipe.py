"""R2: one existing block and park, fixed cameras, authored architectural detail.
Metres Z-up; OBJ imports need the same Y reflection correction as the base lab.
No expansion of the building footprint. Generated meshes are versioned, never replaced.
"""
import json
import math
from pathlib import Path
import IBS25_recipe as base

REV='R2'
def scene():
    data=base.scene()
    data['buildings']=[b for b in data['buildings'] if not b['id'].startswith('B0_')]
    for ident,x,w,floors,style in [('R2_A',-78,24,6,'Stone'),('R2_B',-54,20,8,'Blue'),('R2_C',-30,24,5,'Clay')]:
        data['buildings'].append(dict(id=ident,center_m=[x,-39],width_m=w,depth_m=32,
                                      stories=floors,floor_m=3.4,base_m=.18,style=style))
    data['views']=[dict(v) for v in base.VIEWS]
    data['views'][1].update(position_m=[-18,1,1.7],rotation_deg=[3,-138,0],fov=65)
    data['revision']=REV
    return base.validate_recipe(data)

def lathe(mesh,cx,cy,profile,segments=16):
    # Profiled cast-metal furniture: tapered necks, collars, rounded caps.
    for (z0,r0),(z1,r1) in zip(profile,profile[1:]):
        for i in range(segments):
            t0,t1=i*math.tau/segments,(i+1)*math.tau/segments
            mesh.quad([(cx+r0*math.cos(t0),cy+r0*math.sin(t0),z0),
                       (cx+r0*math.cos(t1),cy+r0*math.sin(t1),z0),
                       (cx+r1*math.cos(t1),cy+r1*math.sin(t1),z1),
                       (cx+r1*math.cos(t0),cy+r1*math.sin(t0),z1)],.5)

def ring(mesh,x,y,w,d,z,thickness=.12):
    for xx,yy,ww,dd in ((x,y-d/2,w,thickness),(x,y+d/2,w,thickness),
                        (x-w/2,y,thickness,d),(x+w/2,y,thickness,d)):
        mesh.slab(xx,yy,ww,dd,z,.12,.02)

def bench(parts,x,y):
    # Eleven seat/back slats, two feet and shaped back brackets; real gaps, no box proxy.
    for i in range(6):
        parts['Metal'].slab(x,y-.28+i*.11,1.85,.075,.66,.045,.012)
    for i in range(5):
        parts['Metal'].slab(x,y+.31+i*.035,1.85,.06,.79+i*.10,.065,.015)
    for dx in (-.69,.69):
        parts['Metal'].slab(x+dx,y,.09,.56,.58,.38,.02)
        parts['Metal'].slab(x+dx,y,.34,.68,.23,.05,.015)
        parts['Metal'].slab(x+dx,y+.37,.07,.09,1.18,.60,.015)
        parts['Metal'].slab(x+dx,y,.07,.59,.91,.06,.015)

def geometry():
    data=scene()
    parts={n:base.Mesh() for n in ('Paving','Markings','Roof','Ground','Soil','Curb',
                                   'Metal','RoofMetal','StoneDetail','BlueDetail','ClayDetail','GlassSign')}
    # Existing four blocks remain; park loses its full concrete plate.
    for x,y in data['blocks']: parts['Paving'].slab(x,y,80,50,.18)
    # Continuous support ground removes the finite lab's black void seen from flight.
    # No extra buildings/roads; the detailed footprint is unchanged.
    for x in range(-1000,1000,100):
        for y in range(-1000,1000,100):
            parts['Ground'].surface(x,y,x+100,y+100,-.08,1.3)
    parts['Soil'].surface(108,-64,164,64,.13,1.3)
    # Park perimeter, connected cross paths and a north/south promenade.
    for x,y,w,d in [(109,0,2,128),(163,0,2,128),(136,-63,52,2),(136,63,52,2),
                     (132,0,4,124),(148,0,2.8,124),(136,-51,52,3),(136,0,52,4),(136,51,52,3)]:
        parts['Paving'].slab(x,y,w,d,.18,.12,.025)
    # Segmented stone curbs follow all blocks, leaving central accessible crossings.
    for cx,cy in data['blocks']:
        for x in range(int(cx-39),int(cx+40),2):
            for y in (cy-25,cy+25):
                if abs(x)<20: continue
                parts['Curb'].slab(x,y,1.97,.28,.23,.23,.045)
        for y in range(int(cy-24),int(cy+25),2):
            for x in (cx-40,cx+40):
                if abs(y)<20: continue
                parts['Curb'].slab(x,y,.28,1.97,.23,.23,.045)
    # Four complete zebra crossings across the 28m carriageways, stop lines behind them.
    for side in (-1,1):
        for t in range(-13,14,2):
            parts['Markings'].surface(t-.45,side*18-1.8,t+.45,side*18+1.8,.017,1)
            parts['Markings'].surface(side*18-1.8,t-.45,side*18+1.8,t+.45,.017,1)
        parts['Markings'].surface(-13,side*22-.18,-1,side*22+.18,.018,1)
        parts['Markings'].surface(side*22-.18,1,side*22+.18,13,.018,1)
    for x in range(-96,100,8):
        if abs(x)>26: parts['Markings'].surface(x,-.06,x+3,.06,.012,1)
    for y in range(-70,72,8):
        if abs(y)>26: parts['Markings'].surface(-.06,y,.06,y+3,.012,1)
    # Ramps at each crossing: connect asphalt to existing walk level without a curb barrier.
    for sx in (-1,1):
        for sy in (-1,1):
            x0,x1=sorted((sx*14,sx*16)); y0,y1=sy*16,sy*20
            parts['Curb'].quad([(x0,y0,.02 if sx==1 else .18),(x1,y0,.18 if sx==1 else .02),
                                (x1,y1,.18 if sx==1 else .02),(x0,y1,.02 if sx==1 else .18)])
    # Profiled bollards and luminaires, outside the clear aerial corridor.
    for x in (-21,21):
        for y in (-21,21):
            lathe(parts['Metal'],x,y,[(.18,.14),(.23,.14),(.25,.08),(.93,.065),(.98,.08),(1.01,0)])
    for x,y in [(-88,-17),(-46,-17),(-18,-26),(18,26),(113,-50),(135,-25),(150,25)]:
        lathe(parts['Metal'],x,y,[(.18,.23),(.27,.23),(.36,.10),(3.7,.065),(3.78,.13),(3.86,.13)])
        parts['Metal'].slab(x,y,.7,.45,3.92,.08,.03)
        parts['GlassSign'].slab(x,y,.57,.32,3.84,.02,.008)
    for x,y in [(-81,-18.5),(-59,-18.5),(-36,-18.5),(128,-48),(128,3),(151,-48),(151,4),(128,48)]:
        bench(parts,x,y)
    # Circular soil tree pits with a bevelled stone collar and slotted metal grating.
    for tree in data['trees']:
        x,y,_=tree['position_m']
        if x<108: continue
        ring(parts['Curb'],x,y,3,3,.24,.22)
        for off in (-1.25,-1,-.75,.75,1,1.25):
            parts['Metal'].slab(x+off,y,.045,2.7,.21,.035,.008)
    for b in data['buildings']:
        x,y=b['center_m'];w,d=b['width_m'],b['depth_m'];h=b['base_m']+b['stories']*b['floor_m']
        parts['Roof'].slab(x,y,w,d,h,.16)
        if not b['id'].startswith('R2_'): continue
        detail=parts[b['style']+'Detail']
        # Full parapet and projecting cap, not a floating roof plate.
        ring(detail,x,y,w-.3,d-.3,h+.95,.24)
        for xx,yy,ww,dd in [(x,y-d/2+.18,w,.28),(x,y+d/2-.18,w,.28),
                           (x-w/2+.18,y,.28,d),(x+w/2-.18,y,.28,d)]:
            detail.slab(xx,yy,ww,dd,h+.85,.85,.025)
        # All ground floors have a canopy, raised signage panel and actual entry modules.
        detail.slab(x,y+d/2+.48,w-1,1.35,3.45,.18,.055)
        for xx in (x-w/2+.25,x+w/2-.25):
            detail.slab(xx,y+d/2+.24,.32,.6,3.33,3.15,.03)
        parts['GlassSign'].slab(x,y+d/2+.37,w*.46,.09,3.12,.39,.025)
        if b['style']=='Stone':
            for floor in range(1,b['stories']):
                detail.slab(x,y+d/2+.22,w,.48,b['base_m']+floor*3.4+.12,.22,.04)
        elif b['style']=='Blue':
            for xx in range(int(x-w/2),int(x+w/2)+1,4):
                detail.slab(xx,y+d/2+.27,.20,.72,h, h-3.6,.025)
        else:
            for floor in range(1,b['stories']):
                z=b['base_m']+floor*3.4
                for xx in (x-6,x+6):
                    detail.slab(xx,y+d/2+.65,5.2,1.45,z,.15,.035)
                    parts['Metal'].slab(xx,y+d/2+1.31,5.15,.06,z+1.05,.055,.012)
                    for post in range(-5,6):
                        parts['Metal'].slab(xx+post*.48,y+d/2+1.31,.035,.035,z+1.05,1.04,.008)
        # Rooftop two packaged HVAC units: chamfered casing, louvres, feet and fan guards.
        for xx in (x-4,x+4):
            for dx in (-.8,.8): parts['RoofMetal'].slab(xx+dx,y,.18,2.4,h+.22,.22,.025)
            parts['RoofMetal'].slab(xx,y,2.2,2.8,h+1.25,1.04,.09)
            for j in range(9):
                parts['Metal'].slab(xx,y+1.42,1.85,.035,h+.35+j*.09,.035,.008)
            for yy in (y-.72,y+.72):
                lathe(parts['Metal'],xx,yy,[(h+1.25,.52),(h+1.30,.52),(h+1.34,.48)])
                for off in range(-4,5):
                    chord=math.sqrt(max(0,.48**2-(off*.1)**2))*2
                    parts['Metal'].slab(xx,yy+off*.1,chord,.025,h+1.35,.02,.004)
            parts['RoofMetal'].slab(xx,y+3.0,.7,3.1,h+.6,.48,.08)
    return {'R2_'+name:mesh for name,mesh in parts.items()}

def export():
    out=Path(__file__).with_name('IBS25_refined_generated')
    payload={name+'.obj':m.obj() for name,m in geometry().items()}
    payload['scene.json']=json.dumps(scene(),indent=2)+'\n'
    for name,content in payload.items():
        if (out/name).exists() and (out/name).read_text()!=content: raise FileExistsError(out/name)
    out.mkdir(exist_ok=True)
    for name,content in payload.items(): base.write_once_or_identical(out/name,content)
    return dict(files=len(payload),bytes=sum(len(v) for v in payload.values()),sha256=base.digest(scene()))

if __name__=='__main__': print(json.dumps(export(),indent=2))
