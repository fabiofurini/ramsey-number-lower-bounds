"""Prepare viewable witnesses, preserving study certificates and provenance."""
import json
import re
from itertools import combinations
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs'
CLASSICAL = [(3,3,5,[1]), (3,4,8,[2,3]), (3,5,13,[2,3,4,6]),
             (3,6,16,[1,2,3,4,6]), (4,4,17,[1,2,4,8]),
             (4,5,24,[1,5,6,7,9,10,12])]
def distance(a,b,N): return min(abs(a-b),N-abs(a-b))
def main():
    rows=[]
    (DOCS/'certificates/classical').mkdir(exist_ok=True)
    for m,n,N,distances in CLASSICAL:
        red=set(distances)
        for target,colour in [(m,False),(n,True)]:
            assert not any(all((distance(a,b,N) in red)==colour for a,b in combinations(vertices,2))
                           for vertices in combinations(range(N),target)),(m,n,N)
        edges=[(a,b) for a,b in combinations(range(N),2) if distance(a,b,N) in red]
        filename=f'classical/R_{m}_{n}_graphSize_{N}.clq'
        lines=['c Classical illustrative representative reconstructed for the website.',
               'c Not a new Ramsey result or a claimed historical original.',
               'c Both forbidden clique sizes checked by exhaustive subset enumeration.',
               'c Listed edges are red; blue is the complement. Licence: CC BY-NC-SA 4.0.',
               f'p edge {N} {len(edges)}']
        lines += [f'e {a+1} {b+1}' for a,b in edges]
        (DOCS/'certificates'/filename).write_text('\n'.join(lines)+'\n')
        rows.append(dict(m=m,n=n,order=N,redDistances=distances,certificate=filename,source='classical'))
    source=(ROOT/'lower-bounds/circulant-ramsey-numbers.md').read_text()
    for m,n,N,filename in re.findall(r'\| R\((\d+),(\d+)\) \| (\d+) \| \[([^\]]+\.clq)\]',source):
        m,n,N=int(m),int(n),int(N);edges=set();expected=None
        for line in (DOCS/'certificates'/filename).read_text().splitlines():
            parts=line.split()
            if parts and parts[0]=='p':assert int(parts[2])==N;expected=int(parts[3])
            if parts and parts[0]=='e':
                a,b=sorted([int(parts[1])-1,int(parts[2])-1]);assert 0<=a<b<N;edges.add((a,b))
        assert len(edges)==expected
        distances=[d for d in range(1,N//2+1) if (0,d) in edges];red=set(distances)
        assert all(((a,b) in edges)==(distance(a,b,N) in red) for a,b in combinations(range(N),2))
        rows.append(dict(m=m,n=n,order=N,redDistances=distances,certificate=filename,source='study'))
    rows.sort(key=lambda row:(row['m'],row['n']));assert len(rows)==26
    (DOCS/'assets/witness-data.js').write_text('window.RAMSEY_WITNESSES = '+json.dumps(rows)+';\n')
    print('Prepared 26 witnesses; independently verified all six small classical representatives.')
    print('Study certificates checked for format and symmetry; large clique exclusions come from the study.')
if __name__=='__main__':main()
