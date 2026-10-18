// mandelbrot viewer: curses mandelbrot explorer
// Copyright (C) 2015  David Ulrich
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Asciiverse changes (see PATCHES.md): the view is a centre point and a
// zoom level, fitted to the terminal and centred, so the whole set shows at
// start and after a resize; zoom stops at kMaxLevel, well before precision
// runs out; more iterations as you zoom; colours come from the 256-colour
// palette; Ctrl+C, SIGTERM and SIGHUP quit like q.

#ifdef _WIN32
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <tgmath.h>

// Each level zooms in by kZoomFactor. Upstream's x++ loops stopped
// advancing once the coordinates outgrew the floating point type (about 120
// levels where long double is plain double, as on Windows and Apple
// silicon), which hung the program. Columns and rows are counted with
// integers now, and zoom stops at level 100 (1.25^100, about 5e9), where a
// cell is still about 1e-11 wide, four orders of magnitude above double
// precision.
static const int kMaxLevel = 100;
static const long double kZoomFactor = 1.25;

// cells per unit horizontally at level 0 are fitted to the terminal; rows
// per unit are that divided by kAspect (upstream's 25 and 10)
static const long double kAspect = 2.5;

// black through red, orange and yellow to white, one colour per band
static const short kRamp[16] = {16, 52, 88, 124, 160, 196, 202, 208,
                                214, 220, 226, 227, 228, 229, 230, 231};

static volatile sig_atomic_t quit_requested = 0;

static void on_signal(int sig) {
	(void)sig;
	quit_requested = 1;
}

static int render(short color, int level, long double cx, long double cy, int width, int height) {
	// fit the whole set (about 3 units wide, 2.5 high) into the terminal
	long double base = width / 3.0L;
	if (height < base) base = height;
	if (base < 1) base = 1;
	long double x_scale = base * pow(kZoomFactor, (long double)level);
	long double y_scale = x_scale / kAspect;

	// upstream stopped at 111 iterations, which leaves deep views blank;
	// each level adds some, so detail keeps coming as you zoom
	int max_k = 111 + 24 * level;

	long double i,j,r;
	int k,kk;
	kk = -1;

	erase();

	for (int row = 0; row < height; row++) {
		move(row,0);
		long double y = cy + (row - height / 2.0L) / y_scale;
		for (int col = 0; col < width; col++) {
			long double x = cx + (col - width / 2.0L) / x_scale;
			for(
				i=k=r=0;
				j=r*r-i*i+x,i=2*r*i+y,j*j+i*i<11&&k++<max_k
				;r=j
			);
			if (color && (k&15) != kk) {
				attron(COLOR_PAIR((k&15) + 1));
				kk = k&15;
			}
			addch(" .:-;!/>)|&IH%*#"[k&15]);
		}
	}

	return 0;
}

int main(void) {
	int level = 0;

	// the centre of the view; the set spans about -2 to 0.47
	long double cx = -0.765;
	long double cy = 0;

	int width = 80;
	int height = 24;

	int c = 0;
	short color = 0;

	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);
#ifdef SIGHUP
	signal(SIGHUP, on_signal);
#endif

	initscr();

	noecho();
	cbreak();
	keypad(stdscr,TRUE);
	curs_set(0);
	// wake up now and then so a signal is noticed
	timeout(200);

	// COLORS is only known after start_color(); upstream read it before,
	// so colour never came on. The palette is no longer redefined, which
	// many terminals (tmux, screen, the Linux console) refuse.
	if (has_colors()) {
		start_color();
		color = COLORS >= 256;
	}

	if (color) {
		for (short n = 0; n < 16; n++) {
			init_pair(n + 1, kRamp[n], 16);
		}
		bkgd(COLOR_PAIR(1));
	}

	int redraw = 1;
	while (!quit_requested) {
		if (c == 'x' || c == 'X' || c == 'q' || c == 'Q') {
			break;
		}

		getmaxyx(stdscr,height,width);
		long double x_scale = (height < width / 3.0L ? height : width / 3.0L) * pow(kZoomFactor, (long double)level);
		if (x_scale < 1) x_scale = 1;
		long double y_scale = x_scale / kAspect;

		if (c == '+' || c == '=') {
			if (level < kMaxLevel) level++;
		}
		else if (c == '-' || c == '_') {
			if (level > 0) level--;
		}
		// arrows move the view by 4 columns or 2 rows
		else if (c == KEY_UP) {
			cy -= 2 / y_scale;
		}
		else if (c == KEY_DOWN) {
			cy += 2 / y_scale;
		}
		else if (c == KEY_LEFT) {
			cx -= 4 / x_scale;
		}
		else if (c == KEY_RIGHT) {
			cx += 4 / x_scale;
		}

		// keep the centre near the set, so panning cannot run off for good
		if (cx < -3) cx = -3;
		if (cx > 2) cx = 2;
		if (cy < -2) cy = -2;
		if (cy > 2) cy = 2;

		if (redraw || c != ERR) {
			render(color,level,cx,cy,width,height);
			refresh();
			redraw = 0;
		}

		c = getch();
	}

	endwin();

	return 0;
}
