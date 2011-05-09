## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src sql webcontent 

opendiasdocdir = ${prefix}/doc/opendias

opendiasdoc_DATA = ChangeLog

EXTRA_DIST = $(opendiasdoc_DATA)

install-exec-hook:
        test ! -e /var/log/opendias && mkdir /var/log/opendias
        test ! -e /var/lib/opendias && mkdir /var/lib/opendias
        test ! -e /etc/opendias && mkdir /etc/opendias
        test ! -e /etc/opendias/opendias.conf && echo /var/lib/opendias >> /etc/opendias/opendias.conf

## Copy all the spec files. Of cource, only one is actually used.
#dist-hook:
#	for specfile in *.spec; do \
#		if test -f $$specfile; then \
#			cp -p $$specfile $(distdir); \
#		fi \
#	done

