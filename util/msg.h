/***********************************************************************
 * $Id: msg.h,v 1.4 2005/12/15 13:37:51 aki Exp $
 *
 * Header file for messaging functions
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

#ifndef MSG_H
#define MSG_H 1
#define MSG_H_INCLUDED 1

#ifndef NDEBUG
# define msg(level, ...)    msg_at(level, __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
 * typedefs
 *======================================================================*/

typedef enum msglvl_type {
    MSGLVL_ERR,	    /* error level */
    MSGLVL_WARNING, /* warning level */
    MSGLVL_NOTICE,  /* normal, bug significant level */
    MSGLVL_INFO,    /* informational level */
    MSGLVL_DEBUG    /* debug level */
} msglvl_t;

/*======================================================================
 * prototypes
 *======================================================================*/

/* messaging functions */
#ifdef NDEBUG
void msg(const msglvl_t lvl, const char *fmt, ...);
#else
void msg_at(const msglvl_t lvl, const char *file, int line, const char *fmt, ...);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MSG_H */
