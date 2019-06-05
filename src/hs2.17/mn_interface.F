
#include "minuit/pilot.h"

C
C  Interface to fetch some of the Minuit common variables.
C  See the long write-up of CERNLIB entry D506.
C
      SUBROUTINE MNGETCOMMON(GETNFCN, GETDCOVAR, GETNFCNMX)
#include "minuit/d506dp.inc"
#include "minuit/d506cm.inc"
      INTEGER GETNFCN, GETNFCNMX
      DOUBLE PRECISION GETDCOVAR
C
C GETNFCN   is the number of FCN calls
C GETDCOVAR is the estimated fractional difference of the covariance
C           matrix. Can be used as a measure of the covariance matrix
C           precision.
C
      GETNFCN = NFCN
      GETDCOVAR = DCOVAR
      GETNFCNMX = NFCNMX

      RETURN
      END
C
C  Interface to set some of the Minuit variables.
C
      SUBROUTINE MNSETNFCNMX(SETNFCNMX)
C
C  Need to reinclude the commons
C
#undef CERNLIB_MINUIT_D506DP_INC
#undef CERNLIB_MINUIT_D506CM_INC
#include "minuit/d506dp.inc"
#include "minuit/d506cm.inc"
      INTEGER SETNFCNMX
C
C NFCNMX is the max number of FCN calls in Minuit.
C Set it to 1 in order to terminate minimization quickly.
C
      NFCNMX = SETNFCNMX
      RETURN
      END
C
C  Routines to open and close Fortran files
C
      SUBROUTINE MNOPEN(FNAME,MODE,LUN,IERR)
      IMPLICIT NONE
      CHARACTER*(*) FNAME
      INTEGER MODE, LUN, IERR
C
      IF (MODE.EQ.0) THEN
* Open the file for read
         OPEN(UNIT=LUN,FILE=FNAME,STATUS='OLD',IOSTAT=IERR)
      ELSE
* Open the file for write
         OPEN(UNIT=LUN,FILE=FNAME,STATUS='NEW',IOSTAT=IERR)
      END IF
C
      RETURN
      END
CCCC
      SUBROUTINE MNCLOS(LUN,IERR)
      IMPLICIT NONE
      INTEGER LUN, IERR
      CLOSE (UNIT=LUN,IOSTAT=IERR)
      RETURN
      END
