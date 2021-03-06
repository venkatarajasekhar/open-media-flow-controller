/*	$Id: ncurses_misc.c,v 1.1.1.1 2003/09/19 17:05:30 gregs Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef SUPPORT_XTERM_RESIZE
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <sys/termios.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif /* SUPPORT_XTERM_RESIZE */

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "zile.h"
#include "extern.h"

#include "term_ncurses.h"

#ifdef SUPPORT_XTERM_RESIZE
void resize_windows(int width, int height)
{
	windowp wp;
	int hdelta = height - ncurses_tp->height;

	/* Resize windows horizontally. */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		wp->fwidth = wp->ewidth = width;

	/* Resize windows vertically. */
	if (hdelta > 0) { /* Increase windows height. */
		for (wp = head_wp; hdelta > 0; wp = wp->next) {
			if (wp == NULL)
				wp = head_wp;
			++wp->fheight;
			++wp->eheight;
			--hdelta;
		}
	} else { /* Decrease windows height. */
		int decreased = TRUE;
		while (decreased) {
			decreased = FALSE;
			for (wp = head_wp; wp != NULL && hdelta < 0; wp = wp->next)
				if (wp->fheight > 2) {
					--wp->fheight;
					--wp->eheight;
					++hdelta;
					decreased = TRUE;
				}
		}
	}

	/*
	 * Sometimes Zile cannot reduce the windows height to a certain
	 * value (too small); take care of this case.
	 */
	ncurses_tp->width = width;
	ncurses_tp->height = height - hdelta;
}

static void resize_sig_handler(int signo)
{
	struct winsize win;
	switch (signo) {
	case SIGWINCH:
		ioctl(STDIN_FILENO, TIOCGWINSZ, &win);
		/* Avoid bogus sizes. */
		if (win.ws_col > 10 && win.ws_row > 5) {
			/* Setup the ncurses library internal data. */
			resizeterm(win.ws_row, win.ws_col);
			/* Resize the Zile windows. */
			resize_windows(COLS, LINES);
		}
		FUNCALL(recenter);
		refresh();
		break;
	}
}
#endif

static void segv_sig_handler(int signo)
{
	fprintf(stderr, "Zile crashed.  Please send a bug report to <sandro@sigala.it>.\r\n");
	zile_exit(2);
}

int ncurses_init(void)
{
	struct sigaction segv_sig;
#ifdef SUPPORT_XTERM_RESIZE
	struct sigaction resize_sig;
#endif

	segv_sig.sa_handler = segv_sig_handler;
	sigemptyset(&segv_sig.sa_mask);
        segv_sig.sa_flags = SA_RESTART;

#ifdef SUPPORT_XTERM_RESIZE
        resize_sig.sa_handler = resize_sig_handler;
        sigemptyset(&resize_sig.sa_mask);
        resize_sig.sa_flags = SA_RESTART;
#endif

	initscr();

	ncurses_tp->width = COLS;
	ncurses_tp->height = LINES;

	sigaction(SIGFPE, &segv_sig, NULL);
	sigaction(SIGSEGV, &segv_sig, NULL);
#ifdef SUPPORT_XTERM_RESIZE
	sigaction(SIGWINCH, &resize_sig, NULL);
#endif

	return TRUE;
}

static void init_colors(void)
{
	int fg = COLOR_WHITE;
	int bg = COLOR_BLACK;

#ifdef NCURSES_VERSION
	if (use_default_colors() == OK) {
		fg = -1;
		bg = -1;
	}
#endif

	/* "WHITE" is used as synonym of "DEFAULT". */
	init_pair(ZILE_COLOR_WHITE, fg, bg);

	init_pair(ZILE_COLOR_BLACK,   COLOR_BLACK,   bg);
	init_pair(ZILE_COLOR_RED,     COLOR_RED,     bg);
	init_pair(ZILE_COLOR_GREEN,   COLOR_GREEN,   bg);
	init_pair(ZILE_COLOR_YELLOW,  COLOR_YELLOW,  bg);
	init_pair(ZILE_COLOR_BLUE,    COLOR_BLUE,    bg);
	init_pair(ZILE_COLOR_MAGENTA, COLOR_MAGENTA, bg);
	init_pair(ZILE_COLOR_CYAN,    COLOR_CYAN,    bg);
}

int ncurses_open(void)
{
	int colors = lookup_bool_variable("colors") && has_colors();

	if (colors)
		start_color();
	noecho();
	nonl();
	raw();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
/*	meta(stdscr, TRUE); */
	if (colors)
		init_colors();

	return TRUE;
}

extern void ncurses_free_rotation_buffers(void);

int ncurses_close(void)
{
	ncurses_free_rotation_buffers();
	endwin();

	return TRUE;
}

void ncurses_refresh(void)
{
	refresh();
}

static void show_splash_screen(char *splash)
{
	int i, bold = 0, red = 0;
	char *p;

	for (i = 0; i < LINES - 2; ++i) {
		move(i, 0);
		clrtoeol();
	}

	move(0, 0);
	for (i = 0, p = splash; *p != '\0'; ++p)
		switch (*p) {
		case '%':
			if (bold)
				attrset(0), bold = 0;
			else
				attrset(A_BOLD), bold = 1;
			break;
		case '$':
			if (red)
				attrset(0), red = 0;
			else
				attrset(A_BOLD | C_FG_RED), red = 1;
			break;
		case  '\n':
			move(++i, 0);
			break;
		default:
			addch(*p);
		}
}

void ncurses_show_about(char *splash, char *minibuf)
{
	if (!lookup_bool_variable("novice-level") &&
	    !lookup_bool_variable("skip-splash-screen")) {
		show_splash_screen(splash);
		minibuf_write(minibuf);
		waitkey(20 * 1000);
		minibuf_clear();
	} else
		minibuf_write(minibuf);
}

void ncurses_clear(void)
{
	erase();
}

void ncurses_beep(void)
{
	beep();
}
