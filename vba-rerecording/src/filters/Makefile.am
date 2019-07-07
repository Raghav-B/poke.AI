SUFFIXES = .asm

noinst_LIBRARIES = lib386.a libfilter.a

lib386_a_SOURCES = 2xSaImmx.asm

.asm.o:
	$(NASM) -f elf -o $@ $<

libfilter_a_SOURCES = \
	2xSaI.cpp		\
	admame.cpp		\
	bilinear.cpp		\
	hq2x.cpp		\
	hq2x.h			\
	interframe.cpp		\
	interp.h		\
	lq2x.h			\
	motionblur.cpp		\
	pixel.cpp		\
	scanline.cpp		\
	simple2x.cpp
