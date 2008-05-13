dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(opendias, 0.1)

prefix=/home/wbooth/install
AC_SUBST(prefix)

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






PKG_CHECK_MODULES(OPENDIAS, [sqlite3 gtk+-2.0])
AC_SUBST(OPENDIAS_CFLAGS)
AC_SUBST(OPENDIAS_LIBS)
dnLOPENDIAS_LIBS="$OPENDIAS_LIBS -lsane "




AC_CHECK_LIB(tiff, TIFFOpen, [], 
		[echo "By not having tiff libs we cannot use tesseract. 
		By not having tesseract, you will not be able to use 
		the OCR functionality."])
AC_CHECK_LIB(tesseract_full, main, [], [echo "By not having tesseract 
		libs installed, you wll not be able to use the OCR 
		functionality."])
AC_CHECK_LIB(espeak, main, [], [echo "By not having espeak 
		libs installed, you wll not be able to use the read 
		aloud functionality."])
AC_CHECK_LIB(sane, main, [], [echo "By not having sane libs installed, 
		you wll not be able to scan in any document."])



AC_OUTPUT([
Makefile
src/Makefile
sql/Makefile
po/Makefile.in
])
