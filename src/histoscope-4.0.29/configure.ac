AC_PREREQ([2.59])

AC_INIT([histoscope], [4.0.29])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_INSTALL

dnl find the C compiler
AC_PROG_CC

dnl set the default destination directory
AC_PREFIX_DEFAULT([/usr/local])

dnl enable libtool
AC_PROG_LIBTOOL

dnl enable yacc
AC_PROG_YACC

AC_ARG_WITH([x11-include-path],
  [AS_HELP_STRING([--with-x11-include-path],
     [location of the X11 include X.h [default=/usr/include/X11]])],
  [X11_CFLAGS="-I$withval"],
  [X11_CFLAGS='-I/usr/include/X11'])
AC_SUBST([X11_CFLAGS])

AC_ARG_WITH([x11-lib-path],
  [AS_HELP_STRING([--with-x11-lib-path],
     [location of the X11 library libX11.so [default=/usr/lib64]])],
  [X11_LIBS="-L$withval -lXt -lX11"],
  [X11_LIBS='-L/usr/lib64 -lXt -lX11'])
AC_SUBST([X11_LIBS])

AC_ARG_WITH([motif-include-path],
  [AS_HELP_STRING([--with-motif-include-path],
     [location of the Motif include Xm/Xm.h [default=/usr/include/X11]])],
  [MOTIF_CFLAGS="-I$withval"],
  [MOTIF_CFLAGS='-I/usr/include/X11'])
AC_SUBST([MOTIF_CFLAGS])

AC_ARG_WITH([motif-lib-path],
  [AS_HELP_STRING([--with-motif-lib-path],
     [location of the Motif library libXm.so [default=/usr/lib64]])],
  [MOTIF_LIBS="-L$withval -lXm"],
  [MOTIF_LIBS='-L/usr/lib64 -lXm'])
AC_SUBST([MOTIF_LIBS])

AC_OUTPUT([Makefile histo/Makefile histo_api/Makefile histo_util/Makefile plot_widgets/Makefile util/Makefile programs/Makefile histoscope.pc])
