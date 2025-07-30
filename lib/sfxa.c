/***********************************************************************
 * Copyright (C) 2005 RIKEN. All rights reserved.
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
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#include "sfxa.h"
#include "sort.h"
#include "lcp.h"

/*======================================================================
 * public function definitions
 *======================================================================*/

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

    sa->idxbits = SFXA_IDXBITS_UNKNOWN;

    /* initialize mmfile */
    if ((ret = mmfile_init(&sa->ftxt, ftxt)) != 0)
	goto bail0;
    if ((ret = mmfile_init(&sa->fidx, fidx)) != 0)
	goto bail1;
    if ((ret = mmfile_init(&sa->flcp, flcp)) != 0)
	goto bail2;

    return 0;

bail2:
    mmfile_free(&sa->fidx);
bail1:
    mmfile_free(&sa->ftxt);
bail0:
    return ret;
}

void sfxa_free(sfxa_t *sa)
{
    mmfile_free(&sa->flcp);
    mmfile_free(&sa->fidx);
    mmfile_free(&sa->ftxt);
}

void sfxa_delete(sfxa_t *sa)
{
    sfxa_free(sa);
    free(sa);
}

int sfxa_build_idx(sfxa_t *sa)
{
    int ret = 0;
    off_t txt_len = 0;		/* the size of text file */
    off_t idx_len = 0;		/* the size of index file */

    /* map text file */
    if ((ret = mmfile_map_shared_rd(&sa->ftxt)) != 0)
	return ret;

    txt_len = mmfile_len(&sa->ftxt);

    /* calculate idxbits and idx_len */
#if SIZEOF_OFF_T < 8
    if (txt_len <= (INT32_MAX / sizeof(int32_t))) {
	sa->idxbits = SFXA_IDXBITS_32;
	idx_len = txt_len * sizeof(int32_t);
    } else {
	ret = errno = EFBIG;
	goto bail1;
    }
#else
    if (txt_len <= INT32_MAX) {
	sa->idxbits = SFXA_IDXBITS_32;
	idx_len = txt_len * sizeof(int32_t);
    } else if (txt_len <= (INT64_MAX / sizeof(int64_t))) {
	sa->idxbits = SFXA_IDXBITS_64;
	idx_len = txt_len * sizeof(int64_t);
    } else {
	ret = errno = EFBIG;
	goto bail1;
    }
#endif
#if SIZEOF_OFF_T > SIZEOF_SIZE_T
    if (idx_len > (SIZE_MAX / 2)) {
	ret = errno = EFBIG;		/* Should this be ENOMEM? */
	goto bail1;
    }
#endif

    /* build index */
    if ((ret = mmfile_map_shared_rw(&sa->fidx, idx_len)) != 0)
	goto bail1;

#if SIZEOF_OFF_T < 8
    sort32(mmfile_ptr(&sa->ftxt), mmfile_ptr(&sa->fidx), (int32_t)txt_len);
#elif SIZEOF_OFF_T >= 8
    if (txt_len <= INT32_MAX) {
	sort32(mmfile_ptr(&sa->ftxt), mmfile_ptr(&sa->fidx), (int32_t)txt_len);
    } else {
	sort64(mmfile_ptr(&sa->ftxt), mmfile_ptr(&sa->fidx), (int64_t)txt_len);
    }
#endif

    mmfile_unmap(&sa->fidx);
bail1:
    mmfile_unmap(&sa->ftxt);
    return ret;
}

int sfxa_build_lcp(sfxa_t *sa)
{
    int ret = 0;
    off_t txt_len = 0;		/* the size of text file */
    off_t idx_len = 0;		/* the size of index file */
    off_t lcp_len = 0;		/* the size of lcp file */

    /* check lcp file path */
    if (mmfile_path(&sa->flcp) == NULL)
	return (errno = EINVAL);

    /* map text file */
    if ((ret = mmfile_map_shared_rd(&sa->ftxt)) != 0)
	return ret;
    txt_len = mmfile_len(&sa->ftxt);

    /* map index file */
    if ((ret = mmfile_map_shared_rd(&sa->fidx)) != 0)
	goto bail1;
    idx_len = mmfile_len(&sa->fidx);

    /* calculate idxbits and idx_len */
#if SIZEOF_OFF_T < 8
    if (txt_len <= (INT32_MAX / sizeof(int32_t))) {
	sa->idxbits = SFXA_IDXBITS_32;
	lcp_len = txt_len * sizeof(int32_t);
    } else {
	ret = errno = EFBIG;
	goto bail2;
    }
#else
    if (txt_len <= INT32_MAX) {
	sa->idxbits = SFXA_IDXBITS_32;
	lcp_len = txt_len * sizeof(int32_t);
    } else if (txt_len <= (INT64_MAX / sizeof(int64_t))) {
	sa->idxbits = SFXA_IDXBITS_64;
	lcp_len = txt_len * sizeof(int64_t);
    } else {
	ret = errno = EFBIG;
	goto bail2;
    }
#endif
#if SIZEOF_OFF_T > SIZEOF_SIZE_T
    if (lcp_len > (SIZE_MAX / 2)) {
	ret = errno = EFBIG;		/* Should this be ENOMEM? */
	goto bail2;
    }
#endif

    /* check lengths */
    if (idx_len != lcp_len) {
	ret = errno = EINVAL;
	goto bail2;
    }

    /* build lcp */
    if ((ret = mmfile_map_shared_rw(&sa->flcp, lcp_len)) != 0)
	goto bail2;

#if SIZEOF_OFF_T < 8
    lcp32(mmfile_ptr(&sa->ftxt), mmfile_ptr(&sa->fidx), mmfile_ptr(&sa->flcp), (int32_t)txt_len);
#elif SIZEOF_OFF_T >= 8
    if (txt_len <= INT32_MAX) {
	lcp32(mmfile_ptr(&sa->ftxt), mmfile_ptr(&sa->fidx), mmfile_ptr(&sa->flcp), (int32_t)txt_len);
    } else {
	lcp64(mmfile_ptr(&sa->ftxt), mmfile_ptr(&sa->fidx), mmfile_ptr(&sa->flcp), (int64_t)txt_len);
    }
#endif

    mmfile_unmap(&sa->flcp);
bail2:
    mmfile_unmap(&sa->fidx);
bail1:
    mmfile_unmap(&sa->ftxt);
    return ret;
}

int sfxa_open(sfxa_t *sa)
{
    int ret = 0;
    off_t txt_len = 0;		/* the size of text file */

    /* map text file */
    if ((ret = mmfile_map_shared_rd(&sa->ftxt)) != 0)
	return ret;

    /* calculate idxbits and idx_len */
    txt_len = mmfile_len(&sa->ftxt);
#if SIZEOF_OFF_T < 8
    if (txt_len <= (INT32_MAX / sizeof(int32_t))) {
	sa->idxbits = SFXA_IDXBITS_32;
    } else {
	ret = errno = EFBIG;
	goto bail1;
    }
#else
    if (txt_len <= INT32_MAX) {
	sa->idxbits = SFXA_IDXBITS_32;
    } else if (txt_len <= (INT64_MAX / sizeof(int64_t))) {
	sa->idxbits = SFXA_IDXBITS_64;
    } else {
	ret = errno = EFBIG;
	goto bail1;
    }
#endif

    /* map other files */
    if ((ret = mmfile_map_shared_rd(&sa->fidx)) != 0)
	goto bail1;
    if (mmfile_path(&sa->flcp) != NULL) {
	if ((ret = mmfile_map_shared_rd(&sa->flcp)) != 0)
	    goto bail2;
    }

    return 0;	/* return sfxa_open with success */

/* failed to open or file was too large */

bail2:
    mmfile_unmap(&sa->fidx);
bail1:
    mmfile_unmap(&sa->ftxt);
    return ret;
}

int sfxa_close(sfxa_t *sa)
{
    int ret = 0;
    /* unmap index file */
    if (mmfile_ptr(&sa->flcp) != NULL && (ret = mmfile_unmap(&sa->flcp)) != 0)
	return ret;
    /* unmap index file */
    if ((ret = mmfile_unmap(&sa->fidx)) != 0)
	return ret;
    /* unmap text file */
    if ((ret = mmfile_unmap(&sa->ftxt)) != 0)
	return ret;
    return ret;
}
