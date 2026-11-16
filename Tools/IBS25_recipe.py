"""Deterministic, editor-independent scene recipe. Coordinates are metres, Z up.

No downloads, editor starts or writes outside the requested S2.5 output directory.
The generated OBJ surfaces are architectural paving, not primitive scenery.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path

BASE = "1d01db54b63250ed0aacb0704a5ac16290324e32"
MAP = "/Game/IB_S25/Lvl_IB_S25_RealismLab"
OWNER = "IBS25_RealismLab_v1"
FACADE = "/Game/IB_Donors/UrbanFacades/ImpactApartments/StaticMeshes/"
DEFAULT_ASSETS = {
    **{name: FACADE + name for name in ("BayWide", "BayDouble", "Entry", "Cornice")},
    "car": "/Game/Car/SM_AutomotiveTP_Car",
    "interior": "/Game/IB_C00/Interiors/M_MVP_WindowInteriors",
    # Explicit dependencies, never replaced by low-poly or basic shapes.
    "tree": None,
    "asphalt_basecolor": None, "asphalt_normal_dx": None, "asphalt_roughness": None,
    "paving_basecolor": None, "paving_normal_dx": None, "paving_roughness": None,
}
VIEWS = [
    {"id": "01_avenue", "position_m": [-92, -2, 1.7], "rotation_deg": [2, 0, 0], "fov": 65},
    {"id": "02_corner_cars", "position_m": [-13, -13, 1.7], "rotation_deg": [3, -135, 0], "fov": 60},
    {"id": "03_facade_10m", "position_m": [-68, -13, 6.5], "rotation_deg": [0, -90, 0], "fov": 55},
    {"id": "04_rooftop", "position_m": [-68, -39, 34], "rotation_deg": [-20, 42, 0], "fov": 70},
    {"id": "05_park", "position_m": [157, -52, 1.7], "rotation_deg": [5, 135, 0], "fov": 65},
]


def scene():
    buildings = []
    for block, (cx, cy) in enumerate(((-54, -39), (54, -39), (-54, 39))):
        for side in (-1, 1):
            buildings.append({"id": f"B{block}_{side:+d}", "center_m": [cx + side * 14, cy],
                              "width_m": 24, "depth_m": 32, "stories": 5 + (block + side) % 4,
                              "floor_m": 3.4, "base_m": 0.18})
    cars = [{"id": f"Car_{i:02d}", "position_m": [x, y, 0.04], "yaw": yaw}
            for i, (x, y, yaw) in enumerate(((-74, -7, 0), (-44, -7, 0), (43, 7, 180),
                                             (76, 7, 180), (-7, 44, 90), (7, -45, -90)))]
    trees = [{"id": f"Tree_{i:02d}", "position_m": [x, y, 0.18], "yaw": (i * 137.5) % 360,
              "height_m": 7.0 + (i % 3) * 0.8}
             for i, (x, y) in enumerate(([(x, y) for x in (120, 144, 160) for y in (-40, -16, 16, 40)]
                                        + [(30, 57), (78, 57), (30, 21), (78, 21)]))]
    return {"schema": 1, "owner": OWNER, "base_commit": BASE, "map": MAP,
            "units": "metres", "seed": 25015, "buildings": buildings, "cars": cars, "trees": trees,
            "blocks": [[x, y] for x in (-54, 54) for y in (-39, 39)],
            "landing_plaza": {"center_m": [54, 39], "clear_size_m": [28, 24]},
            "flight_corridor": {"y_min_m": -5, "y_max_m": 5, "z_min_m": 3},
            "sun": {"pitch": -32, "yaw": -35, "lux": 65000},
            "exposure_ev100": 14, "views": VIEWS,
            "streaming": {"mode": "compact_non_WP_lab", "partition_cell_m": 128,
                          "pcg_used": False, "streaming_validated": False}}


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode()).hexdigest()


def validate_recipe(data):
    if data["map"] != MAP or data["owner"] != OWNER or data["base_commit"] != BASE:
        raise ValueError("Recipe scope/base changed; explicit revision required")
    ids = [x["id"] for key in ("buildings", "cars", "trees", "views") for x in data[key]]
    if len(ids) != len(set(ids)):
        raise ValueError("Duplicate stable actor ID")
    for b in data["buildings"]:
        x, y = b["center_m"]
        if abs(y) - b["depth_m"] / 2 < 10 or abs(x) - b["width_m"] / 2 < 10:
            raise ValueError("Building obstructs street/corridor")
    cx, cy = data["landing_plaza"]["center_m"]
    w, h = data["landing_plaza"]["clear_size_m"]
    for tree in data["trees"]:
        x, y, _ = tree["position_m"]
        if abs(x - cx) < w / 2 + 4 and abs(y - cy) < h / 2 + 4:
            raise ValueError("Tree crown intrudes landing clearance")
    return data


def facade_panels(building):
    """Panel centres + tangent yaw. Bounds fitting is performed by the UE adapter."""
    cx, cy = building["center_m"]
    w, d = building["width_m"], building["depth_m"]
    for face, (x, y, tx, ty, width) in enumerate(((cx, cy-d/2, 1, 0, w),
                                                (cx+w/2, cy, 0, 1, d),
                                                (cx, cy+d/2, -1, 0, w),
                                                (cx-w/2, cy, 0, -1, d))):
        count = round(width / 4)
        for bay in range(count):
            offset = (bay + 0.5 - count/2) * width/count
            for floor in range(building["stories"]):
                module = "Entry" if floor == 0 and bay == count//2 else ("BayWide", "BayDouble")[(bay//2 + face) % 2]
                yield {"id": f"{building['id']}_F{face}_B{bay}_L{floor}", "module": module,
                       "center_m": [x + tx*offset, y + ty*offset,
                                    building["base_m"] + (floor+0.5)*building["floor_m"]],
                       "width_m": width/count, "height_m": building["floor_m"],
                       "yaw": math.degrees(math.atan2(ty, tx)),
                       "interior": module != "Entry" and face == 2 and floor < 2}
            yield {"id": f"{building['id']}_F{face}_B{bay}_Cornice", "module": "Cornice",
                   "center_m": [x + tx*offset, y + ty*offset,
                                building["base_m"] + building["stories"]*building["floor_m"] - 0.12],
                   "width_m": width/count, "height_m": 0.24,
                   "yaw": math.degrees(math.atan2(ty, tx)), "interior": False}


class Mesh:
    """Triangulated Z-up OBJ in centimetres, UVs in real world tile units."""
    def __init__(self):
        self.vertices, self.uvs, self.faces = [], [], []

    def quad(self, points, tile_m=1.0):
        start = len(self.vertices) + 1
        self.vertices.extend(points)
        # Planar UVs for horizontal surfaces; side faces use their edge length and height.
        if max(p[2] for p in points) - min(p[2] for p in points) < 1e-8:
            self.uvs.extend([(p[0]/tile_m, p[1]/tile_m) for p in points])
        else:
            length = math.dist(points[0], points[1])
            height = max(p[2] for p in points) - min(p[2] for p in points)
            self.uvs.extend([(0, 0), (length/tile_m, 0), (length/tile_m, height/tile_m), (0, height/tile_m)])
        self.faces.extend([(start, start+1, start+2), (start, start+2, start+3)])

    def surface(self, x0, y0, x1, y1, z, tile_m):
        self.quad([(x0,y0,z),(x1,y0,z),(x1,y1,z),(x0,y1,z)], tile_m)

    def slab(self, cx, cy, width, depth, z, thickness=0.18, bevel=0.04, tile_m=1.8):
        x, y = cx-width/2, cy-depth/2
        lower = [(x,y,z-thickness),(x+width,y,z-thickness),(x+width,y+depth,z-thickness),(x,y+depth,z-thickness)]
        edge = [(px,py,z-bevel) for px,py,_ in lower]
        top = [(x+bevel,y+bevel,z),(x+width-bevel,y+bevel,z),
               (x+width-bevel,y+depth-bevel,z),(x+bevel,y+depth-bevel,z)]
        self.quad(top, tile_m)
        for i in range(4):
            j = (i+1) % 4
            self.quad([lower[i], lower[j], edge[j], edge[i]], tile_m)
            self.quad([edge[i], edge[j], top[j], top[i]], tile_m)
        self.quad(list(reversed(lower)), tile_m)

    def obj(self):
        lines = ["# IBS25 generated centimetres Z-up; UVs preserve source physical scale"]
        lines += ["v " + " ".join(f"{v*100:.6f}" for v in p) for p in self.vertices]
        lines += [f"vt {u:.6f} {v:.6f}" for u,v in self.uvs]
        lines += ["f " + " ".join(f"{i}/{i}" for i in face) for face in self.faces]
        return "\n".join(lines) + "\n"


def geometry(data):
    meshes = {name: Mesh() for name in ("Asphalt", "Paving", "Markings", "Roof")}
    meshes["Asphalt"].surface(-104,-74,104,74,0,3.0)
    for x,y in data["blocks"]:
        meshes["Paving"].slab(x,y,80,50,0.18)
    # Park transition: a hardscape promenade. Real trees are a required dependency.
    meshes["Paving"].slab(136,0,56,128,0.18)
    for x in range(-96,100,8):
        if abs(x) > 12:
            meshes["Markings"].surface(x,-0.06,x+3,0.06,0.012,1)
    for x in (-12,12):
        for y in (-10,10):
            for offset in range(-4,5,2):
                meshes["Markings"].surface(x-1.5,y+offset-0.4,x+1.5,y+offset+0.4,0.014,1)
    for b in data["buildings"]:
        meshes["Roof"].slab(*b["center_m"], b["width_m"], b["depth_m"],
                            b["base_m"] + b["stories"]*b["floor_m"], 0.16)
    return meshes


def write_once_or_identical(path, content):
    if path.exists() and path.read_text(encoding="utf-8") != content:
        raise FileExistsError(f"Refusing to replace a different generated file: {path}")
    if not path.exists():
        path.write_text(content, encoding="utf-8", newline="\n")


def export(output):
    output = Path(output).resolve()
    if output.name != "IBS25_generated":
        raise ValueError("Output directory must be named IBS25_generated")
    data = validate_recipe(scene())
    payloads = {f"{name}.obj": mesh.obj() for name,mesh in geometry(data).items()}
    payloads["scene.json"] = json.dumps(data, indent=2) + "\n"
    # Validate every existing file before creating anything: no partial overwrite.
    for name, content in payloads.items():
        path = output / name
        if path.exists() and path.read_text(encoding="utf-8") != content:
            raise FileExistsError(path)
    output.mkdir(parents=True, exist_ok=True)
    for name, content in payloads.items():
        write_once_or_identical(output / name, content)
    return {"recipe_sha256": digest(data), "files": len(payloads),
            "panels": sum(len(list(facade_panels(b))) for b in data["buildings"]),
            "bytes": sum(len(c.encode()) for c in payloads.values()), "status": "RECIPE_ONLY_NOT_RENDERED"}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(export(args.output), indent=2))
