*************************************************************************
* Collection of ttbar matrix element calculation functions.
*
*    QQBN: qqbar -> ttbar w/o spin corr. between prod. and decay,
*     GGN:    gg -> ttbar w/o spin corr. between prod. and decay,
*   KSQQB: qqbar -> ttbar w/  spin corr. between prod. and decay,
*    KSGG:    gg -> ttbar w/  spin corr. between prod. and decay,
*   MPQQB: qqbar -> ttbar w/  spin corr. between prod. and decay.
*
* KSQQB and KSGGB are interfaces to TTBBWW and GGTTWW, respectively, 
* which contain the matrix element calculation according to
* R. Kleiss and W.J.Stirling, Z.Phys,C40 (1988) 419-423.
*
* MPQQB are the matrix elements according to
* G.Mahlon and S. Parke, Phys.Lett. B411 (1997) 173-179; hep-ph/9706304.
*
* Original code by courtesy of Stephen Parke <parke@fnal.gov>
* Oct-20-2004: some modifications regarding initialization issues
*              + minor technical adaptions (P Movilla Fernandez)
*
*            Extend the argument list of the above functions
*            by an option flag to steer includion of propagator terms.
*
*            E.g.: KSQQB(...,IOPT)
*            IOPT.EQ.1: Include W propagator but not top quark propagator
*            IOPT.EQ.2: Include top quark propagator but not W propagator
*            IOPT.EQ.3: Include W and top quark propagator
*                 else: Include neither W nor top quark propagator
*
* Comments by Stephen Parke:
* -------------------------
* Suit of routines for Top production at Tevatron
* The input momentum satisfy Energy and Momentum conservation.
* P1+P2+PB+PEB+PNE+PBB+PM+PNM=0
* all momenta are outcomimg, thus P1,P2 have negative energies
* P1**2=P2**2=PNE**2=PNE**2=PM**2=PNM**2=0
* PB**2=PBB*2=M_B**2 for some routines M_B**2 must be zero.
* top momenta, PT=PB+PEB+PNE for some routines PT**2=M_T**2
* tbar momenta, PTB=PBB+PM+PNM for some routines PTB**2=M_T**2
* the masses M_T and M_B as well as widths are given in the
* subroutine MASSES which should be called initially.
* The W momenta need not be on mass shell for any of the routines.
* Relative normalization of all routines is correct.
*
* Tops on mass shell, b-quark mass zero
* no spin correlations between top production and decay
* includes correlations between W prod and decay.
* REAL*8 FUNCTION QQBN(P1,P2,PB,PEB,PNE,PBB,PM,PNM,...)
* REAL*8 FUNCTION  GGN(P1,P2,PB,PEB,PNE,PBB,PM,PNM,...)
*
* Tops on mass shell, b-quark mass zero
* all spin correlations between production and decay
* REAL*8 FUNCTION MPQQB(P1,P2,PB,PEB,PNE,PBB,PM,PNM,...)
*
* Tops on or off  mass shell, b-quark can be massive
* all spin correlations between production and decay
* interface to Kleiss & Stirling
* REAL*8 FUNCTION KSQQB(P1,P2,PB,PEB,PNE,PBB,PM,PNM,...)
* REAL*8 FUNCTION KSGG(P1,P2,PB,PEB,PNE,PBB,PM,PNM,...)
************************************************************************

C Function to return alpha_s (so that it can also be available
C outside of this code, for changing renormalization scale)
      REAL*8 FUNCTION ALPSKS()
      ALPSKS = 0.097D0
      RETURN
      END
C------------------------------------------------------------------------
C qqbar-> ttbar ME w/o spin correlation between prod. and decay.
C (Modified to initialize masses and widths by function arguments.)
C
      REAL*8 FUNCTION QQBN(P1,P2,PB,PEB,PNE,PBB,PM,PNM,
     &                     RMW1,RGW1,RMT1,IOPT1)
      IMPLICIT NONE
      INTEGER I,INIT
      REAL*8 P1(0:3),P2(0:3),PT(0:3),PTB(0:3)
      REAL*8 PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PBB(0:3),PM(0:3),PNM(0:3)
      REAL*8 RMW1,RGW1,RMT1
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      INTEGER IOPT,IOPT1
      COMMON/COPT/IOPT
      REAL*8 DOTP,TDECAY
      REAL*8 MTSQ,D13,D23,D12
      REAL*8 A_S,G_S4,PI
      REAL*8 TWIDTH,ALPSKS
      PARAMETER( PI=3.141592654D0 )
      DATA INIT/0/
      SAVE INIT
C
      IF(INIT.EQ.0)THEN
         INIT=1
         WRITE(6,*)'  qqb-MSQ: No Spin Correl.'
         WRITE(6,*)'  between Prod. and Decay Included'
         IOPT=IOPT1
      ENDIF
      CALL MASSES(0.D0, RMW1, RGW1, RMT1, TWIDTH(RMT1, RMW1, 0.D0))
      A_S=ALPSKS()
      G_S4=(4.*PI*A_S)*(4.*PI*A_S)
      DO I=0,3
         PT(I)=PB(I)+PEB(I)+PNE(I)
         PTB(I)=PBB(I)+PM(I)+PNM(I)
      ENDDO
      D12=DOTP(P1,P2)
      D13=DOTP(P1,PT)
      D23=DOTP(P2,PT)
      MTSQ=DOTP(PT,PT)
      QQBN=((8.D0/2.D0/9.D0)*G_S4
     &     *(D13**2+D23**2+MTSQ*D12)/D12**2)
     &     *TDECAY(PT,PB,PEB,PNE)
     &     *TDECAY(PTB,PBB,PM,PNM)
      RETURN
      END
C------------------------------------------------------------------------
C gg -> ttbar ME w/o spin correlation between prod. and decay
C (Modified to initialize masses and widths by function arguments.)
C
      REAL*8 FUNCTION GGN(P1,P2,PB,PEB,PNE,PBB,PM,PNM,
     &                     RMW1,RGW1,RMT1,IOPT1)
      IMPLICIT NONE
      INTEGER I,INIT
      REAL*8 P1(0:3),P2(0:3),PT(0:3),PTB(0:3)
      REAL*8 PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PBB(0:3),PM(0:3),PNM(0:3)
      REAL*8 RMW1,RGW1,RMT1
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      INTEGER IOPT,IOPT1
      COMMON/COPT/IOPT
      REAL*8 DOTP,MTSQ,D12,D13,D23
      REAL*8 TDECAY
      REAL*8 A_S,G_S4,PI
      REAL*8 TWIDTH,ALPSKS
      PARAMETER( PI=3.141592D0 )
      DATA INIT/0/
      SAVE INIT
C
      IF(INIT.EQ.0)THEN
         INIT=1
         WRITE(6,*)' gg-MSQ: No Spin Correl.'
         WRITE(6,*)' between Prod. and Decay Included'
         IOPT=IOPT1
      ENDIF
      CALL MASSES(0.D0, RMW1, RGW1, RMT1, TWIDTH(RMT1, RMW1, 0.D0))
      A_S=ALPSKS()
      G_S4=(4.*PI*A_S)*(4.*PI*A_S)
      DO I=0,3
         PT(I)=PB(I)+PEB(I)+PNE(I)
         PTB(I)=PBB(I)+PM(I)+PNM(I)
      ENDDO
      D12=DOTP(P1,P2)
      D13=DOTP(P1,PT)
      D23=DOTP(P2,PT)
      MTSQ=DOTP(PT,PT)
      GGN=(G_S4/2.D0/8.D0/3.D0)
     &     *(8.D0/D13/D23-2.D0*9.D0/D12**2)
     &     *(D13**2+D23**2+2.*MTSQ*D12-MTSQ**2*D12**2/D13/D23) 
     &     *TDECAY(PT,PB,PEB,PNE)
     &     *TDECAY(PTB,PBB,PM,PNM)
      RETURN
      END
C------------------------------------------------------------------------
C gg-> ttbar ME including spin correlation between prod. and decay.
C (Modified to initialize masses and widths by function arguments.
C  /XMASS/ is not used here anymore.)
C
      REAL*8 FUNCTION KSGG(PG1,PG2,PB,PEB,PNE,PBB,PM,PNM,
     &                     RMW1,RGW1,RMT1,IOPT1)
      IMPLICIT NONE
      INTEGER I,J,INIT
      REAL*8 PG1(0:3),PG2(0:3),PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PBB(0:3),PM(0:3),PNM(0:3)
      REAL*8 RMW1,RGW1,RMT1
C     REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
C     COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      REAL*8 TWIDTH
      REAL*8 PLAB
      REAL*8 GW,GS
      REAL*8 RMT,RGT,RMW,RGW,RMB,RMTLO,RMTUP
      COMMON/MOM_KS/PLAB(4,10)
      COMMON/COUPS/GW,GS
      COMMON/PARS/RMT,RGT,RMW,RGW,RMB,RMTLO,RMTUP
      INTEGER IOPT,IOPT1
      COMMON/COPT/IOPT
C
      REAL*8 BQMASS,ALPSKS
      REAL*8 G_F,PI,AS
      PARAMETER( G_F=1.16637D-5, PI=3.141592654D0 )
      DATA INIT/0/
      SAVE INIT
C
      AS=ALPSKS()
      GS=DSQRT(4.D0*PI*AS)
      IF(INIT.EQ.0) THEN
         INIT=1
         WRITE(6,*)' gg MSQ: Kleiss-Stirling'
         WRITE(6,*)' AS(360 GeV) = ',AS
         RMB=BQMASS()
         GW=DSQRT(80.4D0**2*G_F/DSQRT(2.0D0))
         IOPT=IOPT1
      ENDIF
      RMT=RMT1
      RMW=RMW1
      RGW=RGW1
C     RGT=1.2D0
      RGT=TWIDTH(RMT,RMW,RMB)
      DO I=1,4
        J=I
        IF(I.EQ.4)J=0
        PLAB(I,1)=-PG1(J)
        PLAB(I,2)=-PG2(J)
        PLAB(I,3)=PBB(J)
        PLAB(I,4)=PM(J)
        PLAB(I,5)=PNM(J)
        PLAB(I,6)=PB(J)
        PLAB(I,8)=PEB(J)
        PLAB(I,7)=PNE(J)
      ENDDO
      CALL GGTTWW(KSGG)
      RETURN
      END
C------------------------------------------------------------------------
C qqbar-> ttbar ME including spin correlation between prod. and decay.
C (Modified to initialize masses and widths by function arguments.
C /XMASS/ is not used here anymore.)
C
      REAL*8 FUNCTION KSQQB(PQ,PQB,PB,PEB,PNE,PBB,PM,PNM,
     &                      RMW1,RGW1,RMT1,IOPT1)
      IMPLICIT NONE
      INTEGER I,J,INIT
      REAL*8 PQ(0:3),PQB(0:3),PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PBB(0:3),PM(0:3),PNM(0:3)
      REAL*8 RMW1,RGW1,RMT1
C     REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
C     COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      REAL*8 TWIDTH
      REAL*8 PLAB
      REAL*8 GW,GS
      REAL*8 RMT,RGT,RMW,RGW,RMB,RMTLO,RMTUP
      COMMON/MOM_KS/PLAB(4,10)
      COMMON/COUPS/GW,GS
      COMMON/PARS/RMT,RGT,RMW,RGW,RMB,RMTLO,RMTUP
      INTEGER IOPT,IOPT1
      COMMON/COPT/IOPT
C
      REAL*8 BQMASS,ALPSKS
      REAL*8 G_F,PI,AS
      PARAMETER( G_F=1.16637D-5, PI=3.141592654D0 )
      DATA INIT/0/
      SAVE INIT
C
      AS=ALPSKS()
      IF(INIT.EQ.0) THEN
         INIT=1
         WRITE(6,*)' qqb MSQ: Kleiss-Stirling'
         WRITE(6,*)' AS(360 GeV) = ',AS
         RMB=BQMASS()
         GW=DSQRT(80.4D0**2*G_F/DSQRT(2.0D0))
         IOPT=IOPT1
      ENDIF
      RMT=RMT1
      RMW=RMW1
      RGW=RGW1
C     RGT=1.2D0
      RGT=TWIDTH(RMT,RMW,RMB)
      GS=DSQRT(4.D0*PI*AS)
      DO I=1,4
         J=I
         IF(I.EQ.4)J=0
         PLAB(I,1)=-PQB(J)
         PLAB(I,2)=-PQ(J)
         PLAB(I,3)=PBB(J)
         PLAB(I,4)=PM(J)
         PLAB(I,5)=PNM(J)
         PLAB(I,6)=PB(J)
         PLAB(I,8)=PEB(J)
         PLAB(I,7)=PNE(J)
      ENDDO
      CALL TTBBWW(1,2,KSQQB)
      RETURN
      END
C------------------------------------------------------------------------
C qqbar-> ttbar ME including spin correlation between prod. and decay.
C (Modified to initialize masses and widths by function arguments.)
C
      REAL*8 FUNCTION MPQQB(P1,P2,PB,PEB,PNE,PBB,PM,PNM,
     &                      RMW1,RGW1,RMT1,IOPT)
C--------tops must be on shell W bosons need not be.
      IMPLICIT NONE
      INTEGER I,INIT
      REAL*8 A_S,G_S,PI
      REAL*8 G_F,GWSQ
      REAL*8 P1(0:3),P2(0:3),PT(0:3),PTB(0:3)
      REAL*8 PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PBB(0:3),PM(0:3),PNM(0:3)
      REAL*8 RMW1,RGW1,RMT1
      REAL*8 P0(0:3),PWP(0:3),PWM(0:3)
      REAL*8 DOTP,FCOS0,FBETA
      REAL*8 CQT,CEQ,CMQB,CET,CMTB,CEM
      REAL*8 CEB,CMBB,WPSQ,WMSQ
      REAL*8 TSQ,TBSQ,BETA
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      INTEGER IOPT
      REAL*8 ALPSKS
      REAL*8 ROOT2
      PARAMETER(PI=3.141592654D0, ROOT2=1.414213562D0)
      PARAMETER(G_F=1.16637D-5,GWSQ=4.D0*ROOT2*80.40D0**2*G_F)
      DATA INIT/0/
      SAVE INIT

      A_S=ALPSKS()
      G_S=DSQRT(4.D0*PI*A_S)
      IF(INIT.EQ.0)THEN
         INIT=1
         WRITE(6,*)'  qqb-MSQ= Analytic; Tops on mass shell'
      ENDIF
      CALL MASSES(0.D0, RMW1, RGW1, RMT1, 0.D0)
      DO I=0,3
         P0(I)=-P1(I)-P2(I)
         PWP(I)=PEB(I)+PNE(I)
         PWM(I)=PM(I)+PNM(I)
         PT(I)=PB(I)+PEB(I)+PNE(I)
         PTB(I)=PBB(I)+PM(I)+PNM(I)
      ENDDO
      CQT=FCOS0(P0,P1,PT)
      CEQ=FCOS0(P0,P1,PEB)
      CET=FCOS0(P0,PEB,PT)
      CEM=FCOS0(P0,PEB,PM)
      CMQB=FCOS0(P0,P2,PM)
      CMTB=FCOS0(P0,PTB,PM)
      CEB=FCOS0(PWP,PEB,PB)
      CMBB=FCOS0(PWM,PM,PBB)
      WPSQ=2.D0*DOTP(PEB,PNE)
      WMSQ=2.D0*DOTP(PM,PNM)
      TSQ=DOTP(PT,PT)
      TBSQ=DOTP(PTB,PTB)
C
      BETA=0.5D0*(FBETA(P0,PT)+FBETA(P0,PTB))
      MPQQB=
     >     0.25D0 * G_S**4 * GWSQ**4
     >     *(TSQ-WPSQ)!/((WPSQ-M_W**2)**2+(M_W*GAM_W)**2)
     >     *(TBSQ-WMSQ)!/((WMSQ-M_W**2)**2+(M_W*GAM_W)**2)
     >     *(TSQ*(1.D0-CEB**2)+WPSQ*(1.D0+CEB)**2)
     >     *(TBSQ*(1.D0-CMBB**2)+WMSQ*(1.D0+CMBB)**2)
     >     *(   (2.D0-BETA**2*(1.D0-CQT**2))
C-------------- without the following three lines the integral is twice
C               the normal answer. This means that these terms average
C               to minus 1/2 of the previous line !!!!
     >     -((1.D0-BETA**2)/((1.D0-BETA*CET)*(1.D0-BETA*CMTB)))   
     >     *((1.D0-BETA*CET)+(1.D0-BETA*CMTB)-(1.D0+CEQ*CMQB)
     >     +BETA*CQT*(CEQ+CMQB)+BETA**2*(1.D0-CQT**2)*(1.D0-CEM)/2.D0)
C--------------
     >     )
      IF( IOPT.EQ.1 .OR. IOPT.EQ.3 ) THEN
         MPQQB=MPQQB
     >        /((WPSQ-M_W**2)**2+(M_W*GAM_W)**2)
     >        /((WMSQ-M_W**2)**2+(M_W*GAM_W)**2)
      ENDIF
      IF( IOPT.EQ.2 .OR. IOPT.EQ.3 ) THEN
         MPQQB=MPQQB
     >        /((TSQ-M_T**2)**2+(M_T*GAM_T)**2)
     >        /((TBSQ-M_T**2)**2+(M_T*GAM_T)**2)
      ENDIF

C        WRITE(6,*)' W-masses',DSQRT(WPSQ),DSQRT(WMSQ)
C        WRITE(6,*)' t-masses',DSQRT(TSQ),DSQRT(TBSQ)
C        WRITE(6,*)' b-masses',DSQRT(DABS(DOTP(PB,PB))),
C     >   DSQRT(DABS(DOTP(PBB,PBB)))

C     color and spin average incoming partons
      MPQQB=MPQQB/3.d0/3.d0/2.d0/2.d0
C     WRITE(6,*)' analytic = ',MPQQB
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION TDECAY(PT,PB,PEB,PNE)
      IMPLICIT NONE
      REAL*8 PT(0:3),PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 GWSQ,G_F,DOTP
      REAL*8 MM_W
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      INTEGER IOPT
      COMMON/COPT/IOPT
      REAL*8 ROOT2
      PARAMETER( G_F=1.16637D-5, MM_W=80.4D0, ROOT2=1.414213562D0)
      PARAMETER(GWSQ=4.D0*ROOT2*MM_W**2*G_F)
C
      TDECAY=2.D0*GWSQ**2
     &    *DOTP(PB,PNE)*DOTP(PT,PEB)
      IF( IOPT.EQ.1 .OR. IOPT.EQ.3 ) THEN
         TDECAY=TDECAY
     &        /((2.D0*DOTP(PEB,PNE)-M_W**2)**2+(M_W*GAM_W)**2)
      ENDIF
      IF( IOPT.EQ.2 .OR. IOPT.EQ.3 ) THEN
         TDECAY=TDECAY
     &        /((DOTP(PT,PT)-M_T**2)**2+(M_T*GAM_T)**2)
      ENDIF
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION TDECR(PT,PB,PEB,PNE)
      IMPLICIT NONE
      INTEGER I
      REAL*8 PT(0:3),PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PP(0:3),PT1(0:3)
      REAL*8 GWSQ,G_F,DOTP,TDTP,MTSQ
      REAL*8 MM_W
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      INTEGER IOPT
      COMMON/COPT/IOPT
      REAL*8 ROOT2
      PARAMETER(G_F=1.16637D-5,MM_W=80.4D0, ROOT2=1.414213562D0)
      PARAMETER(GWSQ=4.D0*ROOT2*MM_W**2*G_F)
C
      DO I=1,3
         PP(I)=PT(I)
      ENDDO
      PP(0)=DSQRT(DABS(PP(1)**2+PP(2)**2+PP(3)**2))
      TDTP=2.D0*DOTP(PT,PP)
      MTSQ=M_T**2
      DO I=0,3
         PT1(I)=PT(I)-MTSQ*PP(I)/TDTP
      ENDDO
      TDECR=2.D0*GWSQ**2
     &    *DOTP(PB,PNE)*DOTP(PT1,PEB)
      IF( IOPT.EQ.1 .OR. IOPT.EQ.3 ) THEN
         TDECR=TDECR
     &        /((2.D0*DOTP(PEB,PNE)-M_W**2)**2+(M_W*GAM_W)**2)
      ENDIF
      IF( IOPT.EQ.2 .OR. IOPT.EQ.3 ) THEN
         TDECR=TDECR
     &        /((DOTP(PT,PT)-M_T**2)**2+(M_T*GAM_T)**2)
      ENDIF
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION TDECL(PT,PB,PEB,PNE)
      IMPLICIT NONE
      INTEGER I
      REAL*8 PT(0:3),PB(0:3),PEB(0:3),PNE(0:3)
      REAL*8 PP(0:3),PT1(0:3)
      REAL*8 GWSQ,G_F,DOTP,TDTP,MTSQ
      REAL*8 MM_W
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
      INTEGER IOPT
      COMMON/COPT/IOPT
      REAL*8 ROOT2
      PARAMETER(G_F=1.16637D-5,MM_W=80.4D0,ROOT2=1.414213562D0)
      PARAMETER(GWSQ=4.D0*ROOT2*MM_W**2*G_F)
C
      DO I=1,3
         PP(I)=PT(I)
      ENDDO
      PP(0)=DSQRT(DABS(PP(1)**2+PP(2)**2+PP(3)**2))
      TDTP=2.D0*DOTP(PT,PP)
      MTSQ=M_T**2
      DO I=0,3
         PT1(I)=PT(I)-MTSQ*PP(I)/TDTP
      ENDDO
      TDECL=2.D0*GWSQ**2
     &    *DOTP(PB,PNE)*DOTP(PP,PEB)*MTSQ/TDTP
      IF( IOPT.EQ.1 .OR. IOPT.EQ.3 ) THEN
         TDECL=TDECL
     &        /((2.D0*DOTP(PEB,PNE)-M_W**2)**2+(M_W*GAM_W)**2)
      ENDIF
      IF( IOPT.EQ.2 .OR. IOPT.EQ.3 ) THEN
         TDECL=TDECL
     &        /((DOTP(PT,PT)-M_T**2)**2+(M_T*GAM_T)**2)
      ENDIF
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION DOTP(P1,P2)
      IMPLICIT NONE
      REAL*8 P1(0:3),P2(0:3)
      DOTP=P1(0)*P2(0)-P1(1)*P2(1)-P1(2)*P2(2)-P1(3)*P2(3)
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION TR4(P1,P2,P3,P4)
      IMPLICIT NONE
      REAL*8 DOTP
      REAL*8 P1(0:3),P2(0:3),P3(0:3),P4(0:3)
      TR4=4.D0*
     &     ( DOTP(P1,P2)*DOTP(P3,P4)
     &     -DOTP(P1,P3)*DOTP(P2,P4)
     &     +DOTP(P1,P4)*DOTP(P3,P2)  )
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION TR6(P1,P2,P3,P4,P5,P6)
      IMPLICIT NONE
      REAL*8 DOTP,TR4
      REAL*8 P1(0:3),P2(0:3),P3(0:3),P4(0:3),P5(0:3),P6(0:3)
      TR6=
     &     +DOTP(P1,P2)*TR4(P3,P4,P5,P6)
     &     -DOTP(P1,P3)*TR4(P2,P4,P5,P6)
     &     +DOTP(P1,P4)*TR4(P2,P3,P5,P6)
     &     -DOTP(P1,P5)*TR4(P2,P3,P4,P6)
     &     +DOTP(P1,P6)*TR4(P2,P3,P4,P5)
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION FCOS0(P0,P1,P2)
C-------angle between p1 and p2 in rest frame of p0
      IMPLICIT NONE
      REAL*8 P0(0:3),P1(0:3),P2(0:3)
      REAL*8 P0DP0,P1DP1,P2DP2,P0DP1,P0DP2,P1DP2
      REAL*8 E1,E2,BETA1,BETA2

      P0DP1=P0(0)*P1(0)-P0(1)*P1(1)-P0(2)*P1(2)-P0(3)*P1(3)
      P0DP2=P0(0)*P2(0)-P0(1)*P2(1)-P0(2)*P2(2)-P0(3)*P2(3)
      P1DP2=P1(0)*P2(0)-P1(1)*P2(1)-P1(2)*P2(2)-P1(3)*P2(3)
      
      P0DP0=P0(0)*P0(0)-P0(1)*P0(1)-P0(2)*P0(2)-P0(3)*P0(3)
      P1DP1=P1(0)*P1(0)-P1(1)*P1(1)-P1(2)*P1(2)-P1(3)*P1(3)
      P2DP2=P2(0)*P2(0)-P2(1)*P2(1)-P2(2)*P2(2)-P2(3)*P2(3)
      
      P1DP1=DABS(P1DP1)
      P2DP2=DABS(P2DP2)

      E1=P0DP1/DSQRT(DABS(P0DP0))
      E2=P0DP2/DSQRT(DABS(P0DP0))

      BETA1=DSQRT(1.D0-P1DP1/E1**2)
      BETA2=DSQRT(1.D0-P2DP2/E2**2)

      FCOS0=(1.D0-P1DP2/(E1*E2))/BETA1/BETA2

      IF(P0DP0.LE.0.D0)WRITE(6,*)' P0DP0 < 0   ',P0DP0
      IF(P1DP1.LT.0.D0)WRITE(6,*)' P1DP1 < 0   ',P1DP1
      IF(P2DP2.LT.0.D0)WRITE(6,*)' P2DP2 < 0   ',P2DP2
      
      IF(BETA1.GT.1.01D0)WRITE(6,*)' BETA1 > 1  ',BETA1
      IF(BETA1.LT.-0.01D0)WRITE(6,*)' BETA1 < 0  ',BETA1
      IF(BETA2.GT.1.01D0)WRITE(6,*)' BETA2 > 1  ',BETA2
      IF(BETA2.LT.-0.01D0)WRITE(6,*)' BETA2 < 0  ',BETA2
      IF(BETA2.LT.-0.01D0)WRITE(6,*)' BETA2 < 0  ',BETA2

      IF(DABS(FCOS0).GT.1.01D0)WRITE(6,*)'|cos_str| > 1 ',FCOS0

      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION FBETA(P0,P1)
C-------beta of p1 in rest frame of p0
      IMPLICIT NONE
      REAL*8 P0(0:3),P1(0:3)
      REAL*8 P0DP0,P1DP1,P0DP1
      REAL*8 E

      P0DP1=P0(0)*P1(0)-P0(1)*P1(1)-P0(2)*P1(2)-P0(3)*P1(3)
      P0DP0=P0(0)*P0(0)-P0(1)*P0(1)-P0(2)*P0(2)-P0(3)*P0(3)
      P1DP1=P1(0)*P1(0)-P1(1)*P1(1)-P1(2)*P1(2)-P1(3)*P1(3)

      IF(P0DP0.LE.0.D0)WRITE(6,*)' P0DP0 < 0   ',P0DP0
      IF(P1DP1.LT.0.D0)WRITE(6,*)' P1DP1 < 0   ',P1DP1

      P1DP1=DABS(P1DP1)

      E=P0DP1/DSQRT(DABS(P0DP0))
      FBETA=DSQRT(1.D0-P1DP1/E**2)

      IF(FBETA.GT.1.01D0)WRITE(6,*)' beta > 1  ',FBETA
      IF(FBETA.LT.-0.01D0)WRITE(6,*)' beta < 0  ',FBETA
        
      RETURN
      END
C------------------------------------------------------------------------
C Modified to initialize /XMASS/ by function arguments.
      SUBROUTINE MASSES( RMB, RMW, RGW, RMT, RGT)
      IMPLICIT NONE
      REAL*8 RMB, RMW, RGW, RMT, RGT
      REAL*8 TWIDTH,HWIDTH
      REAL*8 M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T,M_H,GAM_H
      COMMON/XMASS/M_B,M_W,GAM_W,M_Z,GAM_Z,M_T,GAM_T
C
      M_Z=91.187D0 
      M_H=125.D0
      M_B=RMB
      M_W=RMW
      M_T=RMT
      GAM_Z=2.490D0
      GAM_H=HWIDTH(M_H,M_W,M_B)
      IF( RGW.GE.0. ) THEN
         GAM_W=RGW
      ELSE
         GAM_W=2.07115D0
      ENDIF
      IF( RGT.GE.0. ) THEN
         GAM_T=RGT
      ELSE
         GAM_T=TWIDTH(M_T,M_W,M_B)
      ENDIF
C     G_F*M_W**3/6.D0/ROOT2/PI*(3.D0+3.D0+3.D0)
C     WRITE(6,*) ' Masses etc in GeV'
C     WRITE(6,*) ' m_b= ',M_B
C     WRITE(6,*) ' m_w= ',M_W,'   gam_w= ',GAM_W
C     WRITE(6,*) ' m_z= ',M_Z,'   gam_z= ',GAM_Z
C     WRITE(6,*) ' m_t= ',M_T,'   gam_t= ',GAM_T,' T -> W+ B only'
C     WRITE(6,*) ' m_h= ',M_h,'   gam_h= ',GAM_H,' H -> B Bbar only'
C     WRITE(6,*) ' H --> B Bbar only'
      RETURN
      END
C------------------------------------------------------------------------
      REAL*8 FUNCTION TWIDKS(M_T,M_W,M_B)
      IMPLICIT NONE
      REAL*8 M_T,M_W,M_B

      REAL*8 G_F,PI,ROOT2,K,FAC,OVERA
      PARAMETER(PI=3.141592654D0,ROOT2=1.414213562D0)
      PARAMETER(G_F=1.16639D-5,OVERA=G_F/8.D0/PI/ROOT2)
      FAC=(1.D0-(M_B/M_T)**2)**2
     &     +(M_W/M_T)**2*(1.D0+(M_B/M_T)**2)
     &     -2.D0*(M_W/M_T)**4
      K=DSQRT((M_T**2-(M_W+M_B)**2)*(M_T**2-(M_W-M_B)**2))/2.D0/M_T 
      TWIDKS=OVERA*M_T**3*(2.D0*K/M_T)*FAC

      RETURN
      END
C------------------------------------------------------------------------      
      REAL*8 FUNCTION HWIDTH(M_H,M_W,M_B)
      IMPLICIT NONE
      REAL*8 M_H,M_W,M_B,G_F,PI,ROOT2
      PARAMETER(PI=3.141592654D0,ROOT2=1.414213562D0)
      PARAMETER(G_F=1.16639D-5)
      HWIDTH=3.D0*G_F*M_H*M_B**2*(1.D0-4.D0*M_B**2/M_H**2)**1.5
     >     /(4.D0*ROOT2*PI)
      RETURN
      END

C************************************************************************
C Begin of Kleiss-Stirling code:
C
      SUBROUTINE TTBBWW(I1,I2,WT)
* THE MATRIX ELEMENT FOR THE PROCESS
*    QBAR(I1) Q(I2) ---> T TBAR
* FOLLOWED BY
*    TBAR ---> BBAR(3) E-(4) NUEBAR(5)
*    T    ---> B(6)    NUE(7)  E+(8)
*
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 RPL,RMN,DEN,SPROD
      COMPLEX*16 SPL(10,10),SMN(10,10)
      COMMON/COUPS/GW,GS
      COMMON/CSTD/SPL,SMN
      COMMON/MOM_KS/PLAB(4,10)
      COMMON/PARS/RMT,RGT,RMW,RGW,RMB,RMTLO,RMTUP
      COMMON/COPT/IOPT
C (Mod. to execute initialization block each time.)
C     DATA INIT/0/
C     IF(INIT.NE.0) GOTO 1
C     INIT=1
      SCALE=1D0
      RMW2=RMW**2/SCALE
      RMGW=RMW*RGW/SCALE
      RMT2=RMT**2/SCALE
      RMB2=RMB**2/SCALE
      RMGT=RMT*RGT/SCALE
C 1   CONTINUE
* THE DENOMINATOR
      D12=DOT(1,2)/SCALE
      D34=DOT(3,4)/SCALE
      D35=DOT(3,5)/SCALE
      D45=DOT(4,5)/SCALE
      D67=DOT(6,7)/SCALE
      D68=DOT(6,8)/SCALE
      D78=DOT(7,8)/SCALE
      RMB2A = DOT(6,6)
      RMB2B = DOT(3,3)
      XT=   2.D0*(D67+D68+D78) + RMB2A
      XTBAR=2.D0*(D34+D35+D45) + RMB2B
C-------- PROPAGATORS
      DEN=2.D0*D12
      IF( IOPT.EQ.1 .OR. IOPT.EQ.3 ) THEN
         DEN=DEN
     .        *DCMPLX(2.D0*D45-RMW2,RMGW)
     .        *DCMPLX(2.D0*D78-RMW2,RMGW)
      ENDIF
      IF( IOPT.EQ.2 .OR. IOPT.EQ.3 ) THEN
         DEN=DEN
     .        *DCMPLX(XT   -RMT2,RMGT)
     .        *DCMPLX(XTBAR-RMT2,RMGT)
      ENDIF
*
* THE AUXILIARY VECTORS FROM THE T AND TBAR MOMENTA
      CT   =XT   /(2.D0*(D68+D78))
      CTBAR=XTBAR/(2.D0*(D34+D45))
      DO 11 K=1,4
      PLAB(K,9) =PLAB(K,3)+(1.D0-CTBAR)*PLAB(K,4)+PLAB(K,5)
      PLAB(K,10)=PLAB(K,6)+PLAB(K,7)+(1.D0-CT   )*PLAB(K,8)
   11 CONTINUE
*
      CALL STD(PLAB, SPL, SMN)

* THE SPINOR PART OF THE NUMERATOR
      SPROD=-SPL(8,10)*SMN(9,4)/DCMPLX(SCALE)
      RPL=(SPROD*SMN(10,I2)*SPL(I1,9)+RMT2*SPL(8,I1)*SMN(I2,4))/SCALE
      RMN=(SPROD*SMN(10,I1)*SPL(I2,9)+RMT2*SPL(8,I2)*SMN(I1,4))/SCALE
*
* THE WHOLE THING
      WT=GW**8*(GS/SCALE)**4*
     . D67*D35*128.D0**2*
     . (ABS(RPL)**2 + ABS(RMN)**2)/ABS(DEN)**2
* ADD B,BBAR SPIN SUM, Q,QBAR AVERAGE, COLOUR SUM &AVERAGE
      WT=WT *4.D0         /4.D0        *2.D0       /9.D0
*
      RETURN
      END
C
C
C
      SUBROUTINE GGTTWW(WT)
* THE CROSS SECTION FOR
*    G(1) G(2) ---> T  TBAR
* FOLLOWED BY THE DECAYS
*    TBAR ---> BBAR(3) E-(4) NUEBAR(5)
*    T    ---> B(6) NUE(7) E+(8)   ------MODIFIED!
* CHANGED TO : B(6) U(7) DBAR(8)
*
      IMPLICIT REAL*8(A-H,O-Y),COMPLEX*16 (Z)
      COMPLEX*16 SPL(10,10),SMN(10,10)
      COMMON/AA/PI,W,Q,QCDL,NF,MODE
      COMMON/PARS/RMT,RGT,RMW,RGW,RMB,RMTLO,RMTUP
      COMMON/MOM_KS/PLAB(4,10)
      COMMON/COUPS/GW,GS
      COMMON/CSTD/SPL,SMN
      COMMON/COPT/IOPT
C (Mod. to execute initialization block each time.)
C     DATA INIT/0/
C     IF(INIT.EQ.0) THEN
C     INIT=1
      RMW2=RMW**2
      RMGW=RMW*RGW
      RMB2=RMB**2
      RMT2=RMT**2
      RMGT=RMT*RGT
C     ENDIF
*
* THE AUXILIARY VECTORS FOR THE T AND TBAR MOMENTA
      D34=DOT(3,4)
      D35=DOT(3,5)
      D45=DOT(4,5)
      D67=DOT(6,7)
      D68=DOT(6,8)
      D78=DOT(7,8)
      RMB2A = DOT(6,6)
      RMB2B = DOT(3,3)
      C9 =(D34+D35+D45+0.5D0*RMB2B)/(D34+D45)
      C10=(D67+D68+D78+0.5D0*RMB2A)/(D68+D78)
      DO 1 K=1,4
      PLAB(K,9 )=PLAB(K,3)+(1.D0-C9 )*PLAB(K,4)+PLAB(K,5)
    1 PLAB(K,10)=PLAB(K,6)+PLAB(K,7)+(1.D0-C10)*PLAB(K,8)
*
* THE DENOMINATORS
      XTBAR=2.D0*(D34+D35+D45)+RMB2B
      XT   =2.D0*(D67+D68+D78)+RMB2A
C-------- PROPAGATORS
      ZD0=1.D0
      IF( IOPT.EQ.1 .OR. IOPT.EQ.3 ) THEN
         ZD0=ZD0
     .        *DCMPLX(2.D0*D45-RMW2,RMGW)
     .        *DCMPLX(2.D0*D78-RMW2,RMGW)
      ENDIF
      IF( IOPT.EQ.2 .OR. IOPT.EQ.3 ) THEN
         ZD0=ZD0
     .        *DCMPLX(XTBAR-RMT2,RMGT)
     .        *DCMPLX(XT   -RMT2,RMGT)
      ENDIF
      D13=DOT(1,3)
      D14=DOT(1,4)
      D15=DOT(1,5)
      D1345=2.D0*(D34+D35+D45-D13-D14-D15)+RMB2B
      D16=DOT(1,6)
      D17=DOT(1,7)
      D18=DOT(1,8)
      D1678=2.D0*(D67+D68+D78-D16-D17-D18)+RMB2A
      ZDEN1=ZD0*DCMPLX(D1345-RMT2,RMGT)
      ZDEN2=ZD0*DCMPLX(D1678-RMT2,RMGT)
      D12=DOT(1,2)
      ZDEN3=ZD0*2.D0*D12
*
* THE SPINOR PARTS
      D1T=D16+D17+D18
      D2T=DOT(2,6)+DOT(2,7)+DOT(2,8)
      CALL STD(PLAB, SPL, SMN)
*
      Z81=SMN(8,10)*SPL(10,1)
      Z82=SMN(8,10)*SPL(10,2)
      Z14=SMN(1,9)*SPL(9,4)
      Z24=SMN(2,9)*SPL(9,4)
*
      ZG1PP=(-2.D0*(D1T-D12)*Z82*Z24
     .  +RMT2*(  Z82*SMN(1,2)*SPL(1,4)
     .         - SMN(8,1)*SPL(2,1)*Z24
     .         +2.D0*D2T*SMN(8,1)*SPL(1,4) ))/ZDEN1
      SFAC=DSQRT(4.D0*D1T*D2T-2.D0*XT*D12)
      ZG1PM=SFAC*( -Z81*Z24
     .             +RMT2*SMN(8,2)*SPL(1,4) )/ZDEN1
      ZG1MP=SFAC*( -Z82*Z14
     .             +RMT2*SMN(8,1)*SPL(2,4) )/ZDEN1
      ZG1MM=(-2.D0*D2T*Z81*Z14
     .  +RMT2*(  Z81*SMN(2,1)*SPL(2,4)
     .         - SMN(8,2)*SPL(1,2)*Z14
     .         + 2.D0*(D1T-D12)*SMN(8,2)*SPL(2,4) ))/ZDEN1
*
      ZG2PP=(-2.D0*(D2T-D12)*Z81*Z14
     .  +RMT2*(  Z81*SMN(2,1)*SPL(2,4)
     .         - SMN(8,2)*SPL(1,2)*Z14
     .         + 2.D0*D1T*SMN(8,2)*SPL(2,4) ))/ZDEN2
      ZG2PM=ZG1PM *ZDEN1/ZDEN2
      ZG2MP=ZG1MP *ZDEN1/ZDEN2
      ZG2MM=(-2.D0*D1T*Z82*Z24
     .  +RMT2*(  Z82*SMN(1,2)*SPL(1,4)
     .         - SMN(8,1)*SPL(2,1)*Z24
     .         + 2.D0*(D2T-D12)*SMN(8,1)*SPL(1,4) ))/ZDEN2
*
      ZG3PP=(-Z81*Z14+Z82*Z24
     .  +RMT2*( SMN(8,1)*SPL(1,4) - SMN(8,2)*SPL(2,4) ) )/ZDEN3
      ZG3PM=(0.D0,0.D0)
      ZG3MP=(0.D0,0.D0)
      ZG3MM=ZG3PP
*
      C=1./(2.*D12)
      ZEVPP=C*( ZG1PP+ZG2PP)
      ZEVPM=C*( ZG1PM+ZG2PM)
      ZEVMP=C*( ZG1MP+ZG2MP)
      ZEVMM=C*( ZG1MM+ZG2MM)
      ZODPP=C*(-ZG1PP+ZG2PP)-ZG3PP
      ZODPM=C*(-ZG1PM+ZG2PM)-ZG3PM
      ZODMP=C*(-ZG1MP+ZG2MP)-ZG3MP
      ZODMM=C*(-ZG1MM+ZG2MM)-ZG3MM
*
      XEVEN=ABS(ZEVPP)**2+ABS(ZEVPM)**2
     .     +ABS(ZEVMP)**2+ABS(ZEVMM)**2
      XODD =ABS(ZODPP)**2+ABS(ZODPM)**2
     .     +ABS(ZODMP)**2+ABS(ZODMM)**2
* COLLECT ALL FACTORS
      WT = ( 28./3.*XEVEN + 12.*XODD )
     .   * GW**8 * GS**4 * (128.)**2 *D67 *D35
     .             *4            /4.             /64.
      WT=WT/4.
* FACTORS FOR B,BAR SPINS, GG SPIN AVERAGE, GG COLOUR AVERAGE
      RETURN
      END
C
C
C
      DOUBLE PRECISION FUNCTION DOT(I,J)
      IMPLICIT REAL*8 (A-H,O-Z)
      COMMON/MOM_KS/PLAB(4,10)
           DOT=PLAB(4,I)*PLAB(4,J)-PLAB(3,I)*PLAB(3,J)
     1        -PLAB(2,I)*PLAB(2,J)-PLAB(1,I)*PLAB(1,J)
      RETURN
      END
C
C End of Kleiss-Stirling code.
C************************************************************************
