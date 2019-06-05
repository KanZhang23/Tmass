CXX = g++ -std=c++11 -W -Wall

INCLUDES = -I.

OFILES = JetCorrSystematics.o JetEnergyCorrections.o JetCorrectionsInterface.o \
         interpolateCubicWithDerivatives.o

CXXFLAGS = -g -pg -O3 -fPIC $(INCLUDES) -D__NO_CDFSOFT__

%.o : %.cc
	$(CXX) -c $(CXXFLAGS) -MD $< -o $@
	@sed -i 's,\($*\.o\)[:]*\(.*\),$@: $$\(wildcard\2\)\n\1:\2,g' $*.d

all: libJetUser.a

libJetUser.a: $(OFILES)
	rm -f libJetUser.a
	ar -rs $@ $(OFILES)

clean:
	rm -f *.o *~ *.a *.so *.d

-include $(OFILES:.o=.d)
