# Recording and reusing cut pools (input 31)

This page documents behaviour that is **not** part of the paper. Input 31 is `0` in the published
configuration, which means no cut file is read and none is written, and with that value the solver
behaves exactly as before.

The page has two parts. The first explains what the feature does and which models support it. The
second explains the values of the input and the file format.

> [!IMPORTANT]
> Only the two distance models install a cut pool: input 4 = `3` and input 4 = `5`. The rows are
> indexed by distance variables, which the edge-space models do not have. Asking for a pool with any
> other model prints a warning and the pool is ignored; asking to record one is refused outright.

---

## Part 1 — What the feature does

### The cuts in question

Both distance models separate their two families lazily: given an integral candidate, a
monochromatic clique is searched for, and the inequality of its distance set is added. Those
separated inequalities are the expensive part of a run — each one costs a clique search — and they
are thrown away when the process exits.

Input 31 lets a run write them down, and lets a later run start with them already in the
formulation instead of separating them again.

### What is stored, and why it is not just the coefficients

A recorded row holds two things: the **support**, that is the set of distances the inequality is
over, and the **generating clique**, the vertex set the inequality came from. Storing the clique is
what makes the row reconstructible rather than merely readable. From the clique the loader recovers

- how many times each distance occurs inside it, hence the multiplicity form of the inequality;
- the size of the clique, hence the Turán right-hand side, which depends on that size and on the
  forbidden clique size, and the weaker unit right-hand side as well.

So one recorded row can be reinstalled in whichever of the two strengths the loading run asks for,
rather than being frozen in the strength that produced it. The loader also checks the row for
consistency: the number of pairs must be that of a complete graph on some number of vertices, and
that number must be at least the forbidden clique size. A row that fails either check stops the run
rather than being silently skipped.

### Reuse across orders, with input 4 = `5`

For the circulant model, input 4 = `3`, a pool belongs to the order that produced it. The circular
distance `min(d, t-d)` depends on `t`, so the same coefficients mean something different at another
order, and pools must not be moved.

For the linear-distance model, input 4 = `5`, they can. A no-clique inequality separated at order
`t` stays valid, with the same coefficients, at every larger order: the vertex set still exists and
its differences do not change. See [The linear-distance model](DISTANCE-MODEL.md). So a pool
recorded while solving order `t` can be loaded when solving order `t+1`, by renaming the file to the
target order; nothing in its contents changes. Measured on small instances, this returns the same
verdict with fewer nodes and roughly half the separation calls.

That is the only place where the two distance models differ in what this feature is good for, and it
follows from the geometry rather than from the implementation.

---

## Part 2 — The input and the file format

| Value of input 31 | Effect |
| ---: | --- |
| `0` | The published default. No file is read and none is written. |
| `1` | Before solving, read the pool for this instance and add its rows to the formulation. If the file is absent the run says so and continues without it. With a model other than `3` or `5` a warning is printed and the pool is ignored. |
| `-100` | Record, while solving, every distinct support that separation produces, together with its generating clique. Supported by models `3` and `5` only; with any other model the run stops with a message. |

Both values are experimental and off by default. Recording adds an append per newly separated
support, which is negligible next to the clique search that produced it.

### File names

Two files per run, one per colour, written to and read from `CUTS/` relative to the working
directory:

```
CUTS/t<t>_m<m>_n<n>_id<id>_blue.txt          input 4 = 3
CUTS/t<t>_m<m>_n<n>_id<id>_red.txt

CUTS/t<t>_m<m>_n<n>_id<id>_dist_blue.txt     input 4 = 5
CUTS/t<t>_m<m>_n<n>_id<id>_dist_red.txt
```

The `_dist` token exists so that a pool of the linear-distance model can never be loaded as a
circulant pool, or the other way round: the file names simply do not collide. Note that `<id>` is
input 33, so a loading run must use the same run identifier as the run that recorded the pool, or
rename the file.

### File contents

A one-line header, then one row per recorded support:

```
# RAMSEY_DISTANCE_CLIQUE_V1
1 8 9 | 5 13 14
1 10 11 | 3 13 14
```

Left of the bar, the distances of the support, sorted and deduplicated. Right of the bar, the
vertices of the generating clique, numbered from 1. The two rows above come from a real
`(3,6)` run at order 16.

### Reusing a pool at a larger order

Only with input 4 = `5`. Record at order `t`, then rename both files to the target order and load:

```bash
./RAMSEY 16 3 6 5 1 0 300 1 0 1 300 10 200000 0 1 1 -1 0 1 1 0 0 0 0 1 0 0 1 0 1 -100 1 4242
for c in blue red; do
  cp CUTS/t16_m3_n6_id4242_dist_$c.txt CUTS/t17_m3_n6_id4242_dist_$c.txt
done
./RAMSEY 17 3 6 5 1 0 300 1 0 1 300 10 200000 0 1 1 -1 0 1 1 0 0 0 0 1 0 0 1 0 1 1 1 4242
```

The second run must return the same status it would without the pool. Nothing in the rows is edited;
the rename only tells the solver which file to open.
