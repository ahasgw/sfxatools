/***********************************************************************
 * $Id: sfxa.c,v 1.2 2005/07/05 05:12:57 aki Exp $
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

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>

#include "sfxa.h"

#include <mmfile.h>

/*======================================================================
 * public function definitions
 *======================================================================*/

/*----------------------------------------------------------------------
 * ctor & dtor
 *----------------------------------------------------------------------*/

sfxa_t *sfxa_new(const char *ftxt, const char *fidx, const char *flcp)
{
    sfxa_t *sa = (sfxa_t*)calloc(1, sizeof(sfxa_t));
    if (sa != NULL && sfxa_init(sa, ftxt, fidx, flcp) != 0)
	free(sa), sa = NULL;
    return sa;
}

int sfxa_init(sfxa_t *sa, const char *ftxt, const char *fidx, const char *flcp)
{
    int ret = 0;

    /* initialize mmfile */
    if ((ret = mmfile_init(&sa->ftxt, ftxt)) != 0)
	goto bail0;
    if ((ret = mmfile_init(&sa->fidx, fidx)) != 0)
	goto bail1;
    if ((ret = mmfile_init(&sa->flcp, flcp)) != 0)
	goto bail2;

    /* map file */
    if ((ret = mmfile_map_shared_rd(&sa->ftxt)) != 0)
	goto bail3;
    if ((ret = mmfile_map_shared_rd(&sa->fidx)) != 0)
	goto bail4;
    if (mmfile_path(&sa->flcp) != NULL) {
	if ((ret = mmfile_map_shared_rd(&sa->flcp)) != 0)
	    goto bail5;
    }

    return 0;

bail5:
    mmfile_unmap(&sa->fidx);
bail4:
    mmfile_unmap(&sa->ftxt);
bail3:
    mmfile_free(&sa->flcp);
bail2:
    mmfile_free(&sa->fidx);
bail1:
    mmfile_free(&sa->ftxt);
bail0:
    return ret;
}

int sfxa_free(sfxa_t *sa)
{
    int ret = 0;
    /* unmap index file */
    if (mmfile_ptr(&sa->flcp) != NULL && (ret = mmfile_unmap(&sa->flcp)) != 0)
	return ret;
    mmfile_free(&sa->flcp);
    /* unmap index file */
    if ((ret = mmfile_unmap(&sa->fidx)) != 0)
	return ret;
    mmfile_free(&sa->fidx);
    /* unmap text file */
    if ((ret = mmfile_unmap(&sa->ftxt)) != 0)
	return ret;
    mmfile_free(&sa->ftxt);
    return ret;
}

int sfxa_delete(sfxa_t *sa)
{
    int ret = sfxa_free(sa);
    if (ret == 0)
	free(sa);
    return ret;
}
