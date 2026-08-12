# Solver files

This directory contains the precompiled solver object and the script that links it with a local IBM ILOG CPLEX Studio 20.1 installation.

Two builds of the object are provided: `RAMSEY.dynamic.o` (default) and `RAMSEY.dynamic.glibc228.o`, which targets systems with glibc 2.28 or newer and has no dependency on the system's libstdc++ version. See [Older systems (glibc ≤ 2.28)](../INSTALL.md#older-systems-glibc--228) if the default object fails to link or run.

- [Installation instructions](../INSTALL.md)
- [Running the solver](../RUN.md)
- [Reading the output](../OUTPUT.md)

The generated `RAMSEY` executable is local and must not be committed.

This directory is licensed under the [GNU General Public License v3.0 or later](../../LICENSE); see [`../../NOTICE.md`](../../NOTICE.md) for why.
