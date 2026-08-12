#!/usr/bin/env python3
"""Render a two-colour circulant graph and its paper distance sets as SVG."""

from __future__ import annotations

import argparse
import math
from pathlib import Path


def read_colouring(path: Path) -> list[list[int]]:
    lines = [line.strip() for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]
    try:
        order = int(lines[0])
        matrix_start = lines.index("MATRIX") + 1
    except (IndexError, ValueError) as error:
        raise ValueError("expected an order on the first line and a MATRIX section") from error
    matrix = [[int(value) for value in line.split()] for line in lines[matrix_start : matrix_start + order]]
    if len(matrix) != order or any(len(row) != order for row in matrix):
        raise ValueError("matrix dimensions do not match the graph order")
    if any(value not in (0, 1) for row in matrix for value in row):
        raise ValueError("the colouring matrix must contain only 0 and 1")
    return matrix


def edges_for_colour(matrix: list[list[int]], colour: int) -> list[tuple[int, int]]:
    return [(u, v) for u in range(len(matrix)) for v in range(u + 1, len(matrix)) if matrix[u][v] == colour]


def distances(matrix: list[list[int]], colour: int) -> list[int]:
    order = len(matrix)
    return [distance for distance in range(1, order // 2 + 1) if matrix[0][distance] == colour]


def point(index: int, order: int, centre_x: float, centre_y: float, radius: float) -> tuple[float, float]:
    angle = -math.pi / 2 + 2 * math.pi * index / order
    return centre_x + radius * math.cos(angle), centre_y + radius * math.sin(angle)


def graph_group(
    matrix: list[list[int]],
    colour: int,
    centre_x: float,
    centre_y: float,
    label: str,
    distance_label: str,
    css_colour: str,
) -> str:
    order, radius = len(matrix), 130.0
    coordinates = [point(index, order, centre_x, centre_y, radius) for index in range(order)]
    parts = [f'<text x="{centre_x}" y="34" text-anchor="middle" class="heading">{label}</text>']
    for u, v in edges_for_colour(matrix, colour):
        x1, y1 = coordinates[u]
        x2, y2 = coordinates[v]
        parts.append(f'<line x1="{x1:.2f}" y1="{y1:.2f}" x2="{x2:.2f}" y2="{y2:.2f}" stroke="{css_colour}"/>')
    for index, (x, y) in enumerate(coordinates):
        parts.append(f'<circle cx="{x:.2f}" cy="{y:.2f}" r="12" class="node"/><text x="{x:.2f}" y="{y + 4:.2f}" text-anchor="middle" class="label">{index}</text>')
    distance_text = ", ".join(str(value) for value in distances(matrix, colour))
    parts.append(f'<text x="{centre_x}" y="{centre_y + radius + 38}" text-anchor="middle" class="caption">{distance_label} = {{{distance_text}}}</text>')
    return "\n".join(parts)


def render(matrix: list[list[int]]) -> str:
    blue = graph_group(matrix, 0, 220, 210, "Blue circulant graph", "D₁₇ᵇ", "#2563eb")
    red = graph_group(matrix, 1, 620, 210, "Red circulant graph", "D₁₇ʳ", "#dc2626")
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="840" height="410" viewBox="0 0 840 410" role="img" aria-labelledby="title description">
<title id="title">A circulant 4,4-colouring of K17</title>
<desc id="description">Two complementary circulant graphs on 17 vertices. Their distance sets are D_17^b = {{3, 5, 6, 7}} and D_17^r = {{1, 2, 4, 8}}.</desc>
<style>
  .heading {{ font: 700 18px Arial, sans-serif; fill: #172554; }}
  .caption {{ font: 14px Arial, sans-serif; fill: #334155; }}
  .node {{ fill: white; stroke: #334155; stroke-width: 1.5; }}
  .label {{ font: 11px Arial, sans-serif; fill: #0f172a; }}
  line {{ stroke-width: 1.3; stroke-opacity: .76; }}
</style>
<rect width="100%" height="100%" rx="16" fill="#f8fafc"/>
<text x="420" y="380" text-anchor="middle" class="caption">A circulant (4,4)-colouring of K₁₇</text>
{blue}
{red}
</svg>
'''


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="solver colouring file containing a MATRIX section")
    parser.add_argument("output", type=Path, help="destination SVG file")
    args = parser.parse_args()
    try:
        matrix = read_colouring(args.input)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render(matrix), encoding="utf-8")
    print(f"Wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
