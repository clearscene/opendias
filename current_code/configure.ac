dnl Process this file with autoconf to produce a configure script.

AC_INIT(opendias, [0.7.3], [wbooth@clearscene.net])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC




dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
dnl GETTEXT_PACKAGE=opendias
dnl AC_SUBST(GETTEXT_PACKAGE)
dnl AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
dnl AM_GLIB_GNU_GETTEXT
dnl IT_PROG_INTLTOOL([0.35.0])

AC_CHECK_FUNCS([gethostbyaddr],,AC_MSG_ERROR([Missing function gethostbyaddr]))
AC_CHECK_FUNCS([strcspn],[],AC_MSG_ERROR([Missing function strcspn]))
AC_CHECK_FUNCS([strdup],[],AC_MSG_ERROR([Missing function strdup]))
AC_CHECK_FUNCS([strstr],[],AC_MSG_ERROR([Missing function strstr]))
AC_CHECK_HEADERS([arpa/inet.h],[],AC_MSG_ERROR([Missing header arpa/inet.h]))
AC_CHECK_HEADERS([fcntl.h],[],AC_MSG_ERROR([Missing header fcntl.h]))
AC_CHECK_HEADERS([netdb.h],[],AC_MSG_ERROR([Missing header netdb.h]))
AC_CHECK_HEADERS([netinet/in.h],[],AC_MSG_ERROR([Missing header netinet/in.h]))
AC_CHECK_HEADERS([sys/param.h],[],AC_MSG_ERROR([Missing header sys/param.h]))
AC_CHECK_HEADERS([sys/socket.h],[],AC_MSG_ERROR([Missing header sys/socket.h]))
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T





PKG_CHECK_MODULES(OPENDIAS, [sqlite3 libmicrohttpd uuid])
AC_SUBST(OPENDIAS_CFLAGS)
AC_SUBST(OPENDIAS_LIBS)
AC_PATH_PROG(PKG_CONFIG,pkg-config,no)




imagehandling="no"
AC_CHECK_LIB(freeimage, FreeImage_Load, 
	[AC_CHECK_HEADER(FreeImage.h,
        	imagehandling=yes,
        	AC_MSG_WARN(You are missing the development headers for the freeimage libs))], 
	AC_MSG_WARN(You are missing the freeimage libs) )
scanning="no"
AC_CHECK_LIB(sane, main, 
	[AC_CHECK_HEADER(sane/sane.h,
        	scanning=yes,
        	AC_MSG_WARN(You are missing the development headers for the sane libs))], 
	AC_MSG_WARN(You are missing the sane libs) )
if test "x$scanning" = "xyes" && test "x$imagehandling" = "xyes" ; then
	AC_DEFINE([CAN_SCAN],[],[Have everything that is needed for scanning documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lsane -lfreeimage "
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
	AC_DEFINE([CAN_OCR],[],[Have everything that is needed to OCR documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -ltesseract_full -ltiff "
else
	AC_MSG_WARN(You wll not be able to OCR any document.)
fi


dnl Not used in the app - atm
dnl speak="no"
dnl AC_CHECK_LIB(espeak, main, 
dnl 	[AC_CHECK_HEADER(espeak/speak_lib.h,
dnl         	speak=yes,
dnl         	AC_MSG_WARN(You are missing the development headers for the espeak libs))], 
dnl 	AC_MSG_WARN(You are missing the espeak libs) )
dnl if test "x$speak" = "xyes" ; then
dnl 	AC_DEFINE([CAN_SPEAK],[],[Have everything that is needed speak aloud])
dnl 	OPENDIAS_LIBS="$OPENDIAS_LIBS -lespeak "
dnl else
dnl 	AC_MSG_WARN(You wll not be able to read aloud.)
dnl fi


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
if test "x$xml" = "xyes" && test "x$zzip" = "xyes" && test "x$archive" = "xyes" ; then
	AC_DEFINE([CAN_READODF],[],[Have everything that is needed to read ODF documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -larchive -lxml2 -lzzip "
	OPENDIAS_CFLAGS="$OPENDIAS_CFLAGS $xml2_cflags " 
else
	AC_MSG_WARN(You wll not be able to import any ODF document.)
fi


OPENDIAS_LIBS="$OPENDIAS_LIBS -lpthread "

AC_OUTPUT([
Makefile
src/Makefile
sql/Makefile
webcontent/Makefile
webcontent/style/Makefile
webcontent/includes/Makefile
webcontent/images/Makefile
webcontent/images/datepicker/Makefile
webcontent/images/easySlider/Makefile
webcontent/images/jquery-ui/Makefile
])
