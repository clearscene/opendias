## Process this file with automake to produce Makefile.in

TESTS = main
check_PROGRAMS = main
main_SOURCES = main.c $(top_builddir)/src/utils.h
main_CFLAGS = @CHECK_CFLAGS@
main_LDADD = $(top_builddir)/src/utils.la @CHECK_LIBS@
