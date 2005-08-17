/***********************************************************************
 * $Id: ptrstk.h,v 1.1 2005/08/17 10:11:45 aki Exp $
 *
 * Header file for ptrstk
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

#ifndef PTRSTK_H
#define PTRSTK_H 1
#define PTRSTK_H_INCLUDED 1

#include "mbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

/* ptrstk management data type */
typedef struct mbuf_type ptrstk_t;

/*======================================================================
 * function declarations
 *======================================================================*/

inline static ptrstk_t *ptrstk_new(void);
inline static int ptrstk_init(ptrstk_t *pp);
inline static void ptrstk_free(ptrstk_t *pp);
inline static void ptrstk_delete(ptrstk_t *pp);

inline static size_t ptrstk_push_back(ptrstk_t *pp, void *p);
inline static size_t ptrstk_push_front(ptrstk_t *pp, void *p);
inline static size_t ptrstk_pop_back(ptrstk_t *pp);
inline static size_t ptrstk_pop_front(ptrstk_t *pp);

inline static int ptrstk_replace(ptrstk_t *pp, size_t at, void *p);

inline static void ptrstk_clear(ptrstk_t *pp);
inline static void **ptrstk_ptr(const ptrstk_t *pp);
inline static size_t ptrstk_size(const ptrstk_t *pp);
inline static size_t ptrstk_max_size(void);
inline static int ptrstk_empty(const ptrstk_t *pp);
inline static size_t ptrstk_capacity(const ptrstk_t *pp);

inline static void *ptrstk_back(const ptrstk_t *pp);	/* ptrstk_top */
inline static void *ptrstk_front(const ptrstk_t *pp);	/* ptrstk_bottom */

inline static void *ptrstk_at(const ptrstk_t *pp, size_t at);

/*======================================================================
 * inline function definitions
 *======================================================================*/

inline static ptrstk_t *ptrstk_new(void)
{
    return mbuf_new(NULL, 0);
}

inline static int ptrstk_init(ptrstk_t *pp)
{
    return mbuf_init(pp, NULL, 0);
}

inline static void ptrstk_free(ptrstk_t *pp)
{
    mbuf_free(pp);
}

inline static void ptrstk_delete(ptrstk_t *pp)
{
    mbuf_delete(pp);
}

inline static size_t ptrstk_push_back(ptrstk_t *pp, void *p)
{
    void *ptr = p;
    return mbuf_push_back(pp, &ptr, sizeof(void*)) / sizeof(void*);
}

inline static size_t ptrstk_push_front(ptrstk_t *pp, void *p)
{
    void *ptr = p;
    return mbuf_push_front(pp, &ptr, sizeof(void*)) / sizeof(void*);
}

inline static size_t ptrstk_pop_back(ptrstk_t *pp)
{
    return mbuf_pop_back(pp, sizeof(void*)) / sizeof(void*);
}

inline static size_t ptrstk_pop_front(ptrstk_t *pp)
{
    return mbuf_pop_front(pp, sizeof(void*)) / sizeof(void*);
}

inline static int ptrstk_replace(ptrstk_t *pp, size_t at, void *p)
{
    void *ptr = p;
    at *= sizeof(void*);
    return (at < mbuf_size(pp))
	? mbuf_replace(pp, at, sizeof(void*), &ptr, sizeof(void*))
	: 0;
}

inline static void ptrstk_clear(ptrstk_t *pp)
{
    mbuf_clear(pp);
}

inline static void **ptrstk_ptr(const ptrstk_t *pp)
{
    return (void**)pp->ptr;
}

inline static size_t ptrstk_size(const ptrstk_t *pp)
{
    return pp->cnt / sizeof(void*);
}

inline static size_t ptrstk_max_size(void)
{
    return (SIZE_MAX / sizeof(void*)) - 1;
}

inline static int ptrstk_empty(const ptrstk_t *pp)
{
    return pp->cnt == 0;
}

inline static size_t ptrstk_capacity(const ptrstk_t *pp)
{
    return pp->max / sizeof(void*);
}

inline static void *ptrstk_back(const ptrstk_t *pp)
{
    return (ptrstk_size(pp) > 0)
	? ((void**)ptrstk_ptr(pp))[ptrstk_size(pp) - 1] : NULL;
}

inline static void *ptrstk_front(const ptrstk_t *pp)
{
    return (ptrstk_size(pp) > 0) ? *(void**)ptrstk_ptr(pp) : NULL;
}

inline static void *ptrstk_at(const ptrstk_t *pp, size_t at)
{
    return (ptrstk_size(pp) > at) ? *(((void**)ptrstk_ptr(pp)) + at) : NULL;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PTRSTK_H */
