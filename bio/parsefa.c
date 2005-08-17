/***********************************************************************
 * $Id: parsefa.c,v 1.5 2005/08/17 10:11:40 aki Exp $
 *
 * parsefa
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

#include <sys/types.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <getline.h>
#include <getopt.h>
#include <progname.h>
#include <string.h>
#include <xalloc.h>

#include "parsefa.h"

#include "cmap.h"
#include "msg.h"
#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#if SIZEOF_INTXX_T < 8
# define uintXX_t	    uint32_t
# define parse_fastaXX	    parse_fasta32
#else
# define uintXX_t	    uint64_t
# define parse_fastaXX	    parse_fasta64
#endif

/*======================================================================
 * private function definitions
 *======================================================================*/

static void move(ofiles_t *ofs, const ofnames_t *ofn, uintXX_t *seq_beg, off_t seq_len, uint32_t *hdr_beg, off_t hdr_len);

/*======================================================================
 * function definitions
 *======================================================================*/

/* parse fasta with FILESIZE_MAX split ability */
int parse_fastaXX(ofiles_t *ofiles, const ofnames_t *ofnames, const parsefa_param_t *param)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    uint32_t hdr_beg = 0;
    uint32_t hdr_len = 0;
    uintXX_t seq_beg = 0;
    uintXX_t seq_len = 0;

    while ((read = getline(&line, &len, stdin)) != -1) {
	if (read <= 1)		    /* empty line or delimiter only */
	    continue;
	--read;
	if (*line == '>') {
	    fputc('\0', ofiles->fseq);
	    if (hdr_len > 0) {
		if ((uint64_t)seq_len + 2 > (uint64_t)param->split_size) {
		    msg(MSGLVL_ERR, "sequence too long");
		    exit(EXIT_FAILURE);
		}

		if ((uint64_t)seq_beg + seq_len + 1 > (uint64_t)param->split_size)
		    move(ofiles, ofnames, &seq_beg, seq_len, &hdr_beg, hdr_len);

		/*
		 * hdx[0] = {seq_beg0, seq_end0, hdr_beg0}
		 * hdx[1] = {seq_beg1, seq_end1, hdr_beg1}
		 *  ...
		 * hdx[n-1] = {seq_beg(n-1), seq_end(n-1), hdr_beg(n-1)}
		 */
		{
		    uintXX_t seq_end = seq_beg + seq_len;
		    fwrite(&seq_beg, sizeof(uintXX_t), 1, ofiles->fhdx);
		    fwrite(&seq_end, sizeof(uintXX_t), 1, ofiles->fhdx);
		    fwrite(&hdr_beg, sizeof(uint32_t), 1, ofiles->fhdx);
		}
		seq_beg += (seq_len + 1), seq_len = 0;
		hdr_beg += hdr_len, hdr_len = 0;
	    } else {
		++seq_beg;
	    }
	    line[read] = '\0';  /* truncate delimiter */
	    fputs(line + 1, ofiles->fhdr);
	    fputc('\0', ofiles->fhdr);
	    hdr_len = (uint32_t)read - 1 + 1;
	} else {
	    line[read] = '\0';  /* truncate delimiter */
	    if (param->ignore_case) {
		char *cp;
		for (cp = line; *cp != '\0'; ++cp) {
		    *cp = (char)toupper(*cp);
		}
	    }
# if 1
	    if (param->cmap) {
		char *src, *dst;
		for (src = line, dst = line; *src != '\0'; ++src) {
		    int n = cmap_char2num(param->cmap, *src);
		    if (n != CMAP_UNMAP) {
			*dst = (unsigned char)n;
			++dst;
		    }
		}
		read = dst - line;
	    }
# endif
	    fwrite(line, sizeof(char), read, ofiles->fseq);
	    seq_len += (uintXX_t)read;
	}
    }

    if ((uint64_t)seq_len + 2 > (uint64_t)param->split_size) {
	msg(MSGLVL_ERR, "sequence too long");
	exit(EXIT_FAILURE);
    }

    if ((uint64_t)seq_beg + seq_len + 1 > (uint64_t)param->split_size)
	move(ofiles, ofnames, &seq_beg, seq_len, &hdr_beg, hdr_len);

    {
	uintXX_t seq_end = seq_beg + seq_len;
	fwrite(&seq_beg, sizeof(uintXX_t), 1, ofiles->fhdx);
	fwrite(&seq_end, sizeof(uintXX_t), 1, ofiles->fhdx);
	fwrite(&hdr_beg, sizeof(uint32_t), 1, ofiles->fhdx);
	fputc('\0', ofiles->fseq);
    }

    if (line)
	free(line);

    return 0;
}

static void move(ofiles_t *ofs, const ofnames_t *ofn, uintXX_t *seq_beg, off_t seq_len, uint32_t *hdr_beg, off_t hdr_len)
{
    char *buf;
    ssize_t count;
    ofiles_t newofs;

    buf = (char *)xmalloc(MAX(seq_len + 1, hdr_len));
    /* open new ofiles */
    ofiles_open_next(&newofs, ofs, ofn);
    
    /* seq */
    fseek(ofs->fseq, (long)(*seq_beg - 1), SEEK_SET);
    count = fread(buf, sizeof(char), seq_len + 1, ofs->fseq);
    if (count < seq_len + 1) {
	msg(MSGLVL_ERR, "seq read error:");
	exit(EXIT_FAILURE);
    }
    count = fwrite(buf, sizeof(char), seq_len + 1, newofs.fseq);
    if (count < seq_len + 1) {
	msg(MSGLVL_ERR, "seq write error:");
	exit(EXIT_FAILURE);
    }
    if (ftruncate(fileno(ofs->fseq), (off_t)*seq_beg) != 0) {
	msg(MSGLVL_ERR, "seq truncate error:");
	exit(EXIT_FAILURE);
    }
    *seq_beg = 1;

    /* hdr */
    fseek(ofs->fhdr, *hdr_beg, SEEK_SET);
    count = fread(buf, sizeof(char), hdr_len, ofs->fhdr);
    if (count < hdr_len) {
	msg(MSGLVL_ERR, "hdr read error:");
	exit(EXIT_FAILURE);
    }
    count = fwrite(buf, sizeof(char), hdr_len, newofs.fhdr);
    if (count < hdr_len) {
	msg(MSGLVL_ERR, "hdr write error:");
	exit(EXIT_FAILURE);
    }
    if (ftruncate(fileno(ofs->fhdr), *hdr_beg) != 0) {
	msg(MSGLVL_ERR, "hdr truncate error:");
	exit(EXIT_FAILURE);
    }
    *hdr_beg = 0;

    /* close original ofiles, then, use newofiles as the original one */
    ofiles_subst(ofs, &newofs);
    free(buf), buf = NULL;
}
