#
# Where Tcl libraries are stored?
#
TCL_LIB_DIR := $(shell echo puts [file dirname [info library]] | tclsh)
# include $(TCL_LIB_DIR)/tclConfig.sh
include /usr/lib64/tclConfig.sh

#
# Where is the CERN Program Library?
#
CERN_LIB_DIR = /home/igv/Cernlib/2005/lib

#
# Where is the CDF jet corrections code?
#
JET_CORR_DIR = /home/igv/TopJes/JetUser

#
# Where is various other local code (Histo-Scope, etc)
#
LOCAL_BASE = /usr/local


COFILES = jet_info.o simple_kinematics.o ordered_tree.o ordered_tree_api.o \
         solve_top.o ttbar_phase_space.o sobol.o scrambled_sobol.o n_d_mask.o \
         matrel_mahlon_parke.o wgrid.o gaussgrid.o \
         p_struct_function.o topmass_integrator.o parameter_grid.o \
         quartic_lib.o convolute_breit_wigner.o permutations.o \
         matrel_kleiss_stirling.o matrel_interface.o n_d_random.o \
         range_lookup.o linear_interpolator_2d.o norm_accumulator.o \
         linear_interpolator_1d.o best_leptonic_nuz.o dpolin.o std.o \
         hadside_error_matrix.o leptonic_side_errors.o \
         sym_eigensys.o tag_probability_weight.o \
         range_lookup_flt.o randomize_jet_energy.o \
         topmass_norm.o twidth.o bw_polynomial.o \
         mc_topmass_integrator.o mc_integrator_priors.o unused.o rpoly.o \
         single_jet_probs.o halton.o generate_ttbar_pt.o sobol_f.o \
         ellipse_intersection.o leptonic_side_mask.o \
         linear_interpolator_nd.o linear_interpolator_nd_api.o \
         single_parton_efficiency.o cdf_2d.o cdf_2d_api.o \
         ordered_tree_collection.o cdf_3d.o cdf_3d_api.o parton_syserr.o \
         solve_top_api.o solve_for_jes.o tfs_2015.o

FOFILES = matrel.o pdflib_interface.o

INCLUDES = -I$(TCL_LIB_DIR)/../include -I$(LOCAL_BASE)/include \
           -I$(JET_CORR_DIR) -I.
DEFS = -DUSE_TCL_STUBS
OPTIMIZE = -g -pg -O3
CFLAGS = $(OPTIMIZE) $(TCL_SHLIB_CFLAGS) $(INCLUDES) $(DEFS) \
         -Wall -Wmissing-prototypes -W -Werror -Wno-unused-parameter \
         -Wno-strict-aliasing
FFLAGS = $(OPTIMIZE) $(TCL_SHLIB_CFLAGS) -Wall

%.o : %.c
	gcc -c $(CFLAGS) -MD $< -o $@
	@sed -i 's,\($*\.o\)[:]*\(.*\),$@: $$\(wildcard\2\)\n\1:\2,g' $*.d
%.o : %.f
	gfortran -c $(FFLAGS) $< -o $@

all: configurables libtopmass.a

configurables:
	(cd Configurables; make -f Makefile.profile)

libtopmass.a: $(FOFILES) $(COFILES)
	rm -f libtopmass.a
	ar -rs $@ $(COFILES) $(FOFILES)

clean:
	(cd Configurables; make -f Makefile.profile clean)
	rm -f *.o *.d *.a *.so *~

-include $(COFILES:.o=.d)
