## $Id: Makefile.am,v 1.7 2006/01/12 10:11:04 aki Exp $
##
## Makefile for util directory of sfxatools.
## Copyright (C) 2004, 2005 RIKEN. All rights reserved.
## Written by Aki Hasegawa <aki@gsc.riken.jp>.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
                                                                                
## Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/libgnu
AM_CFLAGS = -DNDEBUG

noinst_LIBRARIES = libutil.a
libutil_a_SOURCES = \
	cmap.c cmap.h \
	mbuf.c mbuf.h ptrstk.h u32stk.h \
	mmfile.c mmfile.h \
	msg.c msg.h \
	bldpath.c bldpath.h \
	strdupcat.c strdupcat.h
