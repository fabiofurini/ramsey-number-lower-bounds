# The optional search additions

This page documents the solver behaviour that is **not** part of the paper's default
configuration. Everything described here is switched off unless you ask for it, and the default
command in [Default RAMSEY parameters](DEFAULT-PARAMETERS.md) is unchanged: with those values the
solver follows exactly the same path as before, node for node.

Two inputs govern the additions:

- input **5**, which enables the partial-colouring propagator and the integral pre-check;
- input **32**, which selects how a separated cut is minimised.

Both act only on the distance model (input 4 = `3`), the formulation used for the circulant results.

---

## Input 5: partial-colouring propagator and integral pre-check

### The rule they both use

In a circulant colouring the colour of an edge depends only on the circular distance
`d = min(|i-j|, t-|i-j|)`, so a colouring is a choice of one colour per distance
`d = 1, ..., floor(t/2)`. Assigning a colour to `d` colours the whole orbit `{i, i+d}` at once.

If assigning distance `d` to colour `c` creates a monochromatic `K_k` in colour `c` that was not
there before, that clique must use an edge of the new orbit, and by rotating and reflecting we may
assume it uses the edge `{0, d}`. Its remaining `k-2` vertices then form a clique of colour `c`
inside the graph induced on the common neighbourhood `N_c(0) ∩ N_c(d)`.

So a new monochromatic `K_k` exists **if and only if** there is a `K_(k-2)` in that induced
subgraph. The test is exact, not heuristic, and it is local: it inspects one small induced graph
instead of the whole colouring. The implementation works on machine-word bitsets, so a single test
costs microseconds. Because it is anchored at the pair `{0, d}`, this page calls it the *anchored
test*.

### `5 = 1` (default) and `5 = 0`

Historical behaviour: nothing is registered, no additional callback is installed, no additional
statistics are printed. Any value other than `2` or `3` behaves this way, so old command lines are
unaffected.

### `5 = 2` — partial-colouring propagator

A branch callback is registered. At every node it reads the distance variables that are already
**fixed** to a colour by branching — it never treats a fractional LP value as a colour — and applies
the anchored test to that fixed part. If the fixed distances alone already force a monochromatic
`K_m` or `K_n`, no completion of that subtree can be a valid colouring, so the node is closed and no
children are created.

The propagator prunes the tree; it adds nothing to the formulation. Measured cost is about four
microseconds per call. It is active for graph orders up to 127.

### `5 = 3` — propagator plus integral pre-check

Everything in `5 = 2`, and in addition: when the LP returns an **integral** candidate — every
distance `0` or `1`, hence a complete colouring — the anchored test is run over all distances of both
colours before the general clique separator is called. On a complete colouring that test is a
complete detector of a monochromatic `K_k`, and it returns the offending clique, which is then used
to build the usual cut. The general maximum-clique separator is therefore bypassed for that
candidate.

The replacement is enabled only where it is exactly equivalent to the request it replaces:

- input 25 = `1` (the separator is asked for a clique of the required size, not for a maximum clique);
- input 9 ≠ `1` (internal separation route, not the CPLEX separation model);
- input 32 = `0` (no cut minimisation);
- graph order at most 127.

If any of these does not hold, the pre-check stays out of the way and the general separator remains
in charge — in particular, `5 = 3` combined with `32 > 0` behaves exactly like `5 = 2`.

### Which one to use

The two options solve different problems: `5 = 2` reduces how many nodes are visited, `5 = 3` also
removes the cost of the general separator at candidate solutions. Their usefulness depends on the
size of the forbidden cliques, and the effect is large in both directions:

| instance family | effect of `5 = 3` against the default |
| --- | --- |
| both targets small, e.g. (5,5), (5,6), (6,6) | dramatic gain: on (5,5) the emptiness proof at order 42 goes from a run unfinished in 60 s to 0.39 s; a (6,6) colouring at order 71 is found in 0.09 s instead of about 156 s |
| moderate targets, e.g. (4,7), (4,8) | roughly neutral to mildly worse |
| one large target, e.g. (3,14) and beyond | progressively worse; on the (3,16) exclusion range it costs about an order of magnitude more time than the default |

The reason is visible in the rule above: the anchored test must look for cliques of size up to
`n-2`, so once `n` is large the general heuristic separator does that job better. As a practical
recipe: use `5 = 3` when both `m` and `n` are small, use `5 = 2` when you want tree pruning without
touching separation, and keep the default for large `n`.

---

## Input 32: cut minimisation

After a violated clique or jump inequality is separated, its support can often be reduced: removing
some distances keeps the inequality valid and makes the row sparser and stronger. Input 32 selects
how much effort goes into that reduction.

| value | behaviour |
| --- | --- |
| `0` | no minimisation; the cut is added exactly as separated |
| `1` | **default**: heuristic minimisation, repeated until a full pass removes nothing |
| `2` | exact minimisation (heuristic plus exact clique check), same repeat-until-fixpoint loop |
| `3` | heuristic minimisation, a single pass |
| `4` | as `3`, but on the upper half of the candidate distances only every other one is tried |
| `5` | as `4`, and the lower half is also strided while the upper half is skipped |

Values `3`, `4` and `5` exist because the removal attempts are not equally likely to succeed: in our
measurements the lower half of the distance range has a success rate about 1.4 to 1.5 times higher
than the upper half, so a full repeated sweep spends much of its time on attempts that fail. All
three thresholds scale with the number of distance variables; no constant is hard-coded.

On the circulant exclusion ranges we have measured so far, `5` and `4` are the fastest — about 0.82
and 0.87 of the default's total time on paired instances — with `3` close to the default, while
switching minimisation off entirely (`0`) costs roughly a factor of two. Minimisation therefore pays
for itself, but its default repeat-to-fixpoint loop is more effort than necessary.

---

## Statistics printed with the additions

With input 5 set to `2` or `3` a run prints, in addition to the usual output:

```
pp_calls              propagator calls
pp_prunes_blue        nodes closed because the fixed part forced a blue clique
pp_prunes_red         the same for red
pp_avg_fixed_at_prune average number of fixed distances when a node was closed
pp_max_fixed_at_prune the maximum of the same quantity
pp_avg_depth_at_prune average tree depth at a prune
pp_fast_calls         integral candidates screened by the pre-check
pp_fast_hits_blue     blue cliques found by the pre-check
pp_fast_hits_red      the same for red
time_propagator       total seconds inside the propagator and the pre-check
```

The cut-minimisation counters (`n_jumps_minimized_*`, `n_minimization_successes_*`,
`time_minimization`, and the low/high half breakdown) are printed whenever input 32 is non-zero.

---

## Reproducing the default exactly

The additions were built so that the published configuration is bit-for-bit unaffected. With the
default command, the pristine and the current code produce identical CPLEX logs, including the same
node sequence; this was checked on the (6,6) instances at orders 71 and 72 with both binaries built
by the same compiler. If you are reproducing the paper, use the default command as given in
[Default RAMSEY parameters](DEFAULT-PARAMETERS.md) and ignore this page.
