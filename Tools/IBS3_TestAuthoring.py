"""Lightweight geometry QA only. Does not execute or emulate Unreal asset/level APIs."""
import importlib.util
import json
import math
from pathlib import Path
import sys
import types
from collections import Counter

sys.modules["unreal"] = types.ModuleType("unreal")  # Import isolation; no mock runtime assertions.
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("ibs3_author", Path(__file__).with_name("IBS3_AuthorLab.py"))
author = importlib.util.module_from_spec(spec)
spec.loader.exec_module(author)


def inspect(path, closed):
    vertices, faces = [], []
    for line in path.read_text().splitlines():
        p = line.split()
        if p[0] == "v":
            vertices.append(tuple(map(float, p[1:])))
        elif p[0] == "f":
            faces.append(tuple(int(v)-1 for v in p[1:]))
    edges = Counter()
    min_dot, max_radius_error, max_sagitta = math.inf, 0, 0
    for v in vertices:
        max_radius_error = max(max_radius_error, abs(math.sqrt(sum(x*x for x in v))-50))
    for a, b, c in faces:
        v, w, u = vertices[a], vertices[b], vertices[c]
        ab, ac = [w[i]-v[i] for i in range(3)], [u[i]-v[i] for i in range(3)]
        cross = (ab[1]*ac[2]-ab[2]*ac[1], ab[2]*ac[0]-ab[0]*ac[2], ab[0]*ac[1]-ab[1]*ac[0])
        dot = sum(cross[i]*v[i] for i in range(3))
        assert dot > 0, "inward or degenerate triangle"
        min_dot = min(min_dot, dot)
        for p, q in ((a,b), (b,c), (c,a)):
            edges[tuple(sorted((p,q)))] += 1
            midpoint = [(vertices[p][i]+vertices[q][i])/2 for i in range(3)]
            max_sagitta = max(max_sagitta, 50-math.sqrt(sum(x*x for x in midpoint)))
    assert max_radius_error < 1e-8
    assert max(edges.values()) == 2
    if closed:
        assert min(edges.values()) == 2, "open globe seam"
        assert len(vertices)-len(edges)+len(faces) == 2
    else:
        assert sum(v == 1 for v in edges.values()) == 512
    return {"vertices":len(vertices), "triangles":len(faces), "bytes":path.stat().st_size,
            "max_vertex_radius_error_source_cm":max_radius_error, "max_edge_sagitta_at_25km_cm":max_sagitta*50000}


root = Path(sys.argv[1] if len(sys.argv)>1 else "IB_S3_GeometryQA").resolve()
root.mkdir(parents=True,exist_ok=True)
author.sphere(root/"sphere.obj")
author.cap(root/"cap.obj")
result = {"scope":"original OBJ topology only; Unreal authoring NOT executed", "sphere":inspect(root/"sphere.obj",True),"cap":inspect(root/"cap.obj",False)}
assert result["cap"]["max_edge_sagitta_at_25km_cm"] < 3
(root/"geometry-report.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
print(json.dumps(result,indent=2))
print("PASS generated geometry: winding, closed globe, manifold edges, radius, cap error")
