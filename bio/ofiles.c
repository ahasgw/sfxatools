/***********************************************************************
 * $Id: ofiles.c,v 1.3 2006/01/12 09:53:33 aki Exp $
 *
 * ofiles.c
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

#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "ofiles.h"
#include "bldpath.h"
#include "msg.h"

/*======================================================================
 * constants definitions
 *======================================================================*/

const char * const  tmpl_fmt = "%s.%%0%dd%s";

/*======================================================================
 * function definitions
 *======================================================================*/

void ofnames_init(ofnames_t *ofns, const char *path, const char *fhdr_ext, const char *fhdx_ext, const char *fseq_ext, int ext_num_len)
{
    ofns->fhdrname = bldpathtmpl(tmpl_fmt, path, fhdr_ext, ext_num_len);
    ofns->fhdxname = bldpathtmpl(tmpl_fmt, path, fhdx_ext, ext_num_len);
    ofns->fseqname = bldpathtmpl(tmpl_fmt, path, fseq_ext, ext_num_len);

    if (!ofns->fhdrname || !ofns->fhdxname || !ofns->fseqname) {
	msg(MSGLVL_ERR, "cannot open files:");
	exit(EXIT_FAILURE);
    }
}

void ofnames_free(ofnames_t *ofns)
{
    free(ofns->fseqname), ofns->fseqname = NULL;
    free(ofns->fhdxname), ofns->fhdxname = NULL;
    free(ofns->fhdrname), ofns->fhdrname = NULL;
}

void ofiles_open(ofiles_t *ofs, const ofnames_t *ofns, int num)
{
    char *name = NULL;
    ofs->num = num;

    name = bldpath(ofns->fhdrname, num);
    if ((ofs->fhdr = fopen(name, "w+")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .hdr output file '%s':", name);
	exit(EXIT_FAILURE);
    }
    free(name);

    name = bldpath(ofns->fhdxname, num);
    if ((ofs->fhdx = fopen(name, "wb")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .hdx output file '%s':", name);
	exit(EXIT_FAILURE);
    }
    free(name);

    name = bldpath(ofns->fseqname, num);
    if ((ofs->fseq = fopen(name, "w+b")) == NULL) {
	msg(MSGLVL_ERR, "cannot open .seq output file '%s':", name);
	exit(EXIT_FAILURE);
    }
    free(name);
}

void ofiles_close(ofiles_t *ofs)
{
    fclose(ofs->fseq), ofs->fseq = NULL;
    fclose(ofs->fhdx), ofs->fhdx = NULL;
    fclose(ofs->fhdr), ofs->fhdr = NULL;
}

void ofiles_open_next(ofiles_t *newofs, const ofiles_t *ofs, const ofnames_t *ofn)
{
    ofiles_open(newofs, ofn, ofs->num + 1);
}

void ofiles_subst(ofiles_t *destofs, const ofiles_t *srcofs)
{
    ofiles_close(destofs);
    *destofs = *srcofs;
}
