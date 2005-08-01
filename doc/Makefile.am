## $Id: Makefile.am,v 1.5 2005/08/01 09:04:47 aki Exp $
##
## Makefile for the doc directory of sfxatools.
## Copyright (C) 2005 RIKEN. All rights reserved.
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

CLEANFILES = mksfxa.1 mklcpa.1 sfxasrch.1 fa2hdsq.1 annotidx.1 bl2dump.1
DISTCLEANFILES = sfxatools.info
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = $(man_MANS)

info_TEXINFOS = 
man_MANS = 

info_TEXINFOS += sfxatools.texi

HELP2MANFLAGS = -p sfxatools

man_MANS += mksfxa.1
mksfxa.1 : $(top_builddir)/src/mksfxa
	if test "x$(HELP2MAN)" != x; then \
	  $(HELP2MAN) $(HELP2MANFLAGS) $< > $@; \
	fi

man_MANS += mklcpa.1
mklcpa.1 : $(top_builddir)/src/mklcpa
	if test "x$(HELP2MAN)" != x; then \
	  $(HELP2MAN) $(HELP2MANFLAGS) $< > $@; \
	fi

man_MANS += sfxasrch.1
sfxasrch.1 : $(top_builddir)/src/sfxasrch
	if test "x$(HELP2MAN)" != x; then \
	  $(HELP2MAN) $(HELP2MANFLAGS) $< > $@; \
	fi

man_MANS += fa2hdsq.1
fa2hdsq.1 : $(top_builddir)/bio/fa2hdsq
	if test "x$(HELP2MAN)" != x; then \
	  $(HELP2MAN) $(HELP2MANFLAGS) $< > $@; \
	fi

man_MANS += annotidx.1
annotidx.1 : $(top_builddir)/bio/annotidx
	if test "x$(HELP2MAN)" != x; then \
	  $(HELP2MAN) $(HELP2MANFLAGS) $< > $@; \
	fi

man_MANS += bl2dump.1
bl2dump.1 : $(top_builddir)/bio/bl2dump
	if test "x$(HELP2MAN)" != x; then \
	  $(HELP2MAN) $(HELP2MANFLAGS) $< > $@; \
	fi
