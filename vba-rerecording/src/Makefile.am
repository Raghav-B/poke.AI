CORE_SUBDIRS = gba gb common filters

EXTRA_SUBDIRS = prof sdl gtk lua

SUBDIRS = $(CORE_SUBDIRS) @VBA_SRC_EXTRA@

DIST_SUBDIRS = $(CORE_SUBDIRS) $(EXTRA_SUBDIRS)

dist_sysconf_DATA = VisualBoyAdvance.cfg

EXTRA_DIST = \
	win32

dist-hook:
	rm -rf `find $(distdir)/win32 -name CVS`
