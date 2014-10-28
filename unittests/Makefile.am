## Process this file with automake to produce Makefile.in

TESTS = core \
        tests_utils \
        tests_simpleLinkedList \
        tests_sessionManagement \
        tests_validation \
        tests_localisation \
        tests_db \
        tests_scanner

check_PROGRAMS = core \
        tests_utils \
        tests_simpleLinkedList \
        tests_sessionManagement \
        tests_validation \
        tests_localisation \
        tests_db \
        tests_scanner

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
  $(top_builddir)/src/libsessionManagement.la \
  $(top_builddir)/src/libsimpleLinkedList.la \
  $(top_builddir)/src/libutils.la \
  @CHECK_LIBS@ 

#--- validation ----------------------------------
tests_validation_SOURCES = \
  $(top_builddir)/src/validation.h \
  tests_validation.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c \
  $(top_builddir)/src/imageProcessing.h \
  $(top_builddir)/src/ocr_plug.h 
tests_validation_CFLAGS = @CHECK_CFLAGS@
tests_validation_LDADD = \
  $(top_builddir)/src/libvalidation.la \
  $(top_builddir)/src/libsimpleLinkedList.la \
  $(top_builddir)/src/libutils.la \
  $(top_builddir)/src/libocrplug.la \
  $(top_builddir)/src/libimageProcessing.la \
  @CHECK_LIBS@ 

#---localisation ----------------------------------
tests_localisation_SOURCES = \
  $(top_builddir)/src/localisation.h \
  tests_localisation.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c 
tests_localisation_CFLAGS = @CHECK_CFLAGS@
tests_localisation_LDADD = \
  $(top_builddir)/src/liblocalisation.la \
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

#--- scanner ----------------------------------
tests_scanner_SOURCES = \
  $(top_builddir)/src/scanner.h \
  tests_scanner.c \
  $(top_builddir)/src/debug.h \
  stubbed_debug.c 
tests_scanner_CFLAGS = @CHECK_CFLAGS@
tests_scanner_LDADD = \
  $(top_builddir)/src/libscanner.la \
  @CHECK_LIBS@ 


