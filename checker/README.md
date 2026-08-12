# Independent (m,n)-coloring checker

`Ramsey_checker` verifies independently whether a graph certificate is an (m,n)-coloring. It checks that the blue graph contains no clique of size `m` and that the red graph contains no clique of size `n`. A successful check certifies the lower bound R(m,n) ≥ t+1, where `t` is the number of vertices in the certificate.

The checker does not require the RAMSEY solver, CPLEX, or COPT. The repository includes a 64-bit Linux executable (`Ramsey_checker`), a variant for older systems (`Ramsey_checker_glibc228`, glibc 2.28 or newer, no dependency on the system's libstdc++ version), and a macOS executable (`Ramsey_checker_macOS`).

This directory is licensed under the [GNU General Public License v3.0 or later](../LICENSE); see [`../NOTICE.md`](../NOTICE.md) for why.

Before running the checker, if the certificate is claimed to be circulant, the user must verify that the graph is circulant with respect to the vertex ordering provided in the certificate. `Ramsey_checker` verifies only the (m,n)-coloring property.

## Run the checker

From the repository root, use:

```text
./checker/Ramsey_checker <certificate.clq> <timeout> <blue-target> <red-target> <circulant-reduction> <output-file>
```

The six positional inputs are:

| # | Effect and possible values |
| ---: | --- |
| 1 | Path to the graph certificate. The stored graph is red and its complement is blue. |
| 2 | Maximum time in seconds for each of the two clique searches; use a positive integer. A timeout does not certify the coloring. |
| 3 | Forbidden clique size in the blue graph: the first parameter of R(m,n). |
| 4 | Forbidden clique size in the red graph: the second parameter of R(m,n). |
| 5 | Reserved search setting; use `0`. |
| 6 | Path of the tab-separated output file. The checker appends one result row. |

For example, the following command checks the published R(3,37) certificate without assuming that it is circulant:

```bash
./checker/Ramsey_checker \
  lower-bounds/certificates/R_3_37_graphSize_271.clq \
  3600 3 37 0 /tmp/r337-check.tsv
```

On macOS, replace `./checker/Ramsey_checker` with `./checker/Ramsey_checker_macOS`.

## Input certificate

The checker reads a graph in DIMACS edge format with vertices numbered from `1` to `t`:

```text
c Optional comment
p edge <number-of-vertices> <number-of-edges>
e <u> <v>
e <u> <v>
...
```

The stored edges form the red graph; all missing edges between distinct vertices form the blue graph. For a certificate of R(m,n), the checker searches for a blue clique of size `m` and a red clique of size `n`.

## Output

The checker prints progress to the terminal and appends one tab-separated row to the file supplied as input 6. The output file has no header and contains 16 columns:

| Column | Meaning |
| ---: | --- |
| 1 | Certificate filename. |
| 2 | Forbidden clique size searched in the blue graph. |
| 3 | Time spent reading the blue graph, in seconds. |
| 4 | Time spent preprocessing the blue graph, in seconds. |
| 5 | Time spent searching the blue graph, in seconds. |
| 6 | Blue-search status: `OK` or `TIMEOUT`. |
| 7 | Search mode reported by the executable. |
| 8 | A blue target clique if one is found; otherwise `{}`. Reported vertices use zero-based numbering. |
| 9 | Forbidden clique size searched in the red graph. |
| 10 | Time spent reading the red graph, in seconds. |
| 11 | Time spent preprocessing the red graph, in seconds. |
| 12 | Time spent searching the red graph, in seconds. |
| 13 | Red-search status: `OK` or `TIMEOUT`. |
| 14 | Search mode reported by the executable. |
| 15 | A red target clique if one is found; otherwise `{}`. Reported vertices use zero-based numbering. |
| 16 | Final verdict: `VALID` or `NOT-VALID`. |

`VALID` means that both searches finished without timeout, no forbidden blue clique was found, and no forbidden red clique was found. Any timeout or forbidden clique produces `NOT-VALID`.

If `Ramsey_checker` reports a missing `GLIBC_*` or `GLIBCXX_*` version, use `Ramsey_checker_glibc228` instead — same usage, same output format.
