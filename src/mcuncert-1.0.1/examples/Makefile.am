noinst_PROGRAMS = example1_qmc example1_pseudomc

example1_qmc_SOURCES = example1_qmc.cc

example1_pseudomc_SOURCES = example1_pseudomc.cc

LDADD = -L../mcuncert -lmcuncert $(DEPS_LIBS) -lm

AM_CPPFLAGS = -I@top_srcdir@/ -I. $(DEPS_CFLAGS)

TESTS = $(noinst_PROGRAMS)

EXTRA_DIST = CmdLine.hh
