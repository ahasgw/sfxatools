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

#include <string.h>

#include "prosite.h"

/*======================================================================
 * macro definitions
 *======================================================================*/

#define PUSH(c) \
    do { if (mbuf_push_back_1(re, (c)) != 1) state = ERR; } while (0)

#define PUSH2(s) \
    do { if (mbuf_push_back(re, (s), 2) != 2) state = ERR; } while (0)

#define POP(n) \
    do { if (mbuf_pop_back(re, (n)) != (n)) state = ERR; } while (0)

#define IUPAC \
	 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': \
    case 'H': case 'I': case 'K': case 'L': case 'M': case 'N': case 'P': \
    case 'Q': case 'R': case 'S': case 'T': case 'V': case 'W': case 'X': \
    case 'Y': case 'Z'

#define DIGIT \
         '0': case '1': case '2': case '3': case '4': case '5': case '6': \
    case '7': case '8': case '9'

/*======================================================================
 * constants definitions
 *======================================================================*/

const char * const  prosite_alphabet = "ABCDEFGHIKLMNPQRSTVWXYZ";

/*======================================================================
 * function definitions
 *======================================================================*/

int prosite_to_regex(const char *pat, mbuf_t *re)
{
    const char *cp = pat;
    const char *cp_end = cp + strlen(pat);
    int cnt = 0;
    int special_cterm = 0;

    enum {
	ERR, S0, S1, S2, P0, P1, P2, Q0, Q1, R0, R1, R2, R3, ACC0, ACC1, ACC2
    } state = S0;

    for (; cp < cp_end; ++cp) {
	if (state == ERR)
	    break;
	switch (state) {
	    case S0:	/* initial state */
		switch (*cp) {
		    case '<': PUSH('^'); state = S1; break;
		    case '[': PUSH('['); state = P0; cnt = 1; break;
		    case '{': PUSH2("[^"); state = Q0; break;
		    case 'x': PUSH('.'); state = ACC0; break;
		    case IUPAC: PUSH(*cp); state = ACC0; break;
		    default: state = ERR; break;
		}
		break;
	    case S1:	/* after '<' was read */
		switch (*cp) {
		    case '<': break;
		    case '[': PUSH('['); state = P0; cnt = 1; break;
		    case '{': PUSH2("[^"); state = Q0; break;
		    case 'x': PUSH('.'); state = ACC0; break;
		    case IUPAC: PUSH(*cp); state = ACC0; break;
		    default: state = ERR; break;
		}
		break;
	    case ACC0:	/* after reduction of atom. accept */
		switch (*cp) {
		    case '.': state = ACC1; break;
		    case '-': state = S0; break;
		    case '(': PUSH('{'); state = R0; break;
		    case '>': PUSH('$'); state = S2; break;
		    default: state = ERR; break;
		}
		break;
	    case S2:	/* after '>' */
		switch (*cp) {
		    case '.': state = ACC1; break;
		    case '>': break;
		    default: state = ERR; break;
		}
		break;
	    case ACC1:	/* accept */
		state = ERR;
		break;
	    case ERR:	/* error */
		break;
	    case P0:	/* just came into square parentheses '[ ]' */
		switch (*cp) {
		    case '>': state = P2; ++special_cterm; break;
		    case IUPAC: PUSH(*cp); state = P1; break;
		    default: state = ERR; break;
		}
		++cnt;
		break;
	    case P1:	/* between square parentheses '[ ]' */
		switch (*cp) {
		    case '>': state = P2; ++special_cterm; break;
		    case ']': PUSH(']'); state = ACC0; break;
		    case IUPAC: PUSH(*cp); break;
		    default: state = ERR; break;
		}
		++cnt;
		break;
	    case P2:	/* square parentheses including '>' */
		switch (*cp) {
		    case '>': ++special_cterm; break;
		    case ']': PUSH(']'); state = ACC2; break;
		    case IUPAC: PUSH(*cp); break;
		    default: state = ERR; break;
		}
		++cnt;
		break;
	    case ACC2:	/* after square parentheses including '>' */
		switch (*cp) {
		    case '.': state = ACC1; break;
		    default: state = ERR; break;
		}
		break;
	    case Q0:	/* just came into curly brackets '{ }' */
		switch (*cp) {
		    case IUPAC: PUSH(*cp); state = Q1; break;
		    default: state = ERR; break;
		}
		break;
	    case Q1:	/* between curly brackets '{ }' */
		switch (*cp) {
		    case '}': PUSH(']'); state = ACC0; break;
		    case IUPAC: PUSH(*cp); break;
		    default: state = ERR; break;
		}
		break;
	    case R0:	/* just came into parenthesis '( )' */
		switch (*cp) {
		    case DIGIT: PUSH(*cp); state = R1; break;
		    default: state = ERR; break;
		}
		break;
	    case R1:	/* between parenthesis '( )' */
		switch (*cp) {
		    case ')': PUSH('}'); state = ACC0; break;
		    case ',': PUSH(','); state = R2; break;
		    case DIGIT: PUSH(*cp); break;
		    default: state = ERR; break;
		}
		break;
	    case R2:	/* just came into comma and parenthesis ', )' */
		switch (*cp) {
		    case DIGIT: PUSH(*cp); state = R3; break;
		    default: state = ERR; break;
		}
		break;
	    case R3:	/* between comma and parenthesis ', )' */
		switch (*cp) {
		    case ')': PUSH('}'); state = ACC0; break;
		    case DIGIT: PUSH(*cp); break;
		    default: state = ERR; break;
		}
		break;
	}
    }

#if 0
    printf("*** cnt: %d, special_cterm: %d ***\n", cnt, special_cterm);
#endif

    /* for the case of '>' occures inside of square brackets */
    if (special_cterm > 0 && (state == ACC1 || state == ACC2)) {
	mbuf_t *copy = mbuf_dup(re);
	if (copy == NULL)
	    state = ERR;
	else {
	    if (state != ERR)
		PUSH('|');
	    if (state != ERR && (mbuf_append(re, copy) != mbuf_size(copy)))
		state = ERR;
	    if (state != ERR)
		POP(cnt - special_cterm);
	    if (state != ERR)
		PUSH('$');
	}
	mbuf_delete(copy);
    }

    PUSH('\0');

    return (state == ACC0 || state == ACC1 || state == ACC2)
	? 0 : (int)(cp - pat);
}
