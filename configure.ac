# Makefile for the toplevel directory of sfxatools.
# Copyright (C) 2005, 2006 RIKEN. All rights reserved.
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

AC_PREREQ([2.70])
AC_INIT([SfxaTools], [0.2.5], [aki@gsc.riken.jp], [sfxatools])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([libgnu/m4])
AC_CONFIG_TESTDIR([tests])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall info-in-builddir])
AC_CONFIG_HEADERS([config.h])
AC_SUBST(ac_aux_dir)
AC_SUBST(AR, [${AR:-ar}])
AC_SUBST(LD, [${LD:-ld}])

# Checks for compiler.
AC_PROG_CC

# Checks for libraries.
gl_EARLY
gl_INIT

# Checks for programs.
AM_PROG_AR
AC_PROG_LN_S
AC_PROG_RANLIB

AC_CHECK_PROGS(HELP2MAN, [help2man])
AM_MISSING_PROG(AUTOM4TE, [autom4te])

# Checks for system services.
AC_SYS_LARGEFILE

# Checks for header files.
AC_CHECK_HEADERS([
                  sys/file.h
                  sys/param.h
                  sys/mman.h
                  ])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_SIZEOF([off_t])
AC_CHECK_SIZEOF([size_t])

AM_CONDITIONAL([IS_SIZEOF_OFF_T_EQ_8], [test "$ac_cv_sizeof_off_t" = "8"])
AC_SUBST([IS_SIZEOF_OFF_T_EQ_8])

# Checks for library functions.
AC_CHECK_FUNCS([
                strdup
                mmap
                munmap
                msync
                ])

# Output
AC_CONFIG_FILES([
                 Makefile
                 libgnu/Makefile
                 util/Makefile
                 lib/Makefile
                 src/Makefile
                 bio/Makefile
                 doc/Makefile
                 sfxatools.spec
                 tests/Makefile
                 ])
AC_CONFIG_FILES([tests/mksfxa],   [chmod +x tests/mksfxa])
AC_CONFIG_FILES([tests/mklcpa],   [chmod +x tests/mklcpa])
AC_CONFIG_FILES([tests/sfxasrch], [chmod +x tests/sfxasrch])
AC_CONFIG_FILES([tests/fa2hdsq],  [chmod +x tests/fa2hdsq])
AC_CONFIG_FILES([tests/annotidx], [chmod +x tests/annotidx])
AC_CONFIG_FILES([tests/bl2dump],  [chmod +x tests/bl2dump])
AC_CONFIG_FILES([tests/psps],     [chmod +x tests/psps])
AC_CONFIG_FILES([tests/nsps],     [chmod +x tests/nsps])

AC_OUTPUT
