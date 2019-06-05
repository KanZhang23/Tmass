C
C  Interface to the linear least squares fitting package TL.
C  See the description of CERNLIB entry E230.
C
      SUBROUTINE TLSSET(M1S, MS, NS, LS, IERS)

      INTEGER M1S, MS, NS, LS, IERS
      INTEGER M1, M, N, L, IER
      COMMON /TLSDIM/ M1, M, N, L, IER
      SAVE /TLSDIM/
C
C  N   is the number of variables
C  M   is the number of equations
C  M1  is the number of constraints
C  L   is the number of equation systems to solve (keep at 1 for simplicity)
C  IER returns the number of parameters solved, or -1001 in case of error
C
      M1 = M1S
      M = MS
      N = NS
      L = LS
      IER = IERS

      RETURN
      END
C
      SUBROUTINE TLSGET(M1S, MS, NS, LS, IERS)

      INTEGER M1S, MS, NS, LS, IERS
      INTEGER M1, M, N, L, IER
      COMMON /TLSDIM/ M1, M, N, L, IER
      
      M1S = M1
      MS = M
      NS = N
      LS = L
      IERS = IER

      RETURN
      END
