/***********************************************************************
 * $Id: parser.y,v 1.2 2005/08/01 11:24:37 aki Exp $
 *
 * search
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

%{

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "search_impl.h"
#include "msg.h"

#include <minmax.h>

#define YYPARSE_PARAM arg
#define YYLEX_PARAM arg

typedef unsigned long long  syms_t;

typedef struct piece_type {
    mbuf_t	    *ranges;
    syms_t	    syms;
    unsigned long   most;
    unsigned long   least;
    unsigned long   head: 1;
    unsigned long   tail: 1;
} piece_t;

%}

%pure_parser

%union {
    char	    sym;
    unsigned long   num;
    syms_t	    syms;
    mbuf_t	    *ranges;
    piece_t	    piece;
}

%token <sym> SYM
%token <num> NUM
%token ESC

%type <ranges> expr branch /* inner */
%type <piece> piece atom
%type <syms> range
%type <syms> syms

%start expr

%{

int yylex(YYSTYPE *lvalp, void *arg);
int yyerror(const char *msg);
void print_piece(const piece_t *p);
void set_piece(piece_t *p, mbuf_t *ranges, syms_t syms, int head, int tail);

inline static syms_t getbit(char c)
    { return (c >= 'A') ? (1ULL << (c - 'A')) : 0ULL; }
inline static char getsym(int n)
    { return n + 'A'; }
inline static syms_t getalloffbits(void)
    { return 0ULL; }
inline static syms_t getallonbits(void)
    { return ~(getalloffbits()); }
inline static syms_t getbits(char c1, char c2)
{
    syms_t ret = 0ULL;
    int i = MIN(c1, c2);
    for (; i <= MAX(c1, c2); ++i) {
	ret |= getbit(i);
    }
    return ret;
}
inline static int chkbit(syms_t bits, char c)
    { return (bits & getbit(c)) != 0; }
inline static int chkbitn(syms_t bits, int n)
    { return (bits & (1ULL << n)) != 0; }
inline static void printbits(syms_t bits)
{
    int i = 0;
    for (; i < CHAR_BIT * sizeof(syms_t); ++i) {
	putc(chkbitn(bits, i) ? '1' : '0', stderr);
    }
    putc('\n', stderr);
}
inline static void printsyms(syms_t bits)
{
    int i = 0;
    for (; i < CHAR_BIT * sizeof(syms_t); ++i) {
	if (chkbitn(bits, i))
	    putc(getsym(i), stderr);
    }
    putc('\n', stderr);
}
inline static void copyback(parser_arg_t *pa, mbuf_t *ranges)
    { pa->ranges_out = ranges; }
inline static mbuf_t *dupranges(parser_arg_t *pa)
    { return mbuf_dup(pa->ranges_in); }
inline static unsigned long getrepmax(parser_arg_t *pa)
    { return pa->repeat_max; }



inline static int search(parser_arg_t *pa, mbuf_t *ranges_in,
	piece_t *p, mbuf_t **ranges_out)
{
    int ret = 0;
    unsigned long j;
    
#if 0
    print_piece(p);
#endif
    
    if (ranges_in == NULL)
	if ((ranges_in = mbuf_dup(pa->ranges_in)) == NULL)
	    return errno;

    for (j = 1; j <= p->most; ++j)
    {
#if 0
	fprintf(stderr, "%s %lu: mbuf_size: %llu\n",
		(j <= p->least ? "least" : "most"), j,
		(unsigned long long)mbuf_size(ranges_in));
#endif

	if ((*ranges_out = mbuf_new(NULL, 0)) == NULL)
	    return errno;

	if (j > p->least) {
	    if (mbuf_append(*ranges_out, ranges_in) != mbuf_size(ranges_in))
		return errno;
	}

	mbuf_clear(pa->chs);
	if (p->syms == getalloffbits()) {
	    head_tail_t ht = (p->head ? head : (p->tail ? tail : none));
	    mbuf_push_back_1(pa->chs, '\0');
	    if ((ret = (*pa->search_func)(ranges_in, *ranges_out,
			    pa->search_arg, pa->chs, ht)) != 0)
		return ret;
	} else {
	    int i;
	    for (i = 0; i < CHAR_BIT * sizeof(syms_t); ++i) {
		if (!chkbitn(p->syms, i))
		    continue;
		mbuf_push_back_1(pa->chs, getsym(i));
	    }
	    if (!mbuf_empty(pa->chs)) {
		if ((ret = (*pa->search_func)(ranges_in, *ranges_out,
				pa->search_arg, pa->chs, none)) != 0)
		    return ret;
	    }
	}

	mbuf_delete(ranges_in);
	ranges_in = *ranges_out;
    }

    return 0;
}

%}

%%

expr
    : branch			{$$ = $1; copyback(arg, $$);}
    | expr '|' branch
	    {mbuf_append($1, $3); $$ = $1; mbuf_delete($3); copyback(arg, $$);}
    ;

/*
inner
    : branch			{$$ = $1;}
    | inner '|' branch		{printf("inner '|' is not supported yet\n");}
    ;
*/

branch
    : piece			{if (search(arg, NULL, &($1), &($$)) != 0)
				    yyerror("Search failed."); }
    | branch piece		{if (search(arg, $1, &($2), &($$)) != 0)
				    yyerror("Search failed."); }
    ;

piece
    : atom			{$$.least = $$.most = 1;}
    | atom '*'			{$$.least = 0; $$.most = getrepmax(arg);}
    | atom '+'			{$$.least = 1; $$.most = getrepmax(arg);}
    | atom '?'			{$$.least = 0; $$.most = 1;}
    | atom '{' NUM '}'		{$$.least = $$.most = $3;}
    | atom '{' NUM ',' '}'	{$$.least = $3; $$.most = getrepmax(arg);}
    | atom '{' NUM ',' NUM '}'	{$$.least = $3; $$.most = $5;}
    ;

atom
    : SYM			{set_piece(&($$), NULL, getbits($1, $1), 0, 0);}
    | '.'			{set_piece(&($$), NULL, getallonbits(), 0, 0);}
    | range			{set_piece(&($$), NULL, $1, 0, 0);}
    | '^'			{set_piece(&($$), NULL, getalloffbits(), 1, 0);}
    | '$'			{set_piece(&($$), NULL, getalloffbits(), 0, 1);}
//    | '(' inner ')'		{set_piece(&($$), $2, getalloffbits(), 0, 0);}
    ;

range
    : '[' syms ']'		{$$ = $2;}
    | '[' '^' syms ']'		{$$ = ~($3);}
    ;

syms
    : SYM			{$$ = getbits($1, $1);} 
    | SYM '-' SYM		{$$ = getbits($1, $3);}
    | syms SYM			{$$ = ($1 | getbits($2, $2));}
    | syms SYM '-' SYM		{$$ = ($1 | getbits($2, $4));}
    ;

%%

enum state { S0 = 0, R0, R1, B0 };

int yylex(YYSTYPE *lvalp, void *arg)
{
    parser_arg_t *pa = (parser_arg_t *)arg;
    char c = *(pa->ptr++);
    int ret = 0;
#if 0
printf("*** yylex() read [%c] ***\n", c);
#endif

    if (c == '\0')
	return 0;
    
    switch (pa->state) {
    case B0:
	switch (c) {
	case ',': ret = (int)c; break;
	case '}': pa->state = S0; ret = (int)c; break;
	default:
	    if (isdigit(c)) {
		char *endp;
		lvalp->num = strtoul((pa->ptr - 1), &endp, 10);
		pa->ptr = (const char *)endp;
		ret = NUM;
	    } else {
		ret = (int)c;
	    }
	    break;
	}
	break;
    case R0:
	switch (c) {
	case '^': ret = '^'; break;
	default: pa->state = R1; lvalp->sym = c; ret = SYM; break;
	}
	break;
    case R1:
	switch (c) {
	case ']': pa->state = S0; ret = (int)c; break;
	case '-':
	    if (*(pa->ptr) != ']') {
		ret = '-';
	    } else {
		lvalp->sym = c;
		ret = SYM;
	    }
	    break;
	default: lvalp->sym = c; ret = SYM; break;
	}
	break;
    default: /* S0 */
	switch (c) {
	case '{': pa->state = B0; ret = (int)c; break;
	case '[': pa->state = R0; ret = (int)c; break;
	default:
	    if (c == '\\') {
		char nx = *(pa->ptr);
		if (nx == '\0') {
		    ret = ESC;
		} else {
		    switch (nx) {   /* oct, hex */
		    case '0': lvalp->sym = '\0'; break;
		    case 'a': lvalp->sym = '\a'; break;
		    case 'b': lvalp->sym = '\b'; break;
		    case 't': lvalp->sym = '\t'; break;
		    case 'n': lvalp->sym = '\n'; break;
		    case 'v': lvalp->sym = '\v'; break;
		    case 'f': lvalp->sym = '\f'; break;
		    case 'r': lvalp->sym = '\r'; break;
		    default: lvalp->sym = nx; break;
		    }
		    pa->ptr++;
		    ret = SYM;
		}
	    } else if (c == '*' || c == '+' || c == '?') {
		char nx = *(pa->ptr);
		if (nx == '*' || nx == '+' || nx == '?') {
		    lvalp->sym = c;
		    ret = SYM;
		} else {
		    ret = (int)c;
		}
	    } else if (c == ')' || c == '(' || c == '.'
		    || c == '^' || c == '$' || c == '|') {
		ret = (int)c;
	    } else {
		lvalp->sym = c;
		ret = SYM;
	    }
	}
	break;
    }
    return ret;
}

int yyerror(const char *s)
{
    errno = EINVAL;
    msg(MSGLVL_ERR, "%s:", s);
    return 1;
}

#if 1
void print_piece(const piece_t *p)
{
    fprintf(stderr, "[\n");
    printbits(p->syms);
    printsyms(p->syms);
    fprintf(stderr, "%s(%ld,%ld)%s\n", (p->head ? "^" : ""),
	    p->least, p->most, (p->tail ? "$" : ""));
    fprintf(stderr, "]\n");
}
#endif

void set_piece(piece_t *p, mbuf_t *ranges, syms_t syms, int head, int tail)
{
    p->ranges = ranges;
    p->syms = syms;
    p->least = 1;
    p->most = 1;
    p->head = (head ? 1 : 0);
    p->tail = (tail ? 1 : 0);
}
