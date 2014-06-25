## Process this file with automake to produce Makefile.in

TESTS = core tests_utils 
check_PROGRAMS = core tests_utils 

#-------------------------------------
core_SOURCES = core.c
core_CFLAGS = @CHECK_CFLAGS@
core_LDADD = @CHECK_LIBS@ 

#-------------------------------------
tests_utils_SOURCES = tests_utils.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c \
  $(top_builddir)/src/utils.h 
tests_utils_CFLAGS = @CHECK_CFLAGS@
tests_utils_LDADD = $(top_builddir)/src/libutils.la @CHECK_LIBS@ 
