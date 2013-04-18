dnl Process this file with autoconf to produce a configure script.

AC_INIT(opendias, [0.9.3b], [wbooth@clearscene.net])
AC_DEFINE([DB_VERSION],[7],[Setting DB version])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC



dnl AC_CHECK_FUNCS([strcspn],[],AC_MSG_ERROR([Missing function strcspn]))
dnl AC_CHECK_FUNCS([strndup],[],AC_MSG_ERROR([Missing function strndup]))
dnl AC_CHECK_FUNCS([strdup],[],AC_MSG_ERROR([Missing function strdup]))
dnl AC_CHECK_FUNCS([strstr],[],AC_MSG_ERROR([Missing function strstr]))
dnl AC_CHECK_FUNCS([memset],[],AC_MSG_ERROR([Missing function memset])) 
dnl AC_CHECK_FUNCS([mkdir],[],AC_MSG_ERROR([Missing function mkdir]))
dnl AC_CHECK_FUNCS([socket],[],AC_MSG_ERROR([Missing function socket]))
dnl AC_CHECK_FUNCS([strerror],[],AC_MSG_ERROR([Missing function strerror]))
dnl AC_CHECK_FUNCS([dup2],[],AC_MSG_ERROR([Missing function dup2]))
dnl AC_CHECK_FUNCS([gettimeofday],[],AC_MSG_ERROR([Missing function gettimeofday]))
dnl AC_CHECK_HEADERS([arpa/inet.h],[],AC_MSG_ERROR([Missing header arpa/inet.h]))
dnl AC_CHECK_HEADERS([fcntl.h],[],AC_MSG_ERROR([Missing header fcntl.h]))
dnl AC_CHECK_HEADERS([netdb.h],[],AC_MSG_ERROR([Missing header netdb.h]))
dnl AC_CHECK_HEADERS([netinet/in.h],[],AC_MSG_ERROR([Missing header netinet/in.h]))
dnl AC_CHECK_HEADERS([sys/param.h],[],AC_MSG_ERROR([Missing header sys/param.h]))
dnl AC_CHECK_HEADERS([sys/socket.h],[],AC_MSG_ERROR([Missing header sys/socket.h]))
dnl AC_CHECK_HEADERS([sys/time.h],[],AC_MSG_ERROR([Missing header sys/time.h]))
dnl AC_FUNC_FORK
dnl AC_FUNC_MALLOC
dnl AC_FUNC_REALLOC
dnl AC_FUNC_MKTIME
dnl AC_HEADER_STDBOOL
dnl AC_TYPE_PID_T
dnl AC_TYPE_SIZE_T
dnl AC_TYPE_SSIZE_T
dnl AC_TYPE_UINT64_T
dnl AC_TYPE_MODE_T
dnl AC_PROG_LN_S





dnl
dnl Ensure weve got everything we need to even start!
dnl

PKG_CHECK_MODULES(OPENDIAS, [sqlite3 libmicrohttpd uuid libssl])
AC_SUBST(OPENDIAS_CFLAGS)
AC_SUBST(OPENDIAS_LIBS)
AC_SUBST(VAR_DIR)
AC_PATH_PROG(PKG_CONFIG,pkg-config,no)




dnl 
dnl Special config options
dnl 

AC_ENABLE([werror],[OPENDIAS_CFLAGS="$OPENDIAS_CFLAGS -Werror"])
AC_ENABLE([open_to_all],[
  AC_DEFINE([OPEN_TO_ALL],[],[No login for users. The application is open to all.])
  LOCALISATION_FLAGS="$LOCALISATION_FLAGS OPEN_TO_ALL"
])
AC_ENABLE([create_test_language],[
  LOCALISATION_FLAGS="$LOCALISATION_FLAGS CREATE_TEST_LANGUAGE"
])
AC_ENABLE([thread_join],[
	AC_DEFINE([THREAD_JOIN],[],[The main process will wait for child threads to finish.])
])
AC_ARG_ENABLE([force_var_at_root],
  [AS_HELP_STRING([--enable-force_var_at_root],[Ignore --prefix for the placement of '/var' and put it at the systems root])],
  [VAR_DIR="/var"],
  [VAR_DIR="${localstatedir}"]
)
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


phash="no"
AC_LANG_PUSH([C++])
AC_CHECK_LIB(pHash, ph_hamming_distance, 
	[AC_CHECK_HEADER(pHash.h,
    phash="yes",
    AC_MSG_WARN(You are missing the development headers for the pHash libs))], 
	AC_MSG_WARN(You are missing the pHash libs) )
AC_LANG_POP([C++])




dnl
dnl Determin what service we can provide based on available libs
dnl


if test "x$imageprocessing" = "xyes" ; then
  OPENDIAS_LIBS="$OPENDIAS_LIBS -llept"
	AC_DEFINE([CAN_IMAGE],[],[Can manipulate images])

  if test "x$scanning" = "xyes" && test "x$imageprocessing" = "xyes" ; then
	  AC_DEFINE([CAN_SCAN],[],[Have everything that is needed for scanning documents])
  	OPENDIAS_LIBS="$OPENDIAS_LIBS -lsane"
  else
	  AC_MSG_WARN(You will not be able to scan in any document.)
  fi


  if test "x$ocr" = "xyes" && test "x$imageprocessing" = "xyes" ; then
   AC_DEFINE([CAN_OCR],[],[Have everything that is needed to OCR documents])
   LOCALISATION_FLAGS="$LOCALISATION_FLAGS CAN_OCR"
  else
   AC_MSG_WARN(You will not be able to OCR any document.)
  fi


  if test "x$phash" = "xyes" ; then
   AC_DEFINE([CAN_PHASH],[],[Have everything that is needed to PHASH documents])
   OPENDIAS_LIBS="$OPENDIAS_LIBS -lpHash"
   LOCALISATION_FLAGS="$LOCALISATION_FLAGS CAN_PHASH"
  else
   AC_MSG_WARN(You will not be able to compare document.)
  fi

else
  AC_MSG_WARN(You will not be able to scan, OCR or compare any document.)
fi


if test "x$magic" = "xyes" ; then

  AC_DEFINE([CAN_MAGIC],[],[Can decide on uploaded file type.])
	OPENDIAS_LIBS="$OPENDIAS_LIBS -lmagic"

  if test "x$xml" = "xyes" && test "x$zzip" = "xyes" ; then
  	AC_DEFINE([CAN_READODF],[],[Have everything that is needed to read ODF documents])
	  OPENDIAS_LIBS="$OPENDIAS_LIBS -lxml2 -lzzip"
  else
	  AC_MSG_WARN(You will not be able to import any ODF documents.)
  fi

  if test "x$pdf" = "xyes"; then
    AC_DEFINE([CAN_PDF],[],[Have everything that is needed to import PDF documents])
    OPENDIAS_LIBS="$OPENDIAS_LIBS -lpoppler-cpp"
  else
    AC_MSG_WARN(You will not be able to import any PDF documents.)
  fi

else
	AC_MSG_WARN(You will not be able to import in any document.)
fi

if test -f ${srcdir}/scripts/build-local-versions.pl; then
  perl ${srcdir}/scripts/build-local-versions.pl "$PACKAGE_STRING" $LOCALISATION_FLAGS
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
