# Reading the solver output

The solver prints progress to the terminal and writes its results below `code/solver/`. A coloring and its visualisations are produced only when a feasible solution is found.

## Main output

| Location | Contents |
| --- | --- |
| `colorings/` | The colorings found by feasible runs. |
| `info_RAMSEY.txt` | One tab-separated summary row for every invocation. Its columns are documented below. |
| `SOLUTION_FILES/` | A separate summary for each test ID. |
| `info_CHECK_SOLUTION.txt` | Verification summaries when solution checking is enabled. |
| `figures/` | LaTeX drawings generated for accepted solutions. |
| `matrices/` | LaTeX matrix visualisations generated for accepted solutions. |

Output filenames contain the instance settings and the test ID. Use a different test ID for every run; reusing one may overwrite its per-run files.

## `info_RAMSEY.txt`

This file has no header. Each invocation appends one tab-separated row. The row first contains the solver statistics listed below and then repeats all 33 positional inputs in the order documented in [`RUN.md`](RUN.md).

### Statistics present for every run

| Column | Meaning |
| ---: | --- |
| 1 | CPLEX objective value at termination. The models are feasibility problems, so this is normally zero. |
| 2 | Best objective bound reported by CPLEX at termination. |
| 3 | Numeric CPLEX solution status. |
| 4 | Number of branch-and-bound nodes explored by CPLEX. |
| 5 | Solver CPU time reported for the CPLEX optimization, in seconds. |
| 6 | Number of model columns at termination. |
| 7 | Number of model rows at termination. |
| 8 | Time spent in the CPLEX-based clique routine, in seconds. |
| 9 | Time spent in the alternative branch-and-bound clique routine, in seconds. |
| 10 | Number of clique-separation calls for the red graph. |
| 11 | Number of clique-separation calls for the blue graph. |
| 12 | Number of ordinary red cuts added. |
| 13 | Number of ordinary blue cuts added. |
| 14 | Number of strengthened red cuts added. |
| 15 | Number of strengthened blue cuts added. |
| 16 | Number of clique-jump cuts added for the blue target. |
| 17 | Number of clique-jump cuts added for the red target. |
| 18 | Number of triangle cuts added for the blue target. |
| 19 | Number of triangle cuts added for the red target. |
| 20 | Number of quadrangle cuts added for the blue target. |
| 21 | Number of quadrangle cuts added for the red target. |
| 22 | Total number of calls to the clique routines. |
| 23 | Number of successful calls to the simple clique heuristic. |
| 24 | Number of successful MNTS calls. |
| 25 | Total time spent in MNTS, in seconds. |
| 26 | Number of successful CliSAT calls. |
| 27 | Number of CliSAT calls completed to optimality. |

### Additional statistics for the distance model

Rows produced with the distance model contain six additional columns before the input vector:

| Column | Meaning |
| ---: | --- |
| 28 | Number of heuristic calls made by the distance-model procedure. |
| 29 | Number of blue circular distances considered during cut minimization. |
| 30 | Number of red circular distances considered during cut minimization. |
| 31 | Number of successful blue minimizations. |
| 32 | Number of successful red minimizations. |
| 33 | Time spent in cut minimization, in seconds. |

For an edge-model row, the copy of positional input 1 starts at column 28. For a distance-model row, it starts at column 34. In both cases, the last 33 columns reproduce positional inputs 1--33 exactly as supplied to the executable.

The file `SOLUTION_FILES/ID_TEST<id>.sol` contains the same row for one test ID. Reusing an ID overwrites this per-run file, whereas `info_RAMSEY.txt` retains its appended history.

## Verification summary

When positional input 20 is `1`, `info_CHECK_SOLUTION.txt` receives a tab-separated row with no header:

| Column | Meaning |
| ---: | --- |
| 1 | Largest blue clique found by the solution check. |
| 2 | Time spent checking the blue graph, in seconds. |
| 3 | Largest red clique found by the solution check. |
| 4 | Time spent checking the red graph, in seconds. |
| 5 | Check result: `1` means that the coloring was accepted; `0` means that it was rejected. |

These five values are followed by the complete 33-input vector. The corresponding file in `SOLUTION_FILES/` stores the same verification row for one test ID.

## Counterexamples

When the solver finds a feasible (m,n)-coloring on `t` vertices, the counterexample is written to:

```text
code/solver/colorings/col_m<m>_n<n>_SIZE<t>_circulant<c>_br<b>_id<id>.txt
```

The filename records the blue target `m`, red target `n`, graph order `t`, circulant setting, branching setting, and test ID. The coloring proves the lower bound R(m,n) ≥ t+1 once its clique conditions have been verified.

### Counterexamples from the edge model

The file contains:

1. the graph order `t` on the first line;
2. a `t` by `t` adjacency matrix, with entries separated by spaces.

For every pair of distinct vertices, `1` means that the edge is blue and `0` means that it is red. The diagonal entries are zero.

### Counterexamples from the distance model

The file contains:

1. the graph order `t` on the first line;
2. the line `JUMPS`;
3. one line containing the selected zero-based distance indices;
4. the line `MATRIX`;
5. the complete `t` by `t` coloring matrix.

A printed jump index `i` represents circular distance `i+1`. In this format, the listed jumps and matrix entries equal to `1` are red, while entries equal to `0` are blue. Diagonal zeros do not represent edges.

The summary files allow a counterexample to be matched with the complete command that produced it. Keep the command, coloring, and summary together when recording a computational result.

## Independent verification

The solver's own solution check is useful during a run. For an independent verification of a reported result, create a graph certificate from the coloring and use the repository's [(m,n)-coloring checker](../checker/README.md).
