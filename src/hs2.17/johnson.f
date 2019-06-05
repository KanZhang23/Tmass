      DOUBLE PRECISION FUNCTION LOGNORM(X,Y,Z,MODE,DPAR,ERRSTA)
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
C     Lognormal pdf needs 4 parameters: norm, mean, width, and skew
C
C Outputs:
C     ERRSTA is the error status of the calculation (0 means OK)
C
C Returns:
C     LOGNORM  is the function value for given arguments and parameters
C
      DOUBLE PRECISION MEAN,WIDTH,SKEW,B1,DIFF,W,TMP,LOGW
      DOUBLE PRECISION EMGAMOVD,DELTA,XI
C
      DOUBLE PRECISION SQR2PI
      PARAMETER(SQR2PI=2.506628274631D0)
C
      MEAN = DPAR(2)
      WIDTH = DABS(DPAR(3))
      IF (WIDTH .EQ. 0.D0) THEN
         LOGNORM = 0.D0
         RETURN
      END IF
      SKEW = DPAR(4)
      IF (SKEW .EQ. 0.D0) THEN
C This is a Gaussian
         DIFF = (X-MEAN)/WIDTH
         LOGNORM = DPAR(1)/SQR2PI/WIDTH*DEXP(-DIFF*DIFF/2.D0)
         RETURN
      END IF
C
      B1 = SKEW*SKEW
      TMP = ((2.D0+B1+DSQRT(B1*(4.D0+B1)))/2.D0)**(1.D0/3.D0)
      W = TMP+1.D0/TMP-1.D0
      LOGW = DLOG(W)
      IF (LOGW .LE. 0.D0) THEN
C This is not different from a Gaussian
         DIFF = (X-MEAN)/WIDTH
         LOGNORM = DPAR(1)/SQR2PI/WIDTH*DEXP(-DIFF*DIFF/2.D0)
         RETURN         
      END IF
      DELTA = DSQRT(1.D0/LOGW)
      EMGAMOVD = WIDTH/DSQRT(W*(W-1.D0))
CCC      XI = PEAK - EMGAMOVD/W
      XI = MEAN - EMGAMOVD*DSQRT(W)
      IF (SKEW .GT. 0.D0) THEN
         DIFF = X-XI
      ELSE
         DIFF = 2.D0*MEAN-X-XI
      END IF
      IF (DIFF .LE. 0.D0) THEN
         LOGNORM = 0.D0
      ELSE
         LOGNORM = DPAR(1)/SQR2PI*DELTA/DIFF*
     >        DEXP(-0.5D0/LOGW*(DLOG(DIFF/EMGAMOVD))**2)
      END IF
C
      RETURN
      END
CCCCCC
CCCCCC
      DOUBLE PRECISION FUNCTION JOHN4M(X,Y,Z,MODE,DPAR,ERRSTA)
C
C  This function is a Johnson's S_u type curve represented
C  using four moments
C
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is calculation mode (may be ignored)
C     DPAR   are the parameters in DOUBLE PRECISION
C
C     Johnson's S_u (moments) needs 5 parameters:
C         norm, mean, sigma, skew, and kurt
C
C     ERRSTA is the error status of the calculation (0 means OK)
C
      IMPLICIT NONE
      DOUBLE PRECISION JOHNSU
      EXTERNAL JOHNSU
      DOUBLE PRECISION X,Y,Z
      INTEGER MODE,IERR,I,ERRSTA
      DATA IERR/0/
      SAVE IERR
      DOUBLE PRECISION DPAR(*),NORM,MEAN,SIGMA,SKEW,KURT
      DOUBLE PRECISION XI,LAMBDA,GAMMA,DELTA,DTMP(5)
      DATA DTMP/0.D0,0.D0,0.D0,0.D0,0.D0/
      SAVE XI,LAMBDA,GAMMA,DELTA,DTMP
      DOUBLE PRECISION SQR2PI
      PARAMETER(SQR2PI=2.506628274631D0)
      LOGICAL ASOLD
C
      ASOLD=.TRUE.
      DO I=2,5
         IF (DPAR(I).NE.DTMP(I)) THEN
            ASOLD=.FALSE.
            DTMP(I)=DPAR(I)
         END IF
      END DO
C
      NORM=DPAR(1)
C
C Pass local copies of the parameters to JONPAR to make sure that
C the argument parameter values are not modified.
      IF (.NOT.ASOLD) THEN
         MEAN=DPAR(2)
         SIGMA=DPAR(3)
         SKEW=DPAR(4)
         KURT=DPAR(5)
         CALL JONPAR(MEAN,SIGMA,SKEW,KURT,XI,LAMBDA,GAMMA,DELTA,IERR)
      END IF
C
      IF (IERR.EQ.0) THEN
         JOHN4M=JOHNSU(X,XI,LAMBDA,GAMMA,DELTA)*NORM
      ELSE IF (DTMP(3).GT.0.D0 .AND. DTMP(4).EQ.0.D0 .AND.
     >        DTMP(5).EQ.3.D0) THEN
         JOHN4M=DEXP(-((X-DTMP(2))/DTMP(3))**2/2.D0)/DTMP(3)/SQR2PI*
     >        NORM
      ELSE
         JOHN4M=0.D0
      END IF
C
      RETURN
      END
CCCCCC
CCCCCC
      DOUBLE PRECISION FUNCTION JOHNSN(X,Y,Z,MODE,DPAR,ERRSTA)
C
C  This function is a Johnson's S_u type curve represented
C  using its "translation" parameters
C
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is calculation mode (may be ignored)
C     DPAR   are the parameters in DOUBLE PRECISION
C
C     Johnson's S_u (translation) needs 5 parameters:
C         norm, xi, lambda, gamma, and delta
C
C     ERRSTA is the error status of the calculation (0 means OK)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DPAR(*)
      DOUBLE PRECISION JOHNSU
      EXTERNAL JOHNSU
C
*      NORM=DPAR(1)
*      XI=DPAR(2)
*      LAMBDA=DPAR(3)
*      GAMMA=DPAR(4)
*      DELTA=DPAR(5)
C
      JOHNSN=JOHNSU(X,DPAR(2),DPAR(3),DPAR(4),DPAR(5))*DPAR(1)
C
      RETURN
      END
CCCCCC
CCCCCC
      DOUBLE PRECISION FUNCTION JOHNB(X,Y,Z,MODE,DPAR,ERRSTA)
C
C  This function is a Johnson's S_b type curve represented
C  using its "translation" parameters
C
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is calculation mode (may be ignored)
C     DPAR   are the parameters in DOUBLE PRECISION
C
C     Johnson's S_b (translation) needs 5 parameters:
C         norm, xi, lambda, gamma, and delta
C
C     ERRSTA is the error status of the calculation (0 means OK)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DPAR(*)
      DOUBLE PRECISION JOHNSB
      EXTERNAL JOHNSB
C     
*      NORM=DPAR(1)
*      XI=DPAR(2)
*      LAMBDA=DPAR(3)
*      GAMMA=DPAR(4)
*      DELTA=DPAR(5)
C
      JOHNB=JOHNSB(X,DPAR(2),DPAR(3),DPAR(4),DPAR(5))*DPAR(1)
C
      RETURN
      END
CCCCCC
CCCCCC
      DOUBLE PRECISION FUNCTION JOHNSU(X,KSI,LAMBDA,GAMMA,DELTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,KSI,LAMBDA,GAMMA,DELTA,TMP
      DOUBLE PRECISION SQR2PI
      PARAMETER(SQR2PI=2.506628274631D0)
C
      IF (LAMBDA.LE.0.D0) THEN
         JOHNSU=0.D0
      ELSE
         TMP=(X-KSI)/LAMBDA
         JOHNSU=DELTA/LAMBDA/SQR2PI/DSQRT(1.D0+TMP*TMP)*
     >      DEXP(-(GAMMA+DELTA*DLOG(TMP+DSQRT(1.D0+TMP*TMP)))**2/2.D0)
      END IF
C
      RETURN
      END
CCCCCC
CCCCCC
      DOUBLE PRECISION FUNCTION JOHNSB(X,KSI,LAMBDA,GAMMA,DELTA)
C
      IMPLICIT NONE
      DOUBLE PRECISION X,KSI,LAMBDA,GAMMA,DELTA
      DOUBLE PRECISION SQR2PI
      PARAMETER(SQR2PI=2.506628274631D0)
C
      IF (LAMBDA.LE.0.D0) THEN
         JOHNSB=0.D0
      ELSEIF (X.LE.KSI .OR. X.GE.(KSI+LAMBDA)) THEN
         JOHNSB=0.D0
      ELSE
         JOHNSB=(DELTA*LAMBDA)/
     >      (DEXP((GAMMA+DELTA*DLOG((X-KSI)/(LAMBDA-X+KSI)))**2/2.D0)*
     >      SQR2PI*(X-KSI)*(LAMBDA-X+KSI))
      END IF
C
      RETURN
      END
CCCCCC
CCCCCC
      SUBROUTINE JONPAR(MEAN,SIGMA,SKEW,KURT,
     >     XI,LAMBDA,GAMMA,DELTA,IERR)
C
C This subroutine calculates values of parameters for Johnson's
C  S_u type curves for given mean, standard deviation, skewness
C  and kurtosis.
C
      IMPLICIT NONE
      DOUBLE PRECISION MEAN,SIGMA,SKEW,KURT,GAMMA,DELTA,XI,LAMBDA
      INTEGER IERR
C
      DOUBLE PRECISION ZERO,ONE,TWO,THREE,FOUR
      PARAMETER(ZERO=0.D0,ONE=1.D0,TWO=2.D0,THREE=3.D0,FOUR=4.D0)
C
      DOUBLE PRECISION TMP,W,B1,B1TMP,DB1DW,M,TOL
      PARAMETER(TOL=1.D-12)
C
      DOUBLE PRECISION SKEWS,KURTS,WS,MS
      DATA SKEWS/0.D0/,KURTS/-1.D0/,WS/0.D0/,MS/0.D0/
      SAVE SKEWS,KURTS,WS,MS
C
      IERR=1
      IF (DABS(SKEW).LT.TOL) SKEW=ZERO
C
C Find BETA2 for lognormal distribution with given skewness and
C  compare with given kurtosis argument. Return with IERR=1 if
C  skewness and kurtosis are outside the region covered by
C  Johnson's S_u curve.
C
      B1=SKEW*SKEW
      TMP=((TWO+B1+DSQRT(B1*(FOUR+B1)))/TWO)**(ONE/THREE)
      W=TMP+ONE/TMP-ONE
      TMP=W*W*(W*(W+TWO)+THREE)-THREE
      IF (KURT.LE.TMP.OR.SIGMA.LE.ZERO) THEN
         GAMMA=ZERO
         DELTA=ZERO
         XI=ZERO
         LAMBDA=ZERO
         RETURN
      END IF
C
C Try to save some time by checking previous arguments
C
      IF (SKEW.EQ.SKEWS .AND. KURT.EQ.KURTS) THEN
         W=WS
         M=MS
         GOTO 20
      END IF
C
C Make a guess for the value of W.
C
      W=DSQRT(DSQRT(TWO*KURT-TWO)-ONE)-B1/KURT
C
C Iterations to get the correct W.
C
 10   CALL BETA1(W,KURT,B1TMP,DB1DW,M)
      IF (DABS(B1-B1TMP)/(DABS(B1)+ONE) .GT. TOL) THEN
         W=W+(B1-B1TMP)/DB1DW
         GOTO 10
      END IF
C
C Sometimes M can become negative due to rounding problems
C
      IF (M.LT.ZERO) THEN
         M=ZERO
      END IF
C
C Remember calculated value
C
      SKEWS=SKEW
      KURTS=KURT
      WS=W
      MS=M
C
C Calculate values of DELTA, GAMMA, LAMBDA and XI
C
 20   DELTA=DSQRT(ONE/DLOG(W))
      LAMBDA=SIGMA/DSQRT((W-ONE)*(TWO*M+W+ONE)/TWO)
      IF (SKEW.EQ.ZERO) THEN
         GAMMA=ZERO
         XI=MEAN
      ELSE
         TMP=DSQRT(M/W)
         GAMMA=-DSIGN(DELTA*DLOG(TMP+DSQRT(TMP**2+ONE)),SKEW)
         XI=MEAN-DSQRT(W)*LAMBDA*DSIGN(TMP,SKEW)
      END IF
      IERR=0
C
      RETURN
      END
CCCCCC
CCCCCC
      SUBROUTINE BETA1(W,B2,B1,DB1DW,M)
      IMPLICIT NONE
      DOUBLE PRECISION W,B2,B1,DB1DW,M
C
C This routine calculates correct values of beta_1 and D[beta_1,W]
C  for given values of W ( = exp(delta^-2)) and beta_2 for Johnson's
C  S_u type curve.
C
C  Input:
C       W  - Johnson's small omega
C       B2 - beta_2
C  Output:
C       B1 - beta_1
C       DB1DW - D[beta_1,w]
C       M  - Johnson's m
C
      DOUBLE PRECISION A,B,C,A0,A1,A2,D,TMP,NUM,DEN,
     >  DADW,DBDW,DCDW,DA0DW,DA1DW,DA2DW,DDDW,DMDW,DTMPDW,DNUMDW,
     >  DDENDW,WP1,WM1,WP2,B2M3
C
      WP1=W+1.D0
      WP2=W+2.D0
      WM1=W-1.D0
      B2M3=B2-3.D0
      A2 = 8.D0*(6.D0+W*(6.D0+W*(3.D0+W)))
      DA2DW = 24.D0*(2.D0+W*WP2)
      A1 = A2*W+8.D0*(W+3.D0)
      DA1DW = DA2DW*W+A2+8.D0
      A0 = WP1**3*(W**2+3.D0)
      DA0DW = WP1**2*(9.D0+W*(2.D0+5.D0*W))
      A = A2*WM1-8.D0*B2M3
      DADW = DA1DW-DA2DW-8.D0
      B = A1*WM1-8.D0*WP1*B2M3
      DBDW = DA1DW*WM1+A1-8.D0*B2M3
      C = A0*WM1-2.D0*B2M3*WP1**2
      DCDW = DA0DW*WM1+A0-4.D0*B2M3*WP1
      D = B**2-4.D0*A*C
      DDDW = 2.D0*B*DBDW-4.D0*(A*DCDW+C*DADW)
      TMP = DSQRT(D)
      DTMPDW = 0.5D0/TMP*DDDW
      M = (TMP-B)/2.D0/A
      DMDW = ((DTMPDW-DBDW)*A-DADW*(TMP-B))/A**2/2.D0
C
      TMP = 4.D0*WP2*M+3.D0*WP1**2
      DTMPDW = 4.D0*(M+DMDW*WP2)+6.D0*WP1
      NUM = M*WM1*TMP**2
      DNUMDW = TMP*(WM1*(DMDW*TMP+2.D0*DTMPDW*M)+M*TMP)
      DEN = 2.D0*(2.D0*M+WP1)**3
      DDENDW = 6.D0*(2.D0*M+WP1)**2*(2.D0*DMDW+1.D0)
      B1 = NUM/DEN
      DB1DW = (DNUMDW*DEN-DDENDW*NUM)/DEN**2
C
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE SBFIT(XBAR,SIGMA,RTB1,B2,GAMMA,DELTA,XLAM,XI,IFAULT)
      IMPLICIT NONE
C
C        ALGORITHM AS 99.2  APPL. STATIST. (1976) VOL.25, P.180
C
C        FINDS PARAMETERS OF JOHNSON SB CURVE WITH
C        GIVEN FIRST FOUR MOMENTS
C
      DOUBLE PRECISION DERIV(4),XBAR,SIGMA,RTB1,B2,GAMMA,
     $  DELTA, XLAM, XI, TT, TOL, RB1, B1, E, U, X, Y, W, F, D,
     $  G, H2, T, RBET, BET2, DUMM, MEAN,
     $  ZERO, ONE, TWO, THREE, HALF, QUART, ONE5,
     $  A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12,
     $  A13, A14, A15, A16, A17, A18, A19, A20, A21, A22
      LOGICAL NEG, FAULT
      INTEGER M, LIMIT, IFAULT
C
      DOUBLE PRECISION DB1,DB1OLD,DB1VOLD,DB2,DB2OLD,DB2VOLD
      DOUBLE PRECISION STEPF,NEWD,NEWG,UPRLIM,LORLIM,MAXDELTA
      INTEGER NOSC
C
      INTEGER FAULTSAV
      DOUBLE PRECISION RTB1SAVE, B2SAVE, DSAVE, GSAVE
      DATA RTB1SAVE/0.D0/, B2SAVE/-12345.D0/, FAULTSAV/1/
      SAVE RTB1SAVE, B2SAVE, DSAVE, GSAVE, FAULTSAV
C
      DATA TT, TOL, LIMIT /1.0D-8, 1.0D-4, 500/
      DATA ZERO, ONE, TWO, THREE, HALF, QUART, ONE5
     $     /0.D0,1.D0,2.D0,3.D0,0.5D0,0.25D0,1.5D0/
      DATA     A1,     A2,     A3,     A4,     A5,     A6,
     $         A7,     A8,     A9,    A10,    A11,    A12,
     $        A13,    A14,    A15,    A16,    A17,    A18,
     $        A19,    A20,    A21,    A22
     $    /0.0124D0, 0.0623D0, 0.4043D0, 0.408D0, 0.479D0,0.485D0,
     $     0.5291D0, 0.5955D0, 0.626D0, 0.64D0, 0.7077D0, 0.7466D0,
     $        0.8D0, 0.9281D0, 1.0614D0, 1.25D0, 1.7973D0, 1.8D0,
     $      2.163D0,  2.5D0, 8.5245D0, 11.346D0/
C
      RB1 = DABS(RTB1)
      B1 = RB1 * RB1
      NEG = RTB1 .LT. ZERO
      IFAULT = 1
C
C Check if we have already calculated this
C
      IF (RTB1SAVE .EQ. RTB1 .AND. B2SAVE .EQ. B2) THEN
         IF (FAULTSAV .NE. 0) THEN
            RETURN
         ELSE
            D = DSAVE
            G = GSAVE
            CALL MOMDER(G, D, MEAN, H2, DUMM, DUMM, DERIV, FAULT)
            IF (FAULT .OR. H2 .LE. ZERO) THEN
               PRINT *,'SBFIT: bad S_b moments'
               FAULTSAV = 1
               RETURN
            END IF
            GOTO 125
         END IF
      END IF
      RTB1SAVE = RTB1
      B2SAVE = B2
      FAULTSAV = 1
C
C        GET D AS FIRST ESTIMATE OF DELTA
C
      E = B1 + ONE
      X = HALF * B1 + ONE
      Y = DABS(RB1) * DSQRT(QUART * B1 + ONE)
      U = (X + Y) ** (ONE / THREE)
      W = U + ONE / U - ONE
      F = W * W * (THREE + W * (TWO + W)) - THREE

C  Return with FAULT = .TRUE. when skewness and kurtosis are outside 
C  of the region covered by Johnson's S_b curve. -- 09/14/2000 IGV
      IF (B2.LE.E .OR. B2.GE.F .OR. SIGMA.LE.ZERO) THEN
         GAMMA=ZERO
         DELTA=ZERO
         XI=ZERO
         XLAM=ZERO
         PRINT *,'SBFIT: parameters out of range'
         RETURN
      END IF
C
      MAXDELTA = 1.D300
      E = (B2 - E) / (F - E)
      IF (RB1 .GT. TOL) GOTO 5
      F = TWO
      GOTO 20
    5 D = ONE / DSQRT(DLOG(W))
      MAXDELTA = D
      IF (D .LT. A10) GOTO 10
      F = TWO - A21 / (D * (D * (D - A19) + A22))
      GOTO 20
   10 F = A16 * D
   20 F = E * F + ONE
      IF (F .LT. A18) GOTO 25
      D = (A9 * F - A4) * (THREE - F) ** (-A5)
      GOTO 30
   25 D = A13 * (F - ONE)
C
C        GET G AS FIRST ESTIMATE OF GAMMA
C
   30 G = ZERO
      IF (B1 .LT. TT) GOTO 70
      IF (D .GT. ONE) GOTO 40
      G = (A12 * D ** A17 + A8) * B1 ** A6
      GOTO 70
   40 IF (D .LE. A20) GOTO 50
      U = A1
      Y = A7
      GOTO 60
   50 U = A2
      Y = A3
   60 G = B1 ** (U * D + Y) * (A14 + D * (A15 * D - A11))
   70 M = 0
      NOSC = 0
      STEPF = ONE
C
C        MAIN ITERATION STARTS HERE
C
   80 M = M + 1
      FAULT = M .GE. LIMIT
      IF (FAULT) THEN
         PRINT *,'SBFIT: iteration limit exceeded, sqrt(B1) = ',
     >        RB1,', B2 = ',B2
         RETURN
      END IF
C
C        GET SKEWNESS, KURTOSIS, AND THEIR DERIVATIVES
C        FOR LATEST G AND D VALUES
C
      CALL MOMDER(G, D, MEAN, H2, RBET, BET2, DERIV, FAULT)
      IF (FAULT) THEN
         PRINT *,'SBFIT: moment calculation failed'
         RETURN
      END IF
      FAULT = H2 .LE. ZERO
      IF (FAULT) THEN
         PRINT *,'SBFIT: second central moment is null, sqrt(B1) = ',
     >        RB1,', B2 = ',B2,', G = ',G,', D = ',D
         RETURN
      END IF

C  Try to detect oscillations
      DB1 = RBET - RB1
      DB2 = BET2 - B2
      IF (M .GT. 3) THEN
         IF ((DB1*DB1OLD.LT.0.D0.AND.DABS(DB1).GE.DABS(DB1OLD).OR.
     >        DB1*DB1VOLD.LT.0.D0.AND.DABS(DB1).GE.DABS(DB1VOLD)).AND.
     >       (DB2*DB2OLD.LT.0.D0.AND.DABS(DB2).GE.DABS(DB2OLD).OR.
     >        DB2*DB2VOLD.LT.0.D0.AND.DABS(DB2).GE.DABS(DB2VOLD))) THEN
            NOSC = NOSC + 1
            STEPF = STEPF/ONE5
            IF (NOSC .GT. 3) THEN
C
C  We see oscillations, but can't do much about it.
C
C  For very small values of delta it is sometimes impossible
C  to specify delta with enough precision. There, very small change
C  in delta results in large changes of B1 and B2 near the lognormal
C  curve. Therefore, small errors in delta result in big errors for
C  B1, B2 near lognormal. I don't yet see a way to avoid this
C  problem. Example of this problem can be encountered, e.g.,
C  for SKEW = 3.76068555, KURT = 15.1429636, which results in
C  DELTA around 3.0E-06 and GAMMA near 1.57.
C
C  Just hope that the system is now sufficiently close
C  to the correct solution.
C
               PRINT *,'SBFIT: oscillations detected, sqrt(B1) =',
     >              RB1,', B2 =',B2
               PRINT *,'SBFIT: current DELTA is',D,', GAMMA is',G
               PRINT *,'SBFIT: missed target skewness by',DABS(DB1),
     >              ', kurtosis by',DABS(DB2)
               GOTO 125
            END IF
         END IF
      END IF

      DB1VOLD = DB1OLD
      DB1OLD  = DB1
      DB2VOLD = DB2OLD
      DB2OLD  = DB2

      T = DERIV(1) * DERIV(4) - DERIV(2) * DERIV(3)
      IF (T .EQ. 0.D0) THEN
         PRINT *,'SBFIT: null iteration determinant, sqrt(B1) =',
     >        RB1,', B2 =',B2
         U = -1.D-2 * G
         Y = -1.D-3 * D
      ELSE
         U = (DERIV(4) * DB1 - DERIV(2) * DB2) / T
         Y = (DERIV(1) * DB2 - DERIV(3) * DB1) / T
      END IF

CC      PRINT *,''
CC      PRINT *,'Iteration ',M,': B1 = ',RBET, ', KURT = ',BET2
CC      PRINT *,'D = ',D,', dD = ',-Y*STEPF
CC      PRINT *,'G = ',G,', dG = ',-U*STEPF
CC      PRINT *,'DB1 = ',DB1,', DB2 = ',DB2
CC      PRINT *,'Derivatives are ',DERIV(1),DERIV(2),DERIV(3),DERIV(4)

C
C        FORM NEW ESTIMATES OF G AND D
C
      UPRLIM = TWO - (ONE/LIMIT)*M
      LORLIM = UPRLIM**ONE5
      IF (B1 .EQ. ZERO) THEN
         G = ZERO
      ELSE
         NEWG = G - STEPF*U
         IF (G .GT. ONE) THEN
            IF (NEWG .GT. G*UPRLIM**2) THEN
               G = G*UPRLIM**2
            ELSEIF (NEWG .LT. G/LORLIM) THEN
               G = G/LORLIM
            ELSE
               G = NEWG
            END IF
         ELSEIF (NEWG .GT. TWO) THEN
            G = TWO
         ELSEIF (NEWG .LT. ZERO) THEN
            G = ZERO
         ELSE
            G = NEWG
         END IF
      END IF
C
      NEWD = D - STEPF*Y
      IF (NEWD .GT. D*UPRLIM) THEN
         D = D*UPRLIM
      ELSEIF (NEWD .LT. D/LORLIM) THEN
         D = D/LORLIM
      ELSE
         D = NEWD
      END IF
      IF (D .GT. MAXDELTA) D = MAXDELTA
C
      IF (DABS(DB1)/(DABS(RB1)+ONE) .GT. TT .OR. 
     >    DABS(DB2)/(DABS(B2)+ONE) .GT. TT) GOTO 80
C
C        END OF ITERATION
C
CC      PRINT *,'Needed ',M,' iterations'
C
  125 DELTA = D
      XLAM = SIGMA / DSQRT(H2)
      IF (NEG) THEN
         GAMMA = -G
         XI = XBAR - XLAM * (ONE - MEAN)
      ELSE
         GAMMA = G
         XI = XBAR - XLAM * MEAN
      END IF
      GSAVE = G
      DSAVE = D

CC      PRINT *,'Normal return'
      IFAULT = 0
      FAULTSAV = 0
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE SBFITLARGEGAMMA(MEAN,SIGMA,SKEW,KURT,
     >                           GAMMA,DELTA,XLAM,XI,FAULT)
      IMPLICIT NONE
C
      DOUBLE PRECISION MEAN,SIGMA,SKEW,KURT,GAMMA,DELTA,XLAM,XI
      INTEGER MODE,FAULT
C
      INTEGER MAXITER,NITER
      PARAMETER(MAXITER=1000)
      DOUBLE PRECISION TOL2D,MAXRATIO
      PARAMETER(TOL2D=1.D-8,MAXRATIO=1.5)
      DOUBLE PRECISION EXC,TRYKURT,B1,TRYB1,DTMP,W,Q
      DOUBLE PRECISION DB1DQ,DB1DQ0,DB1DW,DB1DW0,DKDQ,DKDQ0,DKDW,DKDW0
      DOUBLE PRECISION HIGHVAR,HIGHMEAN
      EXTERNAL HIGHVAR,HIGHMEAN
      DOUBLE PRECISION QSAVE, WSAVE, SKEWSAVE, KURTSAVE
      DATA KURTSAVE/-1234.D0/
      LOGICAL CONVERGED
      DATA CONVERGED/.FALSE./
      SAVE QSAVE, WSAVE, SKEWSAVE, KURTSAVE, CONVERGED
C
      FAULT = 1
      NITER = 0
      B1=SKEW*SKEW
      EXC = KURT-3.D0
      DTMP = ((2.D0+B1+DSQRT(B1*(4.D0+B1)))/2.D0)**(1.D0/3.D0)
      W = DTMP+1.D0/DTMP-1.D0
      TRYKURT = W*W*(W*(W+2.D0)+3.D0)-3.D0
C
C Check for invalid parameters
C 
      IF ((KURT .GE. TRYKURT) .OR. (KURT .LE. B1+1.D0)) THEN
         PRINT *,'ERROR in SBFITLARGEGAMMA: ',
     >        'skew, kurtosis not in SB range'
         RETURN
      END IF
C
C Check for non-zero skewness, kurtosis near the lognormal
C
      IF (SKEW.NE.0.D0) THEN
C
C Check if we already processed these values of skew and kurt
C
         IF (SKEWSAVE.EQ.SKEW .AND. KURTSAVE.EQ.KURT) THEN
            IF (CONVERGED) THEN
               Q = QSAVE
               W = WSAVE
               GOTO 25
            ELSE
               GOTO 30
            END IF
         END IF
         SKEWSAVE = SKEW
         KURTSAVE = KURT
         CONVERGED = .FALSE.
C
         DKDQ0 = -4.D0*W**(7.D0/2.D0)*(((((W+2.D0)*
     >        W+2.D0)*W+1.D0)*W-3.D0)*W-3.D0)
         DB1DQ0 = -6.D0*W**(7.D0/2.D0)*(((W+2.D0)*W-1.D0)*W-2.D0)
         DB1DW0 = 3.D0*W*(2.D0 + W)
         DKDW0 = W*(6.D0 + 6.D0*W + 4.D0*W*W)
         Q = (KURT-TRYKURT)/DKDQ0
         IF (W .LT. 1.2D0) THEN
            MODE = 8
         ELSE IF (W .LT. 1.35D0) THEN
            MODE = 6
         ELSE IF (W .LT. 2.D0) THEN
            MODE = 4
         ELSE
            MODE = 2
         END IF
         CALL HIGHB1(MODE, Q, W, TRYB1, DB1DQ, DB1DW)
         CALL HIGHEXC(MODE, Q, W, TRYKURT, DKDQ, DKDW)
C
C Check that at this value of Q the derivatives did not change
C significantly (in this case we can hope that we are in
C an approximately linear patch of space, and series in Q
C should work). Note that the following test only works when
C the highest degree included in the Q series is even.
C
         IF (.NOT.(DB1DQ/DB1DQ0 .GT. 1.D0/MAXRATIO .AND.
     >        DKDQ/DKDQ0 .GT. 1.D0/MAXRATIO .AND.
     >        DB1DW/DB1DW0 .LT. MAXRATIO. AND.
     >        DKDW/DKDW0 .LT. MAXRATIO)) THEN
            GOTO 30
         END IF
C
C Iterate to get the correct values of Q and W.
         Q = Q/2.D0
 20      NITER = NITER + 1
CC         PRINT *,'Iter ',NITER,', Q = ',Q,', W = ',W
         CALL HIGHB1(MODE, Q, W, TRYB1, DB1DQ, DB1DW)
         CALL HIGHEXC(MODE, Q, W, TRYKURT, DKDQ, DKDW)
         IF (Q .LT. 0.D0 .OR. Q .GT. 1.D0 .OR. W .LT. 1.D0 .OR.
     >        DB1DQ .GT. 0.D0 .OR. DKDQ .GT. 0.D0 .OR.
     >        DB1DW .LT. 0.D0 .OR. DKDW .LT. 0.D0 .OR.
     >        NITER .GE. MAXITER) THEN
C     
C Unlikely to converge to anything reasonable. Abort iterations.
            GOTO 30
         END IF
         IF (DABS(TRYKURT-EXC).GT.TOL2D .OR. 
     >        DABS(TRYB1-B1).GT.TOL2D) THEN
            DTMP = DB1DW*DKDQ - DB1DQ*DKDW
            IF (DTMP .EQ. 0.D0) THEN
C Singular gradient. Just bail out.
               GOTO 30
            END IF
            Q = Q - (DKDW*(B1 - TRYB1) +
     >           DB1DW*(TRYKURT - EXC))/DTMP
            W = W - (DB1DQ*(EXC - TRYKURT) +
     >           DKDQ*(TRYB1 - B1))/DTMP
            GOTO 20
         END IF
C
CC         PRINT *,'Converged to Q = ',Q,', W = ',W
         CONVERGED = .TRUE.
         QSAVE = Q
         WSAVE = W
C
C Calculate distribution parameters
C
 25      DELTA = 1.D0/DSQRT(DLOG(W))
         GAMMA = -DELTA*DLOG(Q)
         XLAM  = SIGMA/DSQRT(HIGHVAR(MODE, Q, W))
         IF (SKEW .GT. 0.D0) THEN
            XI = MEAN - XLAM*HIGHMEAN(MODE, Q, W)
         ELSE
            GAMMA = -GAMMA
            XI = MEAN - XLAM*(1.D0-HIGHMEAN(MODE, Q, W))
         END IF
         FAULT = 0
CC
CC         PRINT *,'SBFITLARGEGAMMA: G is ',GAMMA,', D is ',
CC     >        DELTA,' for SKEW ',SKEW,' and KURT ',KURT
CC
         RETURN
 30   END IF
C
C This is not our special case. Will call the generic routine.
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION HIGHMEAN(MODE, Q, W)
      IMPLICIT NONE
      DOUBLE PRECISION Q, W
      INTEGER MODE
C
C This function calculates second first moment
C of S_b around 0 near the lognormal boundary
C
      INTEGER MAXDEG, USEORDER
      PARAMETER(MAXDEG=12)
      DOUBLE PRECISION DUMM, QC0, QCOEFF(MAXDEG)
C
      USEORDER = MODE
      IF (USEORDER .LT. 1 .OR. USEORDER .GT. MAXDEG) USEORDER = MAXDEG
C
      QC0        = 0.D0
      QCOEFF(1)  = DSQRT(W)
      QCOEFF(2)  = -W**2
      QCOEFF(3)  = W**(9.D0/2.D0)
      QCOEFF(4)  = -W**8
      QCOEFF(5)  = W**(25.D0/2.D0)
      QCOEFF(6)  = -W**18
      QCOEFF(7)  = W**(49.D0/2.D0)
      QCOEFF(8)  = -W**32
      QCOEFF(9)  = W**(81.D0/2.D0)
      QCOEFF(10) = -W**50
      QCOEFF(11) = W**(121.D0/2.D0)
      QCOEFF(12) = -W**72
      CALL POLYVAL(QC0, QCOEFF, USEORDER, Q, HIGHMEAN, DUMM)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION HIGHVAR(MODE, Q, W)
      IMPLICIT NONE
      DOUBLE PRECISION Q, W
      INTEGER MODE
C
C This function calculates second central moment
C of S_b near the lognormal boundary
C
      INTEGER MAXDEG, USEORDER
      PARAMETER(MAXDEG=12)
      DOUBLE PRECISION DUMM, QC0, QCOEFF(MAXDEG)
C
      USEORDER = MODE
      IF (USEORDER .LT. 1 .OR. USEORDER .GT. MAXDEG) USEORDER = MAXDEG
C
C 0th order Q coefficient 
      QC0  = 0.D0
C
C Other coefficients
      QCOEFF(1) = 0.D0
      QCOEFF(2) = (W - 1.D0)*W
      QCOEFF(3) = -2.D0*W**(5.D0/2.D0)*(W*W - 1.D0)
      QCOEFF(4) = W**4*(3.D0*W*W*W*W - 2.D0*W - 1.D0)
      QCOEFF(5) = 2.D0*W**(13.D0/2.D0)*
     >     (-2.D0*W*W*W*W*W*W+W*W+1.D0)
      QCOEFF(6) = W**9*(5.D0*W*W*W*W*W*W*W*W*W-
     >     2.D0*W*W*W*W-2.D0*W-1.D0)
      QCOEFF(7) = 2.D0*W**(25.D0/2.D0)*(1.D0+W*W+W*W*W*W*W*W-
     >     3.D0*W*W*W*W*W*W*W*W*W*W*W*W)
      QCOEFF(8) = W**16*(-1.D0-2.D0*W-2.D0*W*W*W*W-
     >     2.D0*W*W*W*W*W*W*W*W*W+7.D0*W*W*W*W*W*W*W*W*W*W*W*W*W*W*W*W)
      QCOEFF(9) = 2.D0*W**(41.D0/2.D0)*
     >     (1.D0 + W**2 + W**6 + W**12 - 4.D0*W**20)
      QCOEFF(10) = W**25*(-1.D0 - 2.D0*W - 2.D0*W**4 - 2.D0*W**9 - 
     >     2.D0*W**16 + 9.D0*W**25)
      QCOEFF(11) = 2.D0*W**(61.D0/2.D0)*
     >     (1.D0 + W**2 + W**6 + W**12 + W**20 - 5.D0*W**30)
      QCOEFF(12) = W**36*(-1.D0 - 2.D0*W - 2.D0*W**4 - 2.D0*W**9 - 
     >     2.D0*W**16 - 2.D0*W**25 + 11.D0*W**36)
      CALL POLYVAL(QC0, QCOEFF, USEORDER, Q, HIGHVAR, DUMM)
C
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE HIGHB1(MODE, Q, W, B1, DB1DQ, DB1DW)
      IMPLICIT NONE
      DOUBLE PRECISION Q, W, B1, DB1DQ, DB1DW
      INTEGER MODE
C
C This subroutine calculates S_b skewness squared
C near the lognormal boundary
C
      INTEGER MAXN, MAXDEG, USEORDER
      PARAMETER(MAXN=91, MAXDEG=12)
      DOUBLE PRECISION P(MAXN), P0, QC0, DQC0, LEAD, DLEAD
      DOUBLE PRECISION QCOEFF(MAXDEG), QDERIV(MAXDEG)
      INTEGER ORDER, DEG
C
      USEORDER = MODE
      IF (USEORDER .LT. 1 .OR. USEORDER .GT. MAXDEG) USEORDER = MAXDEG
C
C 0th order Q coefficient 
      QC0  = (W - 1.D0)*(W + 2.D0)*(W + 2.D0)
      DQC0 = 3.D0*W*(2.D0 + W)
C
C 1st order Q coefficient
      ORDER = 1
      DEG = 3
      LEAD  = -6.D0*W**(7.D0/2.D0)
      DLEAD = -21.D0*W**(5.D0/2.D0)
      P0    = -2.D0
      P(1)  = -1.D0
      P(2)  = 2.D0
      P(3)  = 1.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 2
      DEG = 6
      LEAD  = 3.D0*W**5
      DLEAD = 15.D0*W**4
      P0    = 10.D0
      P(1)  = 7.D0
      P(2)  = -20.D0
      P(3)  = -22.D0
      P(4)  = 6.D0
      P(5)  = 15.D0
      P(6)  = 4.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 3
      DEG = 10
      LEAD  = -2.D0*W**(13.D0/2.D0)
      DLEAD = -13.D0*W**(11.D0/2.D0)
      P0    = -30.D0
      P(1)  = -30.D0
      P(2)  = 93.D0
      P(3)  = 144.D0
      P(4)  = -37.D0
      P(5)  = -186.D0
      P(6)  = -84.D0
      P(7)  = 42.D0
      P(8)  = 48.D0
      P(9)  = 30.D0
      P(10) = 10.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 4
      DEG = 15
      LEAD  = 3.D0*W**8
      DLEAD = 24.D0*W**7
      P0    = 35.D0
      P(1)  = 50.D0
      P(2)  = -150.D0
      P(3)  = -309.D0
      P(4)  = 74.D0
      P(5)  = 564.D0
      P(6)  = 342.D0
      P(7)  = -262.D0
      P(8)  = -387.D0
      P(9)  = -140.D0
      P(10) = 6.D0
      P(11) = 57.D0
      P(12) = 50.D0
      P(13) = 30.D0
      P(14) = 30.D0
      P(15) = 10.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 5
      DEG = 21
      LEAD  = -6.D0*W**(19.D0/2.D0)
      DLEAD = -57.D0*W**(17.D0/2.D0)
      P0    = -28.D0
      P(1)  = -56.D0
      P(2)  = 154.D0
      P(3)  = 420.D0
      P(4)  = -70.D0
      P(5)  = -995.D0
      P(6)  = -739.D0
      P(7)  = 684.D0
      P(8)  = 1259.D0
      P(9)  = 395.D0
      P(10) = -417.D0
      P(11) = -469.D0
      P(12) = -237.D0
      P(13) = -42.D0
      P(14) = 3.D0
      P(15) = -1.D0
      P(16) = 33.D0
      P(17) = 36.D0
      P(18) = 21.D0
      P(19) = 21.D0
      P(20) = 21.D0
      P(21) = 7.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 6
      DEG = 28
      LEAD  = W**11
      DLEAD = 10.D0*W**10
      P0    = 252.D0
      P(1)  = 686.D0
      P(2)  = -1674.D0
      P(3)  = -6096.D0
      P(4)  = 192.D0
      P(5)  = 17682.D0
      P(6)  = 16299.D0
      P(7)  = -15642.D0
      P(8)  = -35478.D0
      P(9)  = -11988.D0
      P(10) = 20964.D0
      P(11) = 24321.D0
      P(12) = 7614.D0
      P(13) = -4812.D0
      P(14) = -7086.D0
      P(15) = -3921.D0
      P(16) = -1466.D0
      P(17) = -870.D0
      P(18) = 0.D0
      P(19) = 178.D0
      P(20) = -282.D0
      P(21) = -168.D0
      P(22) = 273.D0
      P(23) = 294.D0
      P(24) = 168.D0
      P(25) = 168.D0
      P(26) = 168.D0
      P(27) = 168.D0
      P(28) = 56.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 7
      DEG = 36
      LEAD  = -6.D0*W**(25.D0/2.D0)
      DLEAD = -75.D0*W**(23.D0/2.D0)
      P0    = -60.D0
      P(1)  = -216.D0
      P(2)  = 454.D0
      P(3)  = 2246.D0
      P(4)  = 364.D0
      P(5)  = -7654.D0
      P(6)  = -8844.D0
      P(7)  = 7857.D0
      P(8)  = 22643.D0
      P(9)  = 9117.D0
      P(10) = -18198.D0
      P(11) = -23823.D0
      P(12) = -5625.D0
      P(13) = 10621.D0
      P(14) = 12080.D0
      P(15) = 5443.D0
      P(16) = -282.D0
      P(17) = -2077.D0
      P(18) = -1987.D0
      P(19) = -1615.D0
      P(20) = -595.D0
      P(21) = 181.D0
      P(22) = 4.D0
      P(23) = -174.D0
      P(24) = -8.D0
      P(25) = 26.D0
      P(26) = -58.D0
      P(27) = -100.D0
      P(28) = -36.D0
      P(29) = 60.D0
      P(30) = 64.D0
      P(31) = 36.D0
      P(32) = 36.D0
      P(33) = 36.D0
      P(34) = 36.D0
      P(35) = 36.D0
      P(36) = 12.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 8
      DEG = 45
      LEAD  = 3.D0*W**14
      DLEAD = 42.D0*W**13
      P0    = 165.D0
      P(1)  = 765.D0
      P(2)  = -1332.D0
      P(3)  = -9224.D0
      P(4)  = -3786.D0
      P(5)  = 35783.D0
      P(6)  = 52109.D0
      P(7)  = -39056.D0
      P(8)  = -149618.D0
      P(9)  = -75586.D0
      P(10) = 146309.D0
      P(11) = 222264.D0
      P(12) = 47638.D0
      P(13) = -147026.D0
      P(14) = -162030.D0
      P(15) = -51580.D0
      P(16) = 41048.D0
      P(17) = 61738.D0
      P(18) = 40802.D0
      P(19) = 18082.D0
      P(20) = 2053.D0
      P(21) = -9879.D0
      P(22) = -10581.D0
      P(23) = -4706.D0
      P(24) = -2777.D0
      P(25) = -2507.D0
      P(26) = -437.D0
      P(27) = 1272.D0
      P(28) = 951.D0
      P(29) = -269.D0
      P(30) = -688.D0
      P(31) = -132.D0
      P(32) = 84.D0
      P(33) = -140.D0
      P(34) = -252.D0
      P(35) = -252.D0
      P(36) = -90.D0
      P(37) = 153.D0
      P(38) = 162.D0
      P(39) = 90.D0
      P(40) = 90.D0
      P(41) = 90.D0
      P(42) = 90.D0
      P(43) = 90.D0
      P(44) = 90.D0
      P(45) = 30.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 9
      DEG = 55
      LEAD  = -2.D0*W**(31.D0/2.D0)
      DLEAD = -31.D0*W**(29.D0/2.D0)
      P0    = -330.D0
      P(1)  = -1925.D0
      P(2)  = 2610.D0
      P(3)  = 26691.D0
      P(4)  = 19030.D0
      P(5)  = -114717.D0
      P(6)  = -211338.D0
      P(7)  = 119927.D0
      P(8)  = 661602.D0
      P(9)  = 426405.D0
      P(10) = -735179.D0
      P(11) = -1327434.D0
      P(12) = -313749.D0
      P(13) = 1123470.D0
      P(14) = 1294773.D0
      P(15) = 283509.D0
      P(16) = -621451.D0
      P(17) = -733485.D0
      P(18) = -354930.D0
      P(19) = -1711.D0
      P(20) = 148773.D0
      P(21) = 178935.D0
      P(22) = 144215.D0
      P(23) = 61563.D0
      P(24) = 4026.D0
      P(25) = -5148.D0
      P(26) = -13953.D0
      P(27) = -28428.D0
      P(28) = -25741.D0
      P(29) = -9306.D0
      P(30) = 2043.D0
      P(31) = 1139.D0
      P(32) = -2259.D0
      P(33) = -399.D0
      P(34) = 2584.D0
      P(35) = 2322.D0
      P(36) = 621.D0
      P(37) = -1157.D0
      P(38) = -1545.D0
      P(39) = -222.D0
      P(40) = 183.D0
      P(41) = -249.D0
      P(42) = -465.D0
      P(43) = -465.D0
      P(44) = -465.D0
      P(45) = -165.D0
      P(46) = 285.D0
      P(47) = 300.D0
      P(48) = 165.D0
      P(49) = 165.D0
      P(50) = 165.D0
      P(51) = 165.D0
      P(52) = 165.D0
      P(53) = 165.D0
      P(54) = 165.D0
      P(55) = 55.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 10
      DEG = 66
      LEAD  = 3.D0*W**17
      DLEAD = 51.D0*W**16
      P0    = 286.D0
      P(1)  = 2057.D0
      P(2)  = -1936.D0
      P(3)  = -32520.D0
      P(4)  = -34860.D0
      P(5)  = 150826.D0
      P(6)  = 353338.D0
      P(7)  = -125628.D0
      P(8)  = -1183746.D0
      P(9)  = -974050.D0
      P(10) = 1418658.D0
      P(11) = 3122584.D0
      P(12) = 927403.D0
      P(13) = -3117579.D0
      P(14) = -3935646.D0
      P(15) = -646320.D0
      P(16) = 2687978.D0
      P(17) = 2941146.D0
      P(18) = 991138.D0
      P(19) = -738548.D0
      P(20) = -1200669.D0
      P(21) = -871749.D0
      P(22) = -407430.D0
      P(23) = -14430.D0
      P(24) = 218886.D0
      P(25) = 219889.D0
      P(26) = 137810.D0
      P(27) = 116450.D0
      P(28) = 96601.D0
      P(29) = 29059.D0
      P(30) = -30754.D0
      P(31) = -37672.D0
      P(32) = -17268.D0
      P(33) = -11271.D0
      P(34) = -19970.D0
      P(35) = -18686.D0
      P(36) = -5541.D0
      P(37) = 4698.D0
      P(38) = 6976.D0
      P(39) = 2102.D0
      P(40) = -2441.D0
      P(41) = -920.D0
      P(42) = 1682.D0
      P(43) = 1702.D0
      P(44) = 982.D0
      P(45) = 58.D0
      P(46) = -1174.D0
      P(47) = -1254.D0
      P(48) = -164.D0
      P(49) = 166.D0
      P(50) = -194.D0
      P(51) = -374.D0
      P(52) = -374.D0
      P(53) = -374.D0
      P(54) = -374.D0
      P(55) = -132.D0
      P(56) = 231.D0
      P(57) = 242.D0
      P(58) = 132.D0
      P(59) = 132.D0
      P(60) = 132.D0
      P(61) = 132.D0
      P(62) = 132.D0
      P(63) = 132.D0
      P(64) = 132.D0
      P(65) = 132.D0
      P(66) = 44.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 11
      DEG = 78
      LEAD  = -6.D0*W**(37.D0/2.D0)
      DLEAD = -111.D0*W**(35.D0/2.D0)
      P0    = -182.D0
      P(1)  = -1586.D0
      P(2)  = 781.D0
      P(3)  = 28336.D0
      P(4)  = 42291.D0
      P(5)  = -137784.D0
      P(6)  = -414433.D0
      P(7)  = 49536.D0
      P(8)  = 1464639.D0
      P(9)  = 1530536.D0
      P(10) = -1801393.D0
      P(11) = -4992904.D0
      P(12) = -1957155.D0
      P(13) = 5573072.D0
      P(14) = 7966367.D0
      P(15) = 1230753.D0
      P(16) = -6809648.D0
      P(17) = -7453910.D0
      P(18) = -1670818.D0
      P(19) = 3564318.D0
      P(20) = 4379177.D0
      P(21) = 2262969.D0
      P(22) = 88907.D0
      P(23) = -1015214.D0
      P(24) = -1248207.D0
      P(25) = -897622.D0
      P(26) = -337282.D0
      P(27) = -26122.D0
      P(28) = 26651.D0
      P(29) = 96706.D0
      P(30) = 195808.D0
      P(31) = 186363.D0
      P(32) = 89003.D0
      P(33) = 24720.D0
      P(34) = 30200.D0
      P(35) = 36732.D0
      P(36) = 2230.D0
      P(37) = -35645.D0
      P(38) = -40929.D0
      P(39) = -22212.D0
      P(40) = -2341.D0
      P(41) = 1313.D0
      P(42) = -6165.D0
      P(43) = -7057.D0
      P(44) = -1813.D0
      P(45) = 2279.D0
      P(46) = 4614.D0
      P(47) = 4109.D0
      P(48) = 271.D0
      P(49) = -2275.D0
      P(50) = -985.D0
      P(51) = 875.D0
      P(52) = 1055.D0
      P(53) = 605.D0
      P(54) = 305.D0
      P(55) = -111.D0
      P(56) = -706.D0
      P(57) = -750.D0
      P(58) = -90.D0
      P(59) = 108.D0
      P(60) = -112.D0
      P(61) = -222.D0
      P(62) = -222.D0
      P(63) = -222.D0
      P(64) = -222.D0
      P(65) = -222.D0
      P(66) = -78.D0
      P(67) = 138.D0
      P(68) = 144.D0
      P(69) = 78.D0
      P(70) = 78.D0
      P(71) = 78.D0
      P(72) = 78.D0
      P(73) = 78.D0
      P(74) = 78.D0
      P(75) = 78.D0
      P(76) = 78.D0
      P(77) = 78.D0
      P(78) = 26.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 12
      DEG = 91
      LEAD  = W**20
      DLEAD = 20.D0*W**19
      P0    = 1365.D0
      P(1)  = 14196.D0
      P(2)  = -78.D0
      P(3)  = -283971.D0
      P(4)  = -562931.D0
      P(5)  = 1397418.D0
      P(6)  = 5493786.D0
      P(7)  = 766677.D0
      P(8)  = -20214966.D0
      P(9)  = -26673456.D0
      P(10) = 24054271.D0
      P(11) = 88081932.D0
      P(12) = 45703947.D0
      P(13) = -105557892.D0
      P(14) = -175522173.D0
      P(15) = -32473188.D0
      P(16) = 174415087.D0
      P(17) = 200719128.D0
      P(18) = 30269139.D0
      P(19) = -135656931.D0
      P(20) = -150554658.D0
      P(21) = -54800196.D0
      P(22) = 35179741.D0
      P(23) = 64372509.D0
      P(24) = 50031495.D0
      P(25) = 22287934.D0
      P(26) = -2354235.D0
      P(27) = -14200806.D0
      P(28) = -12641134.D0
      P(29) = -7754682.D0
      P(30) = -5933457.D0
      P(31) = -4209518.D0
      P(32) = -628584.D0
      P(33) = 1954722.D0
      P(34) = 1671745.D0
      P(35) = 398028.D0
      P(36) = 459612.D0
      P(37) = 1423496.D0
      P(38) = 1602261.D0
      P(39) = 792363.D0
      P(40) = -45834.D0
      P(41) = -291714.D0
      P(42) = -60306.D0
      P(43) = 85228.D0
      P(44) = -84909.D0
      P(45) = -269124.D0
      P(46) = -291512.D0
      P(47) = -205308.D0
      P(48) = -44676.D0
      P(49) = 84616.D0
      P(50) = 71052.D0
      P(51) = -14100.D0
      P(52) = -43736.D0
      P(53) = -15102.D0
      P(54) = 11028.D0
      P(55) = 21794.D0
      P(56) = 27363.D0
      P(57) = 20310.D0
      P(58) = -3422.D0
      P(59) = -19080.D0
      P(60) = -8385.D0
      P(61) = 6410.D0
      P(62) = 7686.D0
      P(63) = 4386.D0
      P(64) = 2186.D0
      P(65) = 1086.D0
      P(66) = -798.D0
      P(67) = -5016.D0
      P(68) = -5304.D0
      P(69) = -588.D0
      P(70) = 816.D0
      P(71) = -768.D0
      P(72) = -1560.D0
      P(73) = -1560.D0
      P(74) = -1560.D0
      P(75) = -1560.D0
      P(76) = -1560.D0
      P(77) = -1560.D0
      P(78) = -546.D0
      P(79) = 975.D0
      P(80) = 1014.D0
      P(81) = 546.D0
      P(82) = 546.D0
      P(83) = 546.D0
      P(84) = 546.D0
      P(85) = 546.D0
      P(86) = 546.D0
      P(87) = 546.D0
      P(88) = 546.D0
      P(89) = 546.D0
      P(90) = 546.D0
      P(91) = 182.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      CALL POLYVAL(QC0, QCOEFF, USEORDER, Q, B1, DB1DQ)
      CALL POLYVAL(DQC0, QDERIV, USEORDER, Q, DB1DW, P0)
C
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE HIGHEXC(MODE, Q, W, K, DKDQ, DKDW)
      IMPLICIT NONE
      DOUBLE PRECISION Q, W, K, DKDQ, DKDW
      INTEGER MODE
C
C This subroutine calculates S_b coefficient of excess
C near the lognormal boundary
C
      INTEGER MAXN, MAXDEG, USEORDER
      PARAMETER(MAXN=104, MAXDEG=12)
      DOUBLE PRECISION P(MAXN), P0, QC0, DQC0, LEAD, DLEAD
      DOUBLE PRECISION QCOEFF(MAXDEG), QDERIV(MAXDEG)
      INTEGER ORDER, DEG
C
      USEORDER = MODE
      IF (USEORDER .LT. 1 .OR. USEORDER .GT. MAXDEG) USEORDER = MAXDEG
C
C 0th order Q coefficient 
      QC0  = W*W*(W*(W+2.D0)+3.D0)-6.D0
      DQC0 = W*(6.D0 + 6.D0*W + 4.D0*W*W)
C
C 1st order Q coefficient
      ORDER = 1
      DEG = 5
      LEAD  = -4.D0*W**(7.D0/2.D0)
      DLEAD = -14.0*W**(5.D0/2.D0)
      P0   = -3.D0
      P(1) = -3.D0
      P(2) = 1.D0
      P(3) = 2.D0
      P(4) = 2.D0
      P(5) = 1.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 2
      DEG = 9
      LEAD  = 2.D0*W**5
      DLEAD = 10.D0*W**4
      P0   = 15.D0
      P(1) = 21.D0
      P(2) = -12.D0
      P(3) = -39.D0
      P(4) = -25.D0
      P(5) = -2.D0
      P(6) = 12.D0
      P(7) = 15.D0
      P(8) = 10.D0
      P(9) = 5.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 3
      DEG = 14
      LEAD  = -4.D0*W**(13.D0/2.D0)
      DLEAD = -26.D0*W**(11.D0/2.D0)
      P0    = -15.D0
      P(1)  = -29.D0
      P(2)  = 16.D0
      P(3)  = 81.D0
      P(4)  = 63.D0
      P(5)  = -20.D0
      P(6)  = -66.D0
      P(7)  = -62.D0
      P(8)  = -33.D0
      P(9)  = 0.D0
      P(10) = 15.D0
      P(11) = 20.D0
      P(12) = 15.D0
      P(13) = 10.D0
      P(14) = 5.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 4
      DEG = 20
      LEAD  = W**8
      DLEAD = 8.D0*W**7
      P0    = 105.D0
      P(1)  = 276.D0
      P(2)  = -104.D0
      P(3)  = -982.D0
      P(4)  = -940.D0
      P(5)  = 416.D0
      P(6)  = 1493.D0
      P(7)  = 1222.D0
      P(8)  = 332.D0
      P(9)  = -420.D0
      P(10) = -771.D0
      P(11) = -672.D0
      P(12) = -435.D0
      P(13) = -180.D0
      P(14) = 5.D0
      P(15) = 130.D0
      P(16) = 175.D0
      P(17) = 140.D0
      P(18) = 105.D0
      P(19) = 70.D0
      P(20) = 35.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 5
      DEG = 27
      LEAD  = -4.D0*W**(19.D0/2.D0)
      DLEAD = -38.D0*W**(17.D0/2.D0)
      P0    = -42.D0
      P(1)  = -147.D0
      P(2)  = 15.D0
      P(3)  = 618.D0
      P(4)  = 748.D0
      P(5)  = -334.D0
      P(6)  = -1494.D0
      P(7)  = -1235.D0
      P(8)  = 39.D0
      P(9)  = 962.D0
      P(10) = 1120.D0
      P(11) = 751.D0
      P(12) = 229.D0
      P(13) = -111.D0
      P(14) = -303.D0
      P(15) = -366.D0
      P(16) = -340.D0
      P(17) = -257.D0
      P(18) = -147.D0
      P(19) = -70.D0
      P(20) = 7.D0
      P(21) = 63.D0
      P(22) = 84.D0
      P(23) = 70.D0
      P(24) = 56.D0
      P(25) = 42.D0
      P(26) = 28.D0
      P(27) = 14.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 6
      DEG = 35
      LEAD  = 2.D0*W**11
      DLEAD = 22.D0*W**10
      P0    = 126.D0
      P(1)  = 574.D0
      P(2)  = 141.D0
      P(3)  = -2721.D0
      P(4)  = -4242.D0
      P(5)  = 1480.D0
      P(6)  = 9861.D0
      P(7)  = 8853.D0
      P(8)  = -2122.D0
      P(9)  = -10812.D0
      P(10) = -10104.D0
      P(11) = -3856.D0
      P(12) = 2112.D0
      P(13) = 4851.D0
      P(14) = 4558.D0
      P(15) = 3357.D0
      P(16) = 2028.D0
      P(17) = 827.D0
      P(18) = -42.D0
      P(19) = -593.D0
      P(20) = -902.D0
      P(21) = -1218.D0
      P(22) = -1274.D0
      P(23) = -1022.D0
      P(24) = -672.D0
      P(25) = -434.D0
      P(26) = -196.D0
      P(27) = 42.D0
      P(28) = 224.D0
      P(29) = 294.D0
      P(30) = 252.D0
      P(31) = 210.D0
      P(32) = 168.D0
      P(33) = 126.D0
      P(34) = 84.D0
      P(35) = 42.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 7
      DEG = 44
      LEAD  = -4.D0*W**(25.D0/2.D0)
      DLEAD = -50.D0*W**(23.D0/2.D0)
      P0    = -90.D0
      P(1)  = -522.D0
      P(2)  = -342.D0
      P(3)  = 2685.D0
      P(4)  = 5454.D0
      P(5)  = -979.D0
      P(6)  = -14087.D0
      P(7)  = -14649.D0
      P(8)  = 4741.D0
      P(9)  = 23096.D0
      P(10) = 20086.D0
      P(11) = 1988.D0
      P(12) = -12873.D0
      P(13) = -16005.D0
      P(14) = -10297.D0
      P(15) = -2826.D0
      P(16) = 1774.D0
      P(17) = 3703.D0
      P(18) = 3862.D0
      P(19) = 3011.D0
      P(20) = 2140.D0
      P(21) = 1690.D0
      P(22) = 1522.D0
      P(23) = 992.D0
      P(24) = 334.D0
      P(25) = -96.D0
      P(26) = -354.D0
      P(27) = -696.D0
      P(28) = -1026.D0
      P(29) = -1092.D0
      P(30) = -910.D0
      P(31) = -648.D0
      P(32) = -474.D0
      P(33) = -300.D0
      P(34) = -126.D0
      P(35) = 48.D0
      P(36) = 186.D0
      P(37) = 240.D0
      P(38) = 210.D0
      P(39) = 180.D0
      P(40) = 150.D0
      P(41) = 120.D0
      P(42) = 90.D0
      P(43) = 60.D0
      P(44) = 30.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 8
      DEG = 54
      LEAD  = W**14
      DLEAD = 14.D0*W**13
      P0    = 495.D0
      P(1)  = 3582.D0
      P(2)  = 4023.D0
      P(3)  = -19224.D0
      P(4)  = -51480.D0
      P(5)  = -2106.D0
      P(6)  = 143256.D0
      P(7)  = 179588.D0
      P(8)  = -53457.D0
      P(9)  = -330368.D0
      P(10) = -293644.D0
      P(11) = 32272.D0
      P(12) = 301719.D0
      P(13) = 305476.D0
      P(14) = 123577.D0
      P(15) = -61832.D0
      P(16) = -144304.D0
      P(17) = -131828.D0
      P(18) = -82328.D0
      P(19) = -29828.D0
      P(20) = 7955.D0
      P(21) = 20886.D0
      P(22) = 18113.D0
      P(23) = 13520.D0
      P(24) = 13051.D0
      P(25) = 11340.D0
      P(26) = 7820.D0
      P(27) = 7452.D0
      P(28) = 10581.D0
      P(29) = 11812.D0
      P(30) = 8669.D0
      P(31) = 4224.D0
      P(32) = 1209.D0
      P(33) = -206.D0
      P(34) = -2133.D0
      P(35) = -4572.D0
      P(36) = -6525.D0
      P(37) = -7008.D0
      P(38) = -5997.D0
      P(39) = -4500.D0
      P(40) = -3525.D0
      P(41) = -2550.D0
      P(42) = -1575.D0
      P(43) = -600.D0
      P(44) = 375.D0
      P(45) = 1170.D0
      P(46) = 1485.D0
      P(47) = 1320.D0
      P(48) = 1155.D0
      P(49) = 990.D0
      P(50) = 825.D0
      P(51) = 660.D0
      P(52) = 495.D0
      P(53) = 330.D0
      P(54) = 165.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 9
      DEG = 65
      LEAD  = -4.D0*W**(31.D0/2.D0)
      DLEAD = -62.D0*W**(29.D0/2.D0)
      P0    = -165.D0
      P(1)  = -1463.D0
      P(2)  = -2413.D0
      P(3)  = 7818.D0
      P(4)  = 28141.D0
      P(5)  = 8522.D0
      P(6)  = -82320.D0
      P(7)  = -127571.D0
      P(8)  = 24913.D0
      P(9)  = 258570.D0
      P(10) = 251559.D0
      P(11) = -55702.D0
      P(12) = -336111.D0
      P(13) = -310877.D0
      P(14) = -56256.D0
      P(15) = 173772.D0
      P(16) = 233528.D0
      P(17) = 153848.D0
      P(18) = 39912.D0
      P(19) = -39478.D0
      P(20) = -73650.D0
      P(21) = -69663.D0
      P(22) = -43365.D0
      P(23) = -18165.D0
      P(24) = -3951.D0
      P(25) = 3231.D0
      P(26) = 8770.D0
      P(27) = 9939.D0
      P(28) = 4891.D0
      P(29) = -643.D0
      P(30) = -1398.D0
      P(31) = 1115.D0
      P(32) = 2397.D0
      P(33) = 2001.D0
      P(34) = 1882.D0
      P(35) = 3358.D0
      P(36) = 4974.D0
      P(37) = 5167.D0
      P(38) = 3855.D0
      P(39) = 2082.D0
      P(40) = 1017.D0
      P(41) = 549.D0
      P(42) = -105.D0
      P(43) = -945.D0
      P(44) = -1785.D0
      P(45) = -2475.D0
      P(46) = -2670.D0
      P(47) = -2330.D0
      P(48) = -1815.D0
      P(49) = -1485.D0
      P(50) = -1155.D0
      P(51) = -825.D0
      P(52) = -495.D0
      P(53) = -165.D0
      P(54) = 165.D0
      P(55) = 440.D0
      P(56) = 550.D0
      P(57) = 495.D0
      P(58) = 440.D0
      P(59) = 385.D0
      P(60) = 330.D0
      P(61) = 275.D0
      P(62) = 220.D0
      P(63) = 165.D0
      P(64) = 110.D0
      P(65) = 55.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 10
      DEG = 77
      LEAD  = 2.D0*W**17
      DLEAD = 34.D0*W**16
      P0    = 429.D0
      P(1)  = 4587.D0
      P(2)  = 10252.D0
      P(3)  = -22897.D0
      P(4)  = -114952.D0
      P(5)  = -69535.D0
      P(6)  = 344972.D0
      P(7)  = 673913.D0
      P(8)  = -28074.D0
      P(9)  = -1453736.D0
      P(10) = -1622364.D0
      P(11) = 388893.D0
      P(12) = 2538810.D0
      P(13) = 2314186.D0
      P(14) = -11028.D0
      P(15) = -2050818.D0
      P(16) = -2242561.D0
      P(17) = -1004321.D0
      P(18) = 360274.D0
      P(19) = 1030239.D0
      P(20) = 1008443.D0
      P(21) = 638619.D0
      P(22) = 194690.D0
      P(23) = -128082.D0
      P(24) = -245581.D0
      P(25) = -236991.D0
      P(26) = -200286.D0
      P(27) = -154030.D0
      P(28) = -76263.D0
      P(29) = 7248.D0
      P(30) = 43078.D0
      P(31) = 32141.D0
      P(32) = 16933.D0
      P(33) = 18936.D0
      P(34) = 20576.D0
      P(35) = 7103.D0
      P(36) = -13381.D0
      P(37) = -21357.D0
      P(38) = -12898.D0
      P(39) = -10.D0
      P(40) = 6313.D0
      P(41) = 5030.D0
      P(42) = 4094.D0
      P(43) = 7586.D0
      P(44) = 12304.D0
      P(45) = 15846.D0
      P(46) = 16088.D0
      P(47) = 12122.D0
      P(48) = 7218.D0
      P(49) = 4314.D0
      P(50) = 3110.D0
      P(51) = 1386.D0
      P(52) = -858.D0
      P(53) = -3102.D0
      P(54) = -5346.D0
      P(55) = -7227.D0
      P(56) = -7810.D0
      P(57) = -6919.D0
      P(58) = -5544.D0
      P(59) = -4675.D0
      P(60) = -3806.D0
      P(61) = -2937.D0
      P(62) = -2068.D0
      P(63) = -1199.D0
      P(64) = -330.D0
      P(65) = 539.D0
      P(66) = 1276.D0
      P(67) = 1573.D0
      P(68) = 1430.D0
      P(69) = 1287.D0
      P(70) = 1144.D0
      P(71) = 1001.D0
      P(72) = 858.D0
      P(73) = 715.D0
      P(74) = 572.D0
      P(75) = 429.D0
      P(76) = 286.D0
      P(77) = 143.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD      
C
      ORDER = 11
      DEG = 90
      LEAD  = -4.D0*W**(37.D0/2.D0)
      DLEAD = -74.D0*W**(35.D0/2.D0)
      P0    = -273.D0
      P(1)  = -3471.D0
      P(2)  = -10002.D0
      P(3)  = 14619.D0
      P(4)  = 110360.D0
      P(5)  = 105019.D0
      P(6)  = -330000.D0
      P(7)  = -830587.D0
      P(8)  = -123826.D0
      P(9)  = 1861220.D0
      P(10) = 2460037.D0
      P(11) = -455069.D0
      P(12) = -4244810.D0
      P(13) = -4056814.D0
      P(14) = 511595.D0
      P(15) = 4710014.D0
      P(16) = 4638423.D0
      P(17) = 1165971.D0
      P(18) = -2210291.D0
      P(19) = -3262764.D0
      P(20) = -2277047.D0
      P(21) = -669060.D0
      P(22) = 561572.D0
      P(23) = 1108838.D0
      P(24) = 1031437.D0
      P(25) = 637190.D0
      P(26) = 282409.D0
      P(27) = 71170.D0
      P(28) = -80879.D0
      P(29) = -211069.D0
      P(30) = -252364.D0
      P(31) = -183549.D0
      P(32) = -95165.D0
      P(33) = -58686.D0
      P(34) = -49950.D0
      P(35) = -16112.D0
      P(36) = 36559.D0
      P(37) = 64574.D0
      P(38) = 48127.D0
      P(39) = 13692.D0
      P(40) = -4336.D0
      P(41) = -1521.D0
      P(42) = 4791.D0
      P(43) = -103.D0
      P(44) = -11338.D0
      P(45) = -18462.D0
      P(46) = -17608.D0
      P(47) = -9347.D0
      P(48) = 711.D0
      P(49) = 4729.D0
      P(50) = 3088.D0
      P(51) = 1749.D0
      P(52) = 3693.D0
      P(53) = 6847.D0
      P(54) = 9651.D0
      P(55) = 11706.D0
      P(56) = 11629.D0
      P(57) = 9056.D0
      P(58) = 5769.D0
      P(59) = 3846.D0
      P(60) = 3089.D0
      P(61) = 1980.D0
      P(62) = 519.D0
      P(63) = -942.D0
      P(64) = -2403.D0
      P(65) = -3864.D0
      P(66) = -5109.D0
      P(67) = -5522.D0
      P(68) = -4951.D0
      P(69) = -4056.D0
      P(70) = -3497.D0
      P(71) = -2938.D0
      P(72) = -2379.D0
      P(73) = -1820.D0
      P(74) = -1261.D0
      P(75) = -702.D0
      P(76) = -143.D0
      P(77) = 416.D0
      P(78) = 897.D0
      P(79) = 1092.D0
      P(80) = 1001.D0
      P(81) = 910.D0
      P(82) = 819.D0
      P(83) = 728.D0
      P(84) = 637.D0
      P(85) = 546.D0
      P(86) = 455.D0
      P(87) = 364.D0
      P(88) = 273.D0
      P(89) = 182.D0
      P(90) = 91.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      ORDER = 12
      DEG = 104
      LEAD  = W**20
      DLEAD = 20.D0*W**19
      P0    = 1365.D0
      P(1)  = 20384.D0
      P(2)  = 73190.D0
      P(3)  = -58902.D0
      P(4)  = -800727.D0
      P(5)  = -1083084.D0
      P(6)  = 2291043.D0
      P(7)  = 7675504.D0
      P(8)  = 2849775.D0
      P(9)  = -17499840.D0
      P(10) = -28049831.D0
      P(11) = 2189408.D0
      P(12) = 51413793.D0
      P(13) = 54071644.D0
      P(14) = -9397926.D0
      P(15) = -74198502.D0
      P(16) = -70333432.D0
      P(17) = -5783544.D0
      P(18) = 54059649.D0
      P(19) = 63596638.D0
      P(20) = 31318454.D0
      P(21) = -7259700.D0
      P(22) = -28079642.D0
      P(23) = -28923780.D0
      P(24) = -18048330.D0
      P(25) = -4624552.D0
      P(26) = 4389234.D0
      P(27) = 7126998.D0
      P(28) = 6577811.D0
      P(29) = 5585868.D0
      P(30) = 4281057.D0
      P(31) = 2100616.D0
      P(32) = -59372.D0
      P(33) = -895908.D0
      P(34) = -696756.D0
      P(35) = -680972.D0
      P(36) = -1206699.D0
      P(37) = -1540804.D0
      P(38) = -1141838.D0
      P(39) = -348246.D0
      P(40) = 151579.D0
      P(41) = 153276.D0
      P(42) = 11811.D0
      P(43) = 52140.D0
      P(44) = 250266.D0
      P(45) = 375312.D0
      P(46) = 316215.D0
      P(47) = 130936.D0
      P(48) = -59415.D0
      P(49) = -131568.D0
      P(50) = -70853.D0
      P(51) = 2268.D0
      P(52) = 1167.D0
      P(53) = -49788.D0
      P(54) = -91125.D0
      P(55) = -104564.D0
      P(56) = -89528.D0
      P(57) = -43920.D0
      P(58) = 4613.D0
      P(59) = 23204.D0
      P(60) = 12240.D0
      P(61) = 2444.D0
      P(62) = 12327.D0
      P(63) = 28788.D0
      P(64) = 43379.D0
      P(65) = 56100.D0
      P(66) = 65067.D0
      P(67) = 65098.D0
      P(68) = 52013.D0
      P(69) = 34836.D0
      P(70) = 24891.D0
      P(71) = 21154.D0
      P(72) = 15561.D0
      P(73) = 8112.D0
      P(74) = 663.D0
      P(75) = -6786.D0
      P(76) = -14235.D0
      P(77) = -21684.D0
      P(78) = -28119.D0
      P(79) = -30368.D0
      P(80) = -27495.D0
      P(81) = -22932.D0
      P(82) = -20111.D0
      P(83) = -17290.D0
      P(84) = -14469.D0
      P(85) = -11648.D0
      P(86) = -8827.D0
      P(87) = -6006.D0
      P(88) = -3185.D0
      P(89) = -364.D0
      P(90) = 2457.D0
      P(91) = 4914.D0
      P(92) = 5915.D0
      P(93) = 5460.D0
      P(94) = 5005.D0
      P(95) = 4550.D0
      P(96) = 4095.D0
      P(97) = 3640.D0
      P(98) = 3185.D0
      P(99) = 2730.D0
      P(100) = 2275.D0
      P(101) = 1820.D0
      P(102) = 1365.D0
      P(103) = 910.D0
      P(104) = 455.D0
C      CALL CHECKCOE(P0, P, DEG)
      CALL POLYVAL(P0, P, DEG, W, QCOEFF(ORDER), QDERIV(ORDER))
      QDERIV(ORDER) = QDERIV(ORDER)*LEAD + QCOEFF(ORDER)*DLEAD
      QCOEFF(ORDER) = QCOEFF(ORDER)*LEAD
C
      CALL POLYVAL(QC0, QCOEFF, USEORDER, Q, K, DKDQ)
      CALL POLYVAL(DQC0, QDERIV, USEORDER, Q, DKDW, P0)
C
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE POLYVAL(A0, A, N, X, VALUE, DERIV)
      IMPLICIT NONE
      DOUBLE PRECISION A0, A(*), X, VALUE, DERIV
      INTEGER I, N
C
      IF (N .EQ. 0) THEN
         VALUE = 0.D0
         DERIV = 0.D0
      ELSE IF (N .LT. 0) THEN
         PRINT *,'Bug: fortran routine POLYVAL called with negative N.'
         PRINT *,'Program stopped.'
         STOP
      ELSE
         VALUE = A(N)
         DERIV = N*A(N)
      END IF
C
      DO I=N-1,1,-1
         VALUE = VALUE*X + A(I)
         DERIV = DERIV*X + I*A(I)
      END DO
      VALUE = VALUE*X + A0
C
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE MOMDER(G, D, MEAN, VAR, SKEW, KURT, DERIV, FAULT)
      IMPLICIT NONE
      DOUBLE PRECISION DERIV(*),G,D,MEAN,VAR,SKEW,KURT
      LOGICAL FAULT
C
      FAULT = .FALSE.
      CALL JOHNINTF(G, D, MEAN, VAR, SKEW, KURT,
     >     DERIV(2), DERIV(1), DERIV(4), DERIV(3))
      IF (G .LT. 0.D0) THEN
         DERIV(2) = -DERIV(2)
         DERIV(3) = -DERIV(3)
      END IF
C
      RETURN
      END
CCCCC
CCCCC
C
C The following code is currently not used.
C It is numerically unstable.
C
      SUBROUTINE MOM_ORIG(G, D, A, FAULT)
      IMPLICIT NONE
C
C        ALGORITHM AS 99.3  APPL. STATIST. (1976) VOL.25, P.180
C
C        EVALUATES FIRST SIX MOMENTS OF A JOHNSON
C        SB DISTRIBUTION, USING GOODWIN METHOD
C
      DOUBLE PRECISION A(6),B(6),C(6),G,D,ZZ,VV,RTTWO,RRTPI,W,E,R,
     $  H, T, U, Y, X, V, F, Z, S, P, Q, AA, AB, EXPA, EXPB,
     $  ZERO, QUART, HALF, P75, ONE, TWO, THREE
      LOGICAL L, FAULT
      INTEGER I,K,M,LIMIT
C
      DATA ZZ, VV, LIMIT /1.0D-5, 1.0D-8, 1000/
C
C        RTTWO IS DSQRT(2.0)
C        RRTPI IS RECIPROCAL OF DSQRT(PI)
C        EXPA IS A VALUE SUCH THAT EXP(EXPA) DOES NOT QUITE
C          CAUSE OVERFLOW
C        EXPB IS A VALUE SUCH THAT 1.0 + EXP(-EXPB) MAY BE
C          TAKEN TO BE 1.0
C
      DATA  RTTWO, RRTPI, EXPA, EXPB
     $  /1.414213562D0, 0.5641895835D0, 80.D0, 23.7D0/
      DATA ZERO, QUART, HALF,  P75, ONE, TWO, THREE
     $     /0.D0,  0.25D0,  0.5D0, 0.75D0, 1.D0, 2.D0,3.D0/
C
      FAULT = .FALSE.
      DO 10 I = 1, 6
   10 C(I) = ZERO
      W = G / D
C
C        TRIAL VALUE OF H
C
      IF (W .GT. EXPA) GOTO 140
      E = DEXP(W) + ONE
      R = RTTWO / D
      H = P75
      IF (D .LT. THREE) H = QUART * D
      K = 1
      GOTO 40
C
C        START OF OUTER LOOP
C
   20 K = K + 1
      IF (K .GT. LIMIT) GOTO 140
      DO 30 I = 1, 6
   30 C(I) = A(I)
C
C        NO CONVERGENCE YET - TRY SMALLER H
C
      H = HALF * H
   40 T = W
      U = T
      Y = H * H
      X = TWO * Y
      A(1) = ONE / E
      DO 50 I = 2, 6
   50 A(I) = A(I - 1) / E
      V = Y
      F = R * H
      M = 0
C
C        START OF INNER LOOP
C        TO EVALUATE INFINITE SERIES
C
   60 M = M + 1
      IF (M .GT. LIMIT) GOTO 140
      DO 70 I = 1, 6
   70 B(I) = A(I)
      U = U - F
      Z = ONE
      IF (U .GT. -EXPB) Z = DEXP(U) + Z
      T = T + F
      L = T .GT. EXPB
      IF (.NOT. L) S = DEXP(T) + ONE
      P = DEXP(-V)
      Q = P
      DO 90 I = 1, 6
      AA = A(I)
      P = P / Z
      AB = AA
      AA = AA + P
      IF (AA .EQ. AB) GOTO 100
      IF (L) GOTO 80
      Q = Q / S
      AB = AA
      AA = AA + Q
      L = AA .EQ. AB
   80 A(I) = AA
   90 CONTINUE
  100 Y = Y + X
      V = V + Y
      DO 110 I = 1, 6
      IF (A(I) .EQ. ZERO) GOTO 140
      IF (DABS((A(I) - B(I)) / A(I)) .GT. VV) GOTO 60
  110 CONTINUE
C
C        END OF INNER LOOP
C
      V = RRTPI * H
      DO 120 I = 1, 6
  120 A(I) = V * A(I)
      DO 130 I = 1, 6
      IF (A(I) .EQ. ZERO) GOTO 140
      IF (DABS((A(I) - C(I)) / A(I)) .GT. ZZ) GOTO 20
  130 CONTINUE
C
C        END OF OUTER LOOP
C
      RETURN
  140 FAULT =.TRUE.
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSB4M(X,Y,Z,MODE,DPAR,ERRSTA)
C
C  This function is a Johnson's S_b type curve represented
C  using four moments
C
C     X      is the first variable
C     Y      is the second variable
C     Z      is the third variable
C     MODE   is the calculation mode (may be ignored)
C     DPAR   are the parameters in DOUBLE PRECISION
C
C     Johnson's S_b (moments) needs 5 parameters:
C         norm, mean, sigma, skew, and kurt
C
C     ERRSTA is the error status of the calculation (0 means OK)
C
      IMPLICIT NONE
      DOUBLE PRECISION JOHNSB
      EXTERNAL JOHNSB
      DOUBLE PRECISION X,Y,Z
      INTEGER MODE,I,ERRSTA
      DOUBLE PRECISION DPAR(*),NORM,MEAN,SIGMA,SKEW,KURT
      DOUBLE PRECISION XI,LAMBDA,GAMMA,DELTA,DTMP(5)
      DATA DTMP/0.D0,0.D0,0.D0,0.D0,0.D0/
      SAVE XI,LAMBDA,GAMMA,DELTA,DTMP
      DOUBLE PRECISION SQR2PI
      PARAMETER(SQR2PI=2.506628274631D0)
      LOGICAL ASOLD
      INTEGER IERR
      DATA IERR/0/
      SAVE IERR
C
      ASOLD=.TRUE.
      DO I=2,5
         IF (DPAR(I).NE.DTMP(I)) THEN
            ASOLD=.FALSE.
            DTMP(I)=DPAR(I)
         END IF
      END DO
C
      NORM=DPAR(1)
C
C Pass local copies of the parameters to SBFIT to make sure that
C the argument parameter values are not modified.
      IF (.NOT.ASOLD) THEN
         MEAN=DPAR(2)
         SIGMA=DPAR(3)
         SKEW=DPAR(4)
         KURT=DPAR(5)
         CALL SBFITMOD(MODE,MEAN,SIGMA,SKEW,KURT,
     >                 GAMMA,DELTA,LAMBDA,XI,IERR)
CC         PRINT *,'Sbfit returned ',GAMMA,DELTA,' for ',SKEW,KURT
      END IF
C     
      IF (IERR .EQ. 0) THEN
         JSB4M=JOHNSB(X,XI,LAMBDA,GAMMA,DELTA)*NORM
      ELSE IF (DTMP(3).GT.0.D0 .AND. DTMP(4).EQ.0.D0 .AND.
     >        DTMP(5).EQ.3.D0) THEN
         JSB4M=DEXP(-((X-DTMP(2))/DTMP(3))**2/2.D0)/DTMP(3)/SQR2PI*
     >        NORM
      ELSE
         JSB4M=0.D0
      END IF
C
      RETURN
      END
CCCCC
CCCCC
      INTEGER FUNCTION JOHNSEL(SKEW,KURT)
C
C This function returns the type of Johnson's curve to be fitted
C for given values of skewness and kurtosis. The return values are:
C
C   0   - Skewness and kurtosis form an impossible combination
C   1   - Gaussian
C   2   - Lognormal
C   3   - Johnson's S_b
C   4   - Johnson's S_u
C
      IMPLICIT NONE
      DOUBLE PRECISION SKEW,KURT
C
      DOUBLE PRECISION ONE,TWO,THREE,FOUR
      PARAMETER(ONE=1.D0,TWO=2.D0,THREE=3.D0,FOUR=4.D0)
      DOUBLE PRECISION TOL
      PARAMETER(TOL=1.D-10)
      DOUBLE PRECISION B1,TMP,W
C
      B1=SKEW*SKEW
C
C Check for impossible combination
      IF (KURT .LE. B1+ONE) THEN
         JOHNSEL = 0
         RETURN
      END IF
C
C Check for Gaussian
      IF (B1.LT.TOL .AND. DABS(KURT-THREE).LT.TOL) THEN
         JOHNSEL = 1
         RETURN
      END IF
C
C Check for lognormal, S_b, and S_u
      TMP=((TWO+B1+DSQRT(B1*(FOUR+B1)))/TWO)**(ONE/THREE)
      W=TMP+ONE/TMP-ONE
      TMP=W*W*(W*(W+TWO)+THREE)-THREE
C
      IF (DABS(KURT-TMP) .LT. TOL) THEN
         JOHNSEL = 2
      ELSE IF (KURT .LT. TMP) THEN
         JOHNSEL = 3
      ELSE
         JOHNSEL = 4
      END IF
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JOHNSYS(X,Y,Z,MODE,DPAR,ERRSTA)
C
C  Complete Johnson's system represented using four moments.
C  Needs 5 parameters: norm, mean, sigma, skew, and kurt
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA,STATUS
C
      DOUBLE PRECISION SQR2PI
      PARAMETER(SQR2PI=2.506628274631D0)
      LOGICAL PRINTCHOICE
      PARAMETER(PRINTCHOICE=.FALSE.)
C
      DOUBLE PRECISION JOHN4M, JSB4M, LOGNORM
      INTEGER JOHNSEL
      EXTERNAL JOHN4M, JSB4M, LOGNORM, JOHNSEL
C
      DOUBLE PRECISION DIFF,MEAN,WIDTH,SKEW,KURT
C
      MEAN  = DPAR(2)
      WIDTH = DPAR(3)
      IF (WIDTH .LE. 0.D0) THEN
          JOHNSYS = 0.D0
          RETURN
      END IF
C
      SKEW  = DPAR(4)
      KURT  = DPAR(5)
      STATUS = JOHNSEL(SKEW,KURT)
      IF (STATUS.EQ.0) THEN
         IF (PRINTCHOICE) THEN
            PRINT *,'JOHNSYS: impossible skewness/kurtosis combination.'
         END IF
         JOHNSYS = 0.D0
      ELSEIF (STATUS.EQ.1) THEN
         IF (PRINTCHOICE) THEN
            PRINT *,'JOHNSYS: using Gaussian distribution.'
         END IF
         DIFF = (X-MEAN)/WIDTH
         JOHNSYS = DPAR(1)/SQR2PI/WIDTH*DEXP(-DIFF*DIFF/2.D0)
      ELSEIF (STATUS.EQ.2) THEN
         IF (PRINTCHOICE) THEN
            PRINT *,'JOHNSYS: using lognormal distribution.'
         END IF
         JOHNSYS = LOGNORM(X,Y,Z,MODE,DPAR,ERRSTA)
      ELSEIF (STATUS.EQ.3) THEN
         IF (PRINTCHOICE) THEN
            PRINT *,'JOHNSYS: using S_b distribution.'
         END IF
         JOHNSYS = JSB4M(X,Y,Z,MODE,DPAR,ERRSTA)
      ELSEIF (STATUS.EQ.4) THEN
         IF (PRINTCHOICE) THEN
            PRINT *,'JOHNSYS: using S_u distribution.'
         END IF
         JOHNSYS = JOHN4M(X,Y,Z,MODE,DPAR,ERRSTA)
      ELSE
         PRINT *,'JOHNSYS: invalid branch. This is a bug.'
         PRINT *,'Please report. Program stopped.'
         STOP
      END IF
C
      RETURN
      END
CCCCC
CCCCC
      SUBROUTINE CHECKCOE(P0, P, DEG)
      IMPLICIT NONE
      DOUBLE PRECISION SUM, P0, P(*)
      INTEGER I, DEG
C
      SUM = P0
      DO I=1,DEG
         SUM = SUM+P(I)
      END DO
      IF (DABS(SUM) .GT. 0.2D0) THEN
         PRINT *,'Checksum failed for degree ',DEG,'. Stopping.'
         STOP
      END IF
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBHB1Q(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DB1DQ, DB1DW
C
      CALL HIGHB1(MODE, X, DPAR(1), JSBHB1Q, DB1DQ, DB1DW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBHB1W(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DB1DQ, DB1DW
C
      CALL HIGHB1(MODE, DPAR(1), X, JSBHB1W, DB1DQ, DB1DW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBHEXCQ(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DKDQ, DKDW
C
      CALL HIGHEXC(MODE, X, DPAR(1), JSBHEXCQ, DKDQ, DKDW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBHEXCW(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DKDQ, DKDW
C
      CALL HIGHEXC(MODE, DPAR(1), X, JSBHEXCW, DKDQ, DKDW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBDB1DQ(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DUMM, DB1DW
C
      CALL HIGHB1(MODE, X, Y, DUMM, JSBDB1DQ, DB1DW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBDB1DW(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DUMM, DB1DQ
C
      CALL HIGHB1(MODE, X, Y, DUMM, DB1DQ, JSBDB1DW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBDKDQ(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DUMM, DB1DW
C
      CALL HIGHEXC(MODE, X, Y, DUMM, JSBDKDQ, DB1DW)
C
      RETURN
      END
CCCCC
CCCCC
      DOUBLE PRECISION FUNCTION JSBDKDW(X,Y,Z,MODE,DPAR,ERRSTA)
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
      DOUBLE PRECISION DUMM, DB1DQ
C
      CALL HIGHEXC(MODE, X, Y, DUMM, DB1DQ, JSBDKDW)
C
      RETURN
      END
