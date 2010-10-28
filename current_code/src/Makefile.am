## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(OPENDIAS_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = opendias

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
	scan.c    \
	scan.h    \
	speak.cc    \
	speak.h    \
	utils.c    \
	utils.h    \
	main.c    \
	read_odf.c    \
	read_odf.h    \
	dbaccess.c    \
	dbaccess.h    \
	imageProcessing.h    \
	imageProcessing.c    \
	validation.c    \
	validation.h

opendias_LDFLAGS = 

opendias_LDADD = $(OPENDIAS_LIBS)

