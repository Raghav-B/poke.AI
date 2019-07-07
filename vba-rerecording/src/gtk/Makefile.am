SUBDIRS = images

bin_PROGRAMS = gvba

noinst_LIBRARIES = libgvba.a

gvba_SOURCES = \
	configfile.cpp		\
	configfile.h		\
	filters.cpp		\
	filters.h		\
	input.cpp		\
	input.h			\
	intl.h			\
	joypadconfig.cpp	\
	joypadconfig.h		\
	main.cpp		\
	menuitem.h		\
	screenarea.cpp		\
	screenarea.h		\
	sigccompat.h		\
	system.cpp		\
	tools.cpp		\
	tools.h			\
	windowcallbacks.cpp	\
	window.cpp		\
	window.h

gvba_LDADD = libgvba.a @VBA_LIBS@ @GTKMM_LIBS@ @LIBINTL@ @SDL_LIBS@

gvba_DEPENDENCIES = libgvba.a @VBA_LIBS@

gvba_CPPFLAGS = \
	-DPKGDATADIR=\"$(pkgdatadir)\"			\
	-DLOCALEDIR=\"$(datadir)/locale\"		\
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\"	\
	$(GTKMM_CPPFLAGS)

gvba_CXXFLAGS = @GTKMM_CFLAGS@ @SDL_CFLAGS@

libgvba_a_SOURCES = \
	../AutoBuild.h		\
	../NLS.h		\
	../Port.h

libgvba_a_CPPFLAGS = -DSDL

libgvba_a_CXXFLAGS = -fno-exceptions

dist_pkgdata_DATA = vba.glade
