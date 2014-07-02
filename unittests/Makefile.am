## Process this file with automake to produce Makefile.in

TESTS = core \
        tests_utils \
        tests_simpleLinkedList \
        tests_sessionManagement \
        tests_db 

check_PROGRAMS = core \
        tests_utils \
        tests_simpleLinkedList \
        tests_sessionManagement \
        tests_db 

#--- core ----------------------------------
core_SOURCES = core.c
core_CFLAGS = @CHECK_CFLAGS@
core_LDADD = @CHECK_LIBS@ 

#--- utils ----------------------------------
tests_utils_SOURCES = \
  $(top_builddir)/src/utils.h \
  tests_utils.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c 
tests_utils_CFLAGS = @CHECK_CFLAGS@
tests_utils_LDADD = \
  $(top_builddir)/src/libutils.la \
  @CHECK_LIBS@ 

#--- simplelinkedList ----------------------------------
tests_simpleLinkedList_SOURCES = \
  $(top_builddir)/src/simpleLinkedList.h \
  tests_simpleLinkedList.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c 
tests_simpleLinkedList_CFLAGS = @CHECK_CFLAGS@
tests_simpleLinkedList_LDADD = \
  $(top_builddir)/src/libsimpleLinkedList.la \
  $(top_builddir)/src/libutils.la \
  @CHECK_LIBS@ 


#--- sessionManagement ----------------------------------
tests_sessionManagement_SOURCES = \
  $(top_builddir)/src/sessionManagement.h \
  tests_sessionManagement.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c 
tests_sessionManagement_CFLAGS = @CHECK_CFLAGS@
tests_sessionManagement_LDADD = \
  $(top_builddir)/src/libsimpleLinkedList.la \
  $(top_builddir)/src/libutils.la \
  @CHECK_LIBS@ 

#--- db ----------------------------------
tests_db_SOURCES = \
  $(top_builddir)/src/db.h \
  tests_db.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c \
  sqlite3.h \
  stubbed_sqlite3.h \
  stubbed_sqlite3.c 
tests_db_CFLAGS = @CHECK_CFLAGS@
tests_db_LDADD = \
  $(top_builddir)/src/libdb.la \
  $(top_builddir)/src/libutils.la \
  $(top_builddir)/src/libsimpleLinkedList.la \
  @CHECK_LIBS@ 


