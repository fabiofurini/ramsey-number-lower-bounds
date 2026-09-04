# Default RAMSEY parameters

This file records the public default configuration of the `RAMSEY`
solver. These values correspond to the baseline configuration reported
in the accompanying paper. They are intended for reproducible runs with
the distance-space formulation. The graph order `t`, the forbidden red
clique size `n`, and `test_id` are instance-specific; all other values
are fixed.

Run the solver from `code/solver/`:

```bash
./RAMSEY <t> 3 <n> 3 1 1 7200 1 0 1 7200 10 200000 0 1 1 -1 1 1 0 0 0 0 0 1 0 0 1 0 1 0 1 <test_id>
```

| # | Default value | Meaning of this value |
| ---: | --- | --- |
| 1 | instance-specific | Order of the complete graph to be coloured. |
| 2 | `3` | Avoid a blue clique of order 3. |
| 3 | instance-specific | Avoid a red clique of the requested order. |
| 4 | `3` | Select the distance-space formulation. |
| 5 | `1` | Published behaviour. Values `2` and `3` switch on the optional search additions (partial-colouring propagator, integral pre-check); see [Optional search additions](NEW-OPTIONS.md). |
| 6 | `1` | Impose the circulant restriction. |
| 7 | `7200` | Allow at most 7,200 seconds for the clique routine. |
| 8 | `1` | Use the exact Tur\'an right-hand side in clique cuts. |
| 9 | `0` | Use the internal clique-separation route rather than a CPLEX separation model. |
| 10 | `1` | Enable the MNTS clique-search heuristic. |
| 11 | `7200` | Give an MNTS call at most 7,200 seconds. |
| 12 | `10` | Use 10 MNTS restarts. |
| 13 | `200000` | Allow 200,000 iterations in each MNTS restart. |
| 14 | `0` | Do not add Kalfleisch inequalities. |
| 15 | `1` | Add arithmetic-progression clique cuts. |
| 16 | `1` | Apply coefficient reduction to strengthened distance-space cuts. |
| 17 | `-1` | Disable the cut-loop option. |
| 18 | `1` | Add triangle inequalities. |
| 19 | `1` | Add quadrangle inequalities. |
| 20 | `0` | Do not run the post-solve solution checker. |
| 21 | `0` | Force each separated cut into the relaxation. |
| 22 | `0` | Use the default branching strategy. |
| 23 | `0` | Use depth-first tree exploration. |
| 24 | `0` | Leave CPLEX heuristic frequency at its automatic setting. |
| 25 | `1` | Search for the required clique size rather than a maximum clique. |
| 26 | `0` | Keep a returned larger clique unchanged. |
| 27 | `0` | Keep only the first applicable cut from a larger clique. |
| 28 | `1` | Use seed 1. |
| 29 | `0` | Use CPLEX's default variable-selection rule. |
| 30 | `1` | Run each solver process with one CPLEX thread. |
| 31 | `0` | No cut-file input or output. |
| 32 | `1` | Minimize a separated cut before adding it, repeating until a full pass removes nothing. Values `2`--`5` select other minimization strategies; see [Optional search additions](NEW-OPTIONS.md). |
| 33 | instance-specific | Identify the run and its output files. |

The values above are the baseline of the paper and remain the default. The search additions
documented in [Optional search additions](NEW-OPTIONS.md) are reached only by changing input 5 or
input 32; with the values in this table the solver follows the published path exactly, node for
node.

The baseline enables MNTS. MNTS was set to `0` only in the ablation
experiment, so that the other features could be assessed with an exact
CliSAT fallback. That experiment-specific setting does not replace the
baseline values above.
