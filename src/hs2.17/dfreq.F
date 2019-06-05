*
* $Id: dfreq.F,v 1.1.1.1 1996/02/15 17:48:17 mclareni Exp $
*
* $Log: dfreq.F,v $
* Revision 1.1.1.1  1996/02/15 17:48:17  mclareni
* Kernlib
*
*
      DOUBLE PRECISION FUNCTION DFREQ(DX)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C
      DATA CONST/ 0.70710 67811 86548 D0 /
C     ( CONST=SQRT(1/2). )
C
      DATA XMAX/ 25.8 D0 /
C     ( XMAX=SQRT(-ALOG(RMIN)-10.0), WHERE RMIN IS THE SMALLEST NORMAL-
C     IZED REPRESENTABLE NUMBER.  ERFC(XMAX) IS CLOSE TO THE UNDERFLOW
C     THRESHOLD. )
C
C     ******************************************************************
C
C  START.
      CX=CONST*DBLE(DX)
      IF(CX.LT.-XMAX) THEN
         DFREQ=0.D0
      ELSE
         IF(CX.GE.0.D0) DFREQ = 0.5D0 + 0.5D0*DERF(CX)
         IF(CX.LT.0.D0) DFREQ = 0.5D0*DERFC(-CX)
      ENDIF
      RETURN
      END
