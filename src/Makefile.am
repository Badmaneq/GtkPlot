## Process this file with automake to produce Makefile.in

## Created by Anjuta



AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GTKPLOT_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = gtkplot

gtkplot_SOURCES = \
	main.c \
	gtk-plot.c \
	gtk-plot.h

gtkplot_LDFLAGS = \
	-Wl,--export-dynamic

gtkplot_LDADD = $(GTKPLOT_LIBS) -lm


