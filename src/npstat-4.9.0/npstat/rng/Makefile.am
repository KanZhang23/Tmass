AM_CPPFLAGS = -I@top_srcdir@/ $(DEPS_CFLAGS)

noinst_LTLIBRARIES = librng.la

librng_la_SOURCES = permutation.cc RandomSequenceRepeater.cc \
    convertToSphericalRandom.cc SobolGenerator.cc MersenneTwister.cc \
    HOSobolGenerator.cc RegularSampler1D.cc EquidistantSampler1D.cc

includedir = ${prefix}/include/npstat/rng

include_HEADERS = AbsRandomGenerator.hh \
    convertToSphericalRandom.hh \
    CPP11RandomGen.hh \
    EquidistantSampler1D.hh \
    HOSobolGenerator.hh \
    MersenneTwister.hh \
    permutation.hh \
    RandomSequenceRepeater.hh \
    RegularSampler1D.hh \
    SobolGenerator.hh

EXTRA_DIST = MersenneTwisterImpl.hh 00README.txt
