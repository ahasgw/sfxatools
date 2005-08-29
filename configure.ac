# $Id: configure.ac,v 1.12 2005/08/29 07:33:15 aki Exp $
# vi: set tabstop=8 shiftwidth=8 noexpandtab:
#
# Makefile for the toplevel directory of sfxatools.
# Copyright (C) 2005 RIKEN. All rights reserved.
# Written by Aki Hasegawa <aki@gsc.riken.jp>.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

AC_PREREQ(2.59)
AC_INIT([SfxaTools], [0.2.2.1], [aki@gsc.riken.jp], [sfxatools])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_TESTDIR(tests)
gl_EARLY
AM_INIT_AUTOMAKE([1.9])
AC_CONFIG_HEADERS([config.h])
AC_SUBST(ac_aux_dir)

# Checks for compiler.

# Checks for programs.
AC_PROG_LN_S
AC_PROG_RANLIB

AC_CHECK_PROGS(HELP2MAN, help2man)
AM_MISSING_PROG(AUTOM4TE, autom4te)

# Checks for system services.
AC_SYS_LARGEFILE

# Checks for libraries.
gl_SOURCE_BASE(libgnu)
gl_M4_BASE(libgnu/m4)
gl_LIB(libgnu)
gl_MODULES([c-bs-a assert dummy exit atexit strtod strtol strtoul memchr
 memcmp memcpy memmove memset strcspn strpbrk strstr strerror mktime calloc
 free malloc realloc restrict stdbool stdint ullong_max strtoll strtoull
 strtoimax strtoumax snprintf vsnprintf getsubopt dirname getopt xalloc
 xalloc-die getline error exitfail gettext progname ftruncate getsubopt])
gl_INIT

# Checks for header files.
AC_CHECK_HEADERS(sys/file.h)
AC_CHECK_HEADERS(sys/param.h)
AC_CHECK_HEADERS(sys/mman.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(size_t)

AM_CONDITIONAL(IS_SIZEOF_OFF_T_EQ_8, test "${ac_cv_sizeof_off_t}" = "8")
AC_SUBST(IS_SIZEOF_OFF_T_EQ_8)

# Checks for library functions.
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(mmap)
AC_CHECK_FUNCS(munmap)
AC_CHECK_FUNCS(msync)

# Output
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(libgnu/Makefile)
AC_CONFIG_FILES(util/Makefile)
AC_CONFIG_FILES(lib/Makefile)
AC_CONFIG_FILES(src/Makefile)
AC_CONFIG_FILES(bio/Makefile)
AC_CONFIG_FILES(tests/Makefile)
AC_CONFIG_FILES(tests/mksfxa, [chmod +x tests/mksfxa])
AC_CONFIG_FILES(tests/mklcpa, [chmod +x tests/mklcpa])
AC_CONFIG_FILES(tests/sfxasrch, [chmod +x tests/sfxasrch])
AC_CONFIG_FILES(tests/fa2hdsq, [chmod +x tests/fa2hdsq])
AC_CONFIG_FILES(tests/annotidx, [chmod +x tests/annotidx])
AC_CONFIG_FILES(tests/bl2dump, [chmod +x tests/bl2dump])
AC_CONFIG_FILES(tests/psps, [chmod +x tests/psps])
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(sfxatools.spec)

AC_OUTPUT
