#!/usr/bin/env python3
"""Render a paper-style coloured circular-distance matrix from a DIMACS graph.

In the published certificates, the DIMACS edges represent the red graph, in
agreement with the paper's convention that blue avoids K_m and red avoids
K_n for R(m,n). For each circular distance d, the script colours one cell red
if the entire distance class is present or blue if it is empty.
The resulting SVG uses only standard-library Python and can be embedded in a
GitHub README.
"""

from __future__ import annotations

import argparse
import html
from pathlib import Path


def read_dimacs(path: Path) -> tuple[int, set[tuple[int, int]]]:
    """Return the graph order and normalised, zero-indexed edge set."""
    order: int | None = None
    edges: set[tuple[int, int]] = set()

    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
        parts = raw_line.split()
        if not parts or parts[0] == "c":
            continue
        if parts[0] == "p":
            if len(parts) != 4 or parts[1] != "edge":
                raise ValueError(f"line {line_number}: expected 'p edge <n> <m>'")
            order = int(parts[2])
            if order < 2:
                raise ValueError("the graph order must be at least 2")
        elif parts[0] == "e":
            if order is None:
                raise ValueError(f"line {line_number}: edge before the p-line")
            if len(parts) != 3:
                raise ValueError(f"line {line_number}: expected 'e <u> <v>'")
            u, v = int(parts[1]) - 1, int(parts[2]) - 1
            if not (0 <= u < order and 0 <= v < order) or u == v:
                raise ValueError(f"line {line_number}: invalid edge ({u + 1}, {v + 1})")
            edges.add((u, v) if u < v else (v, u))

    if order is None:
        raise ValueError("missing DIMACS p-line")
    return order, edges


def distance_edges(order: int, distance: int) -> set[tuple[int, int]]:
    """Return the distinct undirected edges in one circular-distance class."""
    return {
        (u, v) if u < v else (v, u)
        for u in range(order)
        for v in [(u + distance) % order]
    }


def classify(order: int, edges: set[tuple[int, int]]) -> list[tuple[int, str, int, int]]:
    """Classify every circular distance as blue, red, or partial."""
    result: list[tuple[int, str, int, int]] = []
    for distance in range(1, order // 2 + 1):
        full_class = distance_edges(order, distance)
        present = len(full_class & edges)
        if present == len(full_class):
            colour = "red"
        elif present == 0:
            colour = "blue"
        else:
            colour = "partial"
        result.append((distance, colour, present, len(full_class)))
    return result


def svg(title: str, order: int, cells: list[tuple[int, str, int, int]], columns: int) -> str:
    """Build a self-contained SVG matrix with a legend."""
    cell_width = min(44.0, max(5.0, 1050.0 / columns))
    cell_height = min(38.0, max(16.0, cell_width * 0.86))
    label_size = min(13.0, max(6.0, cell_width * 0.32))
    left, top = 34, 82
    rows = (len(cells) + columns - 1) // columns
    width = left * 2 + columns * cell_width
    height = top + rows * cell_height + 86
    palette = {"blue": "#6ea8fe", "red": "#f28b82", "partial": "#fbbc04"}
    escaped_title = html.escape(title)
    body = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" role="img" aria-labelledby="title description">',
        f"<title id=\"title\">{escaped_title}</title>",
        '<desc id="description">A coloured circular-distance matrix following the paper convention. Red cells are DIMACS edges and blue cells are complementary edges.</desc>',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{left}" y="31" font-family="Arial, sans-serif" font-size="18" font-weight="700">{escaped_title}</text>',
        f'<text x="{left}" y="55" font-family="Arial, sans-serif" font-size="13">Graph order t = {order}; each cell represents one circular distance.</text>',
    ]
    for index, (distance, colour, present, total) in enumerate(cells):
        row, column = divmod(index, columns)
        x, y = left + column * cell_width, top + row * cell_height
        tooltip = html.escape(f"distance {distance}: {present}/{total} DIMACS edges")
        body.extend(
            [
                f'<g><title>{tooltip}</title>',
                f'<rect x="{x}" y="{y}" width="{cell_width - 3}" height="{cell_height - 3}" rx="3" fill="{palette[colour]}" stroke="#333"/>',
                f'<text x="{x + (cell_width - 3) / 2}" y="{y + cell_height * 0.64:.2f}" text-anchor="middle" font-family="Arial, sans-serif" font-size="{label_size:.1f}">{distance}</text></g>',
            ]
        )
    legend_y = top + rows * cell_height + 30
    for index, (label, colour) in enumerate((("Blue: blue circular distance", "blue"), ("Red: red circular distance", "red"))):
        x = left + index * 230
        body.extend(
            [
                f'<rect x="{x}" y="{legend_y}" width="18" height="18" fill="{palette[colour]}" stroke="#333"/>',
                f'<text x="{x + 25}" y="{legend_y + 14}" font-family="Arial, sans-serif" font-size="12">{label}</text>',
            ]
        )
    body.append("</svg>")
    return "\n".join(body) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description="Render a coloured circular-distance matrix from a DIMACS graph.")
    parser.add_argument("--dimacs", required=True, type=Path, help="input DIMACS edge file; its edges are shown in red, following the paper convention")
    parser.add_argument("--output", required=True, type=Path, help="destination SVG file")
    parser.add_argument("--title", default=None, help="title shown in the SVG")
    parser.add_argument("--columns", type=int, default=15, help="number of distance cells per row (default: 15)")
    args = parser.parse_args()
    if args.columns is not None and args.columns < 1:
        parser.error("--columns must be positive")

    try:
        order, edges = read_dimacs(args.dimacs)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    cells = classify(order, edges)
    title = args.title or f"Circular-distance matrix for {args.dimacs.name}"
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(svg(title, order, cells, args.columns), encoding="utf-8")
    partial = sum(colour == "partial" for _, colour, _, _ in cells)
    print(f"Wrote {args.output} ({len(cells)} distance classes; {partial} partial classes).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
