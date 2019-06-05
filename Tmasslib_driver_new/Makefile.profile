#
# Where Tcl libraries are stored?
#
TCL_LIB_DIR := $(shell echo puts [file dirname [info library]] | tclsh)
# include $(TCL_LIB_DIR)/tclConfig.sh
include /usr/lib64/tclConfig.sh

HISTO_DIR = /usr/local
TOPMASS_DIR = /home/igv/TopJes/Scripts/Tmasslib
JET_USER_DIR = /home/igv/TopJes/JetUser

OFILES = data_integ_mtm3.o parameter_parser.o

PROGRAMS = mtm3sh.c

INCLUDES = -I$(TCL_LIB_DIR)/../include -I$(TOPMASS_DIR) -I$(JET_USER_DIR) \
           -I$(HISTO_DIR)/include -I.

DEFS = -DUSE_TCL_STUBS
OPTIMIZE = -g -pg -O3
CFLAGS = $(OPTIMIZE) $(TCL_SHLIB_CFLAGS) $(INCLUDES) $(DEFS) \
         -Wall -Wmissing-prototypes -W -Werror -Wno-unused-parameter \
         -Wno-strict-aliasing -Wno-unused-but-set-variable

LIBS = -L$(TOPMASS_DIR) -ltopmass \
       -L$(TOPMASS_DIR)/Configurables -lconfigurables \
       -L/usr/share/tcl8.6/hs2.15 -lhs2.15 \
       -L$(HISTO_DIR)/lib -lCHisto \
       -L$(JET_USER_DIR) -lJetUser \
       -L/usr/lib64 -lfftw3 $(TCL_STUB_LIB_FLAG) -ltcl -lstdc++ -ldl -lm

FLIBS = -L/home/igv/Cernlib/2005/lib -lpdflib804 -lkernlib -lmathlib \
        -lpacklib -llapack -lblas -lgfortran

%.o : %.c
	gcc -c $(CFLAGS) -MD $< -o $@
	@sed -i 's,\($*\.o\)[:]*\(.*\),$@: $$\(wildcard\2\)\n\1:\2,g' $*.d

BINARIES = $(PROGRAMS:.c=)

all: $(BINARIES)

$(BINARIES): % : %.o $(OFILES); gcc $(OPTIMIZE) -fPIC -o $@ $^ $(LIBS) $(FLIBS)

clean:
	rm -f $(BINARIES) *.o *.d *.a *.so *~

-include $(PROGRAMS:.c=.d)
