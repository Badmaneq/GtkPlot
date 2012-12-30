## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

gtkplotdocdir = ${prefix}/doc/gtkplot
gtkplotdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


EXTRA_DIST = $(gtkplotdoc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(gtkplotdocdir)
