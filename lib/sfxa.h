/***********************************************************************
 * $Id: sfxa.h,v 1.2 2005/08/17 10:11:43 aki Exp $
 *
 * Header file for sfxa
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

#ifndef SFXA_H
#define SFXA_H 1
#define SFXA_H_INCLUDED 1

#if 0
#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif
#endif

#ifndef MMFILE_H_INCLUDED
# define MMFILE_H_INCLUDED 1
# include "mmfile.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * type definitions
 *======================================================================*/

typedef enum sfxa_idxbits_type {
    SFXA_IDXBITS_UNKNOWN = 0,
    SFXA_IDXBITS_32,
    SFXA_IDXBITS_64
} sfxa_idxbits_t;

/* suffix array data type */
typedef struct sfxa_type {
    mmfile_t        ftxt;
    mmfile_t        fidx;
    mmfile_t        flcp;
    sfxa_idxbits_t  idxbits;
} sfxa_t;

/*======================================================================
 * function declarations
 *======================================================================*/

sfxa_t *sfxa_new(const char *ftxt, const char *fidx, const char *flcp);
int sfxa_init(sfxa_t *sa, const char *ftxt, const char *fidx, const char *flcp);
void sfxa_free(sfxa_t *sa);
void sfxa_delete(sfxa_t *sa);

int sfxa_build_idx(sfxa_t *sa);
int sfxa_build_lcp(sfxa_t *sa);

int sfxa_open(sfxa_t *sa);
int sfxa_close(sfxa_t *sa);

inline static sfxa_idxbits_t sfxa_idxbits(const sfxa_t *sa);

inline static const char *sfxa_txt_path(const sfxa_t *sa);
inline static const char *sfxa_idx_path(const sfxa_t *sa);
inline static const char *sfxa_lcp_path(const sfxa_t *sa);

inline static void *sfxa_txt_ptr(const sfxa_t *sa);
inline static void *sfxa_idx_ptr(const sfxa_t *sa);
inline static void *sfxa_lcp_ptr(const sfxa_t *sa);

inline static off_t sfxa_txt_len(const sfxa_t *sa);
inline static off_t sfxa_idx_len(const sfxa_t *sa);
inline static off_t sfxa_lcp_len(const sfxa_t *sa);

/*======================================================================
 * inline function definitions
 *======================================================================*/

inline static sfxa_idxbits_t sfxa_idxbits(const sfxa_t *sa)
    { return sa->idxbits; }

inline static const char *sfxa_txt_path(const sfxa_t *sa)
    { return (&sa->ftxt ? mmfile_path(&sa->ftxt) : NULL); }
inline static const char *sfxa_idx_path(const sfxa_t *sa)
    { return (&sa->fidx ? mmfile_path(&sa->fidx) : NULL); }
inline static const char *sfxa_lcp_path(const sfxa_t *sa)
    { return (&sa->flcp ? mmfile_path(&sa->flcp) : NULL); }

inline static void *sfxa_txt_ptr(const sfxa_t *sa)
    { return (&sa->ftxt ? mmfile_ptr(&sa->ftxt) : NULL); }
inline static void *sfxa_idx_ptr(const sfxa_t *sa)
    { return (&sa->fidx ? mmfile_ptr(&sa->fidx) : NULL); }
inline static void *sfxa_lcp_ptr(const sfxa_t *sa)
    { return (&sa->flcp ? mmfile_ptr(&sa->flcp) : NULL); }

inline static off_t sfxa_txt_len(const sfxa_t *sa)
    { return (&sa->ftxt ? mmfile_len(&sa->ftxt) : 0); }
inline static off_t sfxa_idx_len(const sfxa_t *sa)
    { return (&sa->fidx ? mmfile_len(&sa->fidx) : 0); }
inline static off_t sfxa_lcp_len(const sfxa_t *sa)
    { return (&sa->flcp ? mmfile_len(&sa->flcp) : 0); }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SFXA_H */
