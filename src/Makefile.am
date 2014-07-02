## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
  -DETC_DIR=\""$(sysconfdir)"\" \
  -DVAR_DIR=\""$(VAR_DIR)"\" \
  -DSHARE_DIR=\""$(datadir)"\" \
  -DTESS_DIR=\""$(TESSERACT_BASE_DIR)"\" \
  $(OPENDIAS_CFLAGS)

AM_CFLAGS =\
	 -Wall

lib_LTLIBRARIES = libutils.la \
                  libdb.la \
                  libsimpleLinkedList.la \
                  libsessionManagement.la

libutils_la_SOURCES = utils.c utils.h
libutils_la_LDFLAGS = $(OPENDIAS_LIBS)

libdb_la_SOURCES = db.c db.h
libdb_la_LDFLAGS = $(OPENDIAS_LIBS)

libsimpleLinkedList_la_SOURCES = simpleLinkedList.c simpleLinkedList.h
libsimpleLinkedList_la_LDFLAGS = $(OPENDIAS_LIBS)

libsessionManagement_la_SOURCES = sessionManagement.c sessionManagement.h
libsessionManagement_la_LDFLAGS = $(OPENDIAS_LIBS)

# --------------------------------------------------------

bin_PROGRAMS =  opendias \
                opendias_worker \
                opendiasconfig

opendiasconfig_LDFLAGS = 
opendiasconfig_LDADD = $(OPENDIAS_LIBS)
opendiasconfig_CFLAGS = $(AM_CFLAGS)
opendiasconfig_SOURCES = \
  db.c    \
  db.h    \
  debug.c    \
  debug.h    \
  utils.h \
  utils.c \
  main.h \
  opendiasconfig.c \
  simpleLinkedList.c    \
  simpleLinkedList.h


opendias_worker_LDFLAGS = 
opendias_worker_LDADD = $(OPENDIAS_LIBS)
opendias_worker_CFLAGS = $(AM_CFLAGS)
opendias_worker_SOURCES = \
  db.c    \
  db.h    \
  debug.c    \
  debug.h    \
  scan.c    \
  scan.h    \
  scanner.c    \
  scanner.h    \
  dbaccess.c    \
  dbaccess.h    \
  ocr_plug.cc    \
  ocr_plug.h    \
  pdf_plug.cc    \
  pdf_plug.h    \
  phash_plug.cc \
  phash_plug.h \
  imageProcessing.h    \
  imageProcessing.c    \
  utils.h \
  utils.c \
  main.h \
  opendias_worker.c \
  validation.c    \
  validation.h    \
  localisation.h    \
  localisation.c    \
  simpleLinkedList.c    \
  simpleLinkedList.h


opendias_LDFLAGS = 
opendias_LDADD = $(OPENDIAS_LIBS)
opendias_CFLAGS = $(AM_CFLAGS)
opendias_SOURCES = \
  db.c    \
  db.h    \
  debug.c    \
  debug.h    \
  doc_editor.c    \
  doc_editor.h    \
  web_handler.c    \
  web_handler.h    \
  pageRender.c    \
  pageRender.h    \
  utils.h \
  utils.c \
  main.h    \
  ocr_plug.cc    \
  ocr_plug.h    \
  pdf_plug.cc    \
  pdf_plug.h    \
  phash_plug.cc \
  phash_plug.h \
  backpopulate.c \
  backpopulate.h \
  main.c    \
  odf_plug.c    \
  odf_plug.h    \
  dbaccess.c    \
  dbaccess.h    \
  imageProcessing.h    \
  imageProcessing.c    \
  localisation.h    \
  localisation.c    \
  validation.c    \
  validation.h    \
  import_doc.c    \
  import_doc.h    \
  saneDispatcher.c    \
  saneDispatcher.h    \
  simpleLinkedList.c    \
  simpleLinkedList.h \
  sessionManagement.c \
  sessionManagement.h


