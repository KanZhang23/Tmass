AM_CXXFLAGS = -O3
AM_CPPFLAGS = -I@top_srcdir@/

lib_LTLIBRARIES = libmcuncert.la

libmcuncert_la_SOURCES = PseudoMCUncertaintyCalculator.cc QMCUncertaintyCalculator.cc cone.cc

libmcuncert_la_LDFLAGS = -version-info 1:0:1

includedir = ${prefix}/include/mcuncert

include_HEADERS = AbsUncertaintyCalculator.hh PseudoMCUncertaintyCalculator.hh QMCUncertaintyCalculator.hh cone.hh
