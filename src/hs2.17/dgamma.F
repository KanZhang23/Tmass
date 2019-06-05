*
* $Id: dgamma.F,v 1.1.1.1 1996/02/15 17:48:17 mclareni Exp $
*
* $Log: dgamma.F,v $
* Revision 1.1.1.1  1996/02/15 17:48:17  mclareni
* Kernlib
*
*
#include "minuit/pilot.h"
      DOUBLE PRECISION FUNCTION DGAMMA(X)
      LOGICAL MFLAG,RFLAG
      REAL SX
      DOUBLE PRECISION X,U,F,ZERO,ONE,THREE,FOUR,PI
      DOUBLE PRECISION C(0:24),H,ALFA,B0,B1,B2
      DATA ZERO /0.0D0/, ONE /1.0D0/, THREE /3.0D0/, FOUR /4.0D0/

      DATA NC /15/
      DATA PI    /3.14159 26535 89793 24D0/
      DATA C( 0) /3.65738 77250 83382 44D0/
      DATA C( 1) /1.95754 34566 61268 27D0/
      DATA C( 2) / .33829 71138 26160 39D0/
      DATA C( 3) / .04208 95127 65575 49D0/
      DATA C( 4) / .00428 76504 82129 09D0/
      DATA C( 5) / .00036 52121 69294 62D0/
      DATA C( 6) / .00002 74006 42226 42D0/
      DATA C( 7) / .00000 18124 02333 65D0/
      DATA C( 8) / .00000 01096 57758 66D0/
      DATA C( 9) / .00000 00059 87184 05D0/
      DATA C(10) / .00000 00003 07690 81D0/
      DATA C(11) / .00000 00000 14317 93D0/
      DATA C(12) / .00000 00000 00651 09D0/
      DATA C(13) / .00000 00000 00025 96D0/
      DATA C(14) / .00000 00000 00001 11D0/
      DATA C(15) / .00000 00000 00000 04D0/

      U=X
      IF(X .LE. ZERO) THEN
       IF(X .EQ. INT(X)) THEN
        CALL KERMTR('C305.1',LGFILE,MFLAG,RFLAG)
        IF(MFLAG) THEN
         SX=X
         IF(LGFILE .EQ. 0) THEN
          WRITE(*,100) SX
         ELSE
          WRITE(LGFILE,100) SX
         END IF
        END IF
        IF(.NOT.RFLAG) CALL ABEND
        DGAMMA=ZERO
        RETURN
       ELSE
        U=ONE-U
       END IF
      END IF
      F=ONE
      IF(U .LT. THREE) THEN
       DO 1 I = 1,INT(FOUR-U)
       F=F/U
    1  U=U+ONE
      ELSE
       DO 2 I = 1,INT(U-THREE)
       U=U-ONE
    2  F=F*U
      END IF
      U=U-THREE
      H=U+U-ONE
      ALFA=H+H
      B1=ZERO
      B2=ZERO
      DO 3 I = NC,0,-1
      B0=C(I)+ALFA*B1-B2
      B2=B1
    3 B1=B0
      U=F*(B0-H*B2)
      IF(X .LT. ZERO) U=PI/(SIN(PI*X)*U)
      DGAMMA=U
      RETURN
  100 FORMAT(1X,'DGAMMA ... ARGUMENT IS NON-POSITIVE INTEGER = ',E15.1)
      END
