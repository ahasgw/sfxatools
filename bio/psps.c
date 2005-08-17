/***********************************************************************
 * $Id: psps.c,v 1.1 2005/08/17 10:11:41 aki Exp $
 *
 * psps
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
#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include <dirname.h>
#include <getopt.h>
#include <getsubopt.h>
#include <progname.h>
#include <xalloc.h>

#include "region.h"
#include "search.h"
#include "cmap.h"

#include "prosite.h"

#include "mmfile.h"
#include "msg.h"
#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#define DEFAULT_F_HDR	(1)	/* whether print information header */
#define DEFAULT_F_POS	(0)	/* whether print position column */
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

typedef struct opts_type {
    char	    *opt_o;	/* output file path */
    char	    *opt_i;	/* input file path */
    char	    *opt_M;	/* mapping file path */
    int		    opt_v;	/* verbose level */
    int		    opt_R;	/* maximum repeat number */
    unsigned int    opt_h: 1;	/* help flag */
    unsigned int    opt_V: 1;	/* version flag */
    unsigned int    opt_q: 1;	/* quiet */
    output_param_t  opt_F;
} opts_t;

/*======================================================================
 * global variables
 *======================================================================*/

static opts_t opts = {
    /* pointers */
    NULL,		/* o: output file path */
    NULL,		/* i: input file path */
    NULL,		/* M: mapping file path */
    /* numerals */
    0,			/* v: verbose level */
    INT_MAX,		/* R: maximum repeat number */
    /* binary flags */
    0,			/* h: help flag */
    0,			/* V: version flag */
    0,			/* q: quiet */
    /* others */
    {
	NULL,
	DEFAULT_F_HDR,
	DEFAULT_F_POS,
	DEFAULT_F_IDX,
	DEFAULT_F_SFX,
	DEFAULT_F_PRE,
	DEFAULT_F_CHOP,
	0
    }
};

static const char alphabet[] = "ABCDEFGHIKLMNPQRSTVWXYZ";

/*======================================================================
 * prototypes
 *======================================================================*/

inline static int get_max_digit(unsigned long long n);
static void search(const sfxa_t *sfxa, const cmap_t *cm, char *pat);
static int search_pattern(const sfxa_t *sfxa, const char *pattern, int patlen);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

inline static int get_max_digit(unsigned long long n)
{
    char buf[64];
    snprintf(buf, 64, "%llu", n);
    return strlen(buf);
}

static void search(const sfxa_t *sfxa, const cmap_t *cm, char *pat)
{
    int ret = 0;
    mbuf_t re;
    time_t beg_tm = 0, end_tm = 0;

    if (opts.opt_v) {
	msg(MSGLVL_INFO, "searching PA pattern '%s'...", pat);
	beg_tm = time(NULL);
    }

    if (mbuf_init(&re, NULL, strlen(pat) + 1) != 0) {
	msg(MSGLVL_ERR, "Failed to prepare pattern conversion:");
	exit(EXIT_FAILURE);
    }

    if ((ret = prosite_to_regex(pat, &re)) != 0) {
	if (errno != 0)
	    msg(MSGLVL_ERR, "Failed to convert a pattern '%s':", pat);
	else
	    msg(MSGLVL_ERR,
		    "Illegal character '%c' in a pattern '%s', (col. %d)",
		    pat[ret - 1], pat, ret);
	exit(EXIT_FAILURE);
    }
    if (opts.opt_v > 1)
	msg(MSGLVL_INFO, "searching RE pattern '%s'...", (const char *)mbuf_ptr(&re));

    if (opts.opt_M) {
	unsigned char *pattern;
	int patlen;
	ret = cmap_translate(cm, mbuf_ptr(&re), &pattern, &patlen);
	if (ret != 0) {
	    msg(MSGLVL_ERR, "Cannot map the query:");
	    exit(EXIT_FAILURE);
	}
	ret = search_pattern(sfxa, (char *)pattern, patlen);
	free(pattern);
    } else {
	ret = search_pattern(sfxa, mbuf_ptr(&re), mbuf_size(&re));
    }

    if (ret != 0) {
	msg(MSGLVL_ERR, "Failed:");
	exit(EXIT_FAILURE);
    }

    mbuf_free(&re);

    if (opts.opt_v) {
	end_tm = time(NULL);
	msg(MSGLVL_INFO, "...done. (%.1f sec.)", difftime(end_tm, beg_tm));
    }
}


/* search pattern using suffix array */
static int search_pattern(const sfxa_t *sfxa, const char *pattern, int patlen)
{
    int ret = 0;
    region_t result;

    if ((ret = region_init(&result, sfxa)) != 0)
	return ret;

    ret = region_search_regexp(&result, pattern, patlen, alphabet, opts.opt_R);

    if (ret == 0) {
	region_narrow_down(&result);
	if (opts.opt_F.hdr) {
	    printf("## pat:'%*s', cnt:%lu\n", patlen, pattern,
		    (unsigned long)region_count(&result));
	}
	if (!opts.opt_q) {
	    opts.opt_F.max_digit = get_max_digit(sfxa_txt_len(sfxa));
	    ret = region_print(&result, &opts.opt_F);
	}
    }

    region_free(&result);
    return 0;
}

/* parse suboption F */
static void parse_subopt_F(char **optionp)
{
    char *val;
    int op;
    enum {
	HDR = 0, NOHDR, POS, NOPOS, IDX, NOIDX, SFX, NOSFX,
	PRE, NOPRE, CHOP, NOCHOP
    };
    static char *subop[] = {
	"hdr", "nohdr", "pos", "nopos", "idx", "noidx", "sfx", "nosfx",
	"pre", "nopre", "chop", "nochop", NULL
    };
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
    opts.opt_q = ((opts.opt_F.hdr == 0 && opts.opt_F.pos == 0
		&& opts.opt_F.idx == 0 && opts.opt_F.sfx == 0
		/* && opts.opt_F.pre <= 0 */
		) ? 1 : 0);
}

/* show version number */
static void show_version(void)
{
    static char fmt[] =
	"psps (%s) %s\n"
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
	"This is psps, prosite pattern search program.\n"
	"\n"
	"Usage: %s [options] <text_file> <index_file> [pattern ...]\n"
	"Options:\n"
	"  -h, --help            display this message\n"
	"  -V, --version         print version number, and exit\n"
	"  -v, --verbose         verbose output\n"
	"  -o, --output=<file>   file to output\n"
	"  -i, --input=<file>    file to input\n"
	"  -q, --quiet           output quietly\n"
	"  -R, --repeat-max=<n>  limit maximum repeat number to <n> (+,*,{m,})\n"
	"  -M, --map=<file>      character mapping file\n"
	"  -F, --format=<comma_separated_subopts>  formatting parameters\n"
	"        [no]hdr         [do not] print information header\n"
	"        [no]pos         [do not] print array position column\n"
	"        [no]idx         [do not] print index column\n"
	"        [no]sfx=<n>     [do not] print suffix at most length <n>\n"
#if 0
	"        [no]pre=<n>     [do not] print <n> characters ahead of the suffix\n"
#endif
	"        [no]chop        [do not] chop suffix beyond delimiter character\n"
	"Report bugs to <%s>.\n"
	;
    fprintf(stdout, fmt, base_name(program_name), PACKAGE_BUGREPORT);
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
	int opt_index = 0;
	static struct option long_opts[] = {
	    {"help",	    no_argument,	NULL, 'h'},
	    {"version",	    no_argument,	NULL, 'V'},
	    {"verbose",	    no_argument,	NULL, 'v'},
	    {"output",      required_argument,	NULL, 'o'},
	    {"input",	    required_argument,	NULL, 'i'},
	    {"quiet",	    no_argument,	NULL, 'q'},
	    {"repeat-max",  required_argument,	NULL, 'R'},
	    {"map",	    required_argument,	NULL, 'M'},
	    {"format",      required_argument,	NULL, 'F'},
	    {0, 0, 0, 0}
	};

	opt = getopt_long(argc, argv, "hVvo:i:qR:M:F:", long_opts, &opt_index);
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    case 'o': opts.opt_o = xstrdup(optarg); break;
	    case 'i': opts.opt_i = xstrdup(optarg); break;
	    case 'q': opts.opt_q = 1; break;
	    case 'R': opts.opt_R = atoi(optarg); break;
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
	if (sfxa_init(&sa, ftxt, fidx, NULL) != 0
		|| sfxa_open(&sa) != 0) {
	    msg(MSGLVL_ERR, "Cannot open suffix array:");
	    exit(EXIT_FAILURE);
	}
    }

    /* redirect input file stream */
    if (opts.opt_i != NULL) {
	if (freopen(opts.opt_i, "r", stdin) == NULL) {
	    msg(MSGLVL_ERR, "Cannot open file '%s':", opts.opt_i);
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
	printf("# txt:'%s', idx:'%s', size:%llu\n",
		sfxa_txt_path(&sa), sfxa_idx_path(&sa), 
		(unsigned long long)sfxa_txt_len(&sa));
    }

    /* search the pattern */
    if (opts.opt_i != NULL || optind == argc) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, stdin)) != -1) {
	    if (read < 1) break;
	    if (line[0] == '#') continue;   /* skip comment line */
	    line[read - 1] = '\0';
	    search(&sa, &cm, line);
	}
	if (line) free(line);
    } else {
	for (; optind < argc; ++optind) {
	    search(&sa, &cm, argv[optind]);
	}
    }

    /* close suffix array */
    sfxa_close(&sa);
    sfxa_free(&sa);

    /* finalize */
    if (opts.opt_M) { cmap_free(&cm); free(opts.opt_M), opts.opt_M = NULL; }
    if (opts.opt_o) { free(opts.opt_o), opts.opt_o = NULL; }
    if (opts.opt_i) { free(opts.opt_i), opts.opt_i = NULL; }

    exit(EXIT_SUCCESS);
}
