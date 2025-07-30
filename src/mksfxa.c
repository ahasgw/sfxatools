/***********************************************************************
 * Copyright (C) 2005 RIKEN. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ***********************************************************************/

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirname.h>
#include <getopt.h>
#include <progname.h>
#include <xalloc.h>

#include "sfxa.h"
#include "msg.h"
#include "strdupcat.h"
#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#ifndef OFF_T_MAX
# if SIZEOF_OFF_T < 8
#  define OFF_T_MAX INT32_MAX
# else
#  define OFF_T_MAX INT64_MAX
# endif
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct opts_type {
    char            *opt_x;     /* file extention */
    int		    opt_v;	/* verbose level */
    int		    opt_X;	/* replace last X extentions with *opt_x */
    unsigned int    opt_h: 1;	/* help flag */
    unsigned int    opt_V: 1;	/* version flag */
} opts_t;

/*======================================================================
 * global variables
 *======================================================================*/

static opts_t opts = {
    /* pointers */
    NULL,			/* x: file extention */
    /* numerals */
    0,				/* v: verbose level */
    0,				/* X: replace last X extentions with *opt_x */
    /* binary flags */
    0,				/* h: help flag */
    0				/* V: version flag */
};

static const char * const g_default_ext = ".idx";

/*======================================================================
 * prototypes
 *======================================================================*/

static int process_file(const char *txtpath);
static char *build_idxfile_name(const char *path);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

/* input from files */
static int process_file(const char *txtpath)
{
    int ret = 0;
    char *idxpath;
    sfxa_t sa;

    if ((idxpath = build_idxfile_name(txtpath)) == NULL)
	return (errno = EEXIST);

    if ((ret = sfxa_init(&sa, txtpath, idxpath, NULL)) != 0)
	return ret;

    if ((ret = sfxa_build_idx(&sa)) != 0)
	return ret;

    free(idxpath), idxpath = NULL;
    return 0;
}

static char *build_idxfile_name(const char *path)
{
#if 0
    char *base = base_name(path);
    char *newpath = strdupcat(base, (opts.opt_x ? opts.opt_x : g_default_ext));
#else
    char *newpath = NULL;
    char *base = strdup(base_name(path));
    if (base != NULL) {
	int n;
	char *cp;
	for (n = 0, cp = base; *cp != '\0'; ++cp)   /* count number of '.' */
	    if (*cp == '.')
		++n;
	for (n = MIN(n, opts.opt_X); n > 0 && cp >= base; --cp) {
	    if (*cp == '.')
		if (--n == 0)			    /* truncate */
		    *cp = '\0';
	}
	newpath = strdupcat(base, (opts.opt_x ? opts.opt_x : g_default_ext));
	free(base);

	if (strcmp(newpath, base_name(path)) == 0) /* if they have same name */
	    free(newpath), newpath = NULL;
    }
#endif
    return newpath;
}

/* show version number */
static void show_version(void)
{
    static char fmt[] =
	"mksfxa (%s) %s\n"
	"\n"
	"Copyright (C) 2005 RIKEN. All rights reserved.\n"
	"This program comes with ABSOLUTELY NO WARRANTY.\n"
	"You may redistribute copies of this program under the terms of the\n"
	"GNU General Public License.\n"
	"For more information about these matters, see the file named COPYING.\n"
	;
    fprintf(stdout, fmt, PACKAGE, VERSION);
}

/* show help */
static void show_help(void)
{
    static char fmt[] =
	"This is mksfxa, suffix array builder program.\n"
	"\n"
	"Usage: %s [options] <text_file> ...\n"
	"Options:\n"
	"  -h, --help           display this message\n"
	"  -V, --version        print version number, and exit\n"
	"  -v, --verbose        verbose output\n"
	"  -x, --ext=<.ext>     set index file extention to <.ext> (default: .idx)\n"
	"  -X, --remove-ext     remove last extention before adding <.ext>\n"
	"Report bugs to <%s>.\n"
	;
    fprintf(stdout, fmt, base_name(program_name), PACKAGE_BUGREPORT);
}

/* main */
int main(int argc, char *argv[])
{
    /* preserve program name */
    set_program_name(argv[0]);

    /* manage opts */
    for (;;) {
	int opt;
	int opt_index = 0;
	static struct option long_opts[] = {
	    {"help",	    no_argument,	NULL, 'h'},
	    {"version",	    no_argument,	NULL, 'V'},
	    {"verbose",	    no_argument,	NULL, 'v'},
	    {"ext",         required_argument,	NULL, 'x'},
	    {"remove-ext",  no_argument,	NULL, 'X'},
	    {0, 0, 0, 0}
	};

	opt = getopt_long(argc, argv, "hVvx:X", long_opts, &opt_index);
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    case 'x': opts.opt_x = xstrdup(optarg); break;
	    case 'X': ++opts.opt_X; break;
	    default: show_help(); exit(EXIT_FAILURE);
	}
    }

    /* show version number and exit */
    if (opts.opt_V) { show_version(); exit(EXIT_SUCCESS); }

    /* show help message and exit */
    if (opts.opt_h) { show_help(); exit(EXIT_SUCCESS); }

    /* check arguments */
    if (optind > argc) {
	msg(MSGLVL_ERR, "No text file specified.");
	exit(EXIT_FAILURE);
    }

    /* process file */
    for (; optind < argc; ++optind) {
	const char *path = argv[optind];
	int ret = 0;
	if (opts.opt_v) msg(MSGLVL_INFO, "processing file '%s'...", path);

	ret = process_file(path);
	if (ret != 0) {
	    msg(MSGLVL_ERR, "failed:");
	    exit(EXIT_FAILURE);
	}

	if (opts.opt_v) msg(MSGLVL_INFO, "...done.");
    }

    /* finalize */
    if (opts.opt_x) { free(opts.opt_x), opts.opt_x = NULL; }

    exit(EXIT_SUCCESS);
}
