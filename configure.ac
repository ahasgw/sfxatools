# $Id: configure.ac,v 1.2 2005/02/18 08:39:53 aki Exp $
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
AC_INIT([SfxaTools], [0.0.2], [aki@gsc.riken.jp], [sfxatools])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([config.h.in])
#AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_TESTDIR(tests)
gl_USE_SYSTEM_EXTENSIONS
#AC_GNU_SOURCE
#AC_AIX
#AC_MINIX
#gl_EARLY
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9])
AC_CONFIG_HEADERS([config.h])
AC_SUBST(ac_aux_dir)

# Checks for compiler.

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

AC_CHECK_PROGS(HELP2MAN, help2man)
AM_MISSING_PROG(AUTOM4TE, autom4te)

# Checks for system services.
AC_SYS_LARGEFILE

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDBOOL
#AC_HEADER_STDC checked by AC_GNU_SOURCE
AC_HEADER_DIRENT
AC_CHECK_HEADERS(assert.h)
AC_CHECK_HEADERS(argz.h)
#AC_CHECK_HEADERS(ctype.h) assumed to be existed by GNULib
AC_CHECK_HEADERS(endian.h)
#AC_CHECK_HEADERS(errno.h) assumed to be existed by GNULib
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(float.h)
##AC_CHECK_HEADERS(inttypes.h) checked by AC_GNU_SOURCE
AC_CHECK_HEADERS(libintl.h)
AC_CHECK_HEADERS(limits.h)
AC_CHECK_HEADERS(locale.h)
AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(math.h)
##AC_CHECK_HEADERS(memory.h) checked by AC_GNU_SOURCE
AC_CHECK_HEADERS(setjmp.h)
AC_CHECK_HEADERS(signal.h)
#AC_CHECK_HEADERS(stdarg.h)
#AC_CHECK_HEADERS(stdbool.h) checked by AC_HEADER_STDBOOL
AC_CHECK_HEADERS(stddef.h)
##AC_CHECK_HEADERS(stdint.h) checked by AC_GNU_SOURCE
AC_CHECK_HEADERS(stdio.h)
##AC_CHECK_HEADERS(stdlib.h) checked by AC_GNU_SOURCE
##AC_CHECK_HEADERS(string.h) checked by AC_GNU_SOURCE
##AC_CHECK_HEADERS(strings.h) checked by AC_GNU_SOURCE
AC_CHECK_HEADERS(sys/file.h)
##AC_CHECK_HEADERS(sys/stat.h) checked by AC_GNU_SOURCE
##AC_CHECK_HEADERS(sys/types.h) checked by AC_GNU_SOURCE
AC_CHECK_HEADERS(time.h)
##AC_CHECK_HEADERS(unistd.h) checked by AC_GNU_SOURCE
AC_CHECK_HEADERS(wchar.h)
#AC_CHECK_HEADERS(pthread.h)
AC_CHECK_HEADERS(sys/mman.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_C_CHAR_UNSIGNED
AC_C_CONST
AC_C_INLINE
#AC_C_LONG_DOUBLE
#AC_C_PROTOTYPES
AC_C_RESTRICT
AC_C_STRINGIZE
AC_C_VOLATILE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])
#AC_CHECK_SIZEOF(int)
#AC_CHECK_SIZEOF(long)
#AC_CHECK_SIZEOF(void*)
AC_CHECK_SIZEOF(off_t)
#AC_CHECK_SIZEOF(_int32)
AC_CHECK_SIZEOF(size_t)
#AC_CHECK_SIZEOF(ssize_t)
#AC_CHECK_SIZEOF([long long])
#AC_CHECK_SIZEOF([long double])

AM_CONDITIONAL(IS_SIZEOF_OFF_T_EQ_8, test "${ac_cv_sizeof_off_t}" = "8")
AC_SUBST(IS_SIZEOF_OFF_T_EQ_8)

# Checks for gnulib.
gl_SOURCE_BASE(libgnu)
gl_M4_BASE(libgnu/m4)
gl_LIB(libgnu)
gl_MODULES([c-bs-a assert dummy exit atexit strtod strtol strtoul memchr
 memcmp memcpy memmove memset strcspn strpbrk strstr strerror mktime
 calloc eealloc free malloc realloc strftime restrict stdbool stdint
 snprintf vsnprintf strtoll strtoull strtoimax strtoumax mathl ftruncate
 getsubopt mkstemp dirname getopt xalloc xalloc-die getline error exitfail
 gettext progname])
gl_INIT

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
#AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MALLOC
#AC_FUNC_MEMCMP	 # AC_FUNC_MMAP test is too sever. We use AC_CHECK_FUNCS.
AC_FUNC_MMAP
#AC_FUNC_REALLOC
#AC_FUNC_STRERROR_R
AC_FUNC_VPRINTF
#AC_CHECK_FUNCS(ftruncate)
AC_CHECK_FUNCS(getdelim)
AC_CHECK_FUNCS(getopt_long)
#AC_CHECK_FUNCS(memmove)
#AC_CHECK_FUNCS(memset)
AC_CHECK_FUNCS(munmap)
AC_CHECK_FUNCS(strchr)
AC_CHECK_FUNCS(strdup)
#AC_CHECK_FUNCS(strerror)
AC_CHECK_FUNCS(strrchr)
AC_CHECK_FUNCS(strtol)
#AC_CHECK_FUNCS([flock])
#AC_CHECK_FUNCS([pthread_create])
#AC_CHECK_FUNCS(getpagesize)
AC_CHECK_FUNCS(mmap)
AC_CHECK_FUNCS(madvise)
AC_CHECK_FUNCS(msync)
#AC_CHECK_FUNCS(munmap)

# Output
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(libgnu/Makefile)
AC_CONFIG_FILES(util/Makefile)
AC_CONFIG_FILES(src/Makefile)
AC_CONFIG_FILES(tests/Makefile)
AC_CONFIG_FILES(tests/mksfxa, [chmod +x tests/mksfxa])
AC_CONFIG_FILES(tests/mklcpa, [chmod +x tests/mklcpa])
AC_CONFIG_FILES(tests/sfxa, [chmod +x tests/sfxa])
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(sfxatools.spec)

AC_OUTPUT
