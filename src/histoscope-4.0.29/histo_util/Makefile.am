noinst_LTLIBRARIES = libHistoUtil.la

libHistoUtil_la_SOURCES = publish.c histoUtil.c \
	     xdrHisto.c hsFile.c getHbookData.c readHbookList.c \
             nfitParam.h   histprotocol.h  publish.h  xdrHisto.h \
             histoUtil.h  getHbookData.h readHbookList.h

INCLUDES = -I. $(DEPS_CFLAGS) $(MOTIF_CFLAGS) $(X11_CFLAGS)

include_HEADERS = hsTypes.h hsFile.h
