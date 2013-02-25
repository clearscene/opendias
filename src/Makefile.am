## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
  -DETC_DIR=\""$(sysconfdir)"\" \
  -DVAR_DIR=\""$(VAR_DIR)"\" \
  -DSHARE_DIR=\""$(datadir)"\" \
  $(OPENDIAS_CFLAGS)

AM_CFLAGS =\
	 -Wall

bin_PROGRAMS = opendias opendiasconfig

opendiasconfig_SOURCES = \
  db.c    \
  db.h    \
  debug.c    \
  debug.h    \
  utils.c    \
  utils.h    \
  opendiasconfig.h \
  opendiasconfig.c \
  simpleLinkedList.c    \
  simpleLinkedList.h

opendiasconfig_LDFLAGS = 

opendiasconfig_LDADD = $(OPENDIAS_LIBS)

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
  main.h    \
  ocr_plug.cc    \
  ocr_plug.h    \
  pdf_plug.cc    \
  pdf_plug.h    \
  phash_plug.cc \
  phash_plug.h \
  scan.c    \
  scan.h    \
  scanner.c    \
  scanner.h    \
  utils.c    \
  utils.h    \
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
  sessionmanagement.c \
  sessionmanagement.h

opendias_LDFLAGS = 

opendias_LDADD = $(OPENDIAS_LIBS)

