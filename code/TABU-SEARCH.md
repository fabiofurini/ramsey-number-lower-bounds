# The tabu search (input 4 = `6`)

This page documents a heuristic that is **not** part of the paper. It is selected only when you ask
for it, and it does not touch the branch-and-cut: the values of input 4 that the paper uses, `1` and
`3`, behave exactly as before.

Unlike every other value of input 4, this one is **not a formulation**. It is a local search, so it
can find a coloring but can never prove that none exists. Use it to obtain a certificate quickly;
use the models of the paper when you need an exclusion.

The page has two parts. The first explains how the method works. The second explains every input of
its command line.

---

## Part 1 — How the method works

The search works directly on the circulant distance vector: one binary entry per circular distance
`1, ..., floor(t/2)`, blue or red. A move flips one distance. The objective is not a linear
relaxation but a weighted count of **violated supports**, where a support is a set of distances that
must not all be blue, or must not all be red — the same clique-avoidance conditions the
branch-and-cut separates, held in a pool rather than in an LP. Flips are evaluated incrementally, so
one iteration costs a scan of the supports touching the flipped distance.

Three mechanisms drive it:

- a **tabu tenure**, drawn from a range, forbidding a distance from being flipped back immediately;
- a **stagnation counter**, which perturbs several distances at once when the best score has not
  improved for a while;
- optional **adaptive weights**, which raise the penalty on supports that keep being violated, so
  the search stops circling the same obstruction.

The support pool is not fixed. It starts from all triangle supports and grows: every so often the
search calls the branch-and-cut's own red separator on the current distance vector, and any red
clique it finds becomes a new support. The heuristic and the exact method therefore share one
separation routine.

When the score reaches zero the search does not trust it. It runs an exact verification, searching
for a blue clique of size `m` and a red clique of size `n` with the same clique solver the models
use, and only if both searches come up empty does it write a certificate.

### Restrictions

Two, both enforced in the code, which prints a message and stops:

- the circulant restriction must be on, since the search works in the circulant distance space;
- the forbidden blue clique size must be `3`, so the search covers the `R(3,n)` family only.

---

## Part 2 — The parameters

This heuristic takes its own **18-input** command line, not the 33 inputs of the models, because
most of those inputs configure a branch-and-cut that is not used here. The argument count is what
tells the two apart, so input 4 = `6` is only recognised on the short line.

```bash
./RAMSEY <t> 3 <n> 6 <time> <seed> <max_iter> <tenure_min> <tenure_max> \
         <stagnation> <perturb> <sep_period> <heur_restarts> <heur_iters> \
         <weight_period> <weight_increment> <test_id>
```

| # | Effect and possible values |
| ---: | --- |
| 1 | Sets the number of vertices `t`; use a positive integer. |
| 2 | Sets the forbidden blue clique size `m`; must be `3`. |
| 3 | Sets the forbidden red clique size `n`; use a positive integer. |
| 4 | Selects the tabu search: use `6`. On this short command line the earlier value `5` is still accepted, with a notice. |
| 5 | Sets the time limit in seconds; the search stops at the limit and reports whether it found a coloring. |
| 6 | Sets the random seed; the search is randomised, so record it with every result. |
| 7 | Sets the maximum number of iterations, as a second stopping rule alongside the time limit. |
| 8 | Sets the minimum tabu tenure: how many iterations a just-flipped distance is at least barred from being flipped back. |
| 9 | Sets the maximum tabu tenure; each flip draws its tenure from the range `[8, 9]`. A wider range diversifies the search. |
| 10 | Sets the stagnation limit: the number of iterations without an improvement of the best score after which the search perturbs the current vector. |
| 11 | Sets the perturbation size: how many distances are flipped at once when stagnation triggers. |
| 12 | Sets the separation period: how many iterations pass between two calls to the exact red separator that adds new supports to the pool. Smaller means a better-informed objective and a higher cost per iteration. |
| 13 | Sets the number of restarts given to the heuristic clique search inside those separator calls. |
| 14 | Sets the number of iterations per restart for that clique search. |
| 15 | Sets the adaptive-weight period: how many iterations between two weight updates. Use `0` to disable adaptive weights, which is the baseline. |
| 16 | Sets the adaptive-weight increment, as a real number: how much the penalty of a still-violated support grows at each update. Ignored when input 15 is `0`. |
| 17 | Sets a unique integer identifying the run and its output files. |

The three groups worth tuning first are the tenure range (inputs 8 and 9), which controls how long
the search is kept away from a distance it has just changed; the stagnation pair (inputs 10 and 11),
which controls how patiently it stays in a region before shaking it; and the separation period
(input 12), which trades a better-informed objective against a higher cost per iteration. Adaptive
weights (inputs 15 and 16) are off in the baseline and are worth trying when the search keeps
returning to the same obstruction.

### Example

A (3,3)-coloring on 5 vertices, ten seconds, seed 1:

```bash
./RAMSEY 5 3 3 6 10 1 1000 3 5 200 2 10 1 100 0 0.0 950001
```

A larger run on `R(3,8)`, twenty seconds, adaptive weights off:

```bash
./RAMSEY 20 3 8 6 20 1 20000 7 14 5000 4 100 5 10000 0 0.0 6001
```

### Output

The run prints one line per statistic, all prefixed `TABU`: the number of iterations, the size the
support pool reached, the best score, how many separator calls and exact verifications were made,
the clique-solver call counts broken down by which routine succeeded, and finally whether a coloring
was found.

When one is found the certificate goes to
`colorings/tabu_m<m>_n<n>_SIZE<t>_id<id>.txt`, listing the order, the blue circular distances and
the red ones. For an independent verification, expand those distances into the blue graph, write it
in DIMACS edge format and pass it to the repository's
[(m,n)-coloring checker](../checker/README.md), as [Reading the output](OUTPUT.md) describes. The
coloring is circulant by construction, so the checker's circulant-reduction input may be set to `1`;
as its own page notes, that input asks the checker to exploit the property, it does not verify it.

Because the search is randomised, two runs with different seeds may return different colorings, and
one may find nothing where the other succeeds. Report the seed and the full command with any result.

### What it does not do

It cannot prove that no coloring exists: a run that ends without a certificate means only that this
search did not find one within its limits. Every exclusion result in the paper comes from the
branch-and-cut, and so does every circulant Ramsey number; see
[Run the solver](RUN.md) and [Default RAMSEY parameters](DEFAULT-PARAMETERS.md).
