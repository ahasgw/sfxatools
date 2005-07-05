/***********************************************************************
 * $Id: annotidx.c,v 1.3 2005/07/05 05:12:53 aki Exp $
 *
 * Annotate index.
 * Copyright (C) 2004,2005 RIKEN. All rights reserved.
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

#include <dirname.h>
#include <getopt.h>
#include <progname.h>
#include <xalloc.h>

#include <mmfile.h>
#include <msg.h>

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct {
    uint32_t	    seq_beg;
    uint32_t	    seq_end;
    uint32_t	    hdr_beg;
} hdx32_elem_t;

typedef struct {
    uint64_t	    seq_beg;
    uint64_t	    seq_end;
    uint32_t	    hdr_beg;
} hdx64_elem_t;

/*======================================================================
 * global variables
 *======================================================================*/

typedef struct opts_type {
    char	    *opt_o;	/* o: output file path */
    int		    opt_v;
    unsigned int    opt_h : 1;
    unsigned int    opt_u : 1;
    unsigned int    opt_V : 1;
    unsigned int    opt_L : 1;
    mmfile_t	    fhdr;	/* header text file */
    mmfile_t	    fhdx;	/* header index file */
} opts_t;

/*======================================================================
 * global variables
 *======================================================================*/

static opts_t opts = {
    NULL,
    0,
    0,
    0,
    0,
    0
	/* fhdr */
	/* fhdx */
};

/*======================================================================
 * prototypes
 *======================================================================*/

static void annotate_index(const char *fpath);
static hdx32_elem_t *bsearch_hdx32(const int32_t idx);
static hdx64_elem_t *bsearch_hdx64(const int64_t idx);
static void show_version(void);
static void show_help(void);
int main(int argc, char **argv);

/*======================================================================
 * function definitions
 *======================================================================*/

/* process files */
static void annotate_index(const char *fpath)
{
    if (opts.opt_v)
	msg(MSGLVL_INFO, "processing '%s'...", (fpath ? fpath : "stdin"));

    {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	/* redirect input file stream */
	if (fpath != NULL) {
	    if (freopen(fpath, "r", stdin) == NULL) {
		msg(MSGLVL_WARNING, "Cannot open file '%':", fpath);
		return;
	    }
	}

	if (!opts.opt_L) {	/* when 32bit sequence index */
	    while ((read = getline(&line, &len, stdin)) != -1) {
		if (read > 1 && *line != '#') {
		    char *endp = NULL;
		    int32_t idx = strtol(line, &endp, 10);
		    if (!errno) {
			hdx32_elem_t *elem;
			elem = bsearch_hdx32(idx);
			if (elem != NULL) {
			    line[read - 1] = '\0';  /* truncate delimiter */
			    while (*endp != '\0' && isspace(*endp))
				++endp;
			    printf("%ld\t%ld\t%s\t%s\n",
				    (long)idx,
				    (long)(idx - elem->seq_beg + 1),
				    endp,
				    (char*)mmfile_ptr(&opts.fhdr) + elem->hdr_beg
				   );
			    continue;
			}
		    }
		}
		printf("%s", line);	/* print the line as is */
	    }
	} else {		/* when 64bit sequence index */
	    while ((read = getline(&line, &len, stdin)) != -1) {
		if (read > 1 && *line != '#') {
		    char *endp = NULL;
		    int64_t idx = strtoll(line, &endp, 10);
		    if (!errno) {
			hdx64_elem_t *elem;
			elem = bsearch_hdx64(idx);
			if (elem != NULL) {
			    line[read - 1] = '\0';  /* truncate delimiter */
			    while (*endp != '\0' && isspace(*endp))
				++endp;
			    printf("%ld\t%ld\t%s\t%s\n",
				    (long)idx,
				    (long)(idx - elem->seq_beg + 1),
				    endp,
				    (char*)mmfile_ptr(&opts.fhdr) + elem->hdr_beg
				   );
			    continue;
			}
		    }
		}
		printf("%s", line);	/* print the line as is */
	    }
	}

	if (line) free(line);
    }

    if(opts.opt_v) msg(MSGLVL_INFO, "...done.");
}

/* binary sesarch header index */
static hdx32_elem_t *bsearch_hdx32(const int32_t idx)
{
    hdx32_elem_t *hdx_ptr = (hdx32_elem_t*)mmfile_ptr(&opts.fhdx);
    int32_t left = 0;
    int32_t right = (int32_t)(mmfile_len(&opts.fhdx) / sizeof(hdx32_elem_t));

    while (left < right) {
	int32_t mid = (left + right) / 2;
	if (hdx_ptr[mid].seq_end <= idx) {
	    left = mid + 1;
	} else if (idx < hdx_ptr[mid].seq_beg) {
	    right = mid;
	} else {
	    return hdx_ptr + mid;
	}
    }
    return NULL;
}

/* binary sesarch header index */
static hdx64_elem_t *bsearch_hdx64(const int64_t idx)
{
    hdx64_elem_t *hdx_ptr = (hdx64_elem_t*)mmfile_ptr(&opts.fhdx);
    int64_t left = 0;
    int64_t right = (int64_t)(mmfile_len(&opts.fhdx) / sizeof(hdx64_elem_t));

    while (left < right) {
	int64_t mid = (left + right) / 2;
	if (hdx_ptr[mid].seq_end <= idx) {
	    left = mid + 1;
	} else if (idx < hdx_ptr[mid].seq_beg) {
	    right = mid;
	} else {
	    return hdx_ptr + mid;
	}
    }
    return NULL;
}

/* show version number */
static void show_version(void)
{
    static char fmt[] =
	"annotidx (%s) %s\n"
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
	"This is annotidx, index annotation program.\n"
	"\n"
	"Usage: %s [options] <text_file> <index_file> [<index> ...]\n"
	"Options:\n"
	"  -h, --help           display this message\n"
	"  -V, --version        print version number\n"
	"  -v, --verbose        verbose output\n"
	"  -o, --output=<file>  file to output\n"
	"  -L, --large-index    large header index\n"
	"Report bugs to <%s>.\n"
	;
    fprintf(stdout, fmt, base_name(program_name), PACKAGE_BUGREPORT);
}

/* main */
int main(int argc, char **argv)
{
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
	    {"output",	    required_argument,	NULL, 'o'},
	    {"large-index", no_argument,	NULL, 'L'},
	    {0, 0, 0, 0}
	};

	opt = getopt_long(argc, argv, "hVvo:L", long_opts, &opt_index);
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    case 'o': opts.opt_o = xstrdup(optarg); break;
	    case 'L': opts.opt_L = 1; break;
	    default: show_help(); exit(EXIT_FAILURE);
	}
    }

    /* show version number and exit */
    if (opts.opt_V) { show_version(); exit(EXIT_SUCCESS); }

    /* show usage and exit */
    if (opts.opt_h) { show_help(); exit(EXIT_SUCCESS); }

    /* check arguments */
    if (argc < optind + 1) {
	msg(MSGLVL_ERR, "Neither text nor index file specified.");
	exit(EXIT_FAILURE);
    }
    if (argc < optind + 2) {
	msg(MSGLVL_ERR, "No index file specified.");
	exit(EXIT_FAILURE);
    }

    /* open output file */
    if (opts.opt_o != NULL) {
	if (freopen(opts.opt_o, "w", stdout) == NULL) {
	    msg(MSGLVL_ERR, "Cannot open file '%s':", opts.opt_o);
	    exit(EXIT_FAILURE);
	}
    }

    /* map header text file */
    if (!(argv[optind]
		&& mmfile_init(&opts.fhdr, argv[optind]) == 0
		&& mmfile_map_shared_rd(&opts.fhdr) == 0))
    {
	msg(MSGLVL_ERR, "cannot open header text file '%s':", argv[optind]);
	exit(EXIT_FAILURE);
    }
    ++optind;

    /* map header index file */
    if (!(argv[optind]
		&& mmfile_init(&opts.fhdx, argv[optind]) == 0
		&& mmfile_map_shared_rd(&opts.fhdx) == 0))
    {
	msg(MSGLVL_ERR, "cannot open header index file '%s':", argv[optind]);
	exit(EXIT_FAILURE);
    }
    ++optind;

    /* process files */
    if (optind < argc) {
	/* input from files */
	for (; optind < argc; ++optind) {
	    annotate_index(argv[optind]);
	}
    } else {
	/* input from stdin */
	annotate_index(NULL);
    }

    /* unmap header index file */
    if (mmfile_unmap(&opts.fhdx) != 0) {
	msg(MSGLVL_ERR, "cannot close header index file:");
	exit(EXIT_FAILURE);
    }

    /* unmap header text file */
    if (mmfile_unmap(&opts.fhdr) != 0) {
	msg(MSGLVL_ERR, "cannot close header text file:");
	exit(EXIT_FAILURE);
    }

    /* finalize */
    if (opts.opt_o) { free(opts.opt_o), opts.opt_o = NULL; }

    exit(EXIT_SUCCESS);
}
