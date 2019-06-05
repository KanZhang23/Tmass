bin_PROGRAMS = histo hsdir nplot

histo_SOURCES = histoMainProgram.c

hsdir_SOURCES = hsdir.c

nplot_SOURCES = nplot.c

INCLUDES = -I../histo -I../histo_util $(DEPS_CFLAGS) $(MOTIF_CFLAGS) $(X11_CFLAGS)

LDADD = ../histo/libHisto.la ../histo_util/libHistoUtil.la \
        ../plot_widgets/libPlotW.la ../util/libHistoUtil.la \
        $(DEPS_LIBS) $(MOTIF_LIBS) $(X11_LIBS) -lm
