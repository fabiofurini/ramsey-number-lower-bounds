# Solver source (reference only, not buildable as-is)

This directory contains the original C++ source of the RAMSEY solver: the branch-and-cut driver, the model formulations, the CPLEX clique-separation glue, the tabu search added after the paper, and the independent solution checker called from within a run.

| File | Role |
| --- | --- |
| `Main.cpp` | Entry point; parses the 33 positional inputs and dispatches to the selected model. It also recognises a shorter 18-input command line, which selects the tabu search; the two are told apart by the argument count. |
| `global_variables.h`, `global_functions.cpp`/`.h` | Shared state and utility routines used across models. |
| `CLIQUE_CPLEX.cpp`/`.h` | CPLEX lazy-constraint callback and clique-cut separation. |
| `RAMSEY_MODEL_1.cpp`/`.h` | Model 1 — full edge-variable formulation. Used for the paper's published results. |
| `RAMSEY_MODEL_3.cpp`/`.h` | Model 3 — projected distance-space (circulant) formulation. Used for the paper's published results, and one of the two models carrying the optional additions of [`../NEW-OPTIONS.md`](../NEW-OPTIONS.md) (partial-colouring propagator, integral pre-check, cut-minimization variants), all off in the default configuration. |
| `RAMSEY_MODEL_2.cpp`/`.h`, `RAMSEY_MODEL_4.cpp`/`.h` | Earlier variants of Models 1 and 3, retained for completeness because they are compiled into the same shipped `RAMSEY.dynamic.o`; not part of the documented public workflow (see [`../README.md`](../README.md)). |
| `RAMSEY_MODEL_5.cpp`/`.h` | Model 5 — projected **linear**-distance (Toeplitz) formulation, over a class strictly larger than the circulant one. Added after the paper; documented in [`../DISTANCE-MODEL.md`](../DISTANCE-MODEL.md). |
| `RAMSEY_TABU_SEARCH.cpp`/`.h` | The driver of the tabu search: builds the support pool, calls the separator and the exact verification, writes the certificate. Not a formulation; documented in [`../TABU-SEARCH.md`](../TABU-SEARCH.md). |
| `RAMSEY_TABU_CORE.cpp`/`.h` | The search itself: incremental scoring of a one-distance flip, tabu tenure, stagnation and perturbation, optional adaptive weights. |
| `check_solution.cpp`/`.h` | Independent post-solve verification of a found coloring. |

## Why this can't be built from this repository alone

Every file here includes headers from **BitGraph** and **coptBG**, the graph and clique-search libraries by Pablo San Segundo (CSIC-UPM). Those libraries are GPL-3.0 and, as explained in [`../../NOTICE.md`](../../NOTICE.md), their source is not published in this repository. Without them, nothing here compiles.

This directory exists for a narrower purpose: to let a reader inspect exactly how the paper's formulations, cuts, and branching are implemented, and to make the method easier to verify and reproduce by reading — even without a local CPLEX/coptBG setup.

To actually run the solver, use the precompiled object and linking scripts in [`../solver/`](../solver/), documented in [`../INSTALL.md`](../INSTALL.md), [`../RUN.md`](../RUN.md), and [`../OUTPUT.md`](../OUTPUT.md). That object file is what `RAMSEY.dynamic.o` (and `RAMSEY.dynamic.glibc228.o`) already contain compiled together with coptBG — this directory is the same application code in readable source form.

## License

Like the rest of [`code/`](../), this directory is licensed under the [GNU General Public License v3.0 or later](../../LICENSE).
