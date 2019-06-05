OFILES = w_reco_efficiency.o random_jet_angles.o random_jet_masses.o \
         random_lepton_momentum.o transfer_function.o \
         johnson_random.o sbshape.o topmass_utils.o tau_pt_spectrum.o \
         random_parton_mass.o poly_3_2d.o single_jet_probabilities.o \
         random_extra_jet.o tf_interpolator.o qqbar_deltaR_efficiency.o \
         ratio_tf_shift.o p0_interpolator.o

FOFILES = johnson.o

INCLUDES = -I..
OPTIMIZE = -O3

CFLAGS = -fPIC $(OPTIMIZE) $(INCLUDES) -Wall -Wmissing-prototypes -W -Werror \
         -Wno-unused-parameter -Wno-strict-aliasing
FFLAGS = -fPIC $(OPTIMIZE) -Wall -Wno-unused-dummy-argument

%.o : %.c
	gcc -c $(CFLAGS) -MD $< -o $@
	@sed -i 's,\($*\.o\)[:]*\(.*\),$@: $$\(wildcard\2\)\n\1:\2,g' $*.d
%.o : %.f
	gfortran -c $(FFLAGS) $< -o $@

all: libconfigurables.a

libconfigurables.a: $(OFILES) $(FOFILES)
	rm -f $@
	ar -rs $@ $(OFILES) $(FOFILES)

clean:
	rm -f *.o *.d *.a *.so *~

-include $(OFILES:.o=.d)
