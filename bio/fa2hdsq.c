/***********************************************************************
 * $Id: fa2hdsq.c,v 1.1 2005/06/11 06:23:09 aki Exp $
 *
 * Fasta to header/sequence separator
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

#include <parsefa.h>

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <progname.h>
#include <string.h>
#include <xalloc.h>

#include <cmap.h>
#include <msg.h>
#include <strdupcat.h>

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct opts_type {
    char	    *opt_M;	    /* mapping file path */
    cmap_t	    *cmap;	    /* character mapping */
    FILE	    *fhdrout;	    /* output file for header */
    FILE	    *fhdxout;	    /* output file for header */
    FILE	    *fseqout;	    /* output file for sequence */
    char	    *hdr_suffix;
    char	    *hdx_suffix;
    char	    *seq_suffix;
    char	    *opt_b;
    int		    opt_v;	    /* verbose level */
    unsigned int    opt_h: 1;
    unsigned int    opt_V: 1;
    unsigned int    opt_I: 1;
    unsigned int    opt_L: 1;
} opts_t;

/*======================================================================
 * global variables
 *======================================================================*/

static opts_t opts = {
    /* pointers */
    NULL,		/* M: mapping file path */
    NULL,		/* character mapping */
    NULL,
    NULL,
    NULL,
    ".hdr",
    ".hdx",
    ".seq",
    NULL,
    /* numerals */
    0,
    /* binary flags */
    0,
    0,
    0,
    0
};

/*======================================================================
 * prototypes
 *======================================================================*/

static void process_file(const char *file);
static void process_stdin(void);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

/* process files */
static void process_file(const char *file)
{
    char *fhdrout_name = NULL;
    char *fhdxout_name = NULL;
    char *fseqout_name = NULL;

    if (opts.opt_v) msg(MSGLVL_INFO, "processing '%s'...", file);

    {
	char *name = xstrdup(file);
	char *base = basename(name);
	fhdrout_name = strdupcat(base, opts.hdr_suffix);
	fhdxout_name = strdupcat(base, opts.hdx_suffix);
	fseqout_name = strdupcat(base, opts.seq_suffix);
	free(name), name = NULL;
    }

    /* open files */
    if (freopen(file, "r", stdin) == NULL) {
	msg(MSGLVL_ERR, "cannot open input file '%s':", file);
	exit(EXIT_FAILURE);
    }
    if ((opts.fhdrout = fopen(fhdrout_name, "w")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .hdr output file '%s':", fhdrout_name);
	exit(EXIT_FAILURE);
    }
    if ((opts.fhdxout = fopen(fhdxout_name, "wb")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .hdx output file '%s':", fhdxout_name);
	exit(EXIT_FAILURE);
    }
    if ((opts.fseqout = fopen(fseqout_name, "wb")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .seq output file '%s':", fseqout_name);
	exit(EXIT_FAILURE);
    }

    parsefa_param_t param = {
	opts.cmap, opts.fseqout, opts.fhdxout, opts.fhdrout, opts.opt_I
    };

    /* parse fasta */
    if (opts.opt_L) {
	parse_fasta64(&param);
    } else {
	parse_fasta32(&param);
    }

    /* close files */
    fclose(opts.fseqout), opts.fseqout = NULL;
    fclose(opts.fhdxout), opts.fhdxout = NULL;
    fclose(opts.fhdrout), opts.fhdrout = NULL;

    free(fseqout_name), fseqout_name = NULL;
    free(fhdxout_name), fhdxout_name = NULL;
    free(fhdrout_name), fhdrout_name = NULL;

    if (opts.opt_v) msg(MSGLVL_INFO, "...done.");
}

/* process files */
static void process_stdin(void)
{
    char *file = ((opts.opt_b == NULL) ? "stdin" : basename(opts.opt_b));
    char *fhdrout_name = NULL;
    char *fhdxout_name = NULL;
    char *fseqout_name = NULL;

    if (opts.opt_v) msg(MSGLVL_INFO, "processing '%s'...", file);

    {
	fhdrout_name = strdupcat(file, opts.hdr_suffix);
	fhdxout_name = strdupcat(file, opts.hdx_suffix);
	fseqout_name = strdupcat(file, opts.seq_suffix);
    }

    /* open files */
    if ((opts.fhdrout = fopen(fhdrout_name, "w")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .hdr output file '%s':", fhdrout_name);
	exit(EXIT_FAILURE);
    }
    if ((opts.fhdxout = fopen(fhdxout_name, "wb")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .hdx output file '%s':", fhdxout_name);
	exit(EXIT_FAILURE);
    }
    if ((opts.fseqout = fopen(fseqout_name, "wb")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .seq output file '%s':", fseqout_name);
	exit(EXIT_FAILURE);
    }

    parsefa_param_t param = {
	opts.cmap, opts.fseqout, opts.fhdxout, opts.fhdrout, opts.opt_I
    };

    /* parse fasta */
    if (opts.opt_L) {
	parse_fasta64(&param);
    } else {
	parse_fasta32(&param);
    }

    /* close files */
    fclose(opts.fseqout), opts.fseqout = NULL;
    fclose(opts.fhdxout), opts.fhdxout = NULL;
    fclose(opts.fhdrout), opts.fhdrout = NULL;

    free(fseqout_name), fseqout_name = NULL;
    free(fhdrout_name), fhdrout_name = NULL;
    free(fhdxout_name), fhdxout_name = NULL;

    if (opts.opt_v) msg(MSGLVL_INFO, "...done.");
}

/* show version number */
static void show_version(void)
{
    fprintf(stdout, "fa2hdsq (%s) %s\n", PACKAGE, VERSION);
}

/* show help */
static void show_help(void)
{
    static char fmt[] =
	"This is fa2hdsq, FASTA format to header/sequence separator program.\n"
	"Copyright (C) 2004-2005 RIKEN. All rights reserved.\n"
	"This program comes with ABSOLUTELY NO WARRANTY.\n"
	"You may redistribute copies of this program under the terms of the\n"
	"GNU General Public License.\n"
	"For more information about these matters, see the file named COPYING.\n"
	"\n"
	"Usage: %s [options] [<file> ...]\n"
	"Options:\n"
#ifdef HAVE_GETOPT_LONG
	"  -h, --help             display this message\n"
	"  -V, --version          print version number, and exit\n"
	"  -v, --verbose          verbose output\n"
	"  -I, --ignore-case      ignore case\n"
	"  -L, --large-index      large header index\n"
	"  -M, --map=<file>       character mapping file\n"
	"  -b, --basename=<name>  base name for newly created file (for stdin)\n"
#else
	"  -h           display this message\n"
	"  -V           print version number, and exit\n"
	"  -v           verbose output\n"
	"  -I           ignore case\n"
	"  -L           large header index\n"
	"  -M <file>    character mapping file\n"
	"  -b <name>    base name for newly created file (for stdin)\n"
#endif
	"Report bugs to %s.\n"
	;
    fprintf(stdout, fmt, program_name, PACKAGE_BUGREPORT);
}

/* main */
int main(int argc, char **argv)
{
    cmap_t cm;

    /* preserve program name */
    set_program_name(argv[0]);

    /* manage options */
    for (;;) {
	int opt;
#ifdef HAVE_GETOPT_LONG
	int opt_index = 0;
	static struct option long_opts[] = {
	    {"help",	    no_argument,	NULL, 'h'},
	    {"version",	    no_argument,	NULL, 'V'},
	    {"verbose",	    no_argument,	NULL, 'v'},
	    {"ignore-case", no_argument,	NULL, 'I'},
	    {"large-index", no_argument,	NULL, 'L'},
	    {"map",	    required_argument,	NULL, 'M'},
	    {"basename",    required_argument,	NULL, 'b'},
	    {0, 0, 0, 0}
	};

	opt = getopt_long(argc, argv, "hVvILM:b:", long_opts, &opt_index);
#else
	opt = getopt(argc, argv, "hVvILM:b:");
#endif
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    case 'I': opts.opt_I = 1; break;
	    case 'L': opts.opt_L = 1; break;
	    case 'M': opts.opt_M = xstrdup(optarg); break;
	    case 'b': opts.opt_b = xstrdup(optarg); break;
	    default: show_help(); exit(EXIT_FAILURE);
	}
    }

    /* show version number and exit */
    if (opts.opt_V) { show_version(); exit(EXIT_SUCCESS); }

    /* show help message and exit */
    if (opts.opt_h) { show_help(); exit(EXIT_SUCCESS); }

    /* prepare cmap */
    if (opts.opt_M) {
	if (cmap_load(&cm, opts.opt_M) != 0) {
	    msg(MSGLVL_ERR, "Cannot open map file '%s':", opts.opt_M);
	    exit(EXIT_FAILURE);
	}
	opts.cmap = &cm;
    }

    /* process files */
    if (optind < argc) {
	for (; optind < argc; ++optind) {
	    process_file(argv[optind]);
	}
    } else {
	process_stdin();
    }

    /* finalize */
    if (opts.opt_M) { cmap_free(&cm); free(opts.opt_M), opts.opt_M = NULL; }
    if (opts.opt_b) { free(opts.opt_b), opts.opt_b = NULL; }

    exit(EXIT_SUCCESS);
}