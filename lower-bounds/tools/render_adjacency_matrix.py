#!/usr/bin/env python3
"""Render a complete blue/red adjacency matrix from a DIMACS graph."""

from __future__ import annotations

import argparse
from pathlib import Path


def read_dimacs(path: Path) -> tuple[int, set[tuple[int, int]]]:
    order: int | None = None
    edges: set[tuple[int, int]] = set()
    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        fields = raw_line.split()
        if not fields or fields[0] == "c":
            continue
        if fields[0] == "p" and len(fields) == 4 and fields[1] == "edge":
            order = int(fields[2])
        elif fields[0] == "e" and len(fields) == 3:
            u, v = int(fields[1]) - 1, int(fields[2]) - 1
            if u == v:
                raise ValueError(f"loop in {path}: {u + 1} {v + 1}")
            edges.add((min(u, v), max(u, v)))
    if order is None:
        raise ValueError(f"missing DIMACS p-line in {path}")
    return order, edges


def row_paths(order: int, edges: set[tuple[int, int]], cell: float, left: float, top: float) -> tuple[str, str]:
    blue_paths: list[str] = []
    red_paths: list[str] = []
    for i in range(order):
        run_start = 0
        run_colour: str | None = None
        for j in range(order + 1):
            if j == order:
                colour = None
            elif i == j:
                colour = "diagonal"
            elif (min(i, j), max(i, j)) in edges:
                colour = "red"
            else:
                colour = "blue"
            if run_colour is None:
                run_start, run_colour = j, colour
                continue
            if colour != run_colour:
                width = (j - run_start) * cell
                if width and run_colour != "diagonal":
                    x, y = left + run_start * cell, top + i * cell
                    command = f"M{x:.2f},{y:.2f}h{width:.2f}v{cell:.2f}h-{width:.2f}z"
                    (blue_paths if run_colour == "blue" else red_paths).append(command)
                if j < order:
                    run_start, run_colour = j, colour
    return "".join(blue_paths), "".join(red_paths)


def svg(order: int, edges: set[tuple[int, int]], title: str) -> str:
    cell = min(18.0, max(1.8, 850.0 / order))
    left, top = 55.0, 55.0
    side = order * cell
    width, height = left + side + 30, top + side + 58
    blue, red = row_paths(order, edges, cell, left, top)
    labels = ""
    if order <= 80:
        labels = "".join(
            f'<text x="{left + (i + 0.5) * cell:.2f}" y="{top - 7:.2f}" text-anchor="middle" font-size="7">{i}</text>'
            f'<text x="{left - 7:.2f}" y="{top + (i + 0.7) * cell:.2f}" text-anchor="end" font-size="7">{i}</text>'
            for i in range(order)
        )
    return "\n".join(
        [
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{width:.0f}" height="{height:.0f}" viewBox="0 0 {width:.0f} {height:.0f}">',
            f'<title>{title}</title>',
            '<desc>Complete adjacency matrix following the paper convention. Red cells are DIMACS edges, blue cells are complementary edges, and the diagonal is white.</desc>',
            '<rect width="100%" height="100%" fill="white"/>',
            f'<text x="{left}" y="24" font-family="Arial, sans-serif" font-size="16" font-weight="bold">{title}</text>',
            f'<rect x="{left}" y="{top}" width="{side:.2f}" height="{side:.2f}" fill="white" stroke="#333"/>',
            f'<path d="{blue}" fill="#6ea8fe"/>',
            f'<path d="{red}" fill="#f28b82"/>',
            labels,
            f'<rect x="{left}" y="{top + side + 18:.2f}" width="15" height="15" fill="#6ea8fe" stroke="#333"/>',
            f'<text x="{left + 22}" y="{top + side + 30:.2f}" font-family="Arial, sans-serif" font-size="12">Blue: blue edges</text>',
            f'<rect x="{left + 150}" y="{top + side + 18:.2f}" width="15" height="15" fill="#f28b82" stroke="#333"/>',
            f'<text x="{left + 172}" y="{top + side + 30:.2f}" font-family="Arial, sans-serif" font-size="12">Red: red edges</text>',
            '</svg>',
        ]
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Render a complete coloured adjacency matrix from a DIMACS graph.")
    parser.add_argument("--dimacs", type=Path, required=True, help="input DIMACS graph; its edges are red, following the paper convention")
    parser.add_argument("--output", type=Path, required=True, help="destination SVG file")
    parser.add_argument("--title", help="title shown in the SVG")
    args = parser.parse_args()
    order, edges = read_dimacs(args.dimacs)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(svg(order, edges, args.title or f"Adjacency matrix: {args.dimacs.stem}"), encoding="utf-8")
    print(f"Wrote {args.output} ({order} x {order} matrix).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
