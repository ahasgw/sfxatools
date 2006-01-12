## $Id: Makefile.am,v 1.4 2006/01/12 10:05:56 aki Exp $
##
## Makefile for lib directory of sfxatools.
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

## suffix rule for building 64bit version (compile with -DSIZE_OF_INITXX_T=8)
SUFFIXES = .64.o
.c.64.o:
@am__fastdepCC_TRUE@	if $(COMPILE) -DSIZEOF_INTXX_T=8 -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $@ $<; \
@am__fastdepCC_TRUE@	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; fi
@AMDEP_TRUE@@am__fastdepCC_FALSE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(COMPILE) -DSIZEOF_INTXX_T=8 -c -o $@ $<

AM_CPPFLAGS = -I$(top_srcdir)/util -I$(top_srcdir)/libgnu
AM_CFLAGS = -DNDEBUG

noinst_LIBRARIES = libsfxa.a
libsfxa_a_SOURCES = \
	lcp.c lcp.h \
	output.c output.h \
	regexp.c regexp.h \
	region.c region.h \
	search.c search.h \
	sfxa.c sfxa.h \
	sort.c sort.h
libsfxa_a_LIBADD = 
if IS_SIZEOF_OFF_T_EQ_8
libsfxa_a_LIBADD += lcp.64.o
libsfxa_a_LIBADD += output.64.o
libsfxa_a_LIBADD += search.64.o
libsfxa_a_LIBADD += sort.64.o
endif

if IS_SIZEOF_OFF_T_EQ_8
clean-local:
	-rm -f *.64.o
endif
