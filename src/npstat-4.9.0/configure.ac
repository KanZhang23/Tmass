AC_PREREQ([2.59])

AC_INIT([npstat], [4.9.0])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_MACRO_DIR([m4])

dnl Make sure FFTW and Geners are installed
PKG_CHECK_MODULES([DEPS], [fftw3 >= 3.1.2 geners >= 1.3.0 kstest >= 2.0.0])

AC_PROG_CXX

dnl Options needed to include fortran libraries
dnl Certain parts of this package rely upon LAPACK and BLAS
AC_F77_LIBRARY_LDFLAGS

AC_PROG_INSTALL

dnl Set the default destination directory
AC_PREFIX_DEFAULT([/usr/local])

dnl Enable libtool
AC_PROG_LIBTOOL

AC_OUTPUT([Makefile npstat/nm/Makefile npstat/rng/Makefile npstat/stat/Makefile npstat/wrap/Makefile npstat/interfaces/Makefile npstat/emsunfold/Makefile npstat/Makefile examples/C++/Makefile npstat.pc])
