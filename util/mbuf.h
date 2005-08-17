/***********************************************************************
 * $Id: mbuf.h,v 1.2 2005/08/17 10:11:45 aki Exp $
 *
 * Header file for mbuf
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

#ifndef MBUF_H
#define MBUF_H 1
#define MBUF_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef STDINT_H_INCLUDED
# define STDINT_H_INCLUDED 1
# include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

/* mbuf management data type */
typedef struct mbuf_type {
    void	*ptr;
    size_t	max;
    size_t	cnt;
} mbuf_t;

/*======================================================================
 * function declarations
 *======================================================================*/

mbuf_t *mbuf_new(const void *p, size_t n);
int mbuf_init(mbuf_t *mp, const void *p, size_t n);
void mbuf_free(mbuf_t *mp);
void mbuf_delete(mbuf_t *mp);

inline static mbuf_t *mbuf_dup(const mbuf_t *mp);
size_t mbuf_assign(mbuf_t *dest, const mbuf_t *src);
size_t mbuf_append(mbuf_t *dest, const mbuf_t *src);

int mbuf_reserve(mbuf_t *mp, size_t n);
int mbuf_resize(mbuf_t *mp, size_t n);
inline static void mbuf_clear(mbuf_t *mp);

size_t mbuf_replace(mbuf_t *mp, size_t at, size_t m, const void *p, size_t n);
inline static size_t mbuf_insert(mbuf_t *mp, size_t at, const void *p, size_t n);

size_t mbuf_push_back(mbuf_t *mp, const void *p, size_t n);
size_t mbuf_push_front(mbuf_t *mp, const void *p, size_t n);
size_t mbuf_pop_back(mbuf_t *mp, size_t n);
size_t mbuf_pop_front(mbuf_t *mp, size_t n);

inline static void *mbuf_ptr(const mbuf_t *mp);
inline static size_t mbuf_size(const mbuf_t *mp);
inline static size_t mbuf_max_size(void);
inline static int mbuf_empty(const mbuf_t *mp);
inline static size_t mbuf_capacity(const mbuf_t *mp);

inline static size_t mbuf_push_back_1(mbuf_t *mp, char c);
inline static size_t mbuf_push_front_1(mbuf_t *mp, char c);
inline static size_t mbuf_pop_back_1(mbuf_t *mp);
inline static size_t mbuf_pop_front_1(mbuf_t *mp);

inline static char mbuf_back_1(const mbuf_t *mp);
inline static char mbuf_front_1(const mbuf_t *mp);

/*======================================================================
 * inline function definitions
 *======================================================================*/

inline static mbuf_t *mbuf_dup(const mbuf_t *mp)
{
    return mbuf_new(mbuf_ptr(mp), mbuf_size(mp));
}

inline static void mbuf_clear(mbuf_t *mp) { mp->cnt = 0; }

inline static size_t mbuf_insert(mbuf_t *mp, size_t at, const void *p, size_t n)
{
    return mbuf_replace(mp, at, 0, p, n);
}

inline static void *mbuf_ptr(const mbuf_t *mp) { return mp->ptr; }
inline static size_t mbuf_size(const mbuf_t *mp) { return mp->cnt; }
inline static size_t mbuf_max_size(void) { return SIZE_MAX - 1; }
inline static int mbuf_empty(const mbuf_t *mp) { return mp->cnt == 0; }
inline static size_t mbuf_capacity(const mbuf_t *mp) { return mp->max; }

inline static size_t mbuf_push_back_1(mbuf_t *mp, char c)
{
    char ch = c;
    return mbuf_push_back(mp, &ch, 1);
}

inline static size_t mbuf_push_front_1(mbuf_t *mp, char c)
{
    char ch = c;
    return mbuf_push_front(mp, &ch, 1);
}

inline static size_t mbuf_pop_back_1(mbuf_t *mp)
{
    return mbuf_pop_back(mp, 1);
}

inline static size_t mbuf_pop_front_1(mbuf_t *mp)
{
    return mbuf_pop_front(mp, 1);
}

inline static char mbuf_back_1(const mbuf_t *mp)
{
    return (mbuf_size(mp) > 0) ? ((char*)mbuf_ptr(mp))[mbuf_size(mp) - 1] : 0;
}

inline static char mbuf_front_1(const mbuf_t *mp)
{
    return (mbuf_size(mp) > 0) ? *(char*)mbuf_ptr(mp) : 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MBUF_H */
