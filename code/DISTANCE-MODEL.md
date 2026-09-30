# The linear-distance model (input 4 = `5`)

This page documents a formulation that is **not** part of the paper. It runs only when you select
it: the values of input 4 that the paper uses, `1` and `3`, behave exactly as before, node for node.

The page has two parts. The first explains how the method works. The second explains every input
whose meaning or behaviour differs from the published configuration.

---

## Part 1 — How the method works

### The class of colorings

The paper's distance model, input 4 = `3`, searches **circulant** colorings. There the color of the
edge `{i,j}` depends on the circular distance `min(|i-j|, t-|i-j|)`, so a coloring is one color per
distance `1, ..., floor(t/2)`, and the blue graph is a circulant graph.

Input 4 = `5` searches **linear-distance** colorings. The color of `{i,j}` depends on the plain
difference `|i-j|`, so a coloring is one color per distance `1, ..., t-1`, and the blue graph is a
Toeplitz graph: constant along each diagonal of the adjacency matrix, with no wrap-around.

The circulant class sits inside this one. A jump set `J` at order `t` is the same coloring as the
distance set `J` together with `{t-j : j in J}`, so every circulant coloring is a linear-distance
coloring. The converse fails, because a linear-distance coloring may give different colors to `d`
and `t-d`. The search space is therefore strictly larger, and it has `t-1` binary variables instead
of `floor(t/2)`.

### The formulation

One binary variable `y_d` per distance `d = 1, ..., t-1`, equal to 1 when that distance is blue.
Writing `D(S)` for the set of distances occurring among the pairs of a vertex set `S`, the two lazy
families are the ones of the paper with the linear distance in place of the circular one:

```
sum over d in D(S) of y_d  <=  |D(S)| - 1      for every S with |S| = m      (no blue K_m)
sum over d in D(S) of y_d  >=  1               for every S with |S| = n      (no red  K_n)
```

The first says that not every distance inside `S` can be blue; the second, that at least one of them
must be. The Turán right-hand sides of the paper apply unchanged, and are used when input 8 is `1`.

### Separation

As in the paper. Given an integral candidate, the corresponding graph is built, a monochromatic
clique of the forbidden size is searched for, and if one is found the inequality of its distance set
is added.

> [!IMPORTANT]
> The clique routine's anchored reduction is now used by this model, but at an anchor **we** fix,
> not one the routine picks. The argument it takes is an `int`: `1` is the circulant behaviour of
> the paper, unchanged; `-1` is this model's mode, which anchors at vertex `t-1`; `0` disables the
> reduction. Passing `1` here is still wrong and still produces invalid colorings — measured, ten
> out of 154 brute-force-checked instances — because the routine would then choose the anchor after
> its own sorting and can land on an interior vertex. Input 6 remains forced to `0`; nothing in the
> command line selects the anchor. Full explanation below and in the `ANCHOR_MODE` note in
> [`source/RAMSEY_MODEL_5.cpp`](source/RAMSEY_MODEL_5.cpp).

One point deserves care, because it is easy to get backwards, and the answer is more specific than
it first looks. The clique routine used by the separator offers a reduction that anchors the search
at one vertex `v`, searching only inside `N(v)` and adding `v` to the answer:

```
omega(G) >= target    <=>    omega(N(v)) >= target - 1
```

That equivalence needs `v` to lie in some clique of the target size. In a **circulant** graph every
vertex does, by vertex-transitivity, so any anchor will do and the reduction is safe. In a
**Toeplitz** graph it holds at the two ends only: translating a clique by `-min(S)` puts one at
vertex `0`, translating by `t-1-max(S)` puts one at vertex `t-1`, and nothing of the sort holds for
the vertices in between. A small measured example, at `t = 11` with blue distances `{2,3}` and red
target 5: the red graph has a `K_5`, yet `1 + omega(N(v))` is only 4 for `v` in `{2,3,7,8}`, so at
such an anchor the reduction concludes that no `K_5` exists.

So anchoring is legitimate in this class — at `0` or at `t-1` — and the propagator of input 5 uses
exactly that, which is why it is complete. The separator now uses it too. What made the routine's
own reduction unusable was never the reduction but the choice of anchor: the routine sorts the graph
and then anchors at whatever sits last.

The fix keeps the sorting and removes the choice. The routine sorts the neighbourhood of one vertex,
and only the vertices inside that neighbourhood are permuted, so a vertex outside it cannot move. In
circulant mode it sorts `N(0)` and anchors at the last position, which is safe because every vertex
is a valid anchor. In this model's mode it sorts `N(t-1)` instead: `t-1` is never its own neighbour,
so it stays at the last position and is the anchor, where translation makes the reduction valid. All
of COPT's ordering, bounds, MNTS and CliSAT machinery is untouched — it simply no longer decides
which vertex is fixed.

The gain is the one the circulant case already had: the exact search runs on `G[N(t-1)]` for
`target - 1` instead of on the whole graph for `target`.

### Why this class behaves differently

Two properties matter, and neither holds for circulant colorings.

**It is hereditary.** Restricting a linear-distance coloring on `t` vertices to `0, ..., t-2` leaves
a linear-distance coloring, with the same variables: the induced graph uses only distances up to
`t-2` and inherits their colors. So feasibility is monotone in the order, and one infeasible order
settles the question — if no linear-distance `(m,n)`-coloring exists at order `t`, none exists at any
larger order. The circulant class is empty at some orders and non-empty at larger ones, which is
exactly why the paper's circulant results require excluding every order up to the known upper bound.

**Cuts survive a change of order.** A no-clique inequality separated at order `t` remains valid, with
the same coefficients, at every larger order: the vertex set still exists and its differences do not
change. So a pool recorded while solving order `t` can be loaded unchanged when solving order `t+1`.
Circulant pools cannot be reused this way, because the folding `d -> min(d, t-d)` depends on `t`.

**What is lost.** The rotations and the reflection of the circulant case are gone, and with them the
common-neighborhood reduction the paper's separator relies on. The variables double, the separator
must search over all vertices, and on instances where both forbidden cliques are small this
formulation is much slower than a direct exhaustive search. It is not a replacement for input 4 =
`3`; it answers a different question about a larger class.

---

## Part 2 — The parameters

Start from the default command of [Default RAMSEY parameters](DEFAULT-PARAMETERS.md) and change
input 4 from `3` to `5`. The command line is the same 33 inputs, documented in
[Run the solver](RUN.md). Below is every input whose meaning or behaviour is not simply inherited.

| Input | What it does in this model |
| ---: | --- |
| 4 | Selects the model. Use `5`. During development this formulation was `6`; that value is now refused with a message, rather than reinterpreted, so that an older command line cannot quietly run something else. |
| 6 | The circulant restriction. **Meaningless here and forced to `0`.** With it set, the CPLEX separation model constrains its clique search in a way that is written for the circulant case. The solver prints a notice and overrides the value, so no setting of input 6 can affect the search in this model. This input does not control the clique routine's anchor, which this model fixes at `t-1` on its own; see the warning above. |
| 5 | The partial-colouring propagator and the integral pre-check of [Optional search additions](NEW-OPTIONS.md), with the same values: `0` or `1` historic behaviour, `2` the propagator, `3` the propagator plus the pre-check. The exact test they use is different here: a Toeplitz graph contains a `K_k` if and only if the subgraph induced on its own distance set contains a `K_(k-1)`, because a clique can be translated so that its smallest vertex becomes 0. Unlike the circulant implementation there is **no limit on the graph order**. |
| 8 | Stronger cuts. `1` uses the Turán right-hand sides, `0` the weaker unit ones, exactly as documented. |
| 9 | The separation route. `0` uses the internal clique solver, `1` the CPLEX separation model. Both search over all vertices in this model. |
| 14 | Degree cuts. Their shape changes. In a circulant graph every vertex has the same degree and a single inequality suffices; here vertex `i` has blue neighbours at distance at most `i` below it and at most `t-1-i` above it, so each vertex gives a different inequality, coefficients are `0`, `1` or `2`, and the middle vertex gives the strongest one. As in the paper, using a known lower bound in place of a Ramsey number turns these into a restriction of the search space: an empty answer then certifies nothing. |
| 15 | Clique cuts on arithmetic progressions `{0, s, 2s, ...}`. All pairwise distances inside such a set are multiples of `s`, which gives the inequality; unlike the circulant case the progression must fit inside the interval, so `s` is bounded by the clique size and the order. |
| 16 | Cover cuts. `1` uses the support form of the two families, `0` the multiplicity form, in which a distance occurring several times inside `S` carries its multiplicity. The support form is the stronger of the two. |
| 18, 19 | Triangle and quadrangle constraints. Enumerated with 0 as the smallest vertex of the inducing set, which is legitimate by the translation argument above. No distance is folded, so the sums `s+u` and `s+u+v` appear directly rather than through a modular reduction. |
| 20 | The post-solve check. Recommended: it verifies the coloring found, and it does not assume anything about the class. |
| 31 | Cut files. `0` off, `1` loads a pool for this instance, `-100` records one. Pools of this model carry a `_dist` token in their file name so that they can never be mixed with circulant pools, whose coefficients mean something different. Because cuts are hereditary here, a recorded pool can be reused at a larger order by renaming the file to that order. |
| 32 | Cut minimization, with the values documented in [Optional search additions](NEW-OPTIONS.md). The strided variants `4` and `5` were tuned on circular distances; they remain valid here, but their rationale does not carry over. |

Every other input keeps its documented meaning.

### Example

A (3,3)-coloring on 5 vertices, 60-second limit, post-solve check on:

```bash
./RAMSEY 5 3 3 5 1 0 60 1 0 1 60 10 200000 0 1 1 -1 1 1 1 0 0 0 0 1 0 0 1 0 1 0 1 1
```

### Output

The coloring is written to `colorings/col_m<m>_n<n>_SIZE<t>_dist_br<b>_id<id>.txt`. Its `JUMPS` line
lists the **blue linear distances**, one-based; the `MATRIX` block is the full adjacency matrix, and
is Toeplitz rather than circulant. Everything else in [Reading the output](OUTPUT.md) is unchanged.

For an independent verification, build a graph certificate from the coloring and run the
repository's [(m,n)-coloring checker](../checker/README.md), as
[Reading the output](OUTPUT.md) describes: write the blue graph in DIMACS edge format, using the
`MATRIX` block, and pass it to the checker. **Set the checker's circulant-reduction input to `0`.**
A coloring of this class need not be circulant, and the checker will report it as `NON-CIRC`; that
is expected here and is not a failure. Everything else about the checker is unchanged.

### Validation

The formulation was checked against two programs written for the purpose and sharing no reasoning
with it: a witness checker whose clique search ranges over all vertices with no structural shortcut,
and an exhaustive enumerator of linear-distance colorings, itself validated against brute-force
enumeration of all `2^(t-1)` distance sets. For every pair with `2 <= m <= n <= 6` and every order up
to 13 the solver matches brute force, and every coloring it produced was accepted by the checker.

One outcome of that campaign is worth stating, because it says what the larger class is worth in
practice: on the pairs decided so far the linear-distance threshold equals the circulant one in every
case but `(4,8)`, where it exceeds it by one, and no new lower bound on any classical Ramsey number
followed.

#### The fixed anchor

The change of the clique routine's anchor was validated separately, since it touches the exact
separation itself rather than the formulation.

| Check | Result |
| --- | --- |
| The `t=11`, blue `{2,3}`, red target 5 counterexample | the anchored mode returns a valid `K_5`; the circulant mode still returns "not found", which is why it must not be used here |
| Every Toeplitz graph for `t = 5..12` — all `2^(t-1)` distance sets, targets 3 to 7 — against an independent brute-force `omega` | 20,400 cases, no mismatch, and every returned clique verified pairwise in original vertex numbering |
| Circulant regression, `t = 5..16`, all jump sets, targets 3 to 6, before and after the change | 3,024 cases, output identical |
| The exhaustive small-instance suite above, rerun end to end | 364 instance-arm combinations, no mismatch, every certificate accepted by the checker |

On the clique call itself the anchored mode is 3 to 7 times faster than the unreduced one when the
target does not exist, which is the case the exact search has to work for. How much of that reaches
the solver depends on the instance: across the recorded campaigns only about 4% of separation calls
reach the exact search at all, the rest being answered by the heuristics, so runs dominated by the
branch-and-bound tree rather than by exact separation are not expected to change materially.
