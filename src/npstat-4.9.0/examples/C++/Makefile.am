noinst_PROGRAMS = lorpe_1d apply_lorpe_1d lorpe_multivariate \
                  n_poly_coeffs ems_unfold_1d dump_qmc

lorpe_1d_SOURCES = lorpe_1d.cc

apply_lorpe_1d_SOURCES = apply_lorpe_1d.cc

lorpe_multivariate_SOURCES = lorpe_multivariate.cc

n_poly_coeffs_SOURCES = n_poly_coeffs.cc

ems_unfold_1d_SOURCES = ems_unfold_1d.cc

dump_qmc_SOURCES = dump_qmc.cc

LDADD = -L../../npstat -lnpstat $(DEPS_LIBS) -llapack -lblas -lbz2 -lz $(FLIBS)

AM_CPPFLAGS = -I@top_srcdir@/ -I. $(DEPS_CFLAGS)

TESTS = $(noinst_PROGRAMS)

EXTRA_DIST = CmdLine.hh 00README.txt
