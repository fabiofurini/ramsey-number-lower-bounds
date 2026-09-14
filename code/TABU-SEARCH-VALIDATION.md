# Generic tabu-search validation

This release extends algorithm `6` to generic `(m,n)` targets, selected complete K3/K4 pools, and
both distance geometries. The following deterministic small campaign was run with the source and
the rebuilt native distributed object; each emitted certificate was converted to DIMACS and accepted
by the independent `(m,n)` checker.

| Case | `t` | `(m,n)` | geometry | K3/K4 mask | Result |
| --- | ---: | --- | --- | ---: | --- |
| `circ_33` | 5 | `(3,3)` | circulant | 3 (both) | valid certificate |
| `linear_33` | 5 | `(3,3)` | linear (`t-1` variables) | 3 (both) | valid certificate |
| `circ_43` | 8 | `(4,3)` | circulant | 2 (red K3) | valid certificate |
| `linear_43` | 8 | `(4,3)` | linear (`t-1` variables) | 2 (red K3) | valid certificate |
| `linear_55_dynamic` | 6 | `(5,5)` | linear (`t-1` variables) | 0 (none) | valid certificate |

The standalone `source/RAMSEY_TABU_CORE_TEST.cpp` additionally checks distance mapping, exhaustive
K3/K4 support enumeration against vertex-subset references in both geometries, complete-pool scores,
incremental flip deltas, and the exact-verification gate.

The portable `RAMSEY.dynamic.glibc228.o` remains the previously released compatibility artifact;
only rebuild it with the established libc++/Zig compatibility toolchain. The regular
`RAMSEY.dynamic.o` has been rebuilt and smoke-tested for this release.
