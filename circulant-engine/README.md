# Circulant search engine (an independent reimplementation of the distance-orbit search)

This directory contains a standalone exact search for two-colour circulant Ramsey colourings. It is
separate from the branch-and-cut solver in [`../code`](../code): no LP, no CPLEX, no cuts. It
enumerates colourings of the circular distances directly, and it can both exhibit a colouring and
prove that none exists at a given order.

## Credit where it is due

The search principle implemented here is **not ours**. It is the algorithm of Jan Goedgebeur and
Steven Van Overberghe, published as

> J. Goedgebeur, S. Van Overberghe, *New bounds for Ramsey numbers*, arXiv:2107.04460,

with their generators available at

> https://github.com/Steven-VO/circulant-Ramsey

Their program `genCyc` is the reference implementation and the yardstick we measure against; on the
orders where both run, our engine visits exactly the same search tree, which is the clearest way of
saying that the idea is theirs. Please cite their paper and repository for the method itself.

This is an independent reimplementation, written from the mathematical statement of the pruning rule
rather than from their code: no source was copied. What we added is (i) a representation that is
generic in the graph order, so the search is not limited to 128 vertices, (ii) an optional exact
clique oracle for the residual queries, (iii) deterministic splitting of an exhaustive job, and
(iv) replay of stored colourings for verification.

## The algorithm in one paragraph

In a circulant graph on `Z_t` the colour of an edge depends only on the circular distance
`d = min(|i-j|, t-|i-j|)`, so a colouring is one choice of colour per distance
`d = 1, ..., floor(t/2)`, and assigning `d` colours the whole orbit `{i, i+d}` at once. If assigning
`d` to colour `c` creates a monochromatic `K_k` that was not present before, that clique must use an
edge of the new orbit, and by rotation and reflection we may assume it uses `{0, d}`; its remaining
`k-2` vertices then form a clique of colour `c` in the graph induced on `N_c(0) ∩ N_c(d)`. So after
each assignment it is necessary and sufficient to ask whether that small induced graph contains a
`K_(k-2)`. The search assigns distances in order and backtracks whenever the answer is yes.

## How the residual query is answered

That question is the only expensive part, and the engine offers three policies:

| mode | behaviour |
| --- | --- |
| `--mode fast` | only the built-in recursive bitset search: pick the least candidate vertex `v`, intersect the candidate set with `N_c(v)`, stop as soon as too few candidates remain |
| `--mode checker` | send every query to the exact maximum-clique solver (COPT-BG) |
| `--mode hybrid` | run the built-in search up to a work budget, then hand the residual graph to the exact solver; `--budget` sets the budget, 1000 by default |

A timeout of the exact solver is never turned into a proof: the run falls back to the unbounded
exact built-in search. Which policy is best depends on the pair: with small forbidden cliques the
built-in search wins by a wide margin, while for `(3,n)` with large `n` the exact oracle is what
makes the instances tractable.

## Usage

```bash
./hybrid_gencyc ORDER BLUE_TARGET RED_TARGET [options]

  --mode fast|checker|hybrid    residual-query policy (default hybrid)
  --budget N                    work budget before delegating (hybrid only)
  --copt-timeout SECONDS        limit for one exact-oracle call
  --max-seconds SECONDS         limit for the whole run
  --all, --max-solutions N      enumerate colourings instead of stopping at the first
  --detailed-stats              per-query work statistics
  --trace-limit N               print the first N pruning witnesses, for inspection
  --no-colour-symmetry          keep distance 1 free when the two targets are equal
  --split-depth D --split-count K --split-index I
                                deterministic partition of an exhaustive run into K jobs
  --replay-matrix FILE          verify a stored colouring instead of searching
```

Examples:

```bash
./hybrid_gencyc 72 6 6 --mode fast          # prove that no circulant (6,6)-colouring exists at 72
./hybrid_gencyc 58 3 13 --mode hybrid       # same question for (3,13) at order 58
./hybrid_gencyc 160 3 26 --mode hybrid --replay-matrix stored_colouring.txt
```

Exit codes: `0` a complete decision, `2` invalid arguments, `3` an incomplete timed run, `4` a
rejected replay matrix. Every run prints `verdict=` and `complete=`; a verdict counts only with
`complete=1`.

## Building

`source/` is provided for inspection and reuse under the same terms as the rest of this repository.
Like the branch-and-cut sources, it does not build from this repository alone: `copt_oracle.cpp`
includes headers from **BitGraph** and **coptBG** by Pablo San Segundo (CSIC-UPM), which are GPL-3.0
and are not redistributed here — see [`../NOTICE.md`](../NOTICE.md). With those libraries installed,
`make -j` in `source/` produces the executable; set `COPT_ROOT` to their install prefix.

## Licence

GNU General Public License v3.0 or later, as the rest of this repository; see
[`../LICENSE`](../LICENSE). The original `genCyc` is likewise GPL-3.0, and the authors above hold
the credit for the method.
