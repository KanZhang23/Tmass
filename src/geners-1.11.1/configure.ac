AC_PREREQ([2.59])

AC_INIT([geners], [1.11.1])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_MACRO_DIR([m4])

dnl enable libtool
LT_INIT
AC_PROG_CXX

AC_PROG_INSTALL

dnl set the default destination directory
AC_PREFIX_DEFAULT([/usr/local])

dnl figure out where is bzip2 (this package does not use pkg-config)
AC_ARG_WITH([bzip2-include-path],
  [AS_HELP_STRING([--with-bzip2-include-path],
    [location of the bzip2 header file bzlib.h [default=/usr/include]])],
  [BZIP2_CFLAGS="-I$withval"],
  [BZIP2_CFLAGS='-I/usr/include'])
AC_SUBST([BZIP2_CFLAGS])

AC_ARG_WITH([bzip2-lib-path],
  [AS_HELP_STRING([--with-bzip2-lib-path],
     [location of the bzip2 library libbz2.so [default=/usr/lib64]])],
  [BZIP2_LIBS="-L$withval -lbz2"],
  [BZIP2_LIBS='-L/usr/lib64 -lbz2'])
AC_SUBST([BZIP2_LIBS])

dnl figure out where is zlib
AC_ARG_WITH([zlib-include-path],
  [AS_HELP_STRING([--with-zlib-include-path],
    [location of the zlib header file zlib.h [default=/usr/include]])],
  [ZLIB_CFLAGS="-I$withval"],
  [ZLIB_CFLAGS='-I/usr/include'])
AC_SUBST([ZLIB_CFLAGS])

AC_ARG_WITH([zlib-lib-path],
  [AS_HELP_STRING([--with-zlib-lib-path],
     [location of the zlib library libz.so [default=/usr/lib64]])],
  [ZLIB_LIBS="-L$withval -lz"],
  [ZLIB_LIBS='-L/usr/lib64 -lz'])
AC_SUBST([ZLIB_LIBS])

AC_OUTPUT([Makefile geners/Makefile examples/Makefile tools/Makefile geners.pc])
