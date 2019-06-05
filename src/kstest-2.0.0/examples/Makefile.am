noinst_PROGRAMS = kscalc

kscalc_SOURCES = kscalc.cc

LDADD = -L../kstest -lkstest $(DEPS_LIBS) -lm

AM_CPPFLAGS = -I@top_srcdir@/ -I. $(DEPS_CFLAGS)

TESTS = $(noinst_PROGRAMS)

EXTRA_DIST = CmdLine.hh
