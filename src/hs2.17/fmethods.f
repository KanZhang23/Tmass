C
C This file should contain only the code which depends on CERNLIB
C
      DOUBLE PRECISION FUNCTION LANDAU(X,Y,Z,MODE,DPAR,ERRSTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
C Inputs:
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is operation mode (may be ignored)
C     DPAR   are the parameters
C
C     Landau pdf needs 3 parameters: norm, peak, and width
C
C Outputs:
C     ERRSTA is the error status of the calculation (0 means OK)
C
C Returns:
C     LANDAU  is the function value for given arguments and parameters
C             WARNING!!! this is just a single precision function.
C             It is based on DENLAN, CERNLIB entry G110
C
      DOUBLE PRECISION WIDTH
      REAL DIFF, DENLAN
      EXTERNAL DENLAN
C
      WIDTH = DABS(DPAR(3))
      IF (WIDTH.EQ.0.D0) THEN
         LANDAU = 0.D0
         RETURN
      END IF
      DIFF = (X-DPAR(2))/WIDTH
C
      LANDAU = DPAR(1)/WIDTH*DENLAN(DIFF)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION STUDCD(X,Y,Z,MODE,DPAR,ERRSTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
C     This is the cdf of Student't t-distribution. It needs
C     1 parameter: N. It is a single precision function.
C
      REAL STUDIS, REALX
      EXTERNAL STUDIS
      INTEGER N
C
      N = DPAR(1)
      IF (N.LT.1) THEN
         STUDCD = 0.D0
         RETURN
      END IF
      REALX = X
      STUDCD = STUDIS(REALX,N)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION CHTAIL(X,Y,Z,MODE,DPAR,ERRSTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
C Inputs:
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is operation mode (may be ignored)
C     DPAR   are the parameters
C
C     This is one minus chi-squared cumulative distribution
C     function. It needs 1 parameter: N.
C
C Outputs:
C     ERRSTA is the error status of the calculation (0 means OK)
C
C Returns:
C     CHICDF is the function value for given arguments and parameters
C     WARNING!!! This is a single precision function!
C
      REAL PROB, REALX
      EXTERNAL PROB
      INTEGER N
C
      N = DPAR(1)
      IF (N.LT.1) THEN
         CHTAIL = 0.D0
         RETURN
      END IF
      IF (X.LE.0.D0) THEN
         CHTAIL = 1.D0
         RETURN
      END IF
      REALX = X
      CHTAIL = PROB(REALX,N)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION GAMPDF(X,Y,Z,MODE,DPAR,ERRSTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
C Inputs:
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is operation mode (may be ignored)
C     DPAR   are the parameters
C
C     Gamma pdf needs 4 parameters: norm, origin, width, and A
C
C Outputs:
C     ERRSTA is the error status of the calculation (0 means OK)
C
C Returns:
C     GAMPDF  is the function value for given arguments and parameters
C
      DOUBLE PRECISION A, WIDTH, DIFF, DGAMMA, NORMW
      EXTERNAL DGAMMA
C
      GAMPDF = 0.D0
      WIDTH = DABS(DPAR(3))
      A = DPAR(4)
      IF ((WIDTH.EQ.0.D0).OR.(A.LE.0.D0)) RETURN
      NORMW = DSQRT(A)
      DIFF = NORMW*(X-DPAR(2))/WIDTH
      IF (DIFF.LE.0.D0) RETURN

      GAMPDF=NORMW*DPAR(1)/WIDTH*DIFF**(A-1.D0)*DEXP(-DIFF)/DGAMMA(A)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION GAMCDF(X,Y,Z,MODE,DPAR,ERRSTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
C Inputs:
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is operation mode (may be ignored)
C     DPAR   are the parameters
C
C     Gamma cdf needs 5 parameters: H_l, H_r, origin, width, and A
C
C Outputs:
C     ERRSTA is the error status of the calculation (0 means OK)
C
C Returns:
C     GAMCDF  is the function value for given arguments and parameters
C
      DOUBLE PRECISION DGAPNC, HL, HR, A, WIDTH, DIFF, X0
      EXTERNAL DGAPNC
C
      HL = DPAR(1)
      HR = DPAR(2)
      X0 = DPAR(3)
      WIDTH = DABS(DPAR(4))
      A = DPAR(5)
C
      GAMCDF = HL
      IF (A.LE.0.D0) RETURN
      IF (WIDTH.EQ.0.D0) THEN
         IF (X .GT. X0) GAMCDF = HR
         RETURN
      END IF
      DIFF = DSQRT(A)*(X-X0)/WIDTH
      IF (DIFF.LE.0.D0) RETURN

      GAMCDF=HL+(HR-HL)*DGAPNC(A,DIFF)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION BETAFUN(X,Y,Z,MODE,DPAR,ERRSTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
C Inputs:
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is operation mode (may be ignored)
C     DPAR   are the parameters
C
C     Beta function needs 5 parameters: norm, min, max, M and N
C
C Outputs:
C     ERRSTA is the error status of the calculation (0 means OK)
C
C Returns:
C     BETAFUN  is the function value for given arguments and parameters
C
      DOUBLE PRECISION DGAMMA, XMIN, XMAX, M, N, BASE
      EXTERNAL DGAMMA
C
      XMIN = DPAR(2)
      XMAX = DPAR(3)
      M = DPAR(4)
      N = DPAR(5)
      BASE = XMAX-XMIN
C
      IF (BASE.LE.0.D0 .OR. M.LE.0.D0 .OR. N.LE.0.D0) THEN
         BETAFUN = 0.D0
      ELSEIF (X.LE.XMIN .OR. X.GE.XMAX) THEN
         BETAFUN = 0.D0
      ELSE
         BETAFUN = ((X-XMIN)/BASE)**(M-1.D0)*((XMAX-X)/BASE)**(N-1.D0)*
     >      DPAR(1)/BASE*DGAMMA(M+N)/DGAMMA(M)/DGAMMA(N)
      END IF
C
      RETURN
      END
