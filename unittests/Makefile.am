## Process this file with automake to produce Makefile.in

TESTS = main
check_PROGRAMS = main
main_SOURCES = main.c $(top_builddir)/src/utils.h
maint_CFLAGS = @CHECK_CFLAGS@
main_LDADD = $(top_builddir)/src/libutils.la @CHECK_LIBS@ 
