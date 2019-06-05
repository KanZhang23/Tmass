      DOUBLE PRECISION FUNCTION P0_DSIG71(PP,WGT,IMODE)
C     ****************************************************
C     
C     Generated by MadGraph5_aMC@NLO v. 2.2.2, 2014-11-06
C     By the MadGraph5_aMC@NLO Development Team
C     Visit launchpad.net/madgraph5 and amcatnlo.web.cern.ch
C     
C     Process: c s~ > w+ u c u~ c~ WEIGHTED=6
C     *   Decay: w+ > e+ ve WEIGHTED=2
C     
C     RETURNS DIFFERENTIAL CROSS SECTION
C     Input:
C     pp    4 momentum of external particles
C     wgt   weight from Monte Carlo
C     Output:
C     Amplitude squared and summed
C     ****************************************************
      IMPLICIT NONE
C     
C     CONSTANTS
C     
      INCLUDE 'phasespace.inc'
      INCLUDE 'nexternal.inc'
      INCLUDE 'maxamps.inc'
      DOUBLE PRECISION       CONV
      PARAMETER (CONV=1D0)  !NO CONV (GeV^n)
      REAL*8     PI
      PARAMETER (PI=3.1415926D0)
C     
C     ARGUMENTS 
C     
      DOUBLE PRECISION PP(0:3,NEXTERNAL), WGT
      INTEGER IMODE
C     
C     LOCAL VARIABLES 
C     
      INTEGER I,ITYPE,LP
      DOUBLE PRECISION U1,UX1,D1,DX1,C1,CX1,S1,SX1,B1,BX1
      DOUBLE PRECISION U2,UX2,D2,DX2,C2,CX2,S2,SX2,B2,BX2
      DOUBLE PRECISION G1,G2
      DOUBLE PRECISION A1,A2
      DOUBLE PRECISION XPQ(-7:7)
      DOUBLE PRECISION DSIGUU
C     
C     EXTERNAL FUNCTIONS
C     
      DOUBLE PRECISION PDG2PDF
C     
C     GLOBAL VARIABLES
C     
      INTEGER              IPROC
      DOUBLE PRECISION PD(0:MAXPROC)
      COMMON /SUBPROC/ PD, IPROC

      INTEGER SUBDIAG(MAXSPROC),IB(2)
      COMMON/TO_SUB_DIAG0/SUBDIAG,IB
      INCLUDE 'coupl.inc'
      INCLUDE 'run.inc'
C     
C     DATA
C     
      DATA U1,UX1,D1,DX1,C1,CX1,S1,SX1,B1,BX1/10*1D0/
      DATA U2,UX2,D2,DX2,C2,CX2,S2,SX2,B2,BX2/10*1D0/
      DATA A1,G1/2*1D0/
      DATA A2,G2/2*1D0/
C     ----------
C     BEGIN CODE
C     ----------
      P0_DSIG71=0D0



      IF (ABS(LPP(IB(1))).GE.1) THEN
        LP=SIGN(1,LPP(IB(1)))
        C1=PDG2PDF(ABS(LPP(IB(1))),4*LP,XBK(IB(1)),DSQRT(Q2FACT(1)))
      ENDIF
      IF (ABS(LPP(IB(2))).GE.1) THEN
        LP=SIGN(1,LPP(IB(2)))
        SX2=PDG2PDF(ABS(LPP(IB(2))),-3*LP,XBK(IB(2)),DSQRT(Q2FACT(2)))
      ENDIF
      PD(0) = 0D0
      IPROC = 0
      IPROC=IPROC+1  ! c s~ > e+ ve u c u~ c~
      PD(IPROC)=C1*SX2
      PD(0)=PD(0)+DABS(PD(IPROC))
      CALL SP0_MATRIX71(PP,DSIGUU)
      IF (DSIGUU.LT.1D199) THEN
        P0_DSIG71=PD(0)*CONV*DSIGUU
      ELSE
        WRITE(*,*) 'Error in matrix element'
        DSIGUU=0D0
        P0_DSIG71=0D0
      ENDIF
      END

