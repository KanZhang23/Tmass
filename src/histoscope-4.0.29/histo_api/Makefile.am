lib_LTLIBRARIES = libCHisto.la

libCHisto_la_SOURCES = HistoClient.c \
	histoApiItems.c histoApiFiles.c histoApiHists.c histoApiArytH.c \
	histoApiNTs.c histoApiStatH.c cBinding.c cApiBinding.c \
        histoApiFiles.h histoApiStatH.h \
        clientXDRsocket1.h  histoApiHists.h  HistoClient.h \
        clientXDRsocket.h   histoApiItems.h  histoVersion.h \
        histoApiArytH.h     histoApiNTs.h    

libCHisto_la_LIBADD = $(DEPS_LIBS) ../histo_util/libHistoUtil.la

INCLUDES = -I. $(DEPS_CFLAGS) $(MOTIF_CFLAGS) $(X11_CFLAGS)

include_HEADERS = histoscope.h
