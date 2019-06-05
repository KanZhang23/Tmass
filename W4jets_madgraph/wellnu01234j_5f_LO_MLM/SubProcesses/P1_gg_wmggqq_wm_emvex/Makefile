include ../../Source/make_opts
FFLAGS+= -w

#gfortran   = g77
#FFLAGS= -O -ffixed-line-length-132
FFLAGS = -g -ffixed-line-length-132 -fPIC
LIBDIR = ../../lib/
#LINKLIBS =  -L../../lib/ -ldhelas3 -lmodel -lgeneric_mw -lcernlib -lpdf -lblocks -ltools -lTF 
#LIBS = $(LIBDIR)libdhelas3.a $(LIBDIR)libgeneric_mw.a $(LIBDIR)libpdf.a $(LIBDIR)libmodel.a $(LIBDIR)libcernlib.a $(LIBDIR)libblocks.a $(LIBDIR)libtools.a $(LIBDIR)libTF.a 
 
OBJS=   $(patsubst %.f,%.o,$(wildcard pauto_dsig*.f)) \
	$(patsubst %.f,%.o,$(wildcard pmatrix*.f))

$(LIBDIR)libME1.$(libext): $(OBJS)
	$(call CREATELIB, $@, $^)

