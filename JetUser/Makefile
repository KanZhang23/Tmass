CXX = g++ -std=c++11 -W -Wall

INCLUDES = -I.

OFILES = JetCorrSystematics.o JetEnergyCorrections.o JetCorrectionsInterface.o \
         interpolateCubicWithDerivatives.o

PROGRAMS = solveEquation.cc

CXXFLAGS = -O3 -fPIC $(INCLUDES) -D__NO_CDFSOFT__

%.o : %.cc
	$(CXX) -c $(CXXFLAGS) -MD $< -o $@
	@sed -i 's,\($*\.o\)[:]*\(.*\),$@: $$\(wildcard\2\)\n\1:\2,g' $*.d

BINARIES = $(PROGRAMS:.cc=)

all: $(BINARIES)

$(BINARIES): libJetUser.so

$(BINARIES): % : %.o; g++ $(OPTIMIZE) -fPIC -o $@ $^ -L. -lJetUser

libJetUser.so: $(OFILES)
	rm -f libJetUser.so
	$(CXX) $(CXXFLAGS) -shared -o $@ $(OFILES)

clean:
	rm -f $(BINARIES) *.o *~ *.a *.so *.d

-include $(OFILES:.o=.d)
-include $(PROGRAMS:.cc=.d)
