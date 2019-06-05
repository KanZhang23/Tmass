      DOUBLE PRECISION FUNCTION LRCRISE(X,Y,Z,MODE,DPAR,ERRSTA)
C
C  LRC filter risetime needs 5 parameters: H_l, H_r, t0, beta, and omega
C
      IMPLICIT NONE
      DOUBLE PRECISION X,Y,Z,DPAR(*)
      INTEGER MODE,ERRSTA
C
      DOUBLE PRECISION DT,LO,HI,T0,BETA,OMEGA
      DOUBLE PRECISION LRCRT
      EXTERNAL LRCRT
C
      LO = DPAR(1)
      HI = DPAR(2)
      T0 = DPAR(3)
      BETA = DPAR(4)
      OMEGA = DPAR(5)
C
      IF (BETA.LT.0.D0 .OR. OMEGA.LE.-BETA) THEN
         LRCRISE = 0.D0
      ELSE
         DT = X - T0
         LRCRISE = LO + (HI-LO)*LRCRT(DT,BETA,OMEGA)
      END IF
C
      RETURN
      END
CCCC
CCCC
CCCC
      DOUBLE PRECISION FUNCTION LRCRT(T,BETA,OMEGA)
C
C This is a response of an L-R-C circuit to a unit voltage step
C at time T0 = 0 taken as voltage across the capacitor
C
C Arguments:
C
C     T     - time
C
C     BETA  - damping parameter. For a real circuit BETA is positive 
C             and equals R/(2*L)
C
C     OMEGA - resonance frequency. For a real circuit OMEGA equals 
C             SQRT(OMEGA0**2 - BETA**2), where OMEGA0 is the resonant
C             frequency of the LC circuit: OMEGA0 = 1/SQRT(L*C).
C             This subroutine also accepts negative values of OMEGA.
C
      IMPLICIT NONE
C
      DOUBLE PRECISION T,BETA,OMEGA,BETAT,OMEGAT,EBT,EOT,EMOT
      DOUBLE PRECISION EPS
      PARAMETER(EPS=1.0D-6)
C
      IF (T.LE.0.D0) THEN
         LRCRT = 0.D0
         RETURN
      END IF
C
      BETAT = BETA*T
      EBT = DEXP(-BETAT)
C
      IF (OMEGA.EQ.0.D0) THEN
         LRCRT = 1.D0 - EBT*(1.D0 + BETAT)
         RETURN
      END IF
C
      OMEGAT = T*DABS(OMEGA)
C
      IF (OMEGAT.LT.EPS) THEN
         IF (OMEGA.GT.0.D0) THEN
            LRCRT = 1.D0 - EBT*(BETAT*(1.D0-OMEGAT*OMEGAT/6.D0) + 
     >           1.D0 - OMEGAT*OMEGAT/2.D0)
         ELSE
            LRCRT = 1.D0 - EBT*(BETAT*(1.D0+OMEGAT*OMEGAT/6.D0) + 
     >           1.D0 + OMEGAT*OMEGAT/2.D0)            
         END IF
      ELSE
         IF (OMEGA.GT.0.D0) THEN
            LRCRT = 1.D0 - EBT*(BETA/OMEGA*DSIN(OMEGAT)+DCOS(OMEGAT))
         ELSE
            EOT = DEXP(OMEGAT)
            EMOT = 1.D0/EOT
            LRCRT = 1.D0 - EBT/2.D0*(BETA/OMEGA*(EMOT-EOT)+EOT+EMOT)
         END IF
      END IF
C
      RETURN
      END
