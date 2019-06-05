# PYTHIA configuration file.
# Generated on Wed Mar 27 14:24:44 CDT 2019 with the user supplied options:
# --enable-64bit
# --enable-shared
# --with-lhapdf6=/cdf/home/kanzhang/Topmass_SL6
# --with-python-include=/usr/include/python2.6

# Install directory prefixes.
PREFIX_BIN=/cdf/home/kanzhang/Topmass_SL6/pythia8240/bin
PREFIX_INCLUDE=/cdf/home/kanzhang/Topmass_SL6/pythia8240/include
PREFIX_LIB=/cdf/home/kanzhang/Topmass_SL6/pythia8240/lib
PREFIX_SHARE=/cdf/home/kanzhang/Topmass_SL6/pythia8240/share/Pythia8

# Compilation flags (see ./configure --help for further documentation).
ENABLE_SHARED=true
CXX=g++
CXX_COMMON=-O2 -m64  -pedantic -W -Wall -Wshadow -fPIC
CXX_SHARED=-shared
CXX_SONAME=-Wl,-soname,
LIB_SUFFIX=.so

# EVTGEN configuration.
EVTGEN_USE=false
EVTGEN_BIN=
EVTGEN_INCLUDE=
EVTGEN_LIB=

# FASTJET3 configuration.
FASTJET3_USE=false
FASTJET3_BIN=
FASTJET3_INCLUDE=
FASTJET3_LIB=

# HEPMC2 configuration.
HEPMC2_USE=false
HEPMC2_BIN=
HEPMC2_INCLUDE=
HEPMC2_LIB=

# HEPMC3 configuration.
HEPMC3_USE=false
HEPMC3_BIN=
HEPMC3_INCLUDE=
HEPMC3_LIB=

# LHAPDF5 configuration.
LHAPDF5_USE=false
LHAPDF5_BIN=
LHAPDF5_INCLUDE=
LHAPDF5_LIB=

# LHAPDF6 configuration.
LHAPDF6_USE=true
LHAPDF6_BIN=/cdf/home/kanzhang/Topmass_SL6/bin
LHAPDF6_INCLUDE=/cdf/home/kanzhang/Topmass_SL6/include
LHAPDF6_LIB=/cdf/home/kanzhang/Topmass_SL6/lib

# POWHEG configuration.
POWHEG_USE=false
POWHEG_BIN=
POWHEG_INCLUDE=
POWHEG_LIB=

# PROMC configuration.
PROMC_USE=false
PROMC_BIN=
PROMC_INCLUDE=
PROMC_LIB=

# ROOT configuration.
ROOT_USE=false
ROOT_BIN=
ROOT_LIBS=
CXX_ROOT=

# GZIP configuration.
GZIP_USE=false
GZIP_BIN=
GZIP_INCLUDE=
GZIP_LIB=

# BOOST configuration.
BOOST_USE=false
BOOST_BIN=
BOOST_INCLUDE=
BOOST_LIB=

# PYTHON configuration.
PYTHON_USE=true
PYTHON_BIN=
PYTHON_INCLUDE=/usr/include/python2.6
PYTHON_LIB=./
