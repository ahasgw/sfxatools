/***********************************************************************
 * $Id: sfxa.c,v 1.8 2005/06/11 06:23:10 aki Exp $
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
#include "cmap.h"
#include "output.h"

#include <mmfile.h>
#include <msg.h>
#include <strdupcat.h>

#include <errno.h>
#include <dirname.h>
#include <minmax.h>
#include <getopt.h>
#include <getsubopt.h>
#include <progname.h>
#include <string.h>
#include <xalloc.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#define DEFAULT_F_HDR	(1)	/* whether print information header */
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
    {
	NULL,
	DEFAULT_F_HDR,
	DEFAULT_F_POS,
	DEFAULT_F_IDX,
	DEFAULT_F_SFX,
	DEFAULT_F_PRE,
	DEFAULT_F_CHOP
    }
};

/*======================================================================
 * prototypes
 *======================================================================*/

static int search_pattern(const sfxa_t *sfxa, const char *pattern, int patlen);
static int dump_suffix_array(const sfxa_t *sfxa);
static int open_suffix_array(sfxa_t *sfxa, const char *ftxt, const char *fidx);
static int close_suffix_array(sfxa_t *sfxa);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

/* search pattern using suffix array */
static int search_pattern(const sfxa_t *sfxa, const char *pattern, int patlen)
{
    int ret = 0;
    off_t len = mmfile_len(&sfxa->ftxt);
#if SIZEOF_OFF_T < 8
    if (len <= (INT32_MAX / sizeof(int32_t))) {
	range32_t result;
	search32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, pattern, patlen, &result);
	if (opts.opt_F.hdr) {
	    printf("# '%*s' (%d)\n",
		    patlen, pattern, result.end - result.beg + 1);
	}
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
		(int32_t)len, pattern, patlen, &result);
	if (opts.opt_F.hdr) {
	    printf("# '%*s' (%d)\n",
		    patlen, pattern, result.end - result.beg + 1);
	}
	output32(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int32_t)len, result.beg, result.end, &opts.opt_F);
    } else if (len <= (INT64_MAX / sizeof(int64_t))) {
	range64_t result;
	search64(mmfile_ptr(&sfxa->ftxt), mmfile_ptr(&sfxa->fidx),
		(int64_t)len, pattern, patlen, &result);
	if (opts.opt_F.hdr) {
	    printf("# '%*s' (%lld)\n",
		    patlen, pattern, result.end - result.beg + 1);
	}
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
    if (opts.opt_F.hdr)
	printf("## txt: '%s', idx:'%s', size:%d\n"
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
    if (opts.opt_F.hdr)
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
    enum {HDR = 0, NOHDR, POS, NOPOS, IDX, NOIDX, SFX, NOSFX, PRE, NOPRE, CHOP, NOCHOP};
    static char *subop[] = {
	"hdr", "nohdr", "pos", "nopos", "idx", "noidx", "sfx", "nosfx", "pre", "nopre", "chop", "nochop", NULL};
    while ((op = getsubopt(optionp, subop, &val)) != -1) {
	switch (op) {
	    case HDR:	opts.opt_F.hdr = 1; break;
	    case NOHDR:	opts.opt_F.hdr = 0; break;

	    case POS:	opts.opt_F.pos = 1; break;
	    case NOPOS:	opts.opt_F.pos = 0; break;

	    case IDX:	opts.opt_F.idx = 1; break;
	    case NOIDX:	opts.opt_F.idx = 0; break;

	    case SFX:	if (val == NULL) {
			    msg(MSGLVL_ERR, "Illegal suboption for %s", subop[op]);
			    exit(EXIT_FAILURE);
			}
			opts.opt_F.sfx = atoi(val);
			break;
	    case NOSFX:	opts.opt_F.sfx = 0; break;

	    case PRE:	if (val == NULL) {
			    msg(MSGLVL_ERR, "Illegal suboption for %s", subop[op]);
			    exit(EXIT_FAILURE);
			}
			opts.opt_F.pre = atoi(val);
			break;
	    case NOPRE:	opts.opt_F.pre = 0; break;

	    case CHOP:	opts.opt_F.chop = 1; break;
	    case NOCHOP:    opts.opt_F.chop = 0; break;

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
	"        [no]hdr        [do not] print information header\n"
	"        [no]pos        [do not] print array position column\n"
	"        [no]idx        [do not] print index column\n"
	"        [no]sfx=<n>    [do not] print suffix at most length <n>\n"
#if 0
	"        [no]pre=<n>    [do not] print <n> characters ahead of the suffix\n"
#endif
	"        [no]chop       [do not] chop suffix beyond delimiter character\n"
#else
	"  -h           display this message\n"
	"  -V           print version number, and exit\n"
	"  -v           verbose output\n"
	"  -o <file>    file to output\n"
	"  -d           dump suffix array\n"
	"  -M <file>    character mapping file\n"
	"  -F <comma_separated_subopts>  formatting parameters\n"
	"     [no]hdr        [do not] print information header\n"
	"     [no]pos        [do not] print array position column\n"
	"     [no]idx        [do not] print index column\n"
	"     [no]sfx=<n>    [do not] print suffix at most length <n>\n"
#if 0
	"     [no]pre=<n>    [do not] print <n> characters ahead of the suffix\n"
#endif
	"     [no]chop       [do not] chop suffix beyond delimiter character\n"
#endif
	"Report bugs to %s.\n"
	;
    fprintf(stdout, fmt, program_name, PACKAGE_BUGREPORT);
}

/* main */
int main(int argc, char **argv)
{
    sfxa_t sa;
    cmap_t cm;

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

    /* prepare cmap */
    if (opts.opt_M) {
	if (cmap_load(&cm, opts.opt_M) != 0) {
	    msg(MSGLVL_ERR, "Cannot open map file '%s':", opts.opt_M);
	    exit(EXIT_FAILURE);
	}
	opts.opt_F.cmap = &cm;
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

    /* print information header */
    if (opts.opt_F.hdr) {
	off_t len = mmfile_len(&sa.ftxt);
#if SIZEOF_OFF_T < 8
	printf("## txt: '%s', idx:'%s', size:%d\n",
		mmfile_path(&sa.ftxt), mmfile_path(&sa.fidx), len);
#else /* SIZEOF_OFF_T >= 8 */
	printf("## txt: '%s', idx:'%s', size:%lld\n",
		mmfile_path(&sa.ftxt), mmfile_path(&sa.fidx), len);
#endif /* SIZEOF_OFF_T >= 8 */
    }

    /* search the pattern */
    if (opts.opt_d) {
	dump_suffix_array(&sa);
    } else {
	for (; optind < argc; ++optind) {
	    int ret = 0;
	    if (opts.opt_v)
		msg(MSGLVL_INFO, "searching pattern '%s'...", argv[optind]);

	    if (opts.opt_M) {
		unsigned char *pattern;
		int patlen;
		ret = cmap_translate(&cm, argv[optind], &pattern, &patlen);
		if (ret != 0) {
		    msg(MSGLVL_ERR, "Cannot map the query:");
		    exit(EXIT_FAILURE);
		}
		ret = search_pattern(&sa, (char *)pattern, patlen);
		free(pattern);
	    } else {
		ret = search_pattern(&sa, argv[optind], strlen(argv[optind]));
	    }

	    if (ret != 0) {
		msg(MSGLVL_ERR, "failed:");
		exit(EXIT_FAILURE);
	    }

	    if (opts.opt_v) msg(MSGLVL_INFO, "...done.");
	}
    }

    /* close suffix array */
    if (close_suffix_array(&sa) != 0) {
	msg(MSGLVL_ERR, "Cannot close suffix array:");
	exit(EXIT_FAILURE);
    }

    /* finalize */
    if (opts.opt_M) { cmap_free(&cm); free(opts.opt_M), opts.opt_M = NULL; }
    if (opts.opt_o) { free(opts.opt_o), opts.opt_o = NULL; }

    exit(EXIT_SUCCESS);
}
