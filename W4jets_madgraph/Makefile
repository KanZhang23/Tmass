#
# For installation, the code in this directory needs the NPStat package
# (with all of its dependencies) from http://npstat.hepforge.org, the
# RK package from http://rk.hepforge.org, and PYTHIA8 from
# http://home.thep.lu.se/~torbjorn/Pythia.html
#
# Adjust the definitions of the NPSTAT_LIBDIR, HISTO_LIBDIR, PYTHIA8_LIBDIR,
# and JET_USER_DIR variables just below.
#
NPSTAT_LIBDIR = /usr/local/lib
HISTO_LIBDIR = /usr/local/lib
PYTHIA8_LIBDIR = /home/igv/Code/pythia8/lib
JET_USER_DIR = /home/igv/TopJes/JetUser

TFOFILES = JetInfo.o JetPtTF.o JetPtEff.o HighEnergyTauTF.o MuonTauTF.o

OFILES = Parameters_sm_no_b_mass.o w4jetsKinematics.o misc_utils.o \
         integrateW4Jets.o JesIntegResult.o matrixElementWeight.o \
         getJESSigma.o WJetsTFSet.o storePartonPtPrior.o pdg2pdf.o

PROGRAMS = testJetPtTF.cc testKinematics.cc testIntegration.cc

OPTIMIZE = -std=c++11 -g
INCLUDES = -I. -I$(NPSTAT_LIBDIR)/../include -I$(HISTO_LIBDIR)/../include \
           -I$(JET_USER_DIR) -I$(PYTHIA8_LIBDIR)/../include
CPPFLAGS = $(OPTIMIZE) $(INCLUDES) -Wall -W -Werror

NOJETUSER = -L$(NPSTAT_LIBDIR) -L/usr/lib \
       -lnpstat -lkstest -lgeners -lfftw3 -llapack -lblas -lrk -lbz2 -lz -lm

LIBS = -L$(PYTHIA8_LIBDIR) -lpythia8 -L$(JET_USER_DIR) -lJetUser \
       -L$(HISTO_LIBDIR) -lCHisto $(NOJETUSER)

FLIBS = -L./wellnu01234j_5f_LO_MLM/lib -lME0 -lME1 -ldhelas -lpdf

%.o : %.cc
	g++ -c $(CPPFLAGS) -fPIC -MD $< -o $@
	@sed -i 's,\($*\.o\)[:]*\(.*\),$@: $$\(wildcard\2\)\n\1:\2,g' $*.d

BINARIES = $(PROGRAMS:.cc=)

all: matrixElement $(BINARIES)

matrixElement:
	(cd ./wellnu01234j_5f_LO_MLM/Source; make)
	(cd ./wellnu01234j_5f_LO_MLM/SubProcesses/P0_gg_wpggqq_wp_epve; make -f Makefile.opt)
	(cd ./wellnu01234j_5f_LO_MLM/SubProcesses/P1_gg_wmggqq_wm_emvex; make -f Makefile.opt)

libW4jetsTFs.so: $(TFOFILES)
	rm -f libW4jetsTFs.so
	g++ $(OPTIMIZE) -shared -fPIC -o $@ $(TFOFILES) $(NOJETUSER)

libW4jets.so: $(OFILES) libW4jetsTFs.so
	rm -f libW4jets.so
	g++ $(OPTIMIZE) -shared -fPIC -o $@ $(OFILES) \
            $(FLIBS) $(LIBS) -L. -lW4jetsTFs

$(BINARIES): % : %.o libW4jets.so; g++ $(OPTIMIZE) -fPIC -o $@ $< \
        -L. -lW4jets -lW4jetsTFs $(LIBS) $(FLIBS) -lgfortran

clean:
	rm -f $(BINARIES) $(FBINARIES) core.* *.so *.o *.d *~
	rm -f ./wellnu01234j_5f_LO_MLM/lib/*.a
	(cd ./wellnu01234j_5f_LO_MLM/Source; make clean)
	(cd ./wellnu01234j_5f_LO_MLM/SubProcesses/P0_gg_wpggqq_wp_epve; rm -f *.o)
	(cd ./wellnu01234j_5f_LO_MLM/SubProcesses/P1_gg_wmggqq_wm_emvex; rm -f *.o)

-include $(OFILES:.o=.d)
-include $(TFOFILES:.o=.d)
-include $(PROGRAMS:.cc=.d)
