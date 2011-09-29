## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src sql webcontent 

opendiasdocdir = ${prefix}/doc/opendias

opendiasdoc_DATA = ChangeLog

EXTRA_DIST = $(opendiasdoc_DATA)

install-exec-hook:
	test ! -e /var/log/opendias && mkdir /var/log/opendias || echo Log directory already exists
	test ! -e /var/lib/opendias && mkdir /var/lib/opendias || echo Lib directory already exists
	test ! -e /etc/opendias && mkdir /etc/opendias || echo openDIAS config directory already exists
	test ! -e /etc/opendias/opendias.conf && echo /var/lib/opendias >> /etc/opendias/opendias.conf || echo config file already exists

## Copy all the spec files. Of cource, only one is actually used.
#dist-hook:
#	for specfile in *.spec; do \
#		if test -f $$specfile; then \
#			cp -p $$specfile $(distdir); \
#		fi \
#	done

