## Process this file with automake to produce Makefile.in

TESTS = main
check_PROGRAMS = main
main_SOURCES = main.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c \
  $(top_builddir)/src/utils.h \
  tests_utils.c
maint_CFLAGS = @CHECK_CFLAGS@
main_LDADD = $(top_builddir)/src/libutils.la @CHECK_LIBS@ 
