/***********************************************************************
 * $Id: sfxa.c,v 1.4 2005/02/23 05:32:45 aki Exp $
 *
 * sfxa
 * Copyright (C) 2005 RIKEN. All rights reserved.
 * Written by Aki Hasegawa <aki@gsc.riken.jp>.
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
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#if HAVE_ASSERT_H
# include <assert.h>
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif

#include "search.h"
#include "output.h"

#include <mmfile.h>
#include <msg.h>
#include <strdupcat.h>

#include <errno.h>
#include <dirname.h>
#include <minmax.h>
#include <getopt.h>
#include <progname.h>
#include <string.h>
#include <stdint.h>
#include <xalloc.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#define DEFAULT_F_POS	(1)	/* whether print position column */
#define DEFAULT_F_IDX	(1)	/* whether print index column */
#define DEFAULT_F_SFX	(50)	/* length of suffix column */
#define DEFAULT_F_PRE	(3)	/* length of substring ahead of the suffix */
#define DEFAULT_F_CHOP	(1)	/* whether chop line after '\0' */

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

typedef struct sfxa_type {
    mmfile_t	    ftxt;
    mmfile_t	    fidx;
} sfxa_t;

typedef struct opts_type {
    char	    *opt_o;	/* output file path */
    char	    *opt_M;	/* mapping file path */
    int		    opt_v;	/* verbose level */
    unsigned int    opt_h: 1;	/* help flag */
    unsigned int    opt_V: 1;	/* version flag */
    unsigned int    opt_d: 1;	/* dump suffix array */
    output_param_t  opt_F;
} opts_t;

/*======================================================================
 * global variables
 *======================================================================*/

static opts_t opts = {
    /* pointers */
    NULL,		/* o: output file path */
    NULL,		/* M: mapping file path */
    /* numerals */
    0,			/* v: verbose level */
    /* binary flags */
    0,			/* h: help flag */
    0,			/* V: version flag */
    0,			/* d: dump suffix array */
    /* others */
    {DEFAULT_F_POS, DEFAULT_F_IDX, DEFAULT_F_SFX, DEFAULT_F_PRE, DEFAULT_F_CHOP}
};

/*======================================================================
 * prototypes
 *======================================================================*/

static int search_pattern(const sfxa_t *sfxa, const char *pattern);
static int dump_suffix_array(const sfxa_t *sfxa);
static int open_suffix_array(sfxa_t *sfxa, const char *ftxt, const char *fidx);
static int close_suffix_array(sfxa_t *sfxa);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

/* search pattern using suffix array */
static int search_pattern(const sfxa_t *sfxa, const char *pattern)
{
    int ret = 0;
    off_t len = mmfile_len(&sfxa->ftxt);
#if SIZEOF_OFF_T < 8
    if (len <= (INT32_MAX / sizeof(int32_t))) {
	range32_t result;
	search32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, pattern, &result);
	output32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, result.beg, result.end, &opts.opt_F);
    } else {
	msg(MSGLVL_WARNING, "Cannot search. File too large");
	ret = errno = EFBIG;
    }
#else /* SIZEOF_OFF_T >= 8 */
    if (len <= INT32_MAX) {
	range32_t result;
	search32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, pattern, &result);
	output32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, result.beg, result.end, &opts.opt_F);
    } else if (len <= (INT64_MAX / sizeof(int64_t))) {
	range64_t result;
	search64(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int64_t)len, pattern, &result);
	output64(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int64_t)len, result.beg, result.end, &opts.opt_F);
    } else {
	msg(MSGLVL_WARNING, "Cannot search. File too large");
	ret = errno = EFBIG;
    }
#endif /* SIZEOF_OFF_T >= 8 */
    return ret;
}

static int dump_suffix_array(const sfxa_t *sfxa)
{
    int ret = 0;
    off_t len = mmfile_len(&sfxa->ftxt);

#if SIZEOF_OFF_T < 8
    if (len > (INT32_MAX / sizeof(int32_t))) {	/* check the size */
	errno = EFBIG;
	msg(MSGLVL_WARNING, "Cannot dump:");
	return errno;
    }
    if (opts.opt_v)
	printf("## txt: '%s', idx:'%s', size:%ld\n"
		"--- dump begin ---\n",
		mmfile_path(&sfxa->ftxt), mmfile_path(&sfxa->fidx), len);
    output32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
	    (int32_t)len, 0, (int32_t)len - 1, &opts.opt_F);
#else /* SIZEOF_OFF_T >= 8 */
    if (len > (INT64_MAX / sizeof(int64_t))) {	/* check the size */
	errno = EFBIG;
	msg(MSGLVL_WARNING, "Cannot dump:");
	return errno;
    }
    if (opts.opt_v)
	printf("## txt: '%s', idx:'%s', size:%lld\n"
		"--- dump begin ---\n",
		mmfile_path(&sfxa->ftxt), mmfile_path(&sfxa->fidx),
		(long long)len);
    if (len <= INT32_MAX) {
	output32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, 0, (int32_t)len - 1, &opts.opt_F);
    } else {
	output64(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int64_t)len, 0, (int64_t)len - 1, &opts.opt_F);
    }
#endif /* SIZEOF_OFF_T >= 8 */
    if (opts.opt_v)
	printf("--- dump end ---\n");
    return ret;
}

static int open_suffix_array(sfxa_t *sfxa, const char *ftxt, const char *fidx)
{
    int ret = 0;
    /* map text file */
    if ((ret = mmfile_init(&sfxa->ftxt, ftxt)) == 0)
	if ((ret = mmfile_map_private_rd(&sfxa->ftxt)) != 0)
	    mmfile_free(&sfxa->ftxt);
    /* map index file */
    if (ret == 0 && (ret = mmfile_init(&sfxa->fidx, fidx)) == 0)
	if ((ret = mmfile_map_private_rd(&sfxa->fidx)) != 0)
	    mmfile_free(&sfxa->fidx);
    return ret;
}

static int close_suffix_array(sfxa_t *sfxa)
{
    int ret = 0;
    /* unmap index file */
    if ((ret = mmfile_unmap(&sfxa->fidx)) != 0)
	return ret;
    /* unmap text file */
    if ((ret = mmfile_unmap(&sfxa->ftxt)) != 0)
	return ret;
    mmfile_free(&sfxa->fidx);
    mmfile_free(&sfxa->ftxt);
    return ret;
}

/* parse suboption F */
static void parse_subopt_F(char **optionp)
{
    char *val;
    int op;
    enum {POS = 0, NOPOS, IDX, NOIDX, SFX, NOSFX, PRE, NOPRE, CHOP, NOCHOP};
    static char *subop[] = {
	"pos", "nopos", "idx", "noidx", "sfx", "nosfx", "pre", "nopre", "chop", "nochop", NULL};
    while ((op = getsubopt(optionp, subop, &val)) != -1) {
	switch (op) {
	    case POS: opts.opt_F.pos = 1; break;
	    case NOPOS: opts.opt_F.pos = 0; break;
	    case IDX: opts.opt_F.idx = 1; break;
	    case NOIDX: opts.opt_F.idx = 0; break;
	    case SFX: opts.opt_F.sfx = atoi(val); break;
	    case NOSFX: opts.opt_F.sfx = 0; break;
	    case PRE: opts.opt_F.pre = atoi(val); break;
	    case NOPRE: opts.opt_F.pre = 0; break;
	    case CHOP: opts.opt_F.chop = 1; break;
	    case NOCHOP: opts.opt_F.chop = 0; break;
	    default: break;
	}
    }
}

/* show version number */
static void show_version(void)
{
    fprintf(stdout, "mksfxa (%s) %s\n", PACKAGE, VERSION);
}

/* show help */
static void show_help(void)
{
    static char fmt[] =
	"This is sfxa, suffix array search program.\n"
	"Copyright (C) 2005 RIKEN. All rights reserved.\n"
	"This program comes with ABSOLUTELY NO WARRANTY.\n"
	"You may redistribute copies of this program under the terms of the\n"
	"GNU General Public License.\n"
	"For more information about these matters, see the file named COPYING.\n"
	"\n"
	"Usage: %s [options] <text_file> <index_file> [pattern ...]\n"
	"Options:\n"
#ifdef HAVE_GETOPT_LONG
	"  -h, --help           display this message\n"
	"  -V, --version        print version number, and exit\n"
	"  -v, --verbose        verbose output\n"
	"  -o, --output=<file>  file to output\n"
	"  -d, --dump           dump suffix array\n"
	"  -M, --map=<file>     character mapping file\n"
	"  -F, --format=<comma_separated_subopts>  formatting parameters\n"
	"        [no]pos        [do not] print array position column\n"
	"        [no]idx        [do not] print index column\n"
	"        [no]sfx=<n>    [do not] print suffix at most length <n>\n"
#if 0
	"        [no]pre=<n>    [do not] print <n> characters ahead of the suffix\n"
#endif
	"        [no]chop       [do not] chop suffix beyond character '\\0'\n"
#else
	"  -h           display this message\n"
	"  -V           print version number, and exit\n"
	"  -v           verbose output\n"
	"  -o <file>    file to output\n"
	"  -d           dump suffix array\n"
	"  -M <file>    character mapping file\n"
	"  -F <comma_separated_subopts>  formatting parameters\n"
	"     [no]pos        [do not] print array position column\n"
	"     [no]idx        [do not] print index column\n"
	"     [no]sfx=<n>    [do not] print suffix at most length <n>\n"
#if 0
	"     [no]pre=<n>    [do not] print <n> characters ahead of the suffix\n"
#endif
	"     [no]chop       [do not] chop suffix beyond character '\\0'\n"
#endif
	"Report bugs to %s.\n"
	;
    fprintf(stdout, fmt, program_name, PACKAGE_BUGREPORT);
}

/* main */
int main(int argc, char **argv)
{
    sfxa_t sa;

    /* preserve program name */
    set_program_name(argv[0]);

    /* manage opts */
    for (;;) {
	int opt;
#ifdef HAVE_GETOPT_LONG
	int opt_index = 0;
	static struct option long_opts[] = {
	    {"help",	    no_argument,	NULL, 'h'},
	    {"version",	    no_argument,	NULL, 'V'},
	    {"verbose",	    no_argument,	NULL, 'v'},
	    {"output",      required_argument,	NULL, 'o'},
	    {"dump",	    no_argument,	NULL, 'd'},
	    {"map",	    required_argument,	NULL, 'M'},
	    {"format",      required_argument,	NULL, 'F'},
	    {0, 0, 0, 0}
	};

	opt = getopt_long(argc, argv, "hVvo:dM:F:", long_opts, &opt_index);
#else
	opt = getopt(argc, argv, "hVvo:dM:F:");
#endif
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    case 'o': opts.opt_o = xstrdup(optarg); break;
	    case 'd': opts.opt_d = 1; break;
	    case 'M': opts.opt_M = xstrdup(optarg); break;
	    case 'F': parse_subopt_F(&optarg); break;
	    default: show_help(); exit(EXIT_FAILURE);
	}
    }

    /* show version number and exit */
    if (opts.opt_V) { show_version(); exit(EXIT_SUCCESS); }

    /* show help message and exit */
    if (opts.opt_h) { show_help(); exit(EXIT_SUCCESS); }

    /* check arguments */
    if (argc < optind + 2) {
	msg(MSGLVL_ERR, "You must specify both of text and index file.");
	exit(EXIT_FAILURE);
    }

    /* open suffix array */
    {
	char *ftxt = argv[optind++];
	char *fidx = argv[optind++];
	if (open_suffix_array(&sa, ftxt, fidx) != 0) {
	    msg(MSGLVL_ERR, "Cannot open suffix array:");
	    exit(EXIT_FAILURE);
	}
    }

    /* redirect output file stream */
    if (opts.opt_o != NULL) {
	if (freopen(opts.opt_o, "w", stdout) == NULL) {
	    msg(MSGLVL_ERR, "Cannot open file '%s':", opts.opt_o);
	    exit(EXIT_FAILURE);
	}
    }

    /* search the pattern */
    if (opts.opt_d) {
	dump_suffix_array(&sa);
    } else {
	for (; optind < argc; ++optind) {
	    const char *pattern = argv[optind];
	    int ret = 0;
	    if (opts.opt_v)
		msg(MSGLVL_INFO, "searching pattern '%s'...", pattern);

	    ret = search_pattern(&sa, pattern);

	    if (opts.opt_v && ret != 0) msg(MSGLVL_NOTICE, "failed:");
	    if (opts.opt_v && ret == 0) msg(MSGLVL_INFO, "...done.");
	}
    }

    /* close suffix array */
    if (close_suffix_array(&sa) != 0) {
	msg(MSGLVL_ERR, "Cannot close suffix array:");
	exit(EXIT_FAILURE);
    }

    /* finalize */
    if (opts.opt_M) { free(opts.opt_M), opts.opt_M = NULL; }
    if (opts.opt_o) { free(opts.opt_o), opts.opt_o = NULL; }

    exit(EXIT_SUCCESS);
}
