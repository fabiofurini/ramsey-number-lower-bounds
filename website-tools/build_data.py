"""Import the paper's published certificates without changing the source repository."""
import csv
import io
import json
import re
import shutil
from itertools import combinations
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REPO = ROOT
DIST = ROOT / "docs"


def main():
    page = (REPO / "lower-bounds/ramsey-number-lower-bounds.md").read_text()
    rows = []
    for n, order, filename in re.findall(r"\| R\(3,(\d+)\) \| (\d+) \| \[([^\]]+\.clq)\]", page):
        n, order = int(n), int(order)
        source = REPO / "lower-bounds/certificates" / filename
        edges = set()
        for line in source.read_text().splitlines():
            parts = line.split()
            if parts and parts[0] == "p":
                assert int(parts[2]) == order
                expected = int(parts[3])
            if parts and parts[0] == "e":
                a, b = int(parts[1]) - 1, int(parts[2]) - 1
                assert a != b and 0 <= a < order and 0 <= b < order
                edges.add(tuple(sorted((a, b))))
        assert len(edges) == expected
        distances = [int((0, d) in edges) for d in range(1, order // 2 + 1)]
        for a, b in combinations(range(order), 2):
            d = min(b - a, order - b + a)
            assert int((a, b) in edges) == distances[d - 1], filename
        # Independent, cheap verification of triangle-freeness in the blue complement.
        blue = [set() for _ in range(order)]
        for a, b in combinations(range(order), 2):
            if (a, b) not in edges:
                blue[a].add(b)
                blue[b].add(a)
        for a in range(order):
            for b in blue[a]:
                assert not (blue[a] & blue[b]), filename
        shutil.copy2(source, DIST / "certificates" / filename)
        rows.append(dict(n=n, order=order, bound=order + 1, redEdges=len(edges),
                         distances=distances, certificate=filename))
    assert len(rows) == 25
    matrix = (REPO / "assets/r44-colouring-k17.txt").read_text().split("MATRIX\n")[1]
    matrix = [[int(x) for x in line.split()] for line in matrix.splitlines() if line.strip()]
    assert len(matrix) == 17
    assert all(len(set(matrix[a][b] for a, b in combinations(group, 2))) > 1
               for group in combinations(range(17), 4))
    small = dict(order=17, redDistances=[d for d in range(1, 9) if matrix[0][d]])
    gallery = [
        dict(m=3,n=3,order=5,distances=[1,0],source="https://github.com/fabiofurini/ramsey-number-lower-bounds"),
        dict(m=4,n=4,order=17,distances=[int(d in small['redDistances']) for d in range(1,9)],source="https://github.com/fabiofurini/ramsey-number-lower-bounds/blob/main/assets/r44-colouring-k17.txt")
    ]
    for m,n,order in [(4,6,33),(5,6,56),(3,15,72),(3,20,110),(3,37,271),(3,49,410)]:
        filename=f"R_{m}_{n}_graphSize_{order}.clq"
        source=REPO/'lower-bounds/certificates'/filename
        edges=set()
        for line in source.read_text().splitlines():
            parts=line.split()
            if parts and parts[0]=='e':edges.add(tuple(sorted((int(parts[1])-1,int(parts[2])-1))))
        distances=[int((0,d) in edges) for d in range(1,order//2+1)]
        for a,b in combinations(range(order),2):
            assert int((a,b) in edges)==distances[min(b-a,order-b+a)-1]
        shutil.copy2(source,DIST/'certificates'/filename)
        gallery.append(dict(m=m,n=n,order=order,distances=distances,source=f"certificates/{filename}"))
    examples = [
        dict(order=5,m=3,n=3,redDistances=[1],description="Two complementary cycles on 5 vertices, with no monochromatic triangle. A construction for R(3,3) ≥ 6."),
        dict(order=6,m=3,n=3,redDistances=[1],description="On 6 vertices a monochromatic triangle is unavoidable, even without circulant symmetry: R(3,3) = 6."),
        dict(order=8,m=3,n=4,redDistances=[2,3],description="An asymmetric example: no blue triangle and no red clique of order 4. Change a distance to explore R(3,4)."),
        dict(order=13,m=3,n=5,redDistances=[2,3,4,6],description="A 13-vertex construction avoiding blue triangles and red cliques of order 5. The two colours have different forbidden sizes."),
        dict(order=17,m=4,n=4,redDistances=small['redDistances'],description="A classical example: 17 vertices and no monochromatic clique of order 4. Try changing a distance.")
    ]
    for m,n,order in [(3,7,21),(3,8,26),(4,6,33),(3,9,35),(5,5,41),(4,7,46),(4,8,51),(5,6,56)]:
        filename=f"R_{m}_{n}_graphSize_{order}.clq"
        source=REPO/'lower-bounds/certificates'/filename
        edges=set()
        for line in source.read_text().splitlines():
            fields=line.split()
            if fields and fields[0]=='e':edges.add(tuple(sorted((int(fields[1])-1,int(fields[2])-1))))
        red=[d for d in range(1,order//2+1) if (0,d) in edges]
        for a,b in combinations(range(order),2):
            assert ((a,b) in edges)==(min(b-a,order-b+a) in red)
        shutil.copy2(source,DIST/'certificates'/filename)
        examples.append(dict(order=order,m=m,n=n,redDistances=red,certificate=filename,
                             description=f"A construction from the project’s certificates: {order} vertices, no blue K{m} and no red K{n}. Both forbidden sizes are checked after each edit."))
    for example in examples:
        if example['order']<=17:
            red=set(example['redDistances']);order=example['order']
            for target,colour in [(example['m'],False),(example['n'],True)]:
                found=any(all((min(b-a,order-b+a) in red)==colour for a,b in combinations(group,2))
                          for group in combinations(range(order),target))
                assert not found or order==6,example
    gallery = [dict(m=e['m'],n=e['n'],order=e['order'],
                    distances=[int(d in e['redDistances']) for d in range(1,e['order']//2+1)],
                    source=f"certificates/{e['certificate']}" if 'certificate' in e else
                    "https://github.com/fabiofurini/ramsey-number-lower-bounds/blob/main/assets/r44-colouring-k17.txt" if e['order']==17 else
                    "https://github.com/fabiofurini/ramsey-number-lower-bounds")
               for e in examples if e['order']!=6]
    data = dict(sourceCommit="38e59da36155bdcbbaec6252586603cc8ef602b4", results=rows, small=small, gallery=gallery, examples=examples)
    (DIST / "assets/data.js").write_text("window.RAMSEY_DATA = " + json.dumps(data, ensure_ascii=False) + ";\n")
    out = io.StringIO()
    writer = csv.writer(out)
    writer.writerow(["m", "n", "vertices", "lower_bound", "certificate"])
    writer.writerows([3, x["n"], x["order"], x["bound"], x["certificate"]] for x in rows)
    (DIST / "certificates/results.csv").write_text(out.getvalue())
    shutil.copy2(REPO / "lower-bounds/LICENSE", DIST / "certificates/LICENSE.txt")
    shutil.copy2(REPO / "LICENSE", DIST / "assets/SOFTWARE-LICENSE.txt")
    print(f"Imported {len(rows)} circulant certificates; checked format, symmetry and blue triangle-freeness.")
    print("Checked the 17-vertex illustration contains no monochromatic K4.")
    print("The red K_n exclusions in the large certificates are results of the paper, not re-proved by this importer.")


if __name__ == "__main__":
    main()
