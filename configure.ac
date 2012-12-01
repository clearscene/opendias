dnl Process this file with autoconf to produce a configure script.

AC_INIT(opendias, [0.9a], [wbooth@clearscene.net])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC



AC_CHECK_FUNCS([strcspn],[],AC_MSG_ERROR([Missing function strcspn]))
AC_CHECK_FUNCS([strndup],[],AC_MSG_ERROR([Missing function strndup]))
AC_CHECK_FUNCS([strdup],[],AC_MSG_ERROR([Missing function strdup]))
AC_CHECK_FUNCS([strstr],[],AC_MSG_ERROR([Missing function strstr]))
AC_CHECK_FUNCS([memset],[],AC_MSG_ERROR([Missing function memset])) 
AC_CHECK_FUNCS([mkdir],[],AC_MSG_ERROR([Missing function mkdir]))
AC_CHECK_FUNCS([socket],[],AC_MSG_ERROR([Missing function socket]))
AC_CHECK_FUNCS([strerror],[],AC_MSG_ERROR([Missing function strerror]))
AC_CHECK_FUNCS([dup2],[],AC_MSG_ERROR([Missing header dup2]))
AC_CHECK_HEADERS([arpa/inet.h],[],AC_MSG_ERROR([Missing header arpa/inet.h]))
AC_CHECK_HEADERS([fcntl.h],[],AC_MSG_ERROR([Missing header fcntl.h]))
AC_CHECK_HEADERS([netdb.h],[],AC_MSG_ERROR([Missing header netdb.h]))
AC_CHECK_HEADERS([netinet/in.h],[],AC_MSG_ERROR([Missing header netinet/in.h]))
AC_CHECK_HEADERS([sys/param.h],[],AC_MSG_ERROR([Missing header sys/param.h]))
AC_CHECK_HEADERS([sys/socket.h],[],AC_MSG_ERROR([Missing header sys/socket.h]))
AC_CHECK_HEADERS([sys/time.h],[],AC_MSG_ERROR([Missing header sys/time.h]))
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_MKTIME
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T
AC_TYPE_MODE_T






dnl
dnl Ensure weve got everything we need to even start!
dnl

PKG_CHECK_MODULES(OPENDIAS, [sqlite3 libmicrohttpd uuid libssl])
AC_SUBST(OPENDIAS_CFLAGS)
AC_SUBST(OPENDIAS_LIBS)
AC_PATH_PROG(PKG_CONFIG,pkg-config,no)




dnl 
dnl Special config options
dnl 

AC_ENABLE([werror],[OPENDIAS_CFLAGS="$OPENDIAS_CFLAGS -Werror"])
AC_ENABLE([thread_join],[
	AC_DEFINE([THREAD_JOIN],[],[The main process will wait for child threads to finish.])
])
OPENDIAS_LIBS="$OPENDIAS_LIBS -lpthread "




dnl 
dnl Find out what we've got to work with
dnl

imageprocessing="no"
AC_CHECK_LIB(lept, main, 
	[AC_CHECK_HEADER(leptonica/allheaders.h,
    imageprocessing="yes",
    AC_MSG_WARN(You are missing the development headers for the leptonica libs))], 
	AC_MSG_WARN(You are missing the leptonica libs) )


scanning="no"
AC_CHECK_LIB(sane, main, 
  [AC_CHECK_HEADER(sane/sane.h,
    scanning="yes",
    AC_MSG_WARN(You are missing the development headers for the sane libs))], 
  AC_MSG_WARN(You are missing the sane libs) )


ocr="no"
AC_LANG_PUSH([C++])
AC_SEARCH_LIBS([_ZN9tesseract11TessBaseAPI8SetImageEPK3Pix], [tesseract tesseract_api],
  [AC_CHECK_HEADER(tesseract/baseapi.h,
    ocr="yes"
    AC_SEARCH_LIBS([_ZNK9tesseract11TessBaseAPI24GetInitLanguagesAsStringEv], [tesseract tesseract_api],
      AC_DEFINE([EXTENDED_OCR],[],[Have everything that is needed to OCR documents])),
    AC_MSG_WARN(You are missing the development headers for the tesseract v3 libs))],
  AC_MSG_WARN(You are missing the tesseract v3 libs) )  
AC_LANG_POP([C++])


pdf="no"
AC_LANG_PUSH([C++])
AC_CHECK_LIB(poppler-cpp, _ZN7poppler8document14load_from_fileERKSsS2_S2_, 
  [AC_CHECK_HEADER(poppler/cpp/poppler-document.h,
    pdf="yes",
    AC_MSG_WARN(You are missing the development headers for the poppler libs))], 
  AC_MSG_WARN(You are missing the poppler libs) )
AC_LANG_POP([C++])


zzip="no"
AC_CHECK_LIB(zzipwrap, main, 
  [AC_CHECK_HEADER(zzip/lib.h,
    zzip="yes",
    AC_MSG_WARN(You are missing the development headers for the zzip libs))], 
  AC_MSG_WARN(You are missing the zzip libs) )


xml="no"
AC_CHECK_LIB(xml2, xmlParseExternalEntity, 
  [AC_PATH_PROG(XML2_CONFIG,xml2-config,no)
  if test "$XML2_CONFIG" != "no"; then
    xml2_cflags=`$XML2_CONFIG --cflags`
    CPPFLAGS="$CPPFLAGS $xml2_cflags"
  fi
  AC_CHECK_HEADER(libxml/parser.h,
    xml="yes",
    AC_MSG_WARN(You are missing the development headers for the xml libs))], 
  AC_MSG_WARN(You are missing the xml libs) )


magic="no"
AC_CHECK_LIB(magic, magic_open, 
  [AC_CHECK_HEADER(magic.h,
    magic="yes",
    AC_MSG_WARN(You are missing the development headers for the magic libs))], 
  AC_MSG_WARN(You are missing the magic libs) )




dnl
dnl Determin what service we can provide based on available libs
dnl


if test "x$scanning" = "xyes" && test "x$imageprocessing" = "xyes" ; then
	AC_DEFINE([CAN_SCAN],[],[Have everything that is needed for scanning documents])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lsane -llept"
else
	AC_MSG_WARN(You wll not be able to scan in any document.)
fi


if test "x$ocr" = "xyes" && test "x$imageprocessing" = "xyes" ; then
 AC_DEFINE([CAN_OCR],[],[Have everything that is needed to OCR documents])
 OPENDIAS_LIBS="$OPENDIAS_LIBS -llept"
else
 AC_MSG_WARN(You wll not be able to OCR any document.)
fi


if test "x$magic" = "xyes" ; then

  AC_DEFINE([CAN_MAGIC],[],[Can decide on uploaded file type.])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lmagic"

  if test "x$xml" = "xyes" && test "x$zzip" = "xyes" ; then
  	AC_DEFINE([CAN_READODF],[],[Have everything that is needed to read ODF documents])
	  OPENDIAS_LIBS="$OPENDIAS_LIBS -lxml2 -lzzip"
  else
	  AC_MSG_WARN(You wll not be able to import any ODF documents.)
  fi

  if test "x$pdf" = "xyes"; then
    AC_DEFINE([CAN_PDF],[],[Have everything that is needed to import PDF documents])
    OPENDIAS_LIBS="$OPENDIAS_LIBS -lpoppler-cpp"
  else
    AC_MSG_WARN(You wll not be able to import any PDF documents.)
  fi

else
	AC_MSG_WARN(You wll not be able to import in any document.)
fi





dnl
dnl Write what we've learnt
dnl

AC_OUTPUT([
Makefile
src/Makefile
sql/Makefile
i18n/Makefile
webcontent/Makefile
webcontent/style/Makefile
webcontent/includes/Makefile
webcontent/includes/local/Makefile
webcontent/images/Makefile
webcontent/images/datepicker/Makefile
webcontent/images/easySlider/Makefile
webcontent/images/jquery-ui/Makefile
])
