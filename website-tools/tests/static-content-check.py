"""Verify static results and gallery figures against downloadable certificates."""
import csv
from html.parser import HTMLParser
from pathlib import Path
import re
from urllib.parse import urlsplit
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / 'docs'


class Page(HTMLParser):
    def __init__(self, path):
        super().__init__()
        self.path = path
        self.result_table = False
        self.rows = []
        self.images = []
        self.feed(path.read_text())

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        for key in ('src', 'href'):
            url = urlsplit(attrs.get(key, ''))
            if url.path and not url.scheme and not url.netloc:
                assert (self.path.parent / url.path).is_file(), (self.path, url.path)
        if tag == 'tbody' and attrs.get('id') == 'results-table':
            self.result_table = True
        if tag == 'tr' and self.result_table:
            assert 'hidden' not in attrs
            self.rows.append({'text': '', 'certificate': None})
        if tag == 'a' and self.result_table:
            self.rows[-1]['certificate'] = urlsplit(attrs['href']).path
        if tag == 'img' and attrs.get('src', '').startswith('assets/gallery/'):
            self.images.append(urlsplit(attrs['src']).path)
            assert attrs.get('alt') and attrs['width'] == attrs['height'] == '600'

    def handle_data(self, data):
        if self.result_table and self.rows:
            self.rows[-1]['text'] += data + ' '

    def handle_endtag(self, tag):
        if tag == 'tbody':
            self.result_table = False


def certificate_header(path):
    for line in path.read_text().splitlines():
        if line.startswith('p '):
            _, _, order, edges = line.split()
            return int(order), int(edges)
    raise AssertionError(path)


pages = {p.name: Page(p) for p in DOCS.glob('*.html')}
rows = pages['results.html'].rows
with (DOCS / 'certificates/results.csv').open() as stream:
    expected = list(csv.DictReader(stream))
assert len(rows) == len(expected) == 25
for row, source in zip(rows, expected):
    n, order, bound = int(source['n']), int(source['vertices']), int(source['lower_bound'])
    assert row['certificate'] == 'certificates/' + source['certificate']
    assert f'R(3,{n}) ≥ {bound}' in row['text'] and bound == order + 1
    assert certificate_header(DOCS / row['certificate'])[0] == order
assert len(pages['gallery.html'].images) == 22
ns = {'svg': 'http://www.w3.org/2000/svg'}
for path in pages['gallery.html'].images:
    m, n, order = map(int, re.search(r'R_(\d+)_(\d+)_(\d+)_', path).groups())
    colour = 'blue' if path.endswith('_blue.svg') else 'red'
    svg = ET.parse(DOCS / path).getroot()
    candidates = list((DOCS / 'certificates').glob(f'**/R_{m}_{n}_graphSize_{order}.clq'))
    assert len(candidates) == 1, candidates
    cert_order, red_edges = certificate_header(candidates[0])
    assert cert_order == order
    expected_edges = red_edges if colour == 'red' else order * (order-1)//2 - red_edges
    assert svg.find('svg:path', ns).attrib['d'].count('M') == expected_edges, path
    assert len(svg.findall('svg:g/svg:circle', ns)) == order, path
    assert 'CC BY-NC-SA 4.0' in svg.find('svg:desc', ns).text
print('PASS 25 static result rows, 22 blue/red SVGs against DIMACS headers, licences, and all local page links.')
