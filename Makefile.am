## Process this file with automake to produce Makefile.in

SUBDIRS = src sql webcontent i18n 

opendiasdocdir = ${prefix}/doc/opendias
opendiasdoc_DATA = ChangeLog
EXTRA_DIST = $(opendiasdoc_DATA)

install-exec-hook:
	test ! -e ${localstatedir}/run && mkdir -p ${localstatedir}/run || echo Run directory already exists
	test ! -e ${localstatedir}/log/opendias && mkdir -p ${localstatedir}/log/opendias || echo Log directory already exists
	test ! -e ${libdir}/opendias && mkdir -p ${libdir}/opendias || echo Lib directory already exists
	test ! -e ${sysconfdir}/opendias && mkdir -p ${sysconfdir}/opendias || echo openDIAS config directory already exists
	test ! -e ${sysconfdir}/opendias/opendias.conf && echo ${libdir}/opendias >> ${sysconfdir}/opendias/opendias.conf || echo config file already exists
	perl -pi -e 's|__libdir__|$(libdir)|g' $(pkgdatadir)/openDIAS.sqlite3.dmp.*.sql
	perl -pi -e 's|__localstatedir__|$(localstatedir)|g' $(pkgdatadir)/openDIAS.sqlite3.dmp.*.sql
	perl -pi -e 's|__package_string__|$(PACKAGE_STRING)|g' $(pkgdatadir)/webcontent/includes/header.txt.*
