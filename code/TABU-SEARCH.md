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

### The state and the constraint pool

The search works directly on the circulant distance vector: one binary entry per circular distance
`1, ..., floor(t/2)`, blue or red. A move flips one entry, so the neighbourhood of the current point
has exactly `floor(t/2)` candidates.

There is no linear relaxation. What the search keeps instead is a **pool of supports**. A support is
a colour together with a set of distances, and it records a condition the coloring must not meet:
a blue support with distance set `S` is *violated* when every distance of `S` is blue, a red support
when every distance of `S` is red. These are the same clique-avoidance conditions the branch-and-cut
separates, held in a list rather than in an LP. Supports are deduplicated on insertion, so the same
colour and distance set is never held twice.

### How a move is evaluated

Each support carries a counter `q`: the number of its distances that currently have the colour which
*saves* it. For a blue support, `q` counts how many of its distances are red. So:

- `q = 0` means the support is fully violated: every distance has the forbidden colour;
- `q = 1` means one distance away from being violated;
- `q >= 2` means comfortably satisfied.

Each support contributes a penalty that depends only on its own `q` and its own weight:

```
penalty(q) = weight           if q = 0
             weight * beta    if q = 1
             0                if q >= 2
```

The coefficient `beta` for near violations exists in the code but is not reachable from the command
line, where it stays `0`. In this build, therefore, the score is exactly the **sum of the weights of
the fully violated supports**, and a score of zero means no support in the pool is violated.

The score is **maintained, not recomputed**. Two structures make that possible: every support stores
its current `q`, and every distance stores the list of supports that contain it. Flipping distance
`d` can only change the counters of the supports in `d`'s list, and each of those changes by exactly
one, up or down depending on the flip direction and the support's colour. So the score after the flip
is the current score plus the sum, over that list alone, of `penalty(q +/- 1) - penalty(q)`.

The consequence is what makes the search practical: evaluating a candidate move costs work
proportional to **how many supports contain that one distance**, not to the size of the pool. The
pool may hold thousands of supports and a flip still touches only a slice of it. Applying the chosen
move repeats the same arithmetic and commits the counters, so evaluation and application share one
code path and cannot drift apart.

#### A worked example

Take `t = 13`, so the distances are `1, ..., 6`, and take `m = 3`, so no three vertices may be all
blue. The vertices `{0, 1, 3}` have circular distances `1`, `2` and `3`, and they form a triangle,
so the pool contains the blue support `{1, 2, 3}`: those three distances must not all be blue.

Suppose the current vector has `1` and `2` blue and `3` red. Then for that support `q = 1`, because
exactly one of its distances, namely `3`, has the colour that saves it. Its penalty is `0`, and it
contributes nothing to the score.

Now consider flipping distance `3` to blue. Distance `3` belongs to several supports, and only those
are inspected; for our support the counter would go from `q = 1` to `q = 0`, its penalty from `0` to
its weight, so the candidate score is the current score plus that weight. If instead we consider
flipping distance `5`, which does not appear in this support at all, the support is not even looked
at: its counter cannot change.

That is the whole evaluation. The search does this for each of the six distances, picks the smallest
resulting score among the admissible ones, and flips.

### How a move is chosen

At each iteration every one of the `floor(t/2)` flips is evaluated as above, and the best is taken.
Two rules qualify that:

- **tabu.** A distance whose tenure has not expired is skipped, so the search cannot immediately
  undo what it just did. The exception is *aspiration*: a tabu distance is considered anyway if its
  candidate score is strictly better than the best score seen so far in the run.
- **last resort.** This distance space is small, so it can happen that every distance is tabu at
  once. In that case the tabu condition is dropped for that iteration and the best move is taken
  regardless, rather than leaving the search with nothing to do.

Ties on the best candidate score are broken uniformly at random, which is where the seed enters.

### One iteration, in order

Written out, the loop is short. `t/2` below is the number of distances.

```text
before the loop:  call the separator once, so the pool is not empty
                  remember the current vector as the best so far

repeat until the time limit or the iteration limit:

    if the score is zero:
        call the separator; if it returned a new support, start the next iteration
        otherwise run the exact verification:
            feasible -> write the certificate and stop, this is the only way to succeed
            not feasible -> the verifier hands back the clique it found;
                            add it as a support and continue
                            (a zero score with nothing new to add would be a bug,
                             and the code refuses to continue in that case)

    every <sep_period> iterations: call the separator and fold in what it finds

    evaluate all t/2 flips incrementally
    pick the best admissible one, breaking ties at random
    flip it, and make that distance tabu for a random tenure in [tenure_min, tenure_max]

    if the score improved on the best so far:
        remember this vector, and call the separator on it

    every <weight_period> iterations: raise the weight of every violated support
    if <stagnation> iterations passed with no improvement: perturb
```

Two points are worth reading off that order. The separator is called at four different moments —
once at the start, periodically, on every improvement, and whenever the score hits zero — so the
pool is enriched exactly when the search is doing well or claims to be done, not at random. And the
exact verification is the **only** authority that can declare success: reaching score zero merely
triggers it.

### Getting unstuck

Two further mechanisms act on the search rather than on a single move.

**Perturbation.** When the best score has not improved for the configured number of iterations, a
random subset of distances of the configured size is flipped at once, through the same incremental
machinery.

**Adaptive weights.** When enabled, every so often the weight of each *fully violated* support is
increased by the configured amount. An obstruction that survives many iterations therefore becomes
progressively more expensive, and the search is pushed away from the region that keeps producing it,
instead of circling it. Supports that are currently satisfied are left untouched.

### Growing the pool, and the final check

The pool is not fixed. It starts from all triangle supports and grows: every so often the search
calls the branch-and-cut's own red separator on the current distance vector, and any red clique it
finds becomes a new support, whose `q` and penalty are folded into the score on insertion. The
heuristic and the exact method therefore share one separation routine.

This is also why a score of zero is not yet an answer. It means only that nothing *in the pool* is
violated, and the pool is whatever separation has produced so far. So when the score reaches zero
the search runs an exact verification, searching for a blue clique of size `m` and a red clique of
size `n` with the same clique solver the models use, and only if both searches come up empty does it
write a certificate.

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
