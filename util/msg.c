/***********************************************************************
 * $Id: msg.c,v 1.3 2005/12/15 13:37:51 aki Exp $
 *
 * messaging function
 * Copyright (C) 2004 RIKEN. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include <dirname.h>
#include <progname.h>

#include "msg.h"

/*======================================================================
 * constant definitions
 *======================================================================*/

static const char * const no_pfx	= "    ";
static const char * const err_pfx	= "(E) ";
static const char * const warning_pfx	= "(W) ";
static const char * const notice_pfx	= "(N) ";
static const char * const info_pfx	= "(I) ";
static const char * const debug_pfx	= "(D) ";

/*======================================================================
 * messaging function
 *======================================================================*/

#ifdef NDEBUG
void msg(const msglvl_t lvl, const char *fmt, ...)
{
    const char *pfx = no_pfx;
    va_list args;

    fflush(stdout);

    if (program_name != NULL)
	fprintf(stderr, "%s:", base_name(program_name));

    switch (lvl) {
	case MSGLVL_ERR:	pfx = err_pfx; break;
	case MSGLVL_WARNING:	pfx = warning_pfx; break;
	case MSGLVL_NOTICE:	pfx = notice_pfx; break;
	case MSGLVL_INFO:	pfx = info_pfx; break;
	case MSGLVL_DEBUG:	pfx = debug_pfx; break;
	default: pfx = no_pfx;
    }
    fprintf(stderr, "%s", pfx);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':')
	fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");
    fflush(stderr);
}
#else
void msg_at(const msglvl_t lvl, const char *file, int line, const char *fmt, ...)
{
    const char *pfx = no_pfx;
    va_list args;

    fflush(stdout);

    if (program_name != NULL)
	fprintf(stderr, "%s:", base_name(program_name));

    fprintf(stderr, "%s(%d):", file, line);

    switch (lvl) {
	case MSGLVL_ERR:	pfx = err_pfx; break;
	case MSGLVL_WARNING:	pfx = warning_pfx; break;
	case MSGLVL_NOTICE:	pfx = notice_pfx; break;
	case MSGLVL_INFO:	pfx = info_pfx; break;
	case MSGLVL_DEBUG:	pfx = debug_pfx; break;
	default: pfx = no_pfx;
    }
    fprintf(stderr, "%s", pfx);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':')
	fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");
    fflush(stderr);
}
#endif
