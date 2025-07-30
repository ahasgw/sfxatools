#! /bin/sh

gnulib-tool --import \
  --dir=. \
  --lib=libgnu \
  --source-base=libgnu \
  --m4-base=libgnu/m4 \
  --doc-base=libgnu/doc \
  --tests-base=libgnu/tests \
  --aux-dir=config \
  --no-changelog \
  --conditional-dependencies \
  assert dirname dummy error exitfail ftruncate getline getsubopt \
  gettext gettimeofday memchr mktime progname snprintf bool stdint-h \
  strerror strstr strtod strtoimax strtol strtoll strtoul strtoull strtoumax \
  vsnprintf xalloc xalloc-die
