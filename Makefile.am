## Process this file with automake to produce Makefile.in

SUBDIRS = src sql webcontent i18n 

opendiasdocdir = ${prefix}/doc/opendias
opendiasdoc_DATA = ChangeLog
EXTRA_DIST = $(opendiasdoc_DATA)

install-exec-hook:
	test ! -e ${VAR_DIR}/run && mkdir -p ${VAR_DIR}/run || echo Run directory already exists
	test ! -e ${VAR_DIR}/log/opendias && mkdir -p ${VAR_DIR}/log/opendias || echo Log directory already exists
	test ! -e ${libdir}/opendias && mkdir -p ${libdir}/opendias || echo Lib directory already exists
	test ! -e ${sysconfdir}/opendias && mkdir -p ${sysconfdir}/opendias || echo openDIAS config directory already exists
	test ! -e ${sysconfdir}/opendias/opendias.conf && echo ${libdir}/opendias >> ${sysconfdir}/opendias/opendias.conf || echo config file already exists
