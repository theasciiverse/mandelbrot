# Changes from upstream

This is the Asciiverse packaging of
[mandelbrot viewer](https://github.com/dulrich/mandelbrot-viewer) by David
Ulrich, a curses Mandelbrot explorer in one C file (2015 to 2016) built on
Ken Perlin's one-line Mandelbrot snippet. Upstream is the `upstream`
remote; the `asciiverse` branch starts from its last commit, `879fb61`
(2016-04-14). Upstream has no tags or version, so the version here is
2016.04.14.

Only the curses program is built. The repository also has a GLSL fragment
shader (`main.frag`, for glslViewer), which is not part of this title, and
its Makefile's default target fails because `viewer.c` is not in the
repository; `make curses` works, and `asciiverse/build.sh` does the same.

## Licence

AGPL-3.0-only: `main.c` says "version 3 of the License" with no "or any
later version", and the README repeats it. LICENSE is the AGPL version 3
text. The source is this repository, as the licence asks.

## Code

- **The view fills the terminal and is centred.** Upstream drew at a fixed
  25 columns per unit from the left edge, so the set sat to one side and
  stayed small on a big terminal. The view is now a centre point and a zoom
  level; at level 0 the whole set is fitted to the terminal (74 columns
  wide at 100x30) and centred, and a resize fits it again.
- **Zoom stops before it breaks.** Upstream walked each row with
  `x++` on a floating point coordinate that grew with the zoom; past about
  120 levels (where `long double` is plain `double`, as on Windows and
  Apple silicon) `x + 1 == x` and the loop never ended, so the program hung.
  Rows and columns are counted with integers now, and zoom stops at level
  100 (1.25^100, about 5e9), where a cell is still four orders of
  magnitude wider than `double` precision. `-` stops at the whole set.
- **More iterations as you zoom.** Upstream stopped at 111 iterations,
  which leaves deeper views blank; the limit is now 111 plus 24 per level.
- **Colour works.** Upstream read `COLORS` before `start_color()`, when it
  is still 0, so colour never came on; and it redefined the palette, which
  tmux, screen and the Linux console refuse. With 256 colours the sixteen
  bands now use fixed palette colours from black through red, orange and
  yellow to white (upstream's intended ramp was sixteen shades of red, which
  the 256-colour palette does not have); with fewer colours it stays
  monochrome, as before.
- **Arrows move 4 columns or 2 rows** (upstream: 1), scaled to the zoom,
  and the centre is kept near the set so panning cannot run off for good.
- **`=` zooms in like `+`**, `_` out like `-`; q and x still quit.
- **Ctrl+C, SIGTERM and SIGHUP quit like q**, with the terminal put back
  (cbreak mode left Ctrl+C killing it mid-screen).
- The cursor is hidden; Windows uses PDCurses' `curses.h`.

## Packaging (new files)

- `asciiverse/build.sh`, `asciiverse/package.sh`: one compiler call, one
  archive per target with the binary, README.md, LICENSE and this file.
- `.github/workflows/release.yml`: static Linux in Alpine (x86_64,
  aarch64), macOS (x86_64, arm64) and Windows x86_64 (MinGW, PDCurses),
  plus SHA256SUMS, on our tags (`*-av.*`).
- `asciiverse/record-preview.py`, `asciiverse/mandelbrot.cast`: the catalog
  preview, 100x30, about 23 s: the whole set, then 34 levels into Seahorse
  Valley from a fixed key sequence (hub's `castrec.py`).
- `.gitignore`: the build folders.

## Tested

Linux: native build at 100x30, resized to 60x16 in tmux (the set fitted
again), q quits with the terminal restored; zoomed past level 100 and back
out past level 0, panned against the limits. ASan and UBSan: a scripted
run of 150 zooms in, panning, 160 zooms out and fast panning was clean.
Windows: cross-built with MinGW and PDCurses 3.9 in Docker, not run.
macOS: not built here.

## Ours

- **How to play it, at the top of README.md.** A short note on installing and
  starting it with av, or from a release archive; the original text follows
  unchanged.
