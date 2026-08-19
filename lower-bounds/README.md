# Lower bounds

This directory contains the graph certificates computed for the paper's Ramsey instances, together with their coloured visualisations and the tools used to render them.

Each certificate is a `.clq` file named `R_<m>_<n>_graphSize_<N>.clq`, representing the red colour class of a two-edge-colouring of a complete graph on `N` vertices; the blue colour class is its complement. This follows the paper's convention: for $R(m,n)$, the blue graph avoids $K_m$ and the red graph avoids $K_n$.

Every certificate has two companion figures with the same basename: `<certificate>.png` (circular-distance matrix) and `<certificate>_adjacency.png` (full adjacency matrix). Renderers for both are in [`tools/`](tools/).

The certificates, matrices, and documentation in this directory are licensed under [CC BY-NC-SA 4.0](LICENSE). The Python rendering scripts in [`tools/`](tools/) are software and are instead licensed under GPL-3.0-or-later; see [`../NOTICE.md`](../NOTICE.md).

## Certificates

The certificates are split across two pages, matching the paper's own distinction between improving the classical Ramsey number and pinning down its circulant-restricted counterpart:

| Page | What it contains |
| --- | --- |
| [`ramsey-number-lower-bounds.md`](ramsey-number-lower-bounds.md) | New best lower bounds on the unrestricted Ramsey numbers $R(3,n)$, $24\le n\le49$, $n\neq27$. |
| [`circulant-ramsey-numbers.md`](circulant-ramsey-numbers.md) | Lower-bound certificates for the circulant Ramsey numbers $R_C(m,n)$, $m=3,4,5$, which the paper establishes as exact values; includes eight new values $R_C(3,n)$, $n=13,\ldots,20$. |

## File naming

Certificate files use the following naming convention:

```text
R_<m>_<n>_graphSize_<N>.clq
```

Here, `m` and `n` are the Ramsey parameters, and `N` is the number of vertices.

For the certificates stored here, the graph $G$ is the red graph and is
intended to contain no clique of size $n$, while its complement $\overline{G}$
is the blue graph and contains no clique of size $m$. Once these two properties have been verified, the graph defines
an $(m,n)$-coloring of $\mathcal{K}_N$ and proves

$$R(m,n)\ge N+1.$$

## File format

Certificates use the DIMACS edge format:

- comment lines begin with `c`;
- the problem line has the form `p edge <vertices> <edges>`;
- edge lines have the form `e u v`.

Vertices are numbered from `1` through `N`. Comment lines may provide
additional provenance information, but verification depends only on the problem
and edge lines.

## Figures

Every certificate has two companion figures sharing its basename:

- **`<certificate>.png`** — circular-distance matrix. One cell per circular distance $\ell\in\{1,\ldots,\lfloor N/2\rfloor\}$, wrapping after 15 cells per row. For each $\ell$, the *distance class* is the set of all $N$ edges $\{i,\,i+\ell \bmod N\}$. A cell is red if every edge of its class is stored in the certificate (red graph, avoiding $K_n$), and blue if none of them are (blue graph, avoiding $K_m$). Because circulant graphs treat all vertices symmetrically, each class is always entirely red or entirely blue — the figure is simply a compact, one-cell-per-distance summary of the whole certificate.
- **`<certificate>_adjacency.png`** — full $N\times N$ adjacency matrix in the same red/blue convention, with the diagonal left blank.

Generate additional figures from a DIMACS certificate with the renderers in [`tools/`](tools/):

```bash
python3 tools/render_circulant_matrix.py --dimacs certificates/R_3_37_graphSize_271.clq --output matrices/R_3_37_graphSize_271.png
python3 tools/render_adjacency_matrix.py --dimacs certificates/R_3_37_graphSize_271.clq --output matrices/R_3_37_graphSize_271_adjacency.png
```

See [`ramsey-number-lower-bounds.md`](ramsey-number-lower-bounds.md) and [`circulant-ramsey-numbers.md`](circulant-ramsey-numbers.md) for worked examples of how to read these figures.

## Verification

Each certificate can be verified independently with the checker in
[`../checker/`](../checker/). See [`../checker/README.md`](../checker/README.md)
for usage instructions and dependencies.

The checker verifies that the stored red graph contains no clique of size $n$
and that its blue complement contains no clique of size $m$. A certificate is
independently verified only after the checker completes successfully.
