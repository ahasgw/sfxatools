/***********************************************************************
 * $Id: regexp.h,v 1.4 2005/10/31 03:03:45 aki Exp $
 *
 * Header file for regexp
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

#ifndef REGEXP_H
#define REGEXP_H 1
#define REGEXP_H_INCLUDED 1

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#ifndef NDEBUG
# include <stdio.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * macro definitions
 *======================================================================*/

#define REGEXP_MAX_REPEAT   (255U)

/*======================================================================
 * type definitions
 *======================================================================*/

/*
 * regexp_code_t
 *
 * MSB                                        LSB
 * +--------------------------------------------+
 * |   kind: 8 |    ch: 8 |  most: 8 | least: 8 |
 * +--------------------------------------------+
 *
 * | KIND_DOWN |     '('  |     most |    least |
 * | KIND_UP   |     ')'  |        0 | goback # |
 * | KIND_OR   |     '|'  |      n/a |      n/a |
 * | KIND_HEAD |     '^'  |      n/a |      n/a |
 * | KIND_TAIL |     '$'  |      n/a |      n/a |
 * | KIND_CH   |      ch  |     most |    least |
 * | KIND_ANY  |     '.'  |     most |    least |
 *
 * | KIND_CHS  |     '['  |     most |    least | or
 * | KIND_CHS  |     '!'  |     most |    least | followed by
 * |                 charbits[0]                | \
 * |                 charbits[1]                |  > regexp_charbits_t
 * |                 charbits[2]                | |
 * |                 charbits[3]                | /
 */
typedef uint32_t regexp_code_t;

typedef uint32_t regexp_charbits_t[4];

typedef struct regexp_type {
    regexp_code_t   *code;
    size_t	    code_len;
    size_t	    error_at;
} regexp_t;

typedef struct regexp_opt_type {
    uint32_t	    max_repeat;
} regexp_opt_t;

typedef enum regexp_code_kind_type {
    KIND_DOWN,	    /* beginning of subexpression */
    KIND_UP,	    /* end of subexpression */
    KIND_OR,	    /* '|': beginning of another possible pattern */
    KIND_HEAD,	    /* '^': beginning of pattern */
    KIND_TAIL,	    /* '$': end of pattern */
    KIND_CH,	    /* single character */
    KIND_CHS,	    /* class of characters */
    KIND_ANY	    /* '.': any character */
} regexp_code_kind_t;

/*======================================================================
 * function declarations
 *======================================================================*/

regexp_t *regexp_new(const char *pat, const regexp_opt_t *optp);
int regexp_init(regexp_t *rxp, const char *pat, const regexp_opt_t *optp);
int regexp_reverse(regexp_t *rxp);
char *regexp_string(const regexp_t *rxp);
void regexp_free(regexp_t *rxp);
void regexp_delete(regexp_t *rxp);

inline static regexp_code_kind_t regexp_code_get_kind(regexp_code_t code);
inline static char regexp_code_get_ch(regexp_code_t code);
inline static unsigned int regexp_code_get_least(regexp_code_t code);
inline static unsigned int regexp_code_get_most(regexp_code_t code);

inline static void regexp_charbits_clr(regexp_charbits_t *bits);
inline static void regexp_charbits_set(regexp_charbits_t *bits, char c);
inline static int regexp_charbits_test(regexp_charbits_t *bits, char c);
inline static void regexp_charbits_read(const uint32_t *p, size_t *i, regexp_charbits_t *bits);
inline static void regexp_charbits_sp_and_print(regexp_charbits_t *bits);
inline static void regexp_charbits_not(regexp_charbits_t *bits);
inline static void regexp_charbits_and(regexp_charbits_t *des, regexp_charbits_t *src);
inline static int regexp_charbits_any(regexp_charbits_t *bits);
#if 0
inline static void regexp_charbits_cpy(regexp_charbits_t *dst, regexp_charbits_t *src);
#endif

#ifndef NDEBUG
void regexp_code_print(FILE *s, const regexp_code_t code);
void regexp_charbits_print(FILE *s, regexp_charbits_t *bits);
void regexp_stack_print(FILE *s, const uint32_t *ptr, size_t n);
#endif

/*======================================================================
 * inline function definitions
 *======================================================================*/

inline static regexp_code_kind_t regexp_code_get_kind(regexp_code_t code) {
    return (regexp_code_kind_t)((code >> 24) & 0xFF);
}
inline static char regexp_code_get_ch(regexp_code_t code) {
    return (char)((code >> 16) & 0xFF);
}
inline static unsigned int regexp_code_get_least(regexp_code_t code) {
    return (unsigned int)(code & 0xFF);
}
inline static unsigned int regexp_code_get_most(regexp_code_t code) {
    return (unsigned int)((code >> 8) & 0xFF);
}

/*----------------------------------------------------------------------*/

inline static void regexp_charbits_clr(regexp_charbits_t *bits) {
    uint32_t *u32p = (uint32_t*)bits;
    u32p[0] = 0U;
    u32p[1] = 0U;
    u32p[2] = 0U;
    u32p[3] = 0U;
}
inline static void regexp_charbits_set(regexp_charbits_t *bits, char c) {
    const size_t uint32_bit = (CHAR_BIT * sizeof(uint32_t));
    (*bits)[c / uint32_bit] |= (1U << (c % uint32_bit));
}
inline static int regexp_charbits_test(regexp_charbits_t *bits, char c) {
    const size_t uint32_bit = (CHAR_BIT * sizeof(uint32_t));
    return ((*bits)[c / uint32_bit] & (1U << (c % uint32_bit)));
}
inline static void regexp_charbits_read(const uint32_t *p, size_t *i, regexp_charbits_t *bits) {
    uint32_t *u32p = (uint32_t*)bits;
    u32p[0] = *(p + ++*i);  /* charbits is begins at next uint32_t */
    u32p[1] = *(p + ++*i);
    u32p[2] = *(p + ++*i);
    u32p[3] = *(p + ++*i);
}
inline static void regexp_charbits_sp_and_print(regexp_charbits_t *bits) {
    uint32_t *u32p = (uint32_t*)bits;
    u32p[0] = 0x00003F80;   /* 0x00 - 0x06, 0x0E - 0x1F excluded */
    u32p[1] = 0xFFFFFFFF;
    u32p[2] = 0xFFFFFFFF;
    u32p[3] = 0x7FFFFFFF;   /* 0x7F excluded */
}
inline static void regexp_charbits_not(regexp_charbits_t *bits) {
    uint32_t *u32p = (uint32_t*)bits;
    u32p[0] = ~(u32p[0]);
    u32p[1] = ~(u32p[1]);
    u32p[2] = ~(u32p[2]);
    u32p[3] = ~(u32p[3]);
}
inline static void regexp_charbits_and(regexp_charbits_t *dst, regexp_charbits_t *src) {
    uint32_t *dstp = (uint32_t*)dst;
    uint32_t *srcp = (uint32_t*)src;
    dstp[0] &= srcp[0];
    dstp[1] &= srcp[1];
    dstp[2] &= srcp[2];
    dstp[3] &= srcp[3];
}
inline static int regexp_charbits_any(regexp_charbits_t *bits) {
    uint32_t *u32p = (uint32_t*)bits;
    return (*u32p++ || *u32p++ || *u32p++ || *u32p++);
}
#if 0
inline static void regexp_charbits_cpy(regexp_charbits_t *dst, regexp_charbits_t *src) {
    uint32_t *dstp = (uint32_t*)dst;
    uint32_t *srcp = (uint32_t*)src;
    dstp[0] = srcp[0];
    dstp[1] = srcp[1];
    dstp[2] = srcp[2];
    dstp[3] = srcp[3];
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REGEXP_H */
