"""Generate readable result tables and gallery figures from the existing datasets.

Run after editing/rebuilding data.js. --check verifies that tracked output is current.
The SVG figures retain the research material's CC BY-NC-SA 4.0 licence.
"""
import argparse
from html import escape
import hashlib
import json
import math
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs'


def read_data(filename, assignment):
    text = (DOCS / 'assets' / filename).read_text()
    assert text.startswith(assignment)
    return json.loads(text[len(assignment):].strip().removesuffix(';'))


def replace_block(text, name, content):
    start, end = f'<!-- BEGIN {name} -->', f'<!-- END {name} -->'
    assert text.count(start) == text.count(end) == 1, name
    return re.sub(re.escape(start) + r'.*?' + re.escape(end),
                  lambda _: start + '\n' + content + '\n' + end, text, flags=re.S)


def graph_svg(graph, colour):
    order = graph['order']
    points = [(300 + 255 * math.cos(2 * math.pi * v / order - math.pi / 2),
               300 + 255 * math.sin(2 * math.pi * v / order - math.pi / 2))
              for v in range(order)]
    segments = []
    for a in range(order):
        for b in range(a + 1, order):
            red = graph['distances'][min(b-a, order-b+a)-1] == 1
            if red != (colour == 'red'):
                continue
            x1, y1 = points[a]
            x2, y2 = points[b]
            segments.append(f'M{x1:.3f},{y1:.3f}L{x2:.3f},{y2:.3f}')
    width = 1.05 if order <= 17 else .72 if order <= 35 else .45
    radius = 4.1 if order <= 17 else 2.6
    target = graph['m'] if colour == 'blue' else graph['n']
    dots = ''.join(f'<circle cx="{x:.3f}" cy="{y:.3f}" r="{radius}"/>' for x, y in points)
    title = f'{colour.capitalize()} graph: {order} vertices, no K{target}'
    svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 600 600" role="img" aria-labelledby="title desc">'
           f'<title id="title">{title}</title>'
           f'<desc id="desc">{len(segments)} {colour} edges in the circulant witness for R({graph["m"]},{graph["n"]}). '
           'The complementary graph uses every remaining edge. Research illustration: CC BY-NC-SA 4.0.</desc>'
           '<circle cx="300" cy="300" r="269.025" fill="none" stroke="#dedfd4" stroke-width=".5" vector-effect="non-scaling-stroke"/>'
           f'<path d="{"".join(segments)}" fill="none" stroke="{"#2e5e86" if colour == "blue" else "#992d43"}" '
           f'stroke-width="{width}" vector-effect="non-scaling-stroke"/>'
           f'<g fill="#3b5360">{dots}</g></svg>\n')
    return svg, len(segments)


def build_static_content(check=False):
    data = read_data('data.js', 'window.RAMSEY_DATA = ')
    witnesses = read_data('witness-data.js', 'window.RAMSEY_WITNESSES = ')
    assert len(data['results']) == 25
    output = {}
    rows = []
    for row in data['results']:
        certificate = escape(row['certificate'], quote=True)
        rows.append(f'<tr><td data-label="Ramsey number">R(3,{row["n"]})</td><td data-label="Vertices">{row["order"]:,}</td>'
                    f'<td data-label="Proven lower bound">R(3,{row["n"]}) ≥ {row["bound"]}</td><td data-label="Download">'
                    f'<a href="certificates/{certificate}" download="{certificate}" '
                    f'aria-label="Download the certificate for R(3,{row["n"]})">Certificate ↓</a></td></tr>')
    page = DOCS / 'results.html'
    output[page] = replace_block(page.read_text(), 'RESULT ROWS', '\n'.join(rows))
    cards = []
    gallery = [graph for graph in data['gallery'] if graph['order'] <= 51]
    assert len(gallery) == 11
    for graph in gallery:
        m, n, order = graph['m'], graph['n'], graph['order']
        witness = next(row for row in witnesses if (row['m'], row['n']) == (m, n))
        assert witness['order'] == order
        assert [int(d in witness['redDistances']) for d in range(1, order//2+1)] == graph['distances']
        views = []
        image_urls = {}
        for colour, target in [('blue', m), ('red', n)]:
            path = f'assets/gallery/R_{m}_{n}_{order}_{colour}.svg'
            svg, count = graph_svg(graph, colour)
            output[DOCS / path] = svg
            image_urls[colour] = image_url = f'{path}?v={hashlib.sha256(svg.encode()).hexdigest()[:12]}'
            views.append(f'<a class="gallery-colour {colour}" href="{image_url}" '
                         f'data-witness="witness.html?m={m}&amp;n={n}&amp;colour={colour}" '
                         f'aria-label="View the {colour} graph on {order} vertices for R({m},{n})">'
                         f'<span class="gallery-colour-label">{colour.capitalize()} · no K<sub>{target}</sub></span>'
                         f'<img src="{image_url}" width="600" height="600" loading="lazy" decoding="async" '
                         f'alt="{colour.capitalize()} circulant graph on {order} vertices with {count:,} edges and no clique of order {target}">'
                         f'<span class="gallery-edge-count">{count:,} edges</span></a>')
        cards.append(f'<article class="gallery-card"><div class="gallery-card-top"><strong>{order}</strong><span>R({m},{n})</span></div>'
                     f'<div class="gallery-pair">{"".join(views)}</div>'
                     f'<p class="gallery-card-proof">Together they prove R({m},{n}) ≥ {order+1}.</p>'
                     f'<a class="gallery-card-bottom gallery-open" href="{image_urls["blue"]}" '
                     f'data-witness="witness.html?m={m}&amp;n={n}">Open the blue graph image ↗</a>'
                     f'<a class="witness-source-link" href="certificates/{escape(witness["certificate"], quote=True)}" download>Graph certificate ↓</a></article>')
    page = DOCS / 'gallery.html'
    output[page] = replace_block(page.read_text(), 'GALLERY CARDS', '\n'.join(cards))
    changed = []
    for path, content in output.items():
        if not path.exists() or path.read_text() != content:
            changed.append(str(path.relative_to(ROOT)))
            if not check:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(content)
    if check and changed:
        raise SystemExit('Stale static content: ' + ', '.join(changed))
    print(f'{"Checked" if check else "Prepared"} 25 static result rows, 11 gallery pairs and 22 SVG figures; {len(changed)} files changed.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    build_static_content(check=parser.parse_args().check)
