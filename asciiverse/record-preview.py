#!/usr/bin/env python3
"""Records the catalog preview at 100x30 with hub/tools/castrec.py: the
whole set, then a fixed key sequence that zooms 34 levels into Seahorse
Valley (-0.7436448, 0.1318253), panning a step at each level to keep the
point centred. The keys are worked out here from the viewer's own step sizes
(4 columns, 2 rows, 1.25 per level, 30 cells per unit at level 0 in a 100x30
terminal); nothing reads the screen. About 23 s.

  asciiverse/record-preview.py out/mandelbrot asciiverse/mandelbrot.cast
"""
import os, shutil, subprocess, sys, tempfile

binary, out = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
castrec = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../hub/tools/castrec.py")
TX, TY, LEVELS = -0.7436447860, 0.1318252536, 34
# arrows as a terminal sends them in keypad mode, which curses turns on
RIGHT, LEFT, UP, DOWN = "s\x1bOC", "s\x1bOD", "s\x1bOA", "s\x1bOB"

cx, cy = -0.765, 0.0
steps = ["w2.5"]
for lv in range(LEVELS + 1):
    xs = 30 * 1.25 ** lv
    ys = xs / 2.5
    while True:
        moved = False
        if abs(cx + 4 / xs - TX) < abs(cx - TX): cx += 4 / xs; steps.append(RIGHT + ";w0.12"); moved = True
        elif abs(cx - 4 / xs - TX) < abs(cx - TX): cx -= 4 / xs; steps.append(LEFT + ";w0.12"); moved = True
        if abs(cy + 2 / ys - TY) < abs(cy - TY): cy += 2 / ys; steps.append(DOWN + ";w0.12"); moved = True
        elif abs(cy - 2 / ys - TY) < abs(cy - TY): cy -= 2 / ys; steps.append(UP + ";w0.12"); moved = True
        if not moved:
            break
    if lv < LEVELS:
        steps.append("k+;w0.35")
steps.append("w3;kq")

home = tempfile.mkdtemp()
try:
    os.mkdir(os.path.join(home, "bin"))
    shutil.copy(binary, os.path.join(home, "bin", "mandelbrot"))
    env = dict(os.environ, HOME=home, PATH=os.path.join(home, "bin") + os.pathsep + os.environ["PATH"])
    subprocess.run([sys.executable, castrec, out, home, ";".join(steps), "--", "mandelbrot"],
                   env=env, check=True)
finally:
    shutil.rmtree(home)
