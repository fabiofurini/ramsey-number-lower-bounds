# Installing the RAMSEY executable

## Requirements

- a 64-bit Linux system;
- GNU C++ (`g++`);
- IBM ILOG CPLEX Studio 20.1;
- the supplied `solver/RAMSEY.dynamic.o` file.

CPLEX is proprietary software and is not distributed with this repository.

## Create the executable

From the repository root, run:

```bash
cd code/solver
chmod +x ramsey_link_with_cplex.sh
./ramsey_link_with_cplex.sh /path/to/CPLEX_Studio201
```

Provide the CPLEX Studio installation directory, not its `cplex` subdirectory. For example, if the CPLEX library is located at

```text
/opt/ibm/ILOG/CPLEX_Studio201/cplex/bin/x86-64_linux/libcplex2010.so
```

use:

```bash
./ramsey_link_with_cplex.sh /opt/ibm/ILOG/CPLEX_Studio201
```

The script creates `code/solver/RAMSEY` and records the supplied CPLEX library path in the executable.

## Check the installation

Run:

```bash
file RAMSEY
ldd RAMSEY
```

`file` should identify a 64-bit Linux executable, and `ldd` should show a valid path for `libcplex2010.so`.

If linking fails, check that:

- `g++` is installed;
- the CPLEX path is correct;
- `cplex/bin/x86-64_linux/libcplex2010.so` exists below that path.

Do not commit the generated `RAMSEY` executable. Continue with [Running the solver](RUN.md).

## Older systems (glibc ≤ 2.28)

If linking or running fails with an error naming a missing `GLIBC_2.3x` or `GLIBCXX_3.4.3x` symbol version, your system's C/C++ runtime predates the one used to build `RAMSEY.dynamic.o`. Use the alternative object instead, built to run on any system with glibc 2.28 or newer (e.g. Debian 10, Ubuntu 18.10, RHEL 8) and no dependency on the system's libstdc++ version:

```bash
cd code/solver
chmod +x ramsey_link_with_cplex_glibc228.sh
./ramsey_link_with_cplex_glibc228.sh /path/to/CPLEX_Studio201
```

This links `RAMSEY.dynamic.glibc228.o` instead of `RAMSEY.dynamic.o`; everything else (requirements, checks, usage) is identical.
