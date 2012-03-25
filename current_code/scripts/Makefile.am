
BUILT_SOURCES = openDIAS
CLEANFILES = openDIAS

sh_SCRIPTS = openDIAS

#sh_DATA = \
#	openDIAS

shdir = \
	$(sysconfdir)/init.d/

EXTRA_DIST = $(sh_SCRIPTS)

openDIAS:
	cat openDIAS.rc |sed "s#TARGET_BINDIR#$(bindir)#"  | \
	sed "s#TARGET_PIDFILE#$(localstatedir)/run/opendias/opendias.pid#" | \
	sed "s#TARGET_SYSCONFDIR#$(sysconfdir)#" > openDIAS

## File created by Steve

