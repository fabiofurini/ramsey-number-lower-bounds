# Running the paper's models

This directory contains the executable package used to run the mathematical models developed in the paper. It can be used to search for new lower bounds on Ramsey numbers and to compute circulant Ramsey numbers.

Two further options were added after the paper and are documented separately: a formulation over a larger class of colorings, and a heuristic. Both are off unless input 4 selects them, and the paper's configuration is unchanged.

The package requires a 64-bit Linux system and a local installation of IBM ILOG CPLEX Studio 20.1. The supplied solver object must first be linked with CPLEX to create the `RAMSEY` executable.

The large-scale experiments reported in the paper used CPLEX 22.1.0.0. The distributed precompiled object and linking scripts currently target CPLEX Studio 20.1; this package requirement is distinct from the version used to produce the reported campaign results.

## Documentation

1. [Install the executable](INSTALL.md): requirements, CPLEX setup, and compilation check.
2. [Run the solver](RUN.md): model selection, command-line inputs, and an example.
3. [Read the output](OUTPUT.md): generated colorings, summaries, checks, and visualisations.
4. [Optional search additions](NEW-OPTIONS.md): what inputs 5 and 32 can switch on beyond the
   paper's configuration, what each setting does, when it helps and when it does not. The default
   configuration is unchanged, so this page is only needed if you want to depart from it.
5. [The linear-distance model](DISTANCE-MODEL.md): input 4 = `5`, a formulation over a class
   strictly larger than the circulant one, in which the color of an edge depends on `|i-j|` rather
   than on the circular distance. Added after the paper; off unless selected.
6. [The tabu search](TABU-SEARCH.md): input 4 = `6`, a heuristic over the circulant distance space
   with its own shorter command line. It can produce a certificate but never an exclusion. Added
   after the paper; off unless selected.

The solver produces graph colorings. A resulting graph certificate can be verified independently with the repository's [(m,n)-coloring checker](../checker/README.md).

## Solver source

A second, independent engine for the circulant question — an exhaustive search over circular-distance
orbits, with no LP — is in [`../circulant-engine/`](../circulant-engine/README.md); it reimplements
the algorithm of Goedgebeur and Van Overberghe, who are credited there, and we use it to cross-check
the branch-and-cut.

[`source/`](source/) contains the original C++ source of the solver (entry point, model formulations, CPLEX cut callback, solution checker) for readers who want to inspect how the method is implemented. It is not buildable on its own — see [`source/README.md`](source/README.md) for why — and is not a substitute for the executable package documented above.
