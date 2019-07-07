bin_PROGRAMS = VisualBoyAdvance

noinst_PROGRAMS = TestEmu

VisualBoyAdvance_SOURCES = \
	SDL.cpp			\
	debugger.cpp		\
	debugger.h		\
	expr-lex.cpp		\
	expr.cpp		\
	expr.cpp.h		\
	exprNode.cpp		\
	exprNode.h		\
	getopt.c		\
	getopt.h		\
	getopt1.c		\
	../AutoBuild.h		\
	../NLS.h		\
	../Port.h

VisualBoyAdvance_LDADD = @VBA_LIBS@ @SDL_LIBS@

VisualBoyAdvance_DEPENDENCIES = @VBA_LIBS@

TestEmu_SOURCES = \
	TestEmu.cpp		\
	debugger.cpp		\
	debugger.h		\
	expr-lex.cpp		\
	expr.cpp		\
	expr.cpp.h		\
	exprNode.cpp		\
	exprNode.h		\
	../AutoBuild.h		\
	../NLS.h		\
	../Port.h

TestEmu_LDADD = @VBA_LIBS@ @SDL_LIBS@

TestEmu_DEPENDENCIES = @VBA_LIBS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@
