# Third-party components and licensing notice

This repository contains software and research material under separate licenses. This file maps each part of the repository to its license, identifies the third-party components, and records their provenance.

## Software — GNU General Public License v3.0 or later

Copyright © 2026 Fabio Furini. The following software is released under the GNU General Public License v3.0 or later (full text in [`LICENSE`](LICENSE)):

- all files in [`code/`](code/) and [`checker/`](checker/);
- the Python renderers in [`assets/render_circulant_colouring.py`](assets/render_circulant_colouring.py) and [`lower-bounds/tools/`](lower-bounds/tools/).

The compiled solver objects (`code/solver/RAMSEY.dynamic.o`, `code/solver/RAMSEY.dynamic.glibc228.o`) and compiled checkers (`checker/Ramsey_checker`, `checker/Ramsey_checker_glibc228`, `checker/Ramsey_checker_macOS`) statically incorporate routines from two libraries by Pablo San Segundo (Intelligent Control Research Group, CSIC-UPM):

- **BitGraph** (`bitscan`/`graph` libraries) — public source at <https://github.com/psanse/BitGraph>, licensed GPL-3.0.
- **coptBG** (built on BitGraph; includes the CliSAT maximum-clique algorithm used by the solver's clique-separation routines) — GPL-3.0, source not published in this repository.

Because these binaries are a combined work incorporating GPL-3.0 code, the whole of `code/` and `checker/` is distributed under the GNU General Public License v3.0 or later. See [`LICENSE`](LICENSE) for the full text.

**Corresponding source and permission.** BitGraph's public source is available from its [official repository](https://github.com/psanse/BitGraph). The source of coptBG is not included in or linked from this repository. Pablo San Segundo, the author and copyright holder of coptBG, has confirmed in writing that the licensing arrangement and public distribution of the compiled RAMSEY material described here are acceptable without publishing coptBG source. The written confirmation is retained by the repository maintainer.

## Research material and documentation — CC BY-NC-SA 4.0

Copyright © 2026 Fabio Furini. The following original research material is released under [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-nc-sa/4.0/) (CC BY-NC-SA 4.0):

- the graph certificates and coloured matrices in [`lower-bounds/certificates/`](lower-bounds/certificates/) and [`lower-bounds/matrices/`](lower-bounds/matrices/), and their accompanying Markdown documentation;
- the visual and data files in [`assets/`](assets/), except for the Python renderer named above.

See [`lower-bounds/LICENSE`](lower-bounds/LICENSE) for the CC license notice and the link to the full legal code. The paper authors retain authorship of the accompanying research; hosting this repository in the `fabiofurini` GitHub account does not transfer ownership or alter authorship.

## CPLEX

IBM ILOG CPLEX Studio is proprietary software and is not distributed with this repository. Users must obtain their own licensed installation; see [`code/INSTALL.md`](code/INSTALL.md).
