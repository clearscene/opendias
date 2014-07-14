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
                  libocrplug.la \
                  libphashplug.la \
                  libimageProcessing.la \
                  libvalidation.la \
                  libsessionManagement.la

libutils_la_SOURCES = utils.c utils.h
libutils_la_LDFLAGS = $(OPENDIAS_LIBS)

libdb_la_SOURCES = db.c db.h
libdb_la_LDFLAGS = $(OPENDIAS_LIBS)

libsimpleLinkedList_la_SOURCES = simpleLinkedList.c simpleLinkedList.h
libsimpleLinkedList_la_LDFLAGS = $(OPENDIAS_LIBS)

libphashplug_la_SOURCES = phash_plug.cc phash_plug.h
libphashplug_la_LDFLAGS = $(OPENDIAS_LIBS)

libocrplug_la_SOURCES = ocr_plug.cc ocr_plug.h
libocrplug_la_LDFLAGS = $(OPENDIAS_LIBS)

libimageProcessing_la_SOURCES = imageProcessing.c imageProcessing.h
libimageProcessing_la_LIBADD = libocrplug.la libphashplug.la
libimageProcessing_la_LDFLAGS = $(OPENDIAS_LIBS)

libvalidation_la_SOURCES = validation.c validation.h
libvalidation_la_LIBADD = libocrplug.la
libvalidation_la_LDFLAGS = $(OPENDIAS_LIBS)

libsessionManagement_la_SOURCES = sessionManagement.c sessionManagement.h
libsessionManagement_la_LDFLAGS = $(OPENDIAS_LIBS)

# --------------------------------------------------------

bin_PROGRAMS =  opendias \
                opendias_worker \
                opendiasconfig

opendiasconfig_LDFLAGS = 
opendiasconfig_LDADD = libsimpleLinkedList.la libdb.la libutils.la $(OPENDIAS_LIBS)
opendiasconfig_CFLAGS = $(AM_CFLAGS)
opendiasconfig_SOURCES = \
  db.h    \
  debug.c    \
  debug.h    \
  utils.h \
  main.h \
  opendiasconfig.c \
  simpleLinkedList.h


opendias_worker_LDFLAGS = 
opendias_worker_LDADD = libphashplug.la libimageProcessing.la libvalidation.la libsimpleLinkedList.la libdb.la libutils.la libocrplug.la $(OPENDIAS_LIBS)
opendias_worker_CFLAGS = $(AM_CFLAGS)
opendias_worker_SOURCES = \
  db.h    \
  debug.c    \
  debug.h    \
  scan.c    \
  scan.h    \
  scanner.c    \
  scanner.h    \
  dbaccess.c    \
  dbaccess.h    \
  ocr_plug.h    \
  pdf_plug.cc    \
  pdf_plug.h    \
  phash_plug.h \
  imageProcessing.h    \
  utils.h \
  main.h \
  opendias_worker.c \
  validation.h    \
  localisation.h    \
  localisation.c    \
  simpleLinkedList.h


opendias_LDFLAGS = 
opendias_LDADD = libphashplug.la libimageProcessing.la libsessionManagement.la libvalidation.la libsimpleLinkedList.la libdb.la libutils.la libocrplug.la $(OPENDIAS_LIBS)
opendias_CFLAGS = $(AM_CFLAGS)
opendias_SOURCES = \
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
  main.h    \
  ocr_plug.h    \
  pdf_plug.cc    \
  pdf_plug.h    \
  phash_plug.h \
  backpopulate.c \
  backpopulate.h \
  main.c    \
  odf_plug.c    \
  odf_plug.h    \
  dbaccess.c    \
  dbaccess.h    \
  imageProcessing.h    \
  localisation.h    \
  localisation.c    \
  validation.h    \
  import_doc.c    \
  import_doc.h    \
  saneDispatcher.c    \
  saneDispatcher.h    \
  simpleLinkedList.h \
  sessionManagement.h


