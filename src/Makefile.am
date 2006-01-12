## $Id: Makefile.am,v 1.6 2006/01/12 10:11:04 aki Exp $
##
## Makefile for the src directory of sfxatools.
## Copyright (C) 2004 RIKEN. All rights reserved.
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

bin_PROGRAMS = 

AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/util -I$(top_srcdir)/libgnu
AM_CFLAGS = -DNDEBUG

## mksfxa
bin_PROGRAMS += mksfxa
mksfxa_SOURCES = mksfxa.c
mksfxa_LDADD = 
mksfxa_LDADD += $(top_builddir)/lib/libsfxa.a
mksfxa_LDADD += $(top_builddir)/util/libutil.a
mksfxa_LDADD += $(top_builddir)/libgnu/libgnu.a

## mklcpa
bin_PROGRAMS += mklcpa
mklcpa_SOURCES = mklcpa.c
mklcpa_LDADD = 
mklcpa_LDADD += $(top_builddir)/lib/libsfxa.a
mklcpa_LDADD += $(top_builddir)/util/libutil.a
mklcpa_LDADD += $(top_builddir)/libgnu/libgnu.a

## sfxasrch
bin_PROGRAMS += sfxasrch
sfxasrch_SOURCES = sfxasrch.c
sfxasrch_LDADD = 
sfxasrch_LDADD += $(top_builddir)/lib/libsfxa.a
sfxasrch_LDADD += $(top_builddir)/util/libutil.a
sfxasrch_LDADD += $(top_builddir)/libgnu/libgnu.a
