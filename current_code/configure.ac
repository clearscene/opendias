dnl Process this file with autoconf to produce a configure script.

AC_INIT(opendias, 0.4.0)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC




dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
GETTEXT_PACKAGE=opendias
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])






PKG_CHECK_MODULES(OPENDIAS, [sqlite3 gtk+-2.0 libmicrohttpd uuid])
AC_SUBST(OPENDIAS_CFLAGS)
AC_SUBST(OPENDIAS_LIBS)
AC_PATH_PROG(PKG_CONFIG,pkg-config,no)




dnl gthreading="no"
dnl if test "$PKG_CONFIG" != "no"; then
dnl 	gthread_cflags=`$PKG_CONFIG --cflags gthread-2.0`
dnl 	CFLAGS="$CFLAGS $gthread_cflags"
dnl 	CPPFLAGS="$CPPFLAGS $gthread_cflags"
dnl 	gthread_libs=`$PKG_CONFIG --libs gthread-2.0`
dnl 	LIBS="$LIBS $gthread_libs"
dnl 	AC_CHECK_LIB(gthread-2.0, g_thread_init,
dnl 		[AC_CHECK_HEADER(glib/gthread.h,
dnl 			gthreading=yes,
dnl 			AC_MSG_WARN(You are missing the development headers for the gthread libs))],
dnl 		 AC_MSG_WARN(You are missing the gthread libs) )
dnl fi
dnl pthreading="no"
dnl AC_CHECK_LIB(pthread, pthread_create,
dnl 	[AC_CHECK_HEADER(pthread.h,
dnl 		pthreading=yes,
dnl 		AC_MSG_WARN(You are missing the development headers for the pthread libs))],
dnl 	 AC_MSG_WARN(You are missing the pthread libs) )
dnl if test "x$gthreading" = "xyes" && test "x$pthreading" = "xyes" ; then
dnl 	AC_DEFINE([CAN_THREAD],[],[Have everything that's needed for multi-threading])
dnl 	OPENDIAS_LIBS="$OPENDIAS_LIBS $gthread_libs -lpthread "
dnl 	OPENDIAS_CFLAGS="$OPENDIAS_CFLAGS $gthread_cflags " 
dnl else
dnl 	AC_MSG_WARN(A single threaded version will be built.)
dnl fi



freeimage="no"
AC_CHECK_LIB(sane, main, 
	[AC_CHECK_HEADER(FreeImage.h,
        	freeimage=yes,
        	AC_MSG_WARN(You are missing the development headers for the FreeImage libs))], 
	AC_MSG_WARN(You are missing the FreeImage libs) )
if test "x$freeimage" = "xyes" ; then
	AC_DEFINE([CAN_IMAGE],[],[Have everything that's needed for scanning documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lfreeimage "
else
	AC_MSG_WARN(You wll not be able to save and load scanned images.)
fi


scanning="no"
AC_CHECK_LIB(sane, main, 
	[AC_CHECK_HEADER(sane/sane.h,
        	scanning=yes,
        	AC_MSG_WARN(You are missing the development headers for the sane libs))], 
	AC_MSG_WARN(You are missing the sane libs) )
if test "x$scanning" = "xyes" ; then
	AC_DEFINE([CAN_SCAN],[],[Have everything that's needed for scanning documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lsane "
else
	AC_MSG_WARN(You wll not be able to scan in any document.)
fi


tiff="no"
AC_CHECK_LIB(tiff, TIFFOpen, 
	[AC_CHECK_HEADER(tiff.h,
        	tiff=yes,
        	AC_MSG_WARN(You are missing the development headers for the tiff libs))], 
	AC_MSG_WARN(You are missing the tiff libs) )
tesseract="no"
AC_CHECK_LIB(tesseract_full, main,
	dnl Would have liked to use AC_CHECK_HEADER, but the checking does not recognise the C++ nature of this header! 
	[AC_CHECK_FILE(/usr/include/tesseract/baseapi.h,
        	tesseract=yes,
        	AC_MSG_WARN(You are missing the development headers for the tesseract libs))], 
	AC_MSG_WARN(You are missing the tesseract libs) )
if test "x$tesseract" = "xyes" && test "x$tiff" = "xyes" ; then
	AC_DEFINE([CAN_OCR],[],[Have everything that's needed to OCR documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -ltesseract_full -ltiff "
else
	AC_MSG_WARN(You wll not be able to OCR any document.)
fi


speak="no"
AC_CHECK_LIB(espeak, main, 
	[AC_CHECK_HEADER(espeak/speak_lib.h,
        	speak=yes,
        	AC_MSG_WARN(You are missing the development headers for the espeak libs))], 
	AC_MSG_WARN(You are missing the espeak libs) )
if test "x$speak" = "xyes" ; then
	AC_DEFINE([CAN_SPEAK],[],[Have everything that's needed speak aloud])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lespeak "
else
	AC_MSG_WARN(You wll not be able to read aloud.)
fi


xml="no"
AC_CHECK_LIB(xml2, xmlParseExternalEntity, 
	[AC_PATH_PROG(XML2_CONFIG,xml2-config,no)
	 if test "$XML2_CONFIG" != "no"; then
		xml2_cflags=`$XML2_CONFIG --cflags`
		CPPFLAGS="$CPPFLAGS $xml2_cflags"
	 fi
	 AC_CHECK_HEADER(libxml/parser.h,
        	xml=yes,
        	AC_MSG_WARN(You are missing the development headers for the xml libs))], 
	AC_MSG_WARN(You are missing the xml libs) )
pixbuf="no"
if test "$PKG_CONFIG" != "no"; then
	pixbuf_cflags=`$PKG_CONFIG --cflags gdk-pixbuf-2.0`
	CFLAGS="$CFLAGS $pixbuf_cflags"
	CPPFLAGS="$CPPFLAGS $pixbuf_cflags"
	pixbuf_libs=`$PKG_CONFIG --libs gdk-pixbuf-2.0`
	LIBS="$LIBS $pixbuf_libs"
	AC_CHECK_LIB(gdk_pixbuf-2.0, gdk_pixbuf_new, 
		[AC_CHECK_HEADER(gdk-pixbuf/gdk-pixbuf.h,
       		 pixbuf=yes,
        	 AC_MSG_WARN(You are missing the development headers for the gdk-picbuf libs))], 
		AC_MSG_WARN(You are missing the gdk-pixbuf libs) )
fi
archive="no"
AC_CHECK_LIB(archive, main, 
	[AC_CHECK_HEADER(archive.h,
        	archive=yes,
        	AC_MSG_WARN(You are missing the development headers for the archive libs))], 
	AC_MSG_WARN(You are missing the archive libs) )
zzip="no"
AC_CHECK_LIB(zzipwrap, main, 
	[AC_CHECK_HEADER(zzip/lib.h,
        	zzip=yes,
        	AC_MSG_WARN(You are missing the development headers for the zzip libs))], 
	AC_MSG_WARN(You are missing the zzip libs) )
if test "x$xml" = "xyes" && test "x$zzip" = "xyes" && test "x$pixbuf" = "xyes" && test "x$archive" = "xyes" ; then
	AC_DEFINE([CAN_READODF],[],[Have everything that's needed to read ODF documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -larchive -lxml2 -lzzip $pixbuf_libs "
	OPENDIAS_CFLAGS="$OPENDIAS_CFLAGS $xml2_cflags $pixbuf_cflags " 
else
	AC_MSG_WARN(You wll not be able to import any ODF document.)
fi


dnl For testing: delete the "dontReConfigureMe" flag. So if 'configure' has been run on its own by a user
dnl then the "test scripts" know we should recompile
dnl rm -rf dontBotherReConfiguringMe-testScript


AC_OUTPUT([
Makefile
src/Makefile
sql/Makefile
po/Makefile.in
])
