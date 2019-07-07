noinst_LIBRARIES = libgba.a

libgba_a_SOURCES = \
	agbprint.cpp	\
	agbprint.h		\
	arm-new.h		\
	armdis.cpp		\
	armdis.h		\
	bios.cpp		\
	bios.h			\
	elf.cpp			\
	elf.h			\
	GBACheats.cpp	\
	GBACheats.h		\
	EEprom.cpp		\
	EEprom.h		\
	Flash.cpp		\
	Flash.h			\
	GBA.cpp			\
	GBA.h			\
	GBA-arm.h		\
	GBA-thumb.h	\
	GBACpu.h		\
	GBAGfx.cpp		\
	GBAGfx.h		\
	GBAGlobals.cpp	\
	GBAGlobals.h	\
	GBAinline.h		\
	GBAMemory.cpp	\
	GBASound.cpp	\
	GBASound.h		\
	Mode0.cpp		\
	Mode1.cpp		\
	Mode2.cpp		\
	Mode3.cpp		\
	Mode4.cpp		\
	Mode5.cpp		\
	remote.cpp		\
	RTC.cpp			\
	RTC.h			\
	Sram.cpp		\
	Sram.h			\

AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@
