# Editing and publishing the Ramsey website

Live website: https://fabiofurini.github.io/ramsey-number-lower-bounds/

The files in this directory are the website source and the published website. There is no compilation step, external JavaScript dependency, CPLEX dependency, or Sites hosting dependency.

## Edit a page

- `index.html`: Ramsey definition, R(4,4) counterexample, graph families, circulant builder and colouring laboratory.
- `distance-colourings.html`: linear-distance twin of the homepage, with curved edges above/below a vertex line, N−1 independent colours, size selection and exact clique checks. Presets embed existing circulant witnesses. This is presented as an open research direction outside the paper.
- `known-numbers.html`: exact classical values, Radziszowski survey citation, circulant study values and their proof procedure.
- `gallery.html`: paired blue/red graph gallery and explanation of complements and lower bounds.
- `witness.html?m=3&n=13`: visual witness page, colour selection, zoom, distance inspection, SVG image export and certificate download.
- `assets/witness-data.js`: 26 study and classical witness distance sets; rebuild with `python3 website-tools/build_witness_data.py`.
- `results.html`: paper results and certificate downloads.
- `method.html`: the mathematical and computational method.
- `assets/style.css`: shared layout, colours, fonts and responsive rules.
- `assets/app.js`: homepage builder and colouring laboratory.
- `assets/gallery.js`: gallery drawing and enlarged views.
- `assets/results.js`: results selector, distance grid and table.
- `assets/graph.js`: graph rendering and exact clique counts in the browser.
- `assets/data.js`: certificate-derived data and example presets.
- `assets/analytics.js`: Cloudflare Web Analytics, using the same public site token as the teaching pages on `fabiofurini.github.io`. Analytics runs only on the live hostname; visits are viewed in the Cloudflare dashboard, filtered by the `/ramsey-number-lower-bounds/` path. It is not a visible public counter.
- `assets/navigation.js`: shared mobile navigation.

On GitHub, open a file, choose **Edit**, make the change and commit it to `main`. GitHub Pages publishes `main` → `/docs` automatically. The deployment status is visible under the repository’s **Actions** tab and **Settings → Pages**. Keep `.nojekyll`: this is a static HTML website.

The `gallery/`, `results/`, `known-numbers/`, `distance-colourings/` and `method/` directories provide short URL redirects; edit the corresponding `.html` files above for the actual page content.

After changing a CSS or JavaScript asset locally, run `python3 website-tools/version_assets.py` and commit the updated HTML files together with the assets. This refreshes their URLs so browsers load the matching version instead of reusing an older cached script or stylesheet.

## Preview locally

From the repository root:

```bash
python3 -m http.server 8765 --bind 127.0.0.1 --directory docs
```

Open http://127.0.0.1:8765/ . Stop the server with Ctrl+C. All computation for the demos runs in the visitor’s browser.

## Verify changes

For exact counting and preset checks, using Node.js:

```bash
node website-tools/tests/clique-counts.cjs
```

For the browser interaction checks, serve the repository root:

```bash
python3 -m http.server 8766 --bind 127.0.0.1
```

Open http://127.0.0.1:8766/website-tools/tests/browser-check.html . It checks the demos, custom circulant construction, all preset sizes, cross-page navigation, gallery and paper-result interactions. `website-tools/tests/distance-check.html` checks the linear-distance UI, independent edits to d and N−d, curved rendering, builder edge counts and a separately enumerated custom graph.

## Regenerate certificate data

```bash
python3 website-tools/build_data.py
```

This uses the source files already present in this repository, verifies DIMACS format, circulant symmetry and blue triangle-freeness for the 25 large certificates, and checks the classical K17 illustration. It rebuilds `assets/data.js` and the downloadable certificate copies. It does not independently prove the large red-clique exclusions; those come from the paper and the research checker.

## Licences and provenance

Website software is GPL-3.0-or-later. Certificate-derived data, graph certificates, illustrations and accompanying research documentation retain CC BY-NC-SA 4.0. See `../NOTICE.md`, `assets/SOFTWARE-LICENSE.txt` and `certificates/LICENSE.txt`.

The paper results and certificates originate from repository snapshot `38e59da36155bdcbbaec6252586603cc8ef602b4`. The 5-vertex and 17-vertex introductory colourings are classical examples. The result table describes the paper and is not a live world-record survey.
