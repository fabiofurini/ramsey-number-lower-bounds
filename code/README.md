# Running the paper's models

This directory contains the executable package used to run the mathematical models developed in the paper. It can be used to search for new lower bounds on Ramsey numbers and to compute circulant Ramsey numbers.

The package requires a 64-bit Linux system and a local installation of IBM ILOG CPLEX Studio 20.1. The supplied solver object must first be linked with CPLEX to create the `RAMSEY` executable.

The large-scale experiments reported in the paper used CPLEX 22.1.0.0. The distributed precompiled object and linking scripts currently target CPLEX Studio 20.1; this package requirement is distinct from the version used to produce the reported campaign results.

## Documentation

1. [Install the executable](INSTALL.md): requirements, CPLEX setup, and compilation check.
2. [Run the solver](RUN.md): model selection, command-line inputs, and an example.
3. [Read the output](OUTPUT.md): generated colorings, summaries, checks, and visualisations.

The solver produces graph colorings. A resulting graph certificate can be verified independently with the repository's [(m,n)-coloring checker](../checker/README.md).

## Solver source

[`source/`](source/) contains the original C++ source of the solver (entry point, model formulations, CPLEX cut callback, solution checker) for readers who want to inspect how the method is implemented. It is not buildable on its own — see [`source/README.md`](source/README.md) for why — and is not a substitute for the executable package documented above.
