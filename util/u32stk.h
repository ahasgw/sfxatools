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

#ifndef U32STK_H
#define U32STK_H 1
#define U32STK_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <stdint.h>

#include "mbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

/* u32stk management data type */
typedef struct mbuf_type u32stk_t;

/*======================================================================
 * function declarations
 *======================================================================*/

inline static u32stk_t *u32stk_new(void);
inline static int u32stk_init(u32stk_t *sp);
inline static void u32stk_free(u32stk_t *sp);
inline static void u32stk_delete(u32stk_t *sp);

inline static size_t u32stk_push_back(u32stk_t *sp, uint32_t i);
inline static size_t u32stk_push_front(u32stk_t *sp, uint32_t i);
inline static size_t u32stk_pop_back(u32stk_t *sp);
inline static size_t u32stk_pop_front(u32stk_t *sp);

inline static int u32stk_replace(u32stk_t *sp, size_t at, uint32_t i);
inline static void u32stk_swap(u32stk_t *sp, size_t at1, size_t at2);

inline static void u32stk_clear(u32stk_t *sp);
inline static uint32_t *u32stk_ptr(const u32stk_t *sp);
inline static size_t u32stk_size(const u32stk_t *sp);
inline static size_t u32stk_back_idx(const u32stk_t *sp);
inline static size_t u32stk_max_size(void);
inline static int u32stk_empty(const u32stk_t *sp);
inline static size_t u32stk_capacity(const u32stk_t *sp);

inline static uint32_t u32stk_back(const u32stk_t *sp);	    /* u32stk_top */
inline static uint32_t u32stk_front(const u32stk_t *sp);    /* u32stk_bottom */

inline static uint32_t u32stk_at(const u32stk_t *sp, size_t at);

/*======================================================================
 * inline function definitions
 *======================================================================*/

inline u32stk_t *u32stk_new(void)
{
    return mbuf_new(NULL, 0);
}

inline int u32stk_init(u32stk_t *sp)
{
    return mbuf_init(sp, NULL, 0);
}

inline void u32stk_free(u32stk_t *sp)
{
    mbuf_free(sp);
}

inline void u32stk_delete(u32stk_t *sp)
{
    mbuf_delete(sp);
}

inline size_t u32stk_push_back(u32stk_t *sp, uint32_t i)
{
    uint32_t j = i;
    return mbuf_push_back(sp, &j, sizeof(uint32_t)) / sizeof(uint32_t);
}

inline size_t u32stk_push_front(u32stk_t *sp, uint32_t i)
{
    uint32_t j = i;
    return mbuf_push_front(sp, &j, sizeof(uint32_t)) / sizeof(uint32_t);
}

inline size_t u32stk_pop_back(u32stk_t *sp)
{
    return mbuf_pop_back(sp, sizeof(uint32_t)) / sizeof(uint32_t);
}

inline size_t u32stk_pop_front(u32stk_t *sp)
{
    return mbuf_pop_front(sp, sizeof(uint32_t)) / sizeof(uint32_t);
}

inline static int u32stk_replace(u32stk_t *sp, size_t at, uint32_t i)
{
    uint32_t j = i;
    at *= sizeof(uint32_t);
    return (at < mbuf_size(sp))
	? mbuf_replace(sp, at, sizeof(uint32_t), &j, sizeof(uint32_t))
	: 0;
}

inline static void u32stk_swap(u32stk_t *sp, size_t at1, size_t at2)
{
    size_t sz = u32stk_size(sp);
    if (at1 != at2 && at1 < sz && at2 < sz) {
	uint32_t t = ((uint32_t*)sp->ptr)[at1];
	((uint32_t*)sp->ptr)[at1] = ((uint32_t*)sp->ptr)[at2];
	((uint32_t*)sp->ptr)[at2] = t;
    }
}

inline static void u32stk_clear(u32stk_t *sp)
{
    mbuf_clear(sp);
}

inline static uint32_t *u32stk_ptr(const u32stk_t *sp)
{
    return (uint32_t*)sp->ptr;
}

inline static size_t u32stk_size(const u32stk_t *sp)
{
    return sp->cnt / sizeof(uint32_t);
}

inline static size_t u32stk_back_idx(const u32stk_t *sp)
{
    return (sp->cnt / sizeof(uint32_t)) - 1;
}

inline static size_t u32stk_max_size(void)
{
    return (SIZE_MAX / sizeof(uint32_t)) - 1;
}

inline static int u32stk_empty(const u32stk_t *sp)
{
    return sp->cnt == 0;
}

inline static size_t u32stk_capacity(const u32stk_t *sp)
{
    return sp->max / sizeof(uint32_t);
}

inline static uint32_t u32stk_back(const u32stk_t *sp)
{
    return (u32stk_size(sp) > 0)
	? (u32stk_ptr(sp))[u32stk_size(sp) - 1] : 0;
}

inline static uint32_t u32stk_front(const u32stk_t *sp)
{
    return (u32stk_size(sp) > 0) ? *(u32stk_ptr(sp)) : 0;
}

inline static uint32_t u32stk_at(const u32stk_t *sp, size_t at)
{
    return (u32stk_size(sp) > at) ? *((u32stk_ptr(sp)) + at) : 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* U32STK_H */
