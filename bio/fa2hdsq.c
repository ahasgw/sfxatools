/***********************************************************************
 * $Id: fa2hdsq.c,v 1.3 2005/07/05 05:12:53 aki Exp $
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
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <dirname.h>
#include <getopt.h>
#include <progname.h>
#include <xalloc.h>

#include <parsefa.h>
#include <ofiles.h>

#include <cmap.h>
#include <msg.h>
#include <strdupcat.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#define DIGITOF_INT64_T	20

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct opts_type {
    char	    *opt_M;	    /* mapping file path */
    cmap_t	    *cmap;	    /* character mapping */
    FILE	    *fhdrout;	    /* output file for header */
    FILE	    *fhdxout;	    /* output file for header */
    FILE	    *fseqout;	    /* output file for sequence */
    char	    *hdr_ext;
    char	    *hdx_ext;
    char	    *seq_ext;
    char	    *opt_b;
    int		    opt_S;	    /* split size */
    int		    opt_d;	    /* digits of file number extension */
    int		    opt_v;	    /* verbose level */
    unsigned int    opt_h: 1;
    unsigned int    opt_V: 1;
    unsigned int    opt_I: 1;
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
    INT32_MAX,
    0,
    0,
    /* binary flags */
    0,
    0,
    0
};

/*======================================================================
 * prototypes
 *======================================================================*/

static void process_file(const char *file);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

/* process files */
static void process_file(const char *file)
{
    const char *ifname = (file ? file : "stdin");
    const char *ofname = (opts.opt_b ? basename(opts.opt_b) : basename(ifname));
    ofnames_t ofnames;
    ofiles_t ofiles;

    if (opts.opt_v) msg(MSGLVL_INFO, "processing '%s'...", ifname);

    /* open files */
    if (file && (freopen(file, "r", stdin) == NULL)) {
	msg(MSGLVL_ERR, "cannot open input file '%s':", file);
	exit(EXIT_FAILURE);
    }
    ofnames_init(&ofnames, ofname,
	    opts.hdr_ext, opts.hdx_ext, opts.seq_ext, opts.opt_d);
    ofiles_open(&ofiles, &ofnames, 0);

    parsefa_param_t param = {opts.cmap, opts.opt_S, opts.opt_I};

    /* parse fasta */
#if SIZEOF_OFF_T < 8
    parse_fasta32(&ofiles, &ofnames, &param);
#else /* SIZEOF_OFF_T */
    if (opts.opt_S == 0) {
	parse_fasta64(&ofiles, &ofnames, &param);
    } else {
	parse_fasta32(&ofiles, &ofnames, &param);
    }
#endif /* SIZEOF_OFF_T */

    /* close files */
    ofiles_close(&ofiles);
    ofnames_free(&ofnames);

    if (opts.opt_v) msg(MSGLVL_INFO, "...done.");
}

/* show version number */
static void show_version(void)
{
    static char fmt[] =
	"fa2hdsq (%s) %s\n"
	"\n"
	"Copyright (C) 2004-2005 RIKEN. All rights reserved.\n"
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
	"This is fa2hdsq, FASTA format to header/sequence separator program.\n"
	"\n"
	"Usage: %s [options] [<file> ...]\n"
	"Options:\n"
	"  -h, --help               display this message\n"
	"  -V, --version            print version number, and exit\n"
	"  -v, --verbose            verbose output\n"
	"  -I, --ignore-case        ignore case\n"
	"  -S, --split-size=<size>  split size of sequence file\n"
	"                             (default: 2GB (max), 0 for large index)\n"
	"  -M, --map=<file>         character mapping file\n"
	"  -b, --basename=<name>    base name for newly created file (for stdin)\n"
	"  -d, --digits=<n>         digits of file number extension\n"
	"Report bugs to <%s>.\n"
	;
    fprintf(stdout, fmt, base_name(program_name), PACKAGE_BUGREPORT);
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
	int opt_index = 0;
	static struct option long_opts[] = {
	    {"help",	    no_argument,	NULL, 'h'},
	    {"version",	    no_argument,	NULL, 'V'},
	    {"verbose",	    no_argument,	NULL, 'v'},
	    {"ignore-case", no_argument,	NULL, 'I'},
	    {"split-size",  required_argument,	NULL, 'S'},
	    {"map",	    required_argument,	NULL, 'M'},
	    {"basename",    required_argument,	NULL, 'b'},
	    {"digits",	    required_argument,  NULL, 'd'},
	    {0, 0, 0, 0}
	};
	opt = getopt_long(argc, argv, "hVvIS:M:b:d:", long_opts, &opt_index);
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    case 'I': opts.opt_I = 1; break;
	    case 'S': opts.opt_S = atoi(optarg); break;
	    case 'M': opts.opt_M = xstrdup(optarg); break;
	    case 'b': opts.opt_b = xstrdup(optarg); break;
	    case 'd': opts.opt_d = atoi(optarg); break;
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

    /* check argments */
    if (opts.opt_S < 0) {
	msg(MSGLVL_ERR, "Split size cannot be negative value.");
	exit(EXIT_FAILURE);
    }
    if (opts.opt_v && opts.opt_S < 1000) {
	msg(MSGLVL_WARNING, "Split size is very small. Use with caution.");
    }

    /* process files */
    if (optind < argc) {
	for (; optind < argc; ++optind) {
	    process_file(argv[optind]);
	}
    } else {
	process_file(NULL);
    }

    /* finalize */
    if (opts.opt_M) { cmap_free(&cm); free(opts.opt_M), opts.opt_M = NULL; }
    if (opts.opt_b) { free(opts.opt_b), opts.opt_b = NULL; }

    exit(EXIT_SUCCESS);
}
