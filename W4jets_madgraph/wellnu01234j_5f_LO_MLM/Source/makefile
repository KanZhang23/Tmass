# Definitions

LIBDIR= ../lib/
BINDIR= ../bin/
PDFDIR= ./PDF/

include make_opts

# Source files

PROCESS= hfill.o matrix.o myamp.o 

# DSAMPLE = dsample.o ranmar.o 

HBOOK = hfill.o hcurve.o hbook1.o hbook2.o

GENERIC = $(alfas_functions).o transpole.o \
           rw_routines.o kin_functions.o open_file.o  setrun.o \
          run_printout.o 

INCLUDEF= coupl.inc genps.inc hbook.inc DECAY/decay.inc psample.inc cluster.inc sudgrid.inc

BANNER  =  write_banner.o  rw_events.o ranmar.o  kin_functions.o open_file.o  rw_routines.o alfas_functions.o



# Locally compiled libraries

# LIBRARIES=$(LIBDIR)libdsample.$(libext) $(LIBDIR)libgeneric.$(libext)
LIBRARIES=$(LIBDIR)libgeneric.$(libext)

# Compile commands

all: $(LIBRARIES) $(LIBDIR)libdhelas.$(libext) $(LIBDIR)libpdf.$(libext) $(LIBDIR)libmodel.$(libext) $(LIBDIR)libblocks.$(libext) $(LIBDIR)libtools.$(libext) $(LIBDIR)libcernlib.$(libext) $(LIBDIR)libTF.$(libext)

# Libraries

# $(LIBDIR)libdsample.$(libext): $(DSAMPLE)
#	$(call CREATELIB, $@, $^)
$(LIBDIR)libgeneric.$(libext): $(GENERIC)
	$(call CREATELIB, $@, $^)
$(LIBDIR)libdhelas.$(libext): DHELAS
	cd DHELAS; make
$(LIBDIR)libpdf.$(libext): PDF
	cd PDF; make	
$(LIBDIR)libcernlib.$(libext): CERNLIB
	cd CERNLIB; make	
$(LIBDIR)libmodel.$(libext): MODEL
	cd MODEL; make	
$(LIBDIR)libblocks.$(libext): MadWeight/blocks
	cd MadWeight/blocks; make	
$(LIBDIR)libtools.$(libext): MadWeight/tools
	cd MadWeight/tools; make	
$(LIBDIR)libTF.$(libext): MadWeight/transfer_function/transfer_function.o MadWeight/transfer_function/nb_tf.inc 
	cd MadWeight/transfer_function; make

# Binaries

$(BINDIR)sum_html: sum_html.o 
	$(FC) $(FFLAGS) -o $@ $^
$(BINDIR)gen_ximprove: gen_ximprove.o ranmar.o rw_routines.o open_file.o
	$(FC) $(FFLAGS) -o $@ $^
$(BINDIR)combine_events: $(COMBINE) $(LIBDIR)libmodel.$(libext) $(LIBDIR)libpdf.$(libext)
	$(FC) $(FFLAGS) -o $@ $(COMBINE) -L$(LIBDIR) -lmodel -lpdf $(lhapdf)
$(BINDIR)gensudgrid: $(GENSUDGRID) $(LIBDIR)libpdf.$(libext) $(LIBDIR)libcernlib.$(libext)
	$(FC) $(FFLAGS) -o $@ $(GENSUDGRID) -L$(LIBDIR) -lmodel -lpdf -lcernlib $(lhapdf)
$(BINDIR)combine_runs: combine_runs.o rw_events.o
	$(FC) $(FFLAGS) -o $@ $^


# Dependencies

setrun.o: setrun.f nexternal.inc leshouche.inc

clean:
	$(RM) *.o $(LIBRARIES)
	cd PDF; make clean; cd ..
	cd DHELAS; make clean; cd ..
	cd CERNLIB; make clean; cd ..
	cd MODEL; make clean; cd ..
