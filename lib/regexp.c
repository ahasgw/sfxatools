/***********************************************************************
 * $Id: regexp.c,v 1.4 2005/10/31 03:03:44 aki Exp $
 *
 * regexp
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
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "regexp.h"
#include "mbuf.h"
#include "u32stk.h"
#if 1
# include "msg.h"
#endif

#include <minmax.h>

/*======================================================================
 * macro definitions
 *======================================================================*/

#define DIGIT \
	'0': case '1': case '2': case '3': case '4': case '5': case '6': \
    case '7': case '8': case '9'

/*======================================================================
 * type definitions
 *======================================================================*/

/*======================================================================
 * prototypes
 *======================================================================*/

static int compile(regexp_t *rxp, const char *pat, const regexp_opt_t *optp);

static int scan_level(u32stk_t *stk, const char **cpp, const regexp_opt_t *optp);
static int scan_repeat(u32stk_t *stk, const char **cpp, size_t at, unsigned int max_repeat);
static int scan_chars(const char **cpp, regexp_charbits_t *cbp, int *not);
static int scan_range(const char **cpp, unsigned int *least, unsigned int *most, unsigned int max_repeat);

static int code_push(u32stk_t *stk, regexp_code_kind_t kind, char c);
static void code_set_ch(u32stk_t *stk, size_t at, char c);
static void code_set_rep(u32stk_t *stk, size_t at, unsigned int least, unsigned int most);

static int charbits_push(u32stk_t *stk, regexp_charbits_t *cbp);
static void charbits_set_series(regexp_charbits_t *cbp, char from, char to);

static int reverse_level(const regexp_code_t **code, const regexp_code_t *code_end, u32stk_t *lifo);

/*======================================================================
 * public function definitions
 *======================================================================*/

regexp_t *regexp_new(const char *pat, const regexp_opt_t *optp)
{
    regexp_t *rxp = (regexp_t*)malloc(sizeof(regexp_t));
    if (rxp != NULL && regexp_init(rxp, pat, optp) != 0) {
	free(rxp);
	return NULL;
    }
    return rxp;
}

int regexp_init(regexp_t *rxp, const char *pat, const regexp_opt_t *optp)
{
    int ret = 0;

    assert(rxp != NULL);
    if (rxp == NULL)
	return EINVAL;
    rxp->code = NULL;
    rxp->code_len = 0;
    rxp->error_at = 0;
    if ((ret = compile(rxp, pat, optp)) != 0)
	return ret;
    return 0;
}

int regexp_reverse(regexp_t *rxp)
{
    int ret = 0;
    u32stk_t lifo;
    const regexp_code_t *cp = rxp->code;
    const regexp_code_t *cp_end = rxp->code + rxp->code_len;

    if ((ret = u32stk_init(&lifo)) != 0)
	return ret;

    if ((ret = reverse_level(&cp, cp_end, &lifo)) != 0)
	return ret;

    /* copy back the code_stack to rxp->code */
    {
	uint32_t *src = u32stk_ptr(&lifo) + rxp->code_len - 1;
	uint32_t *dst = rxp->code;
	while (src >= u32stk_ptr(&lifo))
	    *dst++ = *src--;
    }

    u32stk_free(&lifo);

    return ret;
}

void regexp_free(regexp_t *rxp)
{
    free(rxp->code), rxp->code = NULL;
    rxp->code_len = 0;
    rxp->error_at = 0;
}

void regexp_delete(regexp_t *rxp)
{
    regexp_free(rxp);
    free(rxp);
}
 
/*----------------------------------------------------------------------
 * private function definitions
 *----------------------------------------------------------------------*/

static int compile(regexp_t *rxp, const char *pat, const regexp_opt_t *optp)
{
    int ret = 0;
    const char *cp = pat;
    u32stk_t code_stack;
    size_t sz = 0;
    regexp_opt_t opt = { REGEXP_MAX_REPEAT };
    
    if (optp != NULL)
	opt = *optp;
    opt.max_repeat = MIN(optp->max_repeat, REGEXP_MAX_REPEAT);

    if ((ret = u32stk_init(&code_stack)) != 0)
	return ret;

    if (!code_push(&code_stack, KIND_DOWN, '(')) {  // implicit parenthesis
	ret = errno;
    } else {
	if ((ret = scan_level(&code_stack, &cp, &opt)) == 0) {
	    code_push(&code_stack, KIND_UP, ')');
	    code_set_rep(&code_stack, u32stk_size(&code_stack) - 1,
		    u32stk_size(&code_stack) - 1, 0);

	    code_set_rep(&code_stack, 0, 1, 1);

	    /* copy back the code_stack to rxp->code */
	    rxp->code_len = u32stk_size(&code_stack);
	    sz = rxp->code_len * sizeof(uint32_t);
	    if ((rxp->code = malloc(sz)) == NULL) {
		rxp->code_len = 0;
		ret = errno;
	    } else {
		memcpy(rxp->code, u32stk_ptr(&code_stack), sz);
	    }
	} else {
	    /* error occured */
	    if (errno == 0)
		errno = EINVAL;
	    rxp->error_at = cp - pat - 1;
	}
    }

    u32stk_free(&code_stack);
    return ret;
}

static int scan_level(u32stk_t *stk, const char **cpp, const regexp_opt_t *optp)
{
    const char *beg = *cpp;
    size_t at = 0;
    regexp_charbits_t charbits;
    char ch;
    int not = 0;

    while (**cpp) {
	switch (**cpp) {
	    /* return in the next two case */
	    case '\0':	    /* end of string */
		return 0;
	    case ')':	    /* end of subexpression */
		if (!code_push(stk, KIND_UP, *(*cpp)++))
		    return 1;
		return ')';

	    case '(':	    /* begin of subexpression */
		at = u32stk_size(stk);
		if (!code_push(stk, KIND_DOWN, *(*cpp)++))
		    return 1;
		if (!scan_level(stk, cpp, optp))    /* scan recursively */
		    return 1;
		/* to allow consecutive range specifiers,
		 * loop the following statement. (and check overflow)
		 */
		if (scan_repeat(stk, cpp, at, optp->max_repeat) != 0)
		    return 1;
		/* set jump idx of loop */
		code_set_rep(stk, u32stk_size(stk) - 1,
			u32stk_size(stk) - 1 - at, 0);
		continue;
	    case '[':	    /* begin of character class */
		at = u32stk_size(stk);
		if (!code_push(stk, KIND_CHS, **cpp))
		    return 1;
		if (!scan_chars(cpp, &charbits, &not))
		    return 1;
		if (not)
		    code_set_ch(stk, u32stk_size(stk) - 1, '!');
		if (!charbits_push(stk, &charbits))
		    return 1;
		/* to allow consecutive range specifiers,
		 * loop the following statement. (and check overflow)
		 */
		if (scan_repeat(stk, cpp, at, optp->max_repeat) != 0)
		    return 1;
		continue;

	    /* we do not want to scan_repeat() in the following three cases */
	    case '^':
		{
		    const char *cp;
		    for (cp = *cpp - 1; cp >= beg; --cp) {
			if (*cp != '^' && *cp != '(')
			    return 1;
		    }
		}
		if (!code_push(stk, KIND_HEAD, *(*cpp)++))
		    return 1;
		continue;
	    case '$':
		{
		    const char *cp;
		    for (cp = *cpp + 1; *cp; ++cp) {
			if (*cp != '$' && *cp != ')')
			    return 1;
		    }
		}
		if (!code_push(stk, KIND_TAIL, *(*cpp)++))
		    return 1;
		continue;
	    case '|':
		if (*(*cpp + 1) == ')')
		    return 1;
		if (!code_push(stk, KIND_OR, *(*cpp)++))
		    return 1;
		continue;

		/* if you want characters ']' and '}' always be escaped,
		 * uncomment the following lines
		 */ 
#if 1
	    case ']':
	    case '}':
	    case '?':
	    case '*':
	    case '+':
	    case '{':
		++*cpp;
		return 1;
#endif

	    case '.':
		if (!code_push(stk, KIND_ANY, *(*cpp)++))
		    return 1;
		break;
	    case '\\':
		++*cpp;
		if (**cpp == '\0') {
		    return 1;
		}
		switch (**cpp) {   /* oct, hex */
		    case '0': ch = '\0'; break;
		    case 'a': ch = '\a'; break;
		    case 'b': ch = '\b'; break;
		    case 't': ch = '\t'; break;
		    case 'n': ch = '\n'; break;
		    case 'v': ch = '\v'; break;
		    case 'f': ch = '\f'; break;
		    case 'r': ch = '\r'; break;
		    default: ch = **cpp; break;
		}
		++*cpp;
		if (!code_push(stk, KIND_CH, ch))
		    return 1;
		break;
	    default:
		if (!code_push(stk, KIND_CH, *(*cpp)++))
		    return 1;
		break;
	}

	/* to allow consecutive range specifiers,
	 * loop the following statement. (and check overflow)
	 */
	if (scan_repeat(stk, cpp, u32stk_back_idx(stk), optp->max_repeat) != 0)
	    return 1;
    }
    return 0;
}

static int scan_repeat(u32stk_t *stk, const char **cpp, size_t at, unsigned int max_repeat)
{
    switch (**cpp) {
	case '?':
	    code_set_rep(stk, at, 0, 1);
	    ++*cpp;
	    break;
	case '*':
	    code_set_rep(stk, at, 0, max_repeat);
#if 1
	    msg(MSGLVL_NOTICE,
		    "Maximum repeat number of '*' reset to (%lu).",
		    max_repeat);
#endif
	    ++*cpp;
	    break;
	case '+':
	    code_set_rep(stk, at, 1, max_repeat);
#if 1
	    msg(MSGLVL_NOTICE,
		    "Maximum repeat number of '+' reset to (%lu).",
		    max_repeat);
#endif
	    ++*cpp;
	    break;
	case '{':
	    {
		unsigned int least;
		unsigned int most;
		if (!scan_range(cpp, &least, &most, max_repeat))
		    return 1;
		code_set_rep(stk, at, MIN(least, most), MAX(least, most));
	    }
	    break;
	default:
	    break;
    }
    return 0;
}

static int scan_chars(const char **cpp, regexp_charbits_t *cbp, int *not)
{
    enum { ERR, C0, C1, C2, ACC } state = C0;

    *not = 0;

    regexp_charbits_clr(cbp);
    for (++*cpp; **cpp; ++*cpp) {
	if (state == ERR || state == ACC)
	    break;
	switch (state) {
	    case C0:	/* just came into square parentheses '[ ]' */
		switch (**cpp) {
		    case '^': *not = 1; state = C1; break;
		    default: regexp_charbits_set(cbp, **cpp); state = C2; break;
		}
		break;
	    case C1:	/* after '[^'. it could be a character member ']' */
		regexp_charbits_set(cbp, **cpp); state = C2;
		break;
	    case C2:	/* from 2nd character to close parenthesis ']' */
		switch (**cpp) {
		    case '-':
			{   char bf = *(*cpp - 1), nx = *(*cpp + 1);
			    if (nx == '\0') {
				state = ERR;
			    } else if (nx == ']') {
				/* if '-' followed by ']', '-' is a member */
				regexp_charbits_set(cbp, **cpp);
			    } else {	/* add all characters in [bf, nx] */
				charbits_set_series(cbp, MIN(bf, nx), MAX(bf, nx));
			    }
			}
			break;
		    case ']': state = ACC; break;
		    default: regexp_charbits_set(cbp, **cpp); break;
		}
		break;
	    case ACC:	/* accepted */
		break;
	    case ERR:	/* error */
		break;
	}
    }
    return (state == ACC);
}

static int scan_range(const char **cpp, unsigned int *least, unsigned int *most,
	unsigned int max_repeat)
{
    const char *digit_beg = NULL;

    enum { ERR, R0, R1, R2, R3, ACC } state = R0;

    *least = *most = 0;

    for (++*cpp; **cpp; ++*cpp) {
	if (state == ERR || state == ACC)
	    break;
	switch (state) {
	    case R0:	/* just came into parentheses */
		switch (**cpp) {
		    case DIGIT: digit_beg = *cpp; state = R1; break;
		    default: state = ERR; break;
		}
		break;
	    case R1:	/* between parentheses */
		switch (**cpp) {
		    case '}': *most = *least = atoi(digit_beg); state = ACC; break;
		    case ',': *least = atoi(digit_beg); state = R2; break;
		    case DIGIT: break;
		    default: state = ERR; break;
		}
		break;
	    case R2:	/* just came into comma and parenthesis */
		switch (**cpp) {
		    case DIGIT: digit_beg = *cpp; state = R3; break;
		    case '}': /* *most = 0; */ state = ACC; break;
		    default: state =ERR; break;
		}
		break;
	    case R3:	/* between comma and parenthesis */
		switch (**cpp) {
		    case '}': *most = atoi(digit_beg); state = ACC; break;
		    case DIGIT: break;
		    default: state = ERR; break;
		}
		break;
	    case ACC:	/* accepted */
		break;
	    case ERR:	/* error */
		break;
	}
    }

#if 1
    if (state == ACC && *most == 0) {
	msg(MSGLVL_NOTICE, "Maximum repeat number of {%lu,} set to (%lu).",
		    *least, max_repeat);
	*most = max_repeat;
    }
    if (*most > max_repeat) {
	if (*least > max_repeat) {
	    msg(MSGLVL_ERR,
		    "Minimum repeat number (%lu) exceeds the limit (%lu), canceled.",
		    *least, max_repeat);
	    state = ERR;
	} else {
	    msg(MSGLVL_NOTICE, "Maximum repeat number (%lu) reset to (%lu).",
		    *most, max_repeat);
	    *most = max_repeat;
	}
    }
#endif

    return (state == ACC);
}

/*--------------------------------------------------------------------*/

static int code_push(u32stk_t *stk, regexp_code_kind_t kind, char c)
{
    uint32_t code = 0U;
    code |= ((kind & 0xFF) << 24) | (c << 16) | (1 << 8) | 1;
    return u32stk_push_back(stk, code) == 1;
}

static void code_set_ch(u32stk_t *stk, size_t at, char c)
{
    regexp_code_t *p = (regexp_code_t*)u32stk_ptr(stk);
    p[at] &= 0xFF00FFFF;
    p[at] |= (c << 16);
}

static void code_set_rep(u32stk_t *stk, size_t at, unsigned int least, unsigned int most)
{
    regexp_code_t *p = (regexp_code_t*)u32stk_ptr(stk);
    p[at] &= 0xFFFF0000;
    p[at] |= ((most & 0xFF) << 8) | (least & 0xFF);
}

/*--------------------------------------------------------------------*/

static int charbits_push(u32stk_t *stk, regexp_charbits_t *bits)
{
    uint32_t *u32p = (uint32_t*)bits;
    return (
	    u32stk_push_back(stk, u32p[0]) == 1
	    && u32stk_push_back(stk, u32p[1]) == 1
	    && u32stk_push_back(stk, u32p[2]) == 1
	    && u32stk_push_back(stk, u32p[3]) == 1
	   );
}

static void charbits_set_series(regexp_charbits_t *bits, char from, char to)
{
    /* since character 'from' is already set,
     * we can start from the next character of 'from' */
    for (++from; from <= to; ++from)
	regexp_charbits_set(bits, from);
}

/*--------------------------------------------------------------------*/

static int reverse_level(const regexp_code_t **code, const regexp_code_t *code_end, u32stk_t *lifo)
{
    const regexp_code_t *beg;
    while (*code < code_end) {
	regexp_code_kind_t kind = regexp_code_get_kind(**code);
	switch (kind) {
	    case KIND_UP:
		u32stk_push_back(lifo, **code);
		++*code;
		return 0;

	    case KIND_DOWN:
		beg = *code;
		u32stk_push_back(lifo, **code);
		++*code;
		reverse_level(code, code_end, lifo);
		u32stk_swap(lifo, u32stk_back_idx(lifo),
			u32stk_back_idx(lifo) - (*code - beg - 1));
		break;
	    case KIND_CHS:
		u32stk_push_back(lifo, *(*code + 4));
		u32stk_push_back(lifo, *(*code + 3));
		u32stk_push_back(lifo, *(*code + 2));
		u32stk_push_back(lifo, *(*code + 1));
		u32stk_push_back(lifo, **code);
		*code += 5;
		break;
	    case KIND_HEAD:
		if (!code_push(lifo, KIND_TAIL, '$'))
		    return 1;
		++*code;
		break;
	    case KIND_TAIL:
		if (!code_push(lifo, KIND_HEAD, '^'))
		    return 1;
		++*code;
		break;
	    default:
		u32stk_push_back(lifo, **code);
		++*code;
		break;
	}
    }
    return 0;
}

/*--------------------------------------------------------------------*/
#ifndef NDEBUG

void regexp_code_print(FILE *s, const regexp_code_t code)
{
    char ch;

    fprintf(s, "CODE<kind:%d,{%u,%u},ch:",
	    regexp_code_get_kind(code),
	    regexp_code_get_least(code),
	    regexp_code_get_most(code)
	  );
    ch = regexp_code_get_ch(code);
    switch (ch) {		    /* oct, hex */
	case '\0': fputs("\\0", s); break;
	case '\a': fputs("\\a", s); break;
	case '\b': fputs("\\b", s); break;
	case '\t': fputs("\\t", s); break;
	case '\n': fputs("\\n", s); break;
	case '\v': fputs("\\v", s); break;
	case '\f': fputs("\\f", s); break;
	case '\r': fputs("\\r", s); break;
	default: putc(ch, s); break;
    }
    putc('>', s);
}

void regexp_charbits_print(FILE *s, regexp_charbits_t *bits)
{
    size_t i;
    putc('[', s);
    for (i = 0; i < CHAR_MAX; ++i) {
	if (regexp_charbits_test(bits, (char)i)) {
	    switch (i) {   /* oct, hex */
		case '\0': fputs("\\0", s); break;
		case '\a': fputs("\\a", s); break;
		case '\b': fputs("\\b", s); break;
		case '\t': fputs("\\t", s); break;
		case '\n': fputs("\\n", s); break;
		case '\v': fputs("\\v", s); break;
		case '\f': fputs("\\f", s); break;
		case '\r': fputs("\\r", s); break;
		default: putc((char)i, s); break;
	    }
	}
    }
    putc(']', s);
}

void regexp_stack_print(FILE *s, const uint32_t *ptr, size_t n)
{
    regexp_charbits_t bits;
    size_t i, j, level = 0;

    fprintf(s, "Stack_print(): stack size:%d, (%d bytes)\n",
	    n, n * sizeof(uint32_t));

    for (i = 0; i < n; ++i) {
	regexp_code_t code = (regexp_code_t)(ptr[i]);
	regexp_code_kind_t kind = regexp_code_get_kind(code);
	level += (kind == KIND_UP) ? -1 : 0;

	fprintf(s, "%u\t", i);
	for (j = 0; j < level; ++j)
	    putchar(' ');
	regexp_code_print(s, code);
	if (kind == KIND_CHS) {
	    putc('\t', s);
	    regexp_charbits_read(ptr, &i, &bits);
	    regexp_charbits_print(s, &bits);
	}
	putc('\n', s);

	level += (kind == KIND_DOWN) ? 1 : 0;
    }
}

#endif
char *regexp_string(const regexp_t *rxp)    /* !!! needs error handling */
{
    char *str = NULL;
    size_t i;
    mbuf_t buf;
    regexp_charbits_t bits;

    if (mbuf_init(&buf, NULL, 0) != 0)
	return NULL;

    for (i = 1; i < rxp->code_len - 1; ++i) {
	regexp_code_t code = (regexp_code_t)(rxp->code[i]);
	regexp_code_kind_t kind = regexp_code_get_kind(code);
	uint32_t least = regexp_code_get_least(code);
	uint32_t most = regexp_code_get_most(code);
	char ch = regexp_code_get_ch(code);

	if (kind != KIND_CHS) {
	    switch (ch) {		    /* oct, hex */
		case '\0': mbuf_push_back(&buf, "\\0", 2); break;
		case '\a': mbuf_push_back(&buf, "\\a", 2); break;
		case '\b': mbuf_push_back(&buf, "\\b", 2); break;
		case '\t': mbuf_push_back(&buf, "\\t", 2); break;
		case '\n': mbuf_push_back(&buf, "\\n", 2); break;
		case '\v': mbuf_push_back(&buf, "\\v", 2); break;
		case '\f': mbuf_push_back(&buf, "\\f", 2); break;
		case '\r': mbuf_push_back(&buf, "\\r", 2); break;
		default: mbuf_push_back_1(&buf, ch); break;
	    }
	} else {
	    size_t j;
	    regexp_charbits_read(rxp->code, &i, &bits);
	    mbuf_push_back_1(&buf, '[');
	    if (ch == '!')
		mbuf_push_back_1(&buf, '^');
	    for (j = 0; j < CHAR_MAX; ++j) {
		if (regexp_charbits_test(&bits, (char)j)) {
		    switch (j) {   /* oct, hex */
			case '\0': mbuf_push_back(&buf, "\\0", 2); break;
			case '\a': mbuf_push_back(&buf, "\\a", 2); break;
			case '\b': mbuf_push_back(&buf, "\\b", 2); break;
			case '\t': mbuf_push_back(&buf, "\\t", 2); break;
			case '\n': mbuf_push_back(&buf, "\\n", 2); break;
			case '\v': mbuf_push_back(&buf, "\\v", 2); break;
			case '\f': mbuf_push_back(&buf, "\\f", 2); break;
			case '\r': mbuf_push_back(&buf, "\\r", 2); break;
			default: mbuf_push_back_1(&buf, (char)j); break;
		    }
		}
	    }
	    mbuf_push_back_1(&buf, ']');
	}
	if (kind == KIND_CH || kind == KIND_ANY || kind == KIND_CHS || kind == KIND_UP) {
	    if (kind == KIND_UP) {
		uint32_t down_idx = i - least;
		least = regexp_code_get_least(rxp->code[down_idx]);
		most = regexp_code_get_most(rxp->code[down_idx]);
	    }

	    if (least != most) {
		if (least == 0 && most == 1) {
		    mbuf_push_back_1(&buf, '?');
		} else {
		    char numstr[64];
		    mbuf_push_back_1(&buf, '{');
		    snprintf(numstr, 64, "%d", least);
		    mbuf_push_back(&buf, numstr, strlen(numstr));
		    mbuf_push_back_1(&buf, ',');
		    snprintf(numstr, 64, "%d", most);
		    mbuf_push_back(&buf, numstr, strlen(numstr));
		    mbuf_push_back_1(&buf, '}');
		}
	    } else if (least != 1) {
		char numstr[64];
		mbuf_push_back_1(&buf, '{');
		snprintf(numstr, 64, "%d", least);
		mbuf_push_back(&buf, numstr, strlen(numstr));
		mbuf_push_back_1(&buf, '}');
	    }
	}
    }
    mbuf_push_back_1(&buf, '\0');

    str = strdup((char*)mbuf_ptr(&buf));

    mbuf_free(&buf);
    return str;
}

