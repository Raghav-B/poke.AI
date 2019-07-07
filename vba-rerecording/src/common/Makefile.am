noinst_LIBRARIES = libgbcom.a

libgbcom_a_SOURCES = \
	lua-engine.cpp	\
	memgzio.c		\
	memgzio.h		\
	movie.cpp		\
	movie.h			\
	System.cpp		\
	System.h		\
	SystemGlobals.cpp	\
	SystemGlobals.h		\
	Text.cpp		\
	Text.h			\
	unzip.cpp		\
	unzip.h			\
	Util.cpp		\
	Util.h			\
	vbalua.h

AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@
