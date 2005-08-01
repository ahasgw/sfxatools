## $Id: Makefile.am,v 1.1 2005/08/01 09:04:48 aki Exp $
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
SUFFIXES = .o64
.c.o64:
@am__fastdepCC_TRUE@	if $(COMPILE) -DSIZEOF_INTXX_T=8 -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $@ $<; \
@am__fastdepCC_TRUE@	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; fi
@AMDEP_TRUE@@am__fastdepCC_FALSE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(COMPILE) -DSIZEOF_INTXX_T=8 -c -o $@ $<

AM_CPPFLAGS = -I$(top_srcdir)/util -I$(top_srcdir)/libgnu
AM_CFLAGS = -DNDEBUG
AM_YFLAGS = -d

BUILT_SOURCES = parser.h

noinst_LIBRARIES = libsfxa.a
libsfxa_a_SOURCES = \
	parser.y \
	region.c region.h \
	search.c search.h search_impl.h \
	sfxa.c sfxa.h
libsfxa_a_LIBADD = 
if IS_SIZEOF_OFF_T_EQ_8
libsfxa_a_LIBADD += search.o64
endif

if IS_SIZEOF_OFF_T_EQ_8
clean-local:
	-rm -f *.o64
endif
