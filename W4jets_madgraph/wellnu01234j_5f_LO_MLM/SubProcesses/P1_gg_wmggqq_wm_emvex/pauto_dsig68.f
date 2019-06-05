      DOUBLE PRECISION FUNCTION P1_DSIG68(PP,WGT,IMODE)
C     ****************************************************
C     
C     Generated by MadGraph5_aMC@NLO v. 2.2.2, 2014-11-06
C     By the MadGraph5_aMC@NLO Development Team
C     Visit launchpad.net/madgraph5 and amcatnlo.web.cern.ch
C     
C     Process: d u~ > w- c s c~ s~ WEIGHTED=6 @1
C     *   Decay: w- > e- ve~ WEIGHTED=2
C     Process: d u~ > w- c b c~ b~ WEIGHTED=6 @1
C     *   Decay: w- > e- ve~ WEIGHTED=2
C     Process: d u~ > w- s b s~ b~ WEIGHTED=6 @1
C     *   Decay: w- > e- ve~ WEIGHTED=2
C     Process: s c~ > w- u b u~ b~ WEIGHTED=6 @1
C     *   Decay: w- > e- ve~ WEIGHTED=2
C     Process: s c~ > w- d b d~ b~ WEIGHTED=6 @1
C     *   Decay: w- > e- ve~ WEIGHTED=2
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
      DOUBLE PRECISION P1_DSIGUU
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
      COMMON/TO_SUB_DIAG/SUBDIAG,IB
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
      P1_DSIG68=0D0



      IF (ABS(LPP(IB(1))).GE.1) THEN
        LP=SIGN(1,LPP(IB(1)))
        D1=PDG2PDF(ABS(LPP(IB(1))),1*LP,XBK(IB(1)),DSQRT(Q2FACT(1)))
        S1=PDG2PDF(ABS(LPP(IB(1))),3*LP,XBK(IB(1)),DSQRT(Q2FACT(1)))
      ENDIF
      IF (ABS(LPP(IB(2))).GE.1) THEN
        LP=SIGN(1,LPP(IB(2)))
        CX2=PDG2PDF(ABS(LPP(IB(2))),-4*LP,XBK(IB(2)),DSQRT(Q2FACT(2)))
        UX2=PDG2PDF(ABS(LPP(IB(2))),-2*LP,XBK(IB(2)),DSQRT(Q2FACT(2)))
      ENDIF
      PD(0) = 0D0
      IPROC = 0
      IPROC=IPROC+1  ! d u~ > e- ve~ c s c~ s~
      PD(IPROC)=D1*UX2
      PD(0)=PD(0)+DABS(PD(IPROC))
      IPROC=IPROC+1  ! d u~ > e- ve~ c b c~ b~
      PD(IPROC)=D1*UX2
      PD(0)=PD(0)+DABS(PD(IPROC))
      IPROC=IPROC+1  ! d u~ > e- ve~ s b s~ b~
      PD(IPROC)=D1*UX2
      PD(0)=PD(0)+DABS(PD(IPROC))
      IPROC=IPROC+1  ! s c~ > e- ve~ u b u~ b~
      PD(IPROC)=S1*CX2
      PD(0)=PD(0)+DABS(PD(IPROC))
      IPROC=IPROC+1  ! s c~ > e- ve~ d b d~ b~
      PD(IPROC)=S1*CX2
      PD(0)=PD(0)+DABS(PD(IPROC))
      CALL SP1_MATRIX68(PP,P1_DSIGUU)
      IF (P1_DSIGUU.LT.1D199) THEN
        P1_DSIG68=PD(0)*CONV*P1_DSIGUU
      ELSE
        WRITE(*,*) 'Error in matrix element'
        P1_DSIGUU=0D0
        P1_DSIG68=0D0
      ENDIF
      END

