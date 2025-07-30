/***********************************************************************
 * Copyright (C) 2005, 2006 RIKEN. All rights reserved.
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
#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include <netinet/in.h>

#include <dirname.h>
#include <getopt.h>
#include <progname.h>
#include <xalloc.h>

#include "cmap.h"
#include "mmfile.h"
#include "msg.h"

/*
 * Format of the pin/nin file:
 *
 *   A: [0 . 3]: version number (big endian)
 *   B: [4 . 7]: dump flag [0: nucleotide, 1: protein] (big endian)
 *   C: [8 . 11]: length of the db title (strlen) (big endian)
 *   D: [12 . 11+(C)]: title string
 *   E: [E_ . E_+3]: skip length of the date/time string (big endian)
 *   F: [E_+4 . E_+3+(E)]: date/time string
 *   G: [G_ . G_+3]: number of sequence entry (big endian)
 *   H: [G_+4 . G_+11]: total length of the proper sequence (little endian)
 *   I: [G_+12 . G_+15]: maximum length of the sequence of all (big endian)
 *
 *       E_ = 11+(C)+((4-((C)%4))%4)
 *       G_ = E_+4+(E)
 */

/*======================================================================
 * macro definitions
 *======================================================================*/

#define EXTLEN	4

/*======================================================================
 * type definitions
 *======================================================================*/

typedef struct opts_type {
    int		    opt_v;	/* verbose level */
    unsigned int    opt_h: 1;	/* help flag */
    unsigned int    opt_V: 1;	/* version flag */
} opts_t;

typedef struct info_type {
    uint32_t	version;
    uint32_t	dumpflag;
    size_t	titlelen;
    char	*title;
    size_t	datelen;
    char	*date;
    uint32_t	max_cnt;
    uint64_t	tot_len;
    uint32_t	max_len;
} info_t;

typedef struct offsets_type {
    const uint32_t *def;
    const uint32_t *seq;
    const uint32_t *amb;
} ofs_t;

/*======================================================================
 * global variables
 *======================================================================*/

static opts_t opts = {
    /* numerals */
    0,				/* v: verbose level */
    /* binary flags */
    0,				/* h: help flag */
    0				/* V: version flag */
};

static info_t	info;
static cmap_t	cmap;		/* character mapping */
static cmap_t	cmap_amb;	/* character mapping */

/*======================================================================
 * prototypes
 *======================================================================*/

inline static uint64_t read_uint64el(unsigned char *b);
static int get_cnt(const unsigned char **p);

static void read_indfile(void * const top, ofs_t *ofs);
static void print_indinfo(const info_t *info);
static void print_protein(const char * const hdr, const char * const seq, ofs_t *ofs);
static void print_nucleotide(const char * const hdr, const char * const seq, ofs_t *ofs);
static void process(const char *idxpath);
static mmfile_t *mfrdopen(const char *path);
static void mfclose(mmfile_t *mf);
static void mkmap_NCBIstdaa(cmap_t *cmap);
static void mkmap_NCBI2na(cmap_t *cmap);
static void mkmap_NCBI4na(cmap_t *cmap);
static void show_version(void);
static void show_help(void);

/*======================================================================
 * function definitions
 *======================================================================*/

inline static uint64_t read_uint64el(unsigned char *b)
{
    return
	(   (uint64_t)b[7] << 56
	    | (uint64_t)b[6] << 48
	    | (uint64_t)b[5] << 40
	    | (uint64_t)b[4] << 32
	    | (uint64_t)b[3] << 24
	    | (uint64_t)b[2] << 16
	    | (uint64_t)b[1] << 8
	    | (uint64_t)b[0]
	);
}

static int get_cnt(const unsigned char **p)
{
    int cnt = **p;
    int len;

    if (cnt <= 127) {
	++*p;
	return cnt;
    }

    len = cnt - 128;
    for (cnt = 0, ++*p; len > 0; ++*p, --len)
	cnt = ((cnt << 8) + (int)**p);
    return cnt;
}

/* read index file */
static void read_indfile(void * const top, ofs_t *ofs)
{
    ptrdiff_t off;
    union {
	void *v;
	char *c;
	unsigned char *uc;
	uint32_t *u32;
	uint64_t *u64;
    } ptr;

    ptr.v = top;

    info.version = ntohl(*ptr.u32++);
    info.dumpflag = ntohl(*ptr.u32++);
    info.titlelen = ntohl(*ptr.u32++);
    info.title = strndup(ptr.c, info.titlelen), ptr.c += info.titlelen;
    info.datelen = ntohl(*ptr.u32++);
    info.date = strndup(ptr.c, info.datelen), ptr.c += info.datelen;

    /* align to 4 bytes boundary */
    off = ptr.c - (char *)top;
    if (off & 0x03) {
	off += 0x03UL, off &= ~0x03UL, ptr.c = (char *)top + off;
    }

    info.max_cnt = ntohl(*ptr.u32++);
    if (info.version < 4) {
	info.tot_len = ntohl(*ptr.u32++);
    } else {
	info.tot_len = read_uint64el(ptr.uc),
	    ptr.u64++;
    }
    info.max_len = ntohl(*ptr.u32++);

    ofs->def = ptr.u32;
    ofs->seq = ofs->def + info.max_cnt + 1;
    ofs->amb = (info.dumpflag ? NULL : ofs->seq + info.max_cnt + 1);
}

/* print index info */
static void print_indinfo(const info_t *info)
{
    printf("# version:\t%u\n", info->version);
    printf("# dump flag:\t%d (%s)\n", info->dumpflag,
	    (info->dumpflag ? "protein" : "nucleotide"));
    printf("# db title:\t[%s]\n", info->title);
    printf("# date/time:\t[%s]\n", info->date);
    printf("# max count:\t%d\n", info->max_cnt);
    printf("# total len:\t%lld\n", (long long)info->tot_len);
    printf("# max len:\t%d\n", info->max_len);
}

/* print protein */
static void print_protein(const char * const hdr, const char * const seq, ofs_t *ofs)
{
    uint32_t i;
    for (i = 0; i < info.max_cnt; ++i) {
	uint32_t d = ntohl(*ofs->def++);
	uint32_t s = ntohl(*ofs->seq++), nexts = ntohl(*ofs->seq);
	const unsigned char *dstr = (const unsigned char *)hdr + d;
	uint32_t slen = nexts - s;
	int dlen;
	dstr += 7;
	dlen = get_cnt(&dstr);

	if (opts.opt_v)
	    printf("## === [%u] def:(%d@%u) seq:(%u@%u) ===\n",
		    i, dlen, d, slen, s);

	if (opts.opt_v > 1)
	    printf(">%.*s\n", dlen, dstr);

	if (opts.opt_v > 2) {
	    uint32_t j;
	    for (j = 0; j < slen - 1; ++j) {
		int ch = cmap_num2char(&cmap, *(seq + s + j));
		putchar(ch);
	    }
	    putchar('\n');
	}
    }
}

/* print nucleotide */
static void print_nucleotide(const char * const hdr, const char * const seq, ofs_t *ofs)
{
    char *buf = NULL;
    uint32_t i;

    if (opts.opt_v == 3)
	buf = (char *)xmalloc(info.max_len + 1);

    for (i = 0; i < info.max_cnt; ++i) {
	uint32_t d = ntohl(*ofs->def++);
	uint32_t s = ntohl(*ofs->seq++), nexts = ntohl(*ofs->seq);
	uint32_t a = ntohl(*ofs->amb++);
	const unsigned char *dstr = (const unsigned char *)hdr + d;
	uint32_t alen = nexts - a;
	uint32_t slen = a - s;
	int dlen;
	dstr += 7;
	dlen = get_cnt(&dstr);

	if (opts.opt_v)
	    printf("## === [%u] def:(%d@%u) seq:(%u@%u) amb:(%u@%u) ===\n",
		    i, dlen, d, slen, s, alen, a);

	if (opts.opt_v > 1)
	    printf(">%.*s\n", dlen, dstr);

	if (opts.opt_v == 3) {
	    char *bufp = buf;
	    uint32_t j;
	    for (j = 0; j < slen - 1; ++j) {
		unsigned char ch = *(seq + s + j);
		int k;
		for (k = 6; k >= 0; k -= 2) {
		    *bufp++ = (char)cmap_num2char(&cmap, (ch >> k) & 0x03);
		}
	    }
	    { /* when (j == slen - 1), last seq byte */
		unsigned char ch = *(seq + s + j);
		int len = ch & 0x03;
		int k;
		for (k = 6; k >= (8 - (len * 2)); k -= 2) {
		    *bufp++ = (char)cmap_num2char(&cmap, (ch >> k) & 0x03);
		}
	    }
	    *bufp++ = '\0';

	    if (alen > 0) {
		uint32_t *amblst = (uint32_t*)(seq + a);
		uint32_t listlen = ntohl(*amblst);
		uint32_t j;
		for (j = 1; j <= listlen; ++j) {
		    uint32_t amb = ntohl(*(amblst + j));
		    int ch = (amb >> 28);
		    uint32_t off = amb & ~((uint32_t)(0xf) << 28);
		    buf[off] = (char)cmap_num2char(&cmap_amb, ch);
		}
	    }
	    printf("%s\n", buf);
	}
	else if (opts.opt_v > 3) {
	    uint32_t j;
	    for (j = 0; j < slen - 1; ++j) {
		unsigned char ch = *(seq + s + j);
		int k;
		for (k = 6; k >= 0; k -= 2) {
		    char c = cmap_num2char(&cmap, (ch >> k) & 0x03);
		    putchar(c);
		}
	    }
	    { /* when (j == slen - 1), last seq byte */
		unsigned char ch = *(seq + s + j);
		int len = ch & 0x03;
		int k;
		for (k = 6; k >= (8 - (len * 2)); k -= 2) {
		    char c = cmap_num2char(&cmap, (ch >> k) & 0x03);
		    putchar(c);
		}
	    }
	    putchar('\n');

	    if (alen > 0) {
		uint32_t *amblst = (uint32_t*)(seq + a);
		uint32_t listlen = ntohl(*amblst);
		uint32_t j;
		for (j = 1; j <= listlen; ++j) {
		    uint32_t amb = ntohl(*(amblst + j));
		    int ch = (amb >> 28);
		    uint32_t off = amb & ~((uint32_t)(0xf) << 28);
		    printf("### [%u] -> %c\n", off, cmap_num2char(&cmap_amb, ch));
		}
	    }
	}
    }

    if (opts.opt_v == 3)
	free(buf), buf = NULL;
}

/* process file */
static void process(const char *idxpath)
{
    char *hdrpath = xstrdup(idxpath);
    char *seqpath = xstrdup(idxpath);
    mmfile_t *mf_idx = NULL;
    mmfile_t *mf_hdr = NULL;
    mmfile_t *mf_seq = NULL;
    ofs_t ofs = {NULL, NULL, NULL};
    const char *hdr, *seq;

    /* ind file */
    mf_idx = mfrdopen(idxpath);		    /* open index file */
    read_indfile(mmfile_ptr(mf_idx), &ofs); /* get information from ind file */
    print_indinfo(&info);		    /* print information */

    /* make header file path, then open */
    hdrpath[strlen(idxpath) - EXTLEN] = '\0';
    strcat(hdrpath, (info.dumpflag ? ".phr" : ".nhr"));
    mf_hdr = mfrdopen(hdrpath);
    hdr = (const char *)mmfile_ptr(mf_hdr);

    /* make sequence file paths, then open */
    seqpath[strlen(idxpath) - EXTLEN] = '\0';
    strcat(seqpath, (info.dumpflag ? ".psq" : ".nsq"));
    mf_seq = mfrdopen(seqpath);
    seq = (const char *)mmfile_ptr(mf_seq);

    /* print offset info */
    if (info.dumpflag) {
	mkmap_NCBIstdaa(&cmap);
	print_protein(hdr, seq, &ofs);
    } else {
	mkmap_NCBI2na(&cmap);
	mkmap_NCBI4na(&cmap_amb);
	print_nucleotide(hdr, seq, &ofs);
    }

    /* clean up */
    mfclose(mf_seq);
    mfclose(mf_hdr);
    mfclose(mf_idx);
    free(seqpath);
    free(hdrpath);
}

/* mmfile read open */
static mmfile_t *mfrdopen(const char *path)
{
    mmfile_t *mf = NULL;
    if ((mf = mmfile_new(path)) == NULL || mmfile_map_private_rd(mf) != 0) {
	if (mf)
	    mmfile_delete(mf), mf = NULL;
	msg(MSGLVL_ERR, "Cannot open file '%s':", path);
	exit(EXIT_FAILURE);
    }
    return mf;
}

/* mmfile close */
static void mfclose(mmfile_t *mf)
{
    const char *path = mmfile_path(mf);
    if (mmfile_unmap(mf) != 0) {
	msg(MSGLVL_ERR, "Cannot close file '%s':", path);
	exit(EXIT_FAILURE);
    }
    mmfile_delete(mf);
}

/* make NCBIstdaa character mapping */
static void mkmap_NCBIstdaa(cmap_t *cmap)
{
    cmap_init(cmap);
    cmap_assign(cmap, '-', 0);
    cmap_assign(cmap, 'A', 1);
    cmap_assign(cmap, 'B', 2);
    cmap_assign(cmap, 'C', 3);
    cmap_assign(cmap, 'D', 4);
    cmap_assign(cmap, 'E', 5);
    cmap_assign(cmap, 'F', 6);
    cmap_assign(cmap, 'G', 7);
    cmap_assign(cmap, 'H', 8);
    cmap_assign(cmap, 'I', 9);
    cmap_assign(cmap, 'K', 10);
    cmap_assign(cmap, 'L', 11);
    cmap_assign(cmap, 'M', 12);
    cmap_assign(cmap, 'N', 13);
    cmap_assign(cmap, 'P', 14);
    cmap_assign(cmap, 'Q', 15);
    cmap_assign(cmap, 'R', 16);
    cmap_assign(cmap, 'S', 17);
    cmap_assign(cmap, 'T', 18);
    cmap_assign(cmap, 'V', 19);
    cmap_assign(cmap, 'W', 20);
    cmap_assign(cmap, 'X', 21);
    cmap_assign(cmap, 'Y', 22);
    cmap_assign(cmap, 'Z', 23);
    cmap_assign(cmap, 'U', 24);
    cmap_assign(cmap, '*', 25);
}

/* make NCBI2na character mapping */
static void mkmap_NCBI2na(cmap_t *cmap)
{
    cmap_init(cmap);
    cmap_assign(cmap, 'A', 0);
    cmap_assign(cmap, 'C', 1);
    cmap_assign(cmap, 'G', 2);
    cmap_assign(cmap, 'T', 3);
}

/* make NCBI4na character mapping */
static void mkmap_NCBI4na(cmap_t *cmap)
{
    cmap_init(cmap);
    cmap_assign(cmap, '-', 0);
    cmap_assign(cmap, 'A', 1);
    cmap_assign(cmap, 'C', 2);
    cmap_assign(cmap, 'M', 3);
    cmap_assign(cmap, 'G', 4);
    cmap_assign(cmap, 'R', 5);
    cmap_assign(cmap, 'S', 6);
    cmap_assign(cmap, 'V', 7);
    cmap_assign(cmap, 'T', 8);
    cmap_assign(cmap, 'W', 9);
    cmap_assign(cmap, 'Y', 10);
    cmap_assign(cmap, 'H', 11);
    cmap_assign(cmap, 'K', 12);
    cmap_assign(cmap, 'D', 13);
    cmap_assign(cmap, 'B', 14);
    cmap_assign(cmap, 'N', 15);
}

/* show version number */
static void show_version(void)
{
    static char fmt[] =
	"bl2dump (%s) %s\n"
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
	"This is bl2dump, Blast2 database dump program.\n"
	"\n"
	"Usage: %s [options] <(.pin|.nin)_file>\n"
	"Options:\n"
	"  -h, --help               display this message\n"
	"  -V, --version            print version number, and exit\n"
	"  -v, --verbose            verbose output\n"
	"Report bugs to <%s>.\n"
	;
    fprintf(stdout, fmt, base_name(program_name), PACKAGE_BUGREPORT);
}

/* main */
int main(int argc, char *argv[])
{
    /* preserve program name */
    set_program_name(argv[0]);

    /* manage options */
    for (;;) {
	int opt;
	int opt_index = 0;
	static struct option long_opts[] = {
	    {"help",        no_argument,        NULL, 'h'},
	    {"version",     no_argument,        NULL, 'V'},
	    {"verbose",     no_argument,        NULL, 'v'},
	    {0, 0, 0, 0}
	};

	opt = getopt_long(argc, argv, "hVv", long_opts, &opt_index);
	if (opt == -1)
	    break;

	switch (opt) {
	    case 'h': opts.opt_h = 1; break;
	    case 'V': opts.opt_V = 1; break;
	    case 'v': ++opts.opt_v; break;
	    default: show_help(); exit(EXIT_FAILURE);
	}
    }

    /* show version number and exit */
    if (opts.opt_V) { show_version(); exit(EXIT_SUCCESS); }

    /* show help message and exit */
    if (opts.opt_h) { show_help(); exit(EXIT_SUCCESS); }

    /* check arguments */
    if (argc < optind + 1) {
	printf("usage: %s <.?in_file>\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    /* process file */
    process(argv[optind]);

    /* finalize */
    exit(EXIT_SUCCESS);
}
