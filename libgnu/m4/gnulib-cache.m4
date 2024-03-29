# Copyright (C) 2004 Free Software Foundation, Inc.
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the specification of how gnulib-tool is used.
# It acts as a cache: It is written and read by gnulib-tool.
# In projects using CVS, this file is meant to be stored in CVS,
# like the configure.ac and various Makefile.am files.


# Specification in the form of a command-line invocation:
#   gnulib-tool --import --dir=.. --lib=libgnu --source-base=libgnu --m4-base=libgnu/m4 --aux-dir=config --macro-prefix=gl assert atexit c-bs-a calloc dirname dummy error exit exitfail free ftruncate getline getopt getsubopt gettext gettimeofday malloc memchr memcmp memcpy memmove memset mktime progname realloc restrict snprintf ssize_t stdbool stdint strcspn strerror strndup strpbrk strstr strtod strtoimax strtol strtoll strtoul strtoull strtoumax ullong_max vsnprintf xalloc xalloc-die

# Specification in the form of a few gnulib-tool.m4 macro invocations:
gl_MODULES([assert atexit c-bs-a calloc dirname dummy error exit exitfail free ftruncate getline getopt getsubopt gettext gettimeofday malloc memchr memcmp memcpy memmove memset mktime progname realloc restrict snprintf ssize_t stdbool stdint strcspn strerror strndup strpbrk strstr strtod strtoimax strtol strtoll strtoul strtoull strtoumax ullong_max vsnprintf xalloc xalloc-die])
gl_AVOID([])
gl_SOURCE_BASE([libgnu])
gl_M4_BASE([libgnu/m4])
gl_TESTS_BASE([tests])
gl_LIB([libgnu])
gl_MACRO_PREFIX([gl])
