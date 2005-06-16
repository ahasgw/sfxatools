/***********************************************************************
 * $Id: parsefa.c,v 1.2 2005/06/16 09:59:44 aki Exp $
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
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

#include "parsefa.h"

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
 * function definitions
 *======================================================================*/

/* parse fasta */
int parse_fastaXX(const parsefa_param_t *param)
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
	    fputc('\0', param->fseqout);
	    if (hdr_len > 0) {
		/*
		 * hdx[0] = {seq_beg0, hdr_beg0}
		 * hdx[1] = {seq_beg1, hdr_beg1}
		 *  ...
		 * hdx[n-1] = {seq_beg(n-1), hdr_beg(n-1)}
		 */
		{
		    uintXX_t seq_end = seq_beg + seq_len;
		    fwrite(&seq_beg, sizeof(uintXX_t), 1, param->fhdxout);
		    fwrite(&seq_end, sizeof(uintXX_t), 1, param->fhdxout);
		    fwrite(&hdr_beg, sizeof(uint32_t), 1, param->fhdxout);
		}
		seq_beg += (seq_len + 1), seq_len = 0;
		hdr_beg += hdr_len, hdr_len = 0;
	    } else {
		++seq_beg;
	    }
	    line[read] = '\0';  /* truncate delimiter */
	    fputs(line + 1, param->fhdrout);
	    fputc('\0', param->fhdrout);
	    hdr_len = (uint32_t)read - 1 + 1;
	} else {
	    line[read] = '\0';  /* truncate delimiter */
	    if (param->ignore_case) {
		char *cp;
		for (cp = line; *cp != '\0'; ++cp) {
		    *cp = (char)toupper(*cp);
		}
	    }
#if 1
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
#endif
	    fwrite(line, 1, read, param->fseqout);
	    seq_len += (uintXX_t)read;
	}
    }
    {
	uintXX_t seq_end = seq_beg + seq_len;
	fwrite(&seq_beg, sizeof(uintXX_t), 1, param->fhdxout);
	fwrite(&seq_end, sizeof(uintXX_t), 1, param->fhdxout);
	fwrite(&hdr_beg, sizeof(uint32_t), 1, param->fhdxout);
	fputc('\0', param->fseqout);
    }

    if (line)
	free(line);

    return 0;
}
