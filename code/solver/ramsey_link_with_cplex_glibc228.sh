#!/usr/bin/env bash
set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 /path/to/CPLEX_Studio201"
    exit 2
fi

CPLEX_LIBDIR="$1/cplex/bin/x86-64_linux"

g++ RAMSEY.dynamic.glibc228.o \
    -L"$CPLEX_LIBDIR" \
    -Wl,-rpath,"$CPLEX_LIBDIR" \
    -Wl,-l:libcplex2010.so \
    -lm -ldl -pthread \
    -o RAMSEY
