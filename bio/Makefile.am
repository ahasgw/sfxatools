## $Id: Makefile.am,v 1.5 2005/08/17 10:11:40 aki Exp $
##
## Makefile for the bio directory of biotuils.
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
EXTRA_DIST = NCBIstdaa.map NCBI2na.map

## suffix rule for building 64bit version (compile with -DSIZE_OF_INITXX_T=8)
SUFFIXES = .o64
.c.o64:
@am__fastdepCC_TRUE@	if $(COMPILE) -DSIZEOF_INTXX_T=8 -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $@ $<; \
@am__fastdepCC_TRUE@	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; fi
@AMDEP_TRUE@@am__fastdepCC_FALSE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(COMPILE) -DSIZEOF_INTXX_T=8 -c -o $@ $<


bin_PROGRAMS = 
pkgdata_DATA =

AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/util -I$(top_srcdir)/libgnu
AM_CFLAGS = -DNDEBUG

## fa2hdsq
bin_PROGRAMS += fa2hdsq
fa2hdsq_SOURCES = fa2hdsq.c ofiles.c ofiles.h parsefa.c parsefa.h
fa2hdsq_LDADD = 
if IS_SIZEOF_OFF_T_EQ_8
fa2hdsq_LDADD += parsefa.o64
endif
fa2hdsq_LDADD += $(top_builddir)/util/libutil.a
fa2hdsq_LDADD += $(top_builddir)/libgnu/libgnu.a

## bl2dump
bin_PROGRAMS += bl2dump
bl2dump_SOURCES = bl2dump.c
bl2dump_LDADD = 
bl2dump_LDADD += $(top_builddir)/util/libutil.a
bl2dump_LDADD += $(top_builddir)/libgnu/libgnu.a

## annotidx
bin_PROGRAMS += annotidx
annotidx_SOURCES = annotidx.c
annotidx_LDADD = 
annotidx_LDADD += $(top_builddir)/util/libutil.a
annotidx_LDADD += $(top_builddir)/libgnu/libgnu.a

## psps
bin_PROGRAMS += psps
psps_SOURCES = psps.c prosite.c prosite.h
psps_LDADD = 
psps_LDADD += $(top_builddir)/lib/libsfxa.a
psps_LDADD += $(top_builddir)/util/libutil.a
psps_LDADD += $(top_builddir)/libgnu/libgnu.a

## data
pkgdata_DATA += $(top_srcdir)/bio/NCBIstdaa.map
pkgdata_DATA += $(top_srcdir)/bio/NCBI2na.map

if IS_SIZEOF_OFF_T_EQ_8
clean-local:
	-rm -f *.o64
endif
