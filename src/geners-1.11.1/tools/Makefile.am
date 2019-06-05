noinst_PROGRAMS = cdump gssa_dump gssa_merge cmerge crecover print_items \
                  template_io_boilerplate

cdump_SOURCES = cdump.cc

gssa_dump_SOURCES = gssa_dump.cc

gssa_merge_SOURCES = gssa_merge.cc

cmerge_SOURCES = cmerge.cc

crecover_SOURCES = crecover.cc

print_items_SOURCES = print_items.cc

template_io_boilerplate_SOURCES = template_io_boilerplate.cc

LDADD = -L../geners -lgeners $(DEPS_LIBS) $(BZIP2_LIBS) $(ZLIB_LIBS)

INCLUDES = -I@top_srcdir@/ -I. $(DEPS_CFLAGS) $(BZIP2_CFLAGS) $(ZLIB_CFLAGS)

TESTS = $(noinst_PROGRAMS)

EXTRA_DIST = CmdLine.hh
