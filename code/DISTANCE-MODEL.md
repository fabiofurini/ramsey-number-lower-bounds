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

So anchoring is legitimate in this class — at `0` or at `t-1`, and the propagator of input 5 uses
exactly that, which is why it is complete. What cannot be used is the routine's own reduction,
because the anchor is chosen by its internal ordering rather than by us. This model therefore asks
that routine for a general search over all vertices. The consequence is a cost, not a loss of
strength: the separator does more work than in the circulant case.

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
| 6 | The circulant restriction. **Meaningless here and forced to `0`.** With it set, the CPLEX separation model constrains its clique search in a way that is written for the circulant case. The solver prints a notice and overrides the value, so no setting of input 6 can affect the search in this model. |
| 5 | The partial-colouring propagator and the integral pre-check of [Optional search additions](NEW-OPTIONS.md), with the same values: `0` or `1` historic behaviour, `2` the propagator, `3` the propagator plus the pre-check. The exact test they use is different here: a Toeplitz graph contains a `K_k` if and only if the subgraph induced on its own distance set contains a `K_(k-1)`, because a clique can be translated so that its smallest vertex becomes 0. Unlike the circulant implementation there is **no limit on the graph order**. |
| 8 | Stronger cuts. `1` uses the Turán right-hand sides, `0` the weaker unit ones, exactly as documented. |
| 9 | The separation route. `0` uses the internal clique solver, `1` the CPLEX separation model. Both search over all vertices in this model. |
| 14 | Degree cuts. Their shape changes. In a circulant graph every vertex has the same degree and a single inequality suffices; here vertex `i` has blue neighbours at distance at most `i` below it and at most `t-1-i` above it, so each vertex gives a different inequality, coefficients are `0`, `1` or `2`, and the middle vertex gives the strongest one. As in the paper, using a known lower bound in place of a Ramsey number turns these into a restriction of the search space: an empty answer then certifies nothing. |
| 15 | Clique cuts on arithmetic progressions `{0, s, 2s, ...}`. All pairwise distances inside such a set are multiples of `s`, which gives the inequality; unlike the circulant case the progression must fit inside the interval, so `s` is bounded by the clique size and the order. |
| 16 | Cover cuts. `1` uses the support form of the two families, `0` the multiplicity form, in which a distance occurring several times inside `S` carries its multiplicity. The support form is the stronger of the two. |
| 18, 19 | Triangle and quadrangle constraints. Enumerated with 0 as the smallest vertex of the inducing set, which is legitimate by the translation argument above. No distance is folded, so the sums `s+u` and `s+u+v` appear directly rather than through a modular reduction. |
| 20 | The post-solve check. Recommended: it verifies the coloring found, and it does not assume anything about the class. |
| 31 | Cut files. `0` off, `1` loads a pool for this instance, `-100` records one. Pools of this model carry an `_od` token in their file name so that they can never be mixed with circulant pools, whose coefficients mean something different. Because cuts are hereditary here, a recorded pool can be reused at a larger order by renaming the file to that order. |
| 32 | Cut minimization, with the values documented in [Optional search additions](NEW-OPTIONS.md). The strided variants `4` and `5` were tuned on circular distances; they remain valid here, but their rationale does not carry over. |

Every other input keeps its documented meaning.

### Example

A (3,3)-coloring on 5 vertices, 60-second limit, post-solve check on:

```bash
./RAMSEY 5 3 3 5 1 0 60 1 0 1 60 10 200000 0 1 1 -1 1 1 1 0 0 0 0 1 0 0 1 0 1 0 1 1
```

### Output

The coloring is written to `colorings/col_m<m>_n<n>_SIZE<t>_od_br<b>_id<id>.txt`. Its `JUMPS` line
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
