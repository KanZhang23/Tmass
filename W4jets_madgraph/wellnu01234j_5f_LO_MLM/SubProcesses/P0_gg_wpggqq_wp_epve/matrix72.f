      SUBROUTINE SP0_MATRIX72(P,ANS)
C     
C     Generated by MadGraph5_aMC@NLO v. 2.2.2, 2014-11-06
C     By the MadGraph5_aMC@NLO Development Team
C     Visit launchpad.net/madgraph5 and amcatnlo.web.cern.ch
C     
C     MadGraph StandAlone Version
C     
C     Returns amplitude squared summed/avg over colors
C     and helicities
C     for the point in phase space P(0:3,NEXTERNAL)
C     
C     Process: d d > w+ d d d u~ WEIGHTED=6
C     *   Decay: w+ > e+ ve WEIGHTED=2
C     Process: s s > w+ s s s c~ WEIGHTED=6
C     *   Decay: w+ > e+ ve WEIGHTED=2
C     
      IMPLICIT NONE
C     
C     CONSTANTS
C     
      INTEGER    NEXTERNAL
      PARAMETER (NEXTERNAL=8)
      INTEGER                 NCOMB
      PARAMETER (             NCOMB=256)
C     
C     ARGUMENTS 
C     
      REAL*8 P(0:3,NEXTERNAL),ANS
C     
C     LOCAL VARIABLES 
C     
      INTEGER NHEL(NEXTERNAL,NCOMB),NTRY
      REAL*8 T
      REAL*8 P0_MATRIX72
      INTEGER IHEL,IDEN, I
      INTEGER JC(NEXTERNAL)
      LOGICAL GOODHEL(NCOMB)
      DATA NTRY/0/
      DATA GOODHEL/NCOMB*.FALSE./
      DATA (NHEL(I,   1),I=1,8) /-1,-1,-1,-1,-1,-1,-1,-1/
      DATA (NHEL(I,   2),I=1,8) /-1,-1,-1,-1,-1,-1,-1, 1/
      DATA (NHEL(I,   3),I=1,8) /-1,-1,-1,-1,-1,-1, 1,-1/
      DATA (NHEL(I,   4),I=1,8) /-1,-1,-1,-1,-1,-1, 1, 1/
      DATA (NHEL(I,   5),I=1,8) /-1,-1,-1,-1,-1, 1,-1,-1/
      DATA (NHEL(I,   6),I=1,8) /-1,-1,-1,-1,-1, 1,-1, 1/
      DATA (NHEL(I,   7),I=1,8) /-1,-1,-1,-1,-1, 1, 1,-1/
      DATA (NHEL(I,   8),I=1,8) /-1,-1,-1,-1,-1, 1, 1, 1/
      DATA (NHEL(I,   9),I=1,8) /-1,-1,-1,-1, 1,-1,-1,-1/
      DATA (NHEL(I,  10),I=1,8) /-1,-1,-1,-1, 1,-1,-1, 1/
      DATA (NHEL(I,  11),I=1,8) /-1,-1,-1,-1, 1,-1, 1,-1/
      DATA (NHEL(I,  12),I=1,8) /-1,-1,-1,-1, 1,-1, 1, 1/
      DATA (NHEL(I,  13),I=1,8) /-1,-1,-1,-1, 1, 1,-1,-1/
      DATA (NHEL(I,  14),I=1,8) /-1,-1,-1,-1, 1, 1,-1, 1/
      DATA (NHEL(I,  15),I=1,8) /-1,-1,-1,-1, 1, 1, 1,-1/
      DATA (NHEL(I,  16),I=1,8) /-1,-1,-1,-1, 1, 1, 1, 1/
      DATA (NHEL(I,  17),I=1,8) /-1,-1,-1, 1,-1,-1,-1,-1/
      DATA (NHEL(I,  18),I=1,8) /-1,-1,-1, 1,-1,-1,-1, 1/
      DATA (NHEL(I,  19),I=1,8) /-1,-1,-1, 1,-1,-1, 1,-1/
      DATA (NHEL(I,  20),I=1,8) /-1,-1,-1, 1,-1,-1, 1, 1/
      DATA (NHEL(I,  21),I=1,8) /-1,-1,-1, 1,-1, 1,-1,-1/
      DATA (NHEL(I,  22),I=1,8) /-1,-1,-1, 1,-1, 1,-1, 1/
      DATA (NHEL(I,  23),I=1,8) /-1,-1,-1, 1,-1, 1, 1,-1/
      DATA (NHEL(I,  24),I=1,8) /-1,-1,-1, 1,-1, 1, 1, 1/
      DATA (NHEL(I,  25),I=1,8) /-1,-1,-1, 1, 1,-1,-1,-1/
      DATA (NHEL(I,  26),I=1,8) /-1,-1,-1, 1, 1,-1,-1, 1/
      DATA (NHEL(I,  27),I=1,8) /-1,-1,-1, 1, 1,-1, 1,-1/
      DATA (NHEL(I,  28),I=1,8) /-1,-1,-1, 1, 1,-1, 1, 1/
      DATA (NHEL(I,  29),I=1,8) /-1,-1,-1, 1, 1, 1,-1,-1/
      DATA (NHEL(I,  30),I=1,8) /-1,-1,-1, 1, 1, 1,-1, 1/
      DATA (NHEL(I,  31),I=1,8) /-1,-1,-1, 1, 1, 1, 1,-1/
      DATA (NHEL(I,  32),I=1,8) /-1,-1,-1, 1, 1, 1, 1, 1/
      DATA (NHEL(I,  33),I=1,8) /-1,-1, 1,-1,-1,-1,-1,-1/
      DATA (NHEL(I,  34),I=1,8) /-1,-1, 1,-1,-1,-1,-1, 1/
      DATA (NHEL(I,  35),I=1,8) /-1,-1, 1,-1,-1,-1, 1,-1/
      DATA (NHEL(I,  36),I=1,8) /-1,-1, 1,-1,-1,-1, 1, 1/
      DATA (NHEL(I,  37),I=1,8) /-1,-1, 1,-1,-1, 1,-1,-1/
      DATA (NHEL(I,  38),I=1,8) /-1,-1, 1,-1,-1, 1,-1, 1/
      DATA (NHEL(I,  39),I=1,8) /-1,-1, 1,-1,-1, 1, 1,-1/
      DATA (NHEL(I,  40),I=1,8) /-1,-1, 1,-1,-1, 1, 1, 1/
      DATA (NHEL(I,  41),I=1,8) /-1,-1, 1,-1, 1,-1,-1,-1/
      DATA (NHEL(I,  42),I=1,8) /-1,-1, 1,-1, 1,-1,-1, 1/
      DATA (NHEL(I,  43),I=1,8) /-1,-1, 1,-1, 1,-1, 1,-1/
      DATA (NHEL(I,  44),I=1,8) /-1,-1, 1,-1, 1,-1, 1, 1/
      DATA (NHEL(I,  45),I=1,8) /-1,-1, 1,-1, 1, 1,-1,-1/
      DATA (NHEL(I,  46),I=1,8) /-1,-1, 1,-1, 1, 1,-1, 1/
      DATA (NHEL(I,  47),I=1,8) /-1,-1, 1,-1, 1, 1, 1,-1/
      DATA (NHEL(I,  48),I=1,8) /-1,-1, 1,-1, 1, 1, 1, 1/
      DATA (NHEL(I,  49),I=1,8) /-1,-1, 1, 1,-1,-1,-1,-1/
      DATA (NHEL(I,  50),I=1,8) /-1,-1, 1, 1,-1,-1,-1, 1/
      DATA (NHEL(I,  51),I=1,8) /-1,-1, 1, 1,-1,-1, 1,-1/
      DATA (NHEL(I,  52),I=1,8) /-1,-1, 1, 1,-1,-1, 1, 1/
      DATA (NHEL(I,  53),I=1,8) /-1,-1, 1, 1,-1, 1,-1,-1/
      DATA (NHEL(I,  54),I=1,8) /-1,-1, 1, 1,-1, 1,-1, 1/
      DATA (NHEL(I,  55),I=1,8) /-1,-1, 1, 1,-1, 1, 1,-1/
      DATA (NHEL(I,  56),I=1,8) /-1,-1, 1, 1,-1, 1, 1, 1/
      DATA (NHEL(I,  57),I=1,8) /-1,-1, 1, 1, 1,-1,-1,-1/
      DATA (NHEL(I,  58),I=1,8) /-1,-1, 1, 1, 1,-1,-1, 1/
      DATA (NHEL(I,  59),I=1,8) /-1,-1, 1, 1, 1,-1, 1,-1/
      DATA (NHEL(I,  60),I=1,8) /-1,-1, 1, 1, 1,-1, 1, 1/
      DATA (NHEL(I,  61),I=1,8) /-1,-1, 1, 1, 1, 1,-1,-1/
      DATA (NHEL(I,  62),I=1,8) /-1,-1, 1, 1, 1, 1,-1, 1/
      DATA (NHEL(I,  63),I=1,8) /-1,-1, 1, 1, 1, 1, 1,-1/
      DATA (NHEL(I,  64),I=1,8) /-1,-1, 1, 1, 1, 1, 1, 1/
      DATA (NHEL(I,  65),I=1,8) /-1, 1,-1,-1,-1,-1,-1,-1/
      DATA (NHEL(I,  66),I=1,8) /-1, 1,-1,-1,-1,-1,-1, 1/
      DATA (NHEL(I,  67),I=1,8) /-1, 1,-1,-1,-1,-1, 1,-1/
      DATA (NHEL(I,  68),I=1,8) /-1, 1,-1,-1,-1,-1, 1, 1/
      DATA (NHEL(I,  69),I=1,8) /-1, 1,-1,-1,-1, 1,-1,-1/
      DATA (NHEL(I,  70),I=1,8) /-1, 1,-1,-1,-1, 1,-1, 1/
      DATA (NHEL(I,  71),I=1,8) /-1, 1,-1,-1,-1, 1, 1,-1/
      DATA (NHEL(I,  72),I=1,8) /-1, 1,-1,-1,-1, 1, 1, 1/
      DATA (NHEL(I,  73),I=1,8) /-1, 1,-1,-1, 1,-1,-1,-1/
      DATA (NHEL(I,  74),I=1,8) /-1, 1,-1,-1, 1,-1,-1, 1/
      DATA (NHEL(I,  75),I=1,8) /-1, 1,-1,-1, 1,-1, 1,-1/
      DATA (NHEL(I,  76),I=1,8) /-1, 1,-1,-1, 1,-1, 1, 1/
      DATA (NHEL(I,  77),I=1,8) /-1, 1,-1,-1, 1, 1,-1,-1/
      DATA (NHEL(I,  78),I=1,8) /-1, 1,-1,-1, 1, 1,-1, 1/
      DATA (NHEL(I,  79),I=1,8) /-1, 1,-1,-1, 1, 1, 1,-1/
      DATA (NHEL(I,  80),I=1,8) /-1, 1,-1,-1, 1, 1, 1, 1/
      DATA (NHEL(I,  81),I=1,8) /-1, 1,-1, 1,-1,-1,-1,-1/
      DATA (NHEL(I,  82),I=1,8) /-1, 1,-1, 1,-1,-1,-1, 1/
      DATA (NHEL(I,  83),I=1,8) /-1, 1,-1, 1,-1,-1, 1,-1/
      DATA (NHEL(I,  84),I=1,8) /-1, 1,-1, 1,-1,-1, 1, 1/
      DATA (NHEL(I,  85),I=1,8) /-1, 1,-1, 1,-1, 1,-1,-1/
      DATA (NHEL(I,  86),I=1,8) /-1, 1,-1, 1,-1, 1,-1, 1/
      DATA (NHEL(I,  87),I=1,8) /-1, 1,-1, 1,-1, 1, 1,-1/
      DATA (NHEL(I,  88),I=1,8) /-1, 1,-1, 1,-1, 1, 1, 1/
      DATA (NHEL(I,  89),I=1,8) /-1, 1,-1, 1, 1,-1,-1,-1/
      DATA (NHEL(I,  90),I=1,8) /-1, 1,-1, 1, 1,-1,-1, 1/
      DATA (NHEL(I,  91),I=1,8) /-1, 1,-1, 1, 1,-1, 1,-1/
      DATA (NHEL(I,  92),I=1,8) /-1, 1,-1, 1, 1,-1, 1, 1/
      DATA (NHEL(I,  93),I=1,8) /-1, 1,-1, 1, 1, 1,-1,-1/
      DATA (NHEL(I,  94),I=1,8) /-1, 1,-1, 1, 1, 1,-1, 1/
      DATA (NHEL(I,  95),I=1,8) /-1, 1,-1, 1, 1, 1, 1,-1/
      DATA (NHEL(I,  96),I=1,8) /-1, 1,-1, 1, 1, 1, 1, 1/
      DATA (NHEL(I,  97),I=1,8) /-1, 1, 1,-1,-1,-1,-1,-1/
      DATA (NHEL(I,  98),I=1,8) /-1, 1, 1,-1,-1,-1,-1, 1/
      DATA (NHEL(I,  99),I=1,8) /-1, 1, 1,-1,-1,-1, 1,-1/
      DATA (NHEL(I, 100),I=1,8) /-1, 1, 1,-1,-1,-1, 1, 1/
      DATA (NHEL(I, 101),I=1,8) /-1, 1, 1,-1,-1, 1,-1,-1/
      DATA (NHEL(I, 102),I=1,8) /-1, 1, 1,-1,-1, 1,-1, 1/
      DATA (NHEL(I, 103),I=1,8) /-1, 1, 1,-1,-1, 1, 1,-1/
      DATA (NHEL(I, 104),I=1,8) /-1, 1, 1,-1,-1, 1, 1, 1/
      DATA (NHEL(I, 105),I=1,8) /-1, 1, 1,-1, 1,-1,-1,-1/
      DATA (NHEL(I, 106),I=1,8) /-1, 1, 1,-1, 1,-1,-1, 1/
      DATA (NHEL(I, 107),I=1,8) /-1, 1, 1,-1, 1,-1, 1,-1/
      DATA (NHEL(I, 108),I=1,8) /-1, 1, 1,-1, 1,-1, 1, 1/
      DATA (NHEL(I, 109),I=1,8) /-1, 1, 1,-1, 1, 1,-1,-1/
      DATA (NHEL(I, 110),I=1,8) /-1, 1, 1,-1, 1, 1,-1, 1/
      DATA (NHEL(I, 111),I=1,8) /-1, 1, 1,-1, 1, 1, 1,-1/
      DATA (NHEL(I, 112),I=1,8) /-1, 1, 1,-1, 1, 1, 1, 1/
      DATA (NHEL(I, 113),I=1,8) /-1, 1, 1, 1,-1,-1,-1,-1/
      DATA (NHEL(I, 114),I=1,8) /-1, 1, 1, 1,-1,-1,-1, 1/
      DATA (NHEL(I, 115),I=1,8) /-1, 1, 1, 1,-1,-1, 1,-1/
      DATA (NHEL(I, 116),I=1,8) /-1, 1, 1, 1,-1,-1, 1, 1/
      DATA (NHEL(I, 117),I=1,8) /-1, 1, 1, 1,-1, 1,-1,-1/
      DATA (NHEL(I, 118),I=1,8) /-1, 1, 1, 1,-1, 1,-1, 1/
      DATA (NHEL(I, 119),I=1,8) /-1, 1, 1, 1,-1, 1, 1,-1/
      DATA (NHEL(I, 120),I=1,8) /-1, 1, 1, 1,-1, 1, 1, 1/
      DATA (NHEL(I, 121),I=1,8) /-1, 1, 1, 1, 1,-1,-1,-1/
      DATA (NHEL(I, 122),I=1,8) /-1, 1, 1, 1, 1,-1,-1, 1/
      DATA (NHEL(I, 123),I=1,8) /-1, 1, 1, 1, 1,-1, 1,-1/
      DATA (NHEL(I, 124),I=1,8) /-1, 1, 1, 1, 1,-1, 1, 1/
      DATA (NHEL(I, 125),I=1,8) /-1, 1, 1, 1, 1, 1,-1,-1/
      DATA (NHEL(I, 126),I=1,8) /-1, 1, 1, 1, 1, 1,-1, 1/
      DATA (NHEL(I, 127),I=1,8) /-1, 1, 1, 1, 1, 1, 1,-1/
      DATA (NHEL(I, 128),I=1,8) /-1, 1, 1, 1, 1, 1, 1, 1/
      DATA (NHEL(I, 129),I=1,8) / 1,-1,-1,-1,-1,-1,-1,-1/
      DATA (NHEL(I, 130),I=1,8) / 1,-1,-1,-1,-1,-1,-1, 1/
      DATA (NHEL(I, 131),I=1,8) / 1,-1,-1,-1,-1,-1, 1,-1/
      DATA (NHEL(I, 132),I=1,8) / 1,-1,-1,-1,-1,-1, 1, 1/
      DATA (NHEL(I, 133),I=1,8) / 1,-1,-1,-1,-1, 1,-1,-1/
      DATA (NHEL(I, 134),I=1,8) / 1,-1,-1,-1,-1, 1,-1, 1/
      DATA (NHEL(I, 135),I=1,8) / 1,-1,-1,-1,-1, 1, 1,-1/
      DATA (NHEL(I, 136),I=1,8) / 1,-1,-1,-1,-1, 1, 1, 1/
      DATA (NHEL(I, 137),I=1,8) / 1,-1,-1,-1, 1,-1,-1,-1/
      DATA (NHEL(I, 138),I=1,8) / 1,-1,-1,-1, 1,-1,-1, 1/
      DATA (NHEL(I, 139),I=1,8) / 1,-1,-1,-1, 1,-1, 1,-1/
      DATA (NHEL(I, 140),I=1,8) / 1,-1,-1,-1, 1,-1, 1, 1/
      DATA (NHEL(I, 141),I=1,8) / 1,-1,-1,-1, 1, 1,-1,-1/
      DATA (NHEL(I, 142),I=1,8) / 1,-1,-1,-1, 1, 1,-1, 1/
      DATA (NHEL(I, 143),I=1,8) / 1,-1,-1,-1, 1, 1, 1,-1/
      DATA (NHEL(I, 144),I=1,8) / 1,-1,-1,-1, 1, 1, 1, 1/
      DATA (NHEL(I, 145),I=1,8) / 1,-1,-1, 1,-1,-1,-1,-1/
      DATA (NHEL(I, 146),I=1,8) / 1,-1,-1, 1,-1,-1,-1, 1/
      DATA (NHEL(I, 147),I=1,8) / 1,-1,-1, 1,-1,-1, 1,-1/
      DATA (NHEL(I, 148),I=1,8) / 1,-1,-1, 1,-1,-1, 1, 1/
      DATA (NHEL(I, 149),I=1,8) / 1,-1,-1, 1,-1, 1,-1,-1/
      DATA (NHEL(I, 150),I=1,8) / 1,-1,-1, 1,-1, 1,-1, 1/
      DATA (NHEL(I, 151),I=1,8) / 1,-1,-1, 1,-1, 1, 1,-1/
      DATA (NHEL(I, 152),I=1,8) / 1,-1,-1, 1,-1, 1, 1, 1/
      DATA (NHEL(I, 153),I=1,8) / 1,-1,-1, 1, 1,-1,-1,-1/
      DATA (NHEL(I, 154),I=1,8) / 1,-1,-1, 1, 1,-1,-1, 1/
      DATA (NHEL(I, 155),I=1,8) / 1,-1,-1, 1, 1,-1, 1,-1/
      DATA (NHEL(I, 156),I=1,8) / 1,-1,-1, 1, 1,-1, 1, 1/
      DATA (NHEL(I, 157),I=1,8) / 1,-1,-1, 1, 1, 1,-1,-1/
      DATA (NHEL(I, 158),I=1,8) / 1,-1,-1, 1, 1, 1,-1, 1/
      DATA (NHEL(I, 159),I=1,8) / 1,-1,-1, 1, 1, 1, 1,-1/
      DATA (NHEL(I, 160),I=1,8) / 1,-1,-1, 1, 1, 1, 1, 1/
      DATA (NHEL(I, 161),I=1,8) / 1,-1, 1,-1,-1,-1,-1,-1/
      DATA (NHEL(I, 162),I=1,8) / 1,-1, 1,-1,-1,-1,-1, 1/
      DATA (NHEL(I, 163),I=1,8) / 1,-1, 1,-1,-1,-1, 1,-1/
      DATA (NHEL(I, 164),I=1,8) / 1,-1, 1,-1,-1,-1, 1, 1/
      DATA (NHEL(I, 165),I=1,8) / 1,-1, 1,-1,-1, 1,-1,-1/
      DATA (NHEL(I, 166),I=1,8) / 1,-1, 1,-1,-1, 1,-1, 1/
      DATA (NHEL(I, 167),I=1,8) / 1,-1, 1,-1,-1, 1, 1,-1/
      DATA (NHEL(I, 168),I=1,8) / 1,-1, 1,-1,-1, 1, 1, 1/
      DATA (NHEL(I, 169),I=1,8) / 1,-1, 1,-1, 1,-1,-1,-1/
      DATA (NHEL(I, 170),I=1,8) / 1,-1, 1,-1, 1,-1,-1, 1/
      DATA (NHEL(I, 171),I=1,8) / 1,-1, 1,-1, 1,-1, 1,-1/
      DATA (NHEL(I, 172),I=1,8) / 1,-1, 1,-1, 1,-1, 1, 1/
      DATA (NHEL(I, 173),I=1,8) / 1,-1, 1,-1, 1, 1,-1,-1/
      DATA (NHEL(I, 174),I=1,8) / 1,-1, 1,-1, 1, 1,-1, 1/
      DATA (NHEL(I, 175),I=1,8) / 1,-1, 1,-1, 1, 1, 1,-1/
      DATA (NHEL(I, 176),I=1,8) / 1,-1, 1,-1, 1, 1, 1, 1/
      DATA (NHEL(I, 177),I=1,8) / 1,-1, 1, 1,-1,-1,-1,-1/
      DATA (NHEL(I, 178),I=1,8) / 1,-1, 1, 1,-1,-1,-1, 1/
      DATA (NHEL(I, 179),I=1,8) / 1,-1, 1, 1,-1,-1, 1,-1/
      DATA (NHEL(I, 180),I=1,8) / 1,-1, 1, 1,-1,-1, 1, 1/
      DATA (NHEL(I, 181),I=1,8) / 1,-1, 1, 1,-1, 1,-1,-1/
      DATA (NHEL(I, 182),I=1,8) / 1,-1, 1, 1,-1, 1,-1, 1/
      DATA (NHEL(I, 183),I=1,8) / 1,-1, 1, 1,-1, 1, 1,-1/
      DATA (NHEL(I, 184),I=1,8) / 1,-1, 1, 1,-1, 1, 1, 1/
      DATA (NHEL(I, 185),I=1,8) / 1,-1, 1, 1, 1,-1,-1,-1/
      DATA (NHEL(I, 186),I=1,8) / 1,-1, 1, 1, 1,-1,-1, 1/
      DATA (NHEL(I, 187),I=1,8) / 1,-1, 1, 1, 1,-1, 1,-1/
      DATA (NHEL(I, 188),I=1,8) / 1,-1, 1, 1, 1,-1, 1, 1/
      DATA (NHEL(I, 189),I=1,8) / 1,-1, 1, 1, 1, 1,-1,-1/
      DATA (NHEL(I, 190),I=1,8) / 1,-1, 1, 1, 1, 1,-1, 1/
      DATA (NHEL(I, 191),I=1,8) / 1,-1, 1, 1, 1, 1, 1,-1/
      DATA (NHEL(I, 192),I=1,8) / 1,-1, 1, 1, 1, 1, 1, 1/
      DATA (NHEL(I, 193),I=1,8) / 1, 1,-1,-1,-1,-1,-1,-1/
      DATA (NHEL(I, 194),I=1,8) / 1, 1,-1,-1,-1,-1,-1, 1/
      DATA (NHEL(I, 195),I=1,8) / 1, 1,-1,-1,-1,-1, 1,-1/
      DATA (NHEL(I, 196),I=1,8) / 1, 1,-1,-1,-1,-1, 1, 1/
      DATA (NHEL(I, 197),I=1,8) / 1, 1,-1,-1,-1, 1,-1,-1/
      DATA (NHEL(I, 198),I=1,8) / 1, 1,-1,-1,-1, 1,-1, 1/
      DATA (NHEL(I, 199),I=1,8) / 1, 1,-1,-1,-1, 1, 1,-1/
      DATA (NHEL(I, 200),I=1,8) / 1, 1,-1,-1,-1, 1, 1, 1/
      DATA (NHEL(I, 201),I=1,8) / 1, 1,-1,-1, 1,-1,-1,-1/
      DATA (NHEL(I, 202),I=1,8) / 1, 1,-1,-1, 1,-1,-1, 1/
      DATA (NHEL(I, 203),I=1,8) / 1, 1,-1,-1, 1,-1, 1,-1/
      DATA (NHEL(I, 204),I=1,8) / 1, 1,-1,-1, 1,-1, 1, 1/
      DATA (NHEL(I, 205),I=1,8) / 1, 1,-1,-1, 1, 1,-1,-1/
      DATA (NHEL(I, 206),I=1,8) / 1, 1,-1,-1, 1, 1,-1, 1/
      DATA (NHEL(I, 207),I=1,8) / 1, 1,-1,-1, 1, 1, 1,-1/
      DATA (NHEL(I, 208),I=1,8) / 1, 1,-1,-1, 1, 1, 1, 1/
      DATA (NHEL(I, 209),I=1,8) / 1, 1,-1, 1,-1,-1,-1,-1/
      DATA (NHEL(I, 210),I=1,8) / 1, 1,-1, 1,-1,-1,-1, 1/
      DATA (NHEL(I, 211),I=1,8) / 1, 1,-1, 1,-1,-1, 1,-1/
      DATA (NHEL(I, 212),I=1,8) / 1, 1,-1, 1,-1,-1, 1, 1/
      DATA (NHEL(I, 213),I=1,8) / 1, 1,-1, 1,-1, 1,-1,-1/
      DATA (NHEL(I, 214),I=1,8) / 1, 1,-1, 1,-1, 1,-1, 1/
      DATA (NHEL(I, 215),I=1,8) / 1, 1,-1, 1,-1, 1, 1,-1/
      DATA (NHEL(I, 216),I=1,8) / 1, 1,-1, 1,-1, 1, 1, 1/
      DATA (NHEL(I, 217),I=1,8) / 1, 1,-1, 1, 1,-1,-1,-1/
      DATA (NHEL(I, 218),I=1,8) / 1, 1,-1, 1, 1,-1,-1, 1/
      DATA (NHEL(I, 219),I=1,8) / 1, 1,-1, 1, 1,-1, 1,-1/
      DATA (NHEL(I, 220),I=1,8) / 1, 1,-1, 1, 1,-1, 1, 1/
      DATA (NHEL(I, 221),I=1,8) / 1, 1,-1, 1, 1, 1,-1,-1/
      DATA (NHEL(I, 222),I=1,8) / 1, 1,-1, 1, 1, 1,-1, 1/
      DATA (NHEL(I, 223),I=1,8) / 1, 1,-1, 1, 1, 1, 1,-1/
      DATA (NHEL(I, 224),I=1,8) / 1, 1,-1, 1, 1, 1, 1, 1/
      DATA (NHEL(I, 225),I=1,8) / 1, 1, 1,-1,-1,-1,-1,-1/
      DATA (NHEL(I, 226),I=1,8) / 1, 1, 1,-1,-1,-1,-1, 1/
      DATA (NHEL(I, 227),I=1,8) / 1, 1, 1,-1,-1,-1, 1,-1/
      DATA (NHEL(I, 228),I=1,8) / 1, 1, 1,-1,-1,-1, 1, 1/
      DATA (NHEL(I, 229),I=1,8) / 1, 1, 1,-1,-1, 1,-1,-1/
      DATA (NHEL(I, 230),I=1,8) / 1, 1, 1,-1,-1, 1,-1, 1/
      DATA (NHEL(I, 231),I=1,8) / 1, 1, 1,-1,-1, 1, 1,-1/
      DATA (NHEL(I, 232),I=1,8) / 1, 1, 1,-1,-1, 1, 1, 1/
      DATA (NHEL(I, 233),I=1,8) / 1, 1, 1,-1, 1,-1,-1,-1/
      DATA (NHEL(I, 234),I=1,8) / 1, 1, 1,-1, 1,-1,-1, 1/
      DATA (NHEL(I, 235),I=1,8) / 1, 1, 1,-1, 1,-1, 1,-1/
      DATA (NHEL(I, 236),I=1,8) / 1, 1, 1,-1, 1,-1, 1, 1/
      DATA (NHEL(I, 237),I=1,8) / 1, 1, 1,-1, 1, 1,-1,-1/
      DATA (NHEL(I, 238),I=1,8) / 1, 1, 1,-1, 1, 1,-1, 1/
      DATA (NHEL(I, 239),I=1,8) / 1, 1, 1,-1, 1, 1, 1,-1/
      DATA (NHEL(I, 240),I=1,8) / 1, 1, 1,-1, 1, 1, 1, 1/
      DATA (NHEL(I, 241),I=1,8) / 1, 1, 1, 1,-1,-1,-1,-1/
      DATA (NHEL(I, 242),I=1,8) / 1, 1, 1, 1,-1,-1,-1, 1/
      DATA (NHEL(I, 243),I=1,8) / 1, 1, 1, 1,-1,-1, 1,-1/
      DATA (NHEL(I, 244),I=1,8) / 1, 1, 1, 1,-1,-1, 1, 1/
      DATA (NHEL(I, 245),I=1,8) / 1, 1, 1, 1,-1, 1,-1,-1/
      DATA (NHEL(I, 246),I=1,8) / 1, 1, 1, 1,-1, 1,-1, 1/
      DATA (NHEL(I, 247),I=1,8) / 1, 1, 1, 1,-1, 1, 1,-1/
      DATA (NHEL(I, 248),I=1,8) / 1, 1, 1, 1,-1, 1, 1, 1/
      DATA (NHEL(I, 249),I=1,8) / 1, 1, 1, 1, 1,-1,-1,-1/
      DATA (NHEL(I, 250),I=1,8) / 1, 1, 1, 1, 1,-1,-1, 1/
      DATA (NHEL(I, 251),I=1,8) / 1, 1, 1, 1, 1,-1, 1,-1/
      DATA (NHEL(I, 252),I=1,8) / 1, 1, 1, 1, 1,-1, 1, 1/
      DATA (NHEL(I, 253),I=1,8) / 1, 1, 1, 1, 1, 1,-1,-1/
      DATA (NHEL(I, 254),I=1,8) / 1, 1, 1, 1, 1, 1,-1, 1/
      DATA (NHEL(I, 255),I=1,8) / 1, 1, 1, 1, 1, 1, 1,-1/
      DATA (NHEL(I, 256),I=1,8) / 1, 1, 1, 1, 1, 1, 1, 1/
      DATA IDEN/216/
C     ----------
C     BEGIN CODE
C     ----------
      NTRY=NTRY+1
      DO IHEL=1,NEXTERNAL
        JC(IHEL) = +1
      ENDDO
      ANS = 0D0
      DO IHEL=1,NCOMB
        IF (GOODHEL(IHEL) .OR. NTRY .LT. 2) THEN
          T=P0_MATRIX72(P ,NHEL(1,IHEL),JC(1))
          ANS=ANS+T
          IF (T .NE. 0D0 .AND. .NOT.    GOODHEL(IHEL)) THEN
            GOODHEL(IHEL)=.TRUE.
          ENDIF
        ENDIF
      ENDDO
      ANS=ANS/DBLE(IDEN)
      END


      REAL*8 FUNCTION P0_MATRIX72(P,NHEL,IC)
C     
C     Generated by MadGraph5_aMC@NLO v. 2.2.2, 2014-11-06
C     By the MadGraph5_aMC@NLO Development Team
C     Visit launchpad.net/madgraph5 and amcatnlo.web.cern.ch
C     
C     Returns amplitude squared summed/avg over colors
C     for the point with external lines W(0:6,NEXTERNAL)
C     
C     Process: d d > w+ d d d u~ WEIGHTED=6
C     *   Decay: w+ > e+ ve WEIGHTED=2
C     Process: s s > w+ s s s c~ WEIGHTED=6
C     *   Decay: w+ > e+ ve WEIGHTED=2
C     
      IMPLICIT NONE
C     
C     CONSTANTS
C     
      INTEGER    NGRAPHS
      PARAMETER (NGRAPHS=96)
      INTEGER    NEXTERNAL
      PARAMETER (NEXTERNAL=8)
      INTEGER    NWAVEFUNCS, NCOLOR
      PARAMETER (NWAVEFUNCS=55, NCOLOR=6)
      REAL*8     ZERO
      PARAMETER (ZERO=0D0)
      COMPLEX*16 IMAG1
      PARAMETER (IMAG1=(0D0,1D0))
C     
C     ARGUMENTS 
C     
      REAL*8 P(0:3,NEXTERNAL)
      INTEGER NHEL(NEXTERNAL), IC(NEXTERNAL)
C     
C     LOCAL VARIABLES 
C     
      INTEGER I,J
      COMPLEX*16 ZTEMP
      REAL*8 DENOM(NCOLOR), CF(NCOLOR,NCOLOR)
      COMPLEX*16 AMP(NGRAPHS), JAMP(NCOLOR)
      COMPLEX*16 W(18,NWAVEFUNCS)
      COMPLEX*16 DUM0,DUM1
      DATA DUM0, DUM1/(0D0, 0D0), (1D0, 0D0)/
C     
C     GLOBAL VARIABLES
C     
      INCLUDE 'coupl.inc'
C     
C     COLOR DATA
C     
      DATA DENOM(1)/1/
      DATA (CF(I,  1),I=  1,  6) /   27,    9,    9,    3,    3,    9/
C     1 T(5,1) T(6,2) T(7,8)
      DATA DENOM(2)/1/
      DATA (CF(I,  2),I=  1,  6) /    9,   27,    3,    9,    9,    3/
C     1 T(5,1) T(6,8) T(7,2)
      DATA DENOM(3)/1/
      DATA (CF(I,  3),I=  1,  6) /    9,    3,   27,    9,    9,    3/
C     1 T(5,2) T(6,1) T(7,8)
      DATA DENOM(4)/1/
      DATA (CF(I,  4),I=  1,  6) /    3,    9,    9,   27,    3,    9/
C     1 T(5,2) T(6,8) T(7,1)
      DATA DENOM(5)/1/
      DATA (CF(I,  5),I=  1,  6) /    3,    9,    9,    3,   27,    9/
C     1 T(5,8) T(6,1) T(7,2)
      DATA DENOM(6)/1/
      DATA (CF(I,  6),I=  1,  6) /    9,    3,    3,    9,    9,   27/
C     1 T(5,8) T(6,2) T(7,1)
C     ----------
C     BEGIN CODE
C     ----------
      CALL IXXXXX(P(0,1),ZERO,NHEL(1),+1*IC(1),W(1,1))
      CALL IXXXXX(P(0,2),ZERO,NHEL(2),+1*IC(2),W(1,2))
      CALL IXXXXX(P(0,3),ZERO,NHEL(3),-1*IC(3),W(1,3))
      CALL OXXXXX(P(0,4),ZERO,NHEL(4),+1*IC(4),W(1,4))
      CALL FFV2_3(W(1,3),W(1,4),GC_100,MDL_MW,MDL_WW,W(1,5))
      CALL OXXXXX(P(0,5),ZERO,NHEL(5),+1*IC(5),W(1,4))
      CALL OXXXXX(P(0,6),ZERO,NHEL(6),+1*IC(6),W(1,3))
      CALL OXXXXX(P(0,7),ZERO,NHEL(7),+1*IC(7),W(1,6))
      CALL IXXXXX(P(0,8),ZERO,NHEL(8),-1*IC(8),W(1,7))
      CALL FFV2_2(W(1,7),W(1,5),GC_100,ZERO,ZERO,W(1,8))
      CALL FFV1P0_3(W(1,1),W(1,4),GC_11,ZERO,ZERO,W(1,9))
      CALL FFV1P0_3(W(1,8),W(1,3),GC_11,ZERO,ZERO,W(1,10))
      CALL FFV1_1(W(1,6),W(1,9),GC_11,ZERO,ZERO,W(1,11))
C     Amplitude(s) for diagram number 1
      CALL FFV1_0(W(1,2),W(1,11),W(1,10),GC_11,AMP(1))
      CALL FFV1_2(W(1,2),W(1,9),GC_11,ZERO,ZERO,W(1,12))
C     Amplitude(s) for diagram number 2
      CALL FFV1_0(W(1,12),W(1,6),W(1,10),GC_11,AMP(2))
      CALL FFV1P0_3(W(1,8),W(1,6),GC_11,ZERO,ZERO,W(1,13))
      CALL FFV1_1(W(1,3),W(1,9),GC_11,ZERO,ZERO,W(1,14))
C     Amplitude(s) for diagram number 3
      CALL FFV1_0(W(1,2),W(1,14),W(1,13),GC_11,AMP(3))
C     Amplitude(s) for diagram number 4
      CALL FFV1_0(W(1,12),W(1,3),W(1,13),GC_11,AMP(4))
      CALL FFV1P0_3(W(1,2),W(1,3),GC_11,ZERO,ZERO,W(1,15))
      CALL FFV1_2(W(1,8),W(1,9),GC_11,ZERO,ZERO,W(1,16))
C     Amplitude(s) for diagram number 5
      CALL FFV1_0(W(1,16),W(1,6),W(1,15),GC_11,AMP(5))
      CALL FFV1_2(W(1,8),W(1,15),GC_11,ZERO,ZERO,W(1,17))
C     Amplitude(s) for diagram number 6
      CALL FFV1_0(W(1,17),W(1,6),W(1,9),GC_11,AMP(6))
C     Amplitude(s) for diagram number 7
      CALL VVV1_0(W(1,9),W(1,15),W(1,13),GC_10,AMP(7))
      CALL FFV1P0_3(W(1,2),W(1,6),GC_11,ZERO,ZERO,W(1,18))
C     Amplitude(s) for diagram number 8
      CALL FFV1_0(W(1,16),W(1,3),W(1,18),GC_11,AMP(8))
C     Amplitude(s) for diagram number 9
      CALL VVV1_0(W(1,9),W(1,18),W(1,10),GC_10,AMP(9))
      CALL FFV1_2(W(1,8),W(1,18),GC_11,ZERO,ZERO,W(1,16))
C     Amplitude(s) for diagram number 10
      CALL FFV1_0(W(1,16),W(1,3),W(1,9),GC_11,AMP(10))
      CALL FFV1P0_3(W(1,1),W(1,3),GC_11,ZERO,ZERO,W(1,19))
      CALL FFV1P0_3(W(1,8),W(1,4),GC_11,ZERO,ZERO,W(1,20))
      CALL FFV1_1(W(1,6),W(1,19),GC_11,ZERO,ZERO,W(1,21))
C     Amplitude(s) for diagram number 11
      CALL FFV1_0(W(1,2),W(1,21),W(1,20),GC_11,AMP(11))
      CALL FFV1_2(W(1,2),W(1,19),GC_11,ZERO,ZERO,W(1,22))
C     Amplitude(s) for diagram number 12
      CALL FFV1_0(W(1,22),W(1,6),W(1,20),GC_11,AMP(12))
      CALL FFV1_1(W(1,4),W(1,19),GC_11,ZERO,ZERO,W(1,23))
C     Amplitude(s) for diagram number 13
      CALL FFV1_0(W(1,2),W(1,23),W(1,13),GC_11,AMP(13))
C     Amplitude(s) for diagram number 14
      CALL FFV1_0(W(1,22),W(1,4),W(1,13),GC_11,AMP(14))
      CALL FFV1P0_3(W(1,2),W(1,4),GC_11,ZERO,ZERO,W(1,24))
      CALL FFV1_2(W(1,8),W(1,19),GC_11,ZERO,ZERO,W(1,25))
C     Amplitude(s) for diagram number 15
      CALL FFV1_0(W(1,25),W(1,6),W(1,24),GC_11,AMP(15))
      CALL FFV1_2(W(1,8),W(1,24),GC_11,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 16
      CALL FFV1_0(W(1,26),W(1,6),W(1,19),GC_11,AMP(16))
C     Amplitude(s) for diagram number 17
      CALL VVV1_0(W(1,19),W(1,24),W(1,13),GC_10,AMP(17))
C     Amplitude(s) for diagram number 18
      CALL FFV1_0(W(1,25),W(1,4),W(1,18),GC_11,AMP(18))
C     Amplitude(s) for diagram number 19
      CALL VVV1_0(W(1,19),W(1,18),W(1,20),GC_10,AMP(19))
C     Amplitude(s) for diagram number 20
      CALL FFV1_0(W(1,16),W(1,4),W(1,19),GC_11,AMP(20))
      CALL FFV1P0_3(W(1,1),W(1,6),GC_11,ZERO,ZERO,W(1,16))
      CALL FFV1_1(W(1,3),W(1,16),GC_11,ZERO,ZERO,W(1,25))
C     Amplitude(s) for diagram number 21
      CALL FFV1_0(W(1,2),W(1,25),W(1,20),GC_11,AMP(21))
      CALL FFV1_2(W(1,2),W(1,16),GC_11,ZERO,ZERO,W(1,27))
C     Amplitude(s) for diagram number 22
      CALL FFV1_0(W(1,27),W(1,3),W(1,20),GC_11,AMP(22))
      CALL FFV1_1(W(1,4),W(1,16),GC_11,ZERO,ZERO,W(1,28))
C     Amplitude(s) for diagram number 23
      CALL FFV1_0(W(1,2),W(1,28),W(1,10),GC_11,AMP(23))
C     Amplitude(s) for diagram number 24
      CALL FFV1_0(W(1,27),W(1,4),W(1,10),GC_11,AMP(24))
      CALL FFV1_2(W(1,8),W(1,16),GC_11,ZERO,ZERO,W(1,29))
C     Amplitude(s) for diagram number 25
      CALL FFV1_0(W(1,29),W(1,3),W(1,24),GC_11,AMP(25))
C     Amplitude(s) for diagram number 26
      CALL FFV1_0(W(1,26),W(1,3),W(1,16),GC_11,AMP(26))
C     Amplitude(s) for diagram number 27
      CALL VVV1_0(W(1,16),W(1,24),W(1,10),GC_10,AMP(27))
C     Amplitude(s) for diagram number 28
      CALL FFV1_0(W(1,29),W(1,4),W(1,15),GC_11,AMP(28))
C     Amplitude(s) for diagram number 29
      CALL VVV1_0(W(1,16),W(1,15),W(1,20),GC_10,AMP(29))
C     Amplitude(s) for diagram number 30
      CALL FFV1_0(W(1,17),W(1,4),W(1,16),GC_11,AMP(30))
      CALL FFV1_2(W(1,1),W(1,24),GC_11,ZERO,ZERO,W(1,17))
C     Amplitude(s) for diagram number 31
      CALL FFV1_0(W(1,17),W(1,6),W(1,10),GC_11,AMP(31))
      CALL FFV1_1(W(1,6),W(1,24),GC_11,ZERO,ZERO,W(1,29))
C     Amplitude(s) for diagram number 32
      CALL FFV1_0(W(1,1),W(1,29),W(1,10),GC_11,AMP(32))
C     Amplitude(s) for diagram number 33
      CALL FFV1_0(W(1,17),W(1,3),W(1,13),GC_11,AMP(33))
      CALL FFV1_1(W(1,3),W(1,24),GC_11,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 34
      CALL FFV1_0(W(1,1),W(1,26),W(1,13),GC_11,AMP(34))
      CALL FFV1_2(W(1,1),W(1,15),GC_11,ZERO,ZERO,W(1,8))
C     Amplitude(s) for diagram number 35
      CALL FFV1_0(W(1,8),W(1,6),W(1,20),GC_11,AMP(35))
      CALL FFV1_1(W(1,6),W(1,15),GC_11,ZERO,ZERO,W(1,30))
C     Amplitude(s) for diagram number 36
      CALL FFV1_0(W(1,1),W(1,30),W(1,20),GC_11,AMP(36))
C     Amplitude(s) for diagram number 37
      CALL FFV1_0(W(1,8),W(1,4),W(1,13),GC_11,AMP(37))
      CALL FFV1_1(W(1,4),W(1,15),GC_11,ZERO,ZERO,W(1,31))
C     Amplitude(s) for diagram number 38
      CALL FFV1_0(W(1,1),W(1,31),W(1,13),GC_11,AMP(38))
      CALL FFV1_2(W(1,1),W(1,18),GC_11,ZERO,ZERO,W(1,13))
C     Amplitude(s) for diagram number 39
      CALL FFV1_0(W(1,13),W(1,3),W(1,20),GC_11,AMP(39))
      CALL FFV1_1(W(1,3),W(1,18),GC_11,ZERO,ZERO,W(1,32))
C     Amplitude(s) for diagram number 40
      CALL FFV1_0(W(1,1),W(1,32),W(1,20),GC_11,AMP(40))
C     Amplitude(s) for diagram number 41
      CALL FFV1_0(W(1,13),W(1,4),W(1,10),GC_11,AMP(41))
      CALL FFV1_1(W(1,4),W(1,18),GC_11,ZERO,ZERO,W(1,20))
C     Amplitude(s) for diagram number 42
      CALL FFV1_0(W(1,1),W(1,20),W(1,10),GC_11,AMP(42))
      CALL FFV2_1(W(1,3),W(1,5),GC_100,ZERO,ZERO,W(1,10))
      CALL FFV1P0_3(W(1,7),W(1,10),GC_11,ZERO,ZERO,W(1,33))
C     Amplitude(s) for diagram number 43
      CALL FFV1_0(W(1,2),W(1,11),W(1,33),GC_11,AMP(43))
C     Amplitude(s) for diagram number 44
      CALL FFV1_0(W(1,12),W(1,6),W(1,33),GC_11,AMP(44))
      CALL FFV1_2(W(1,7),W(1,9),GC_11,ZERO,ZERO,W(1,34))
C     Amplitude(s) for diagram number 45
      CALL FFV1_0(W(1,34),W(1,10),W(1,18),GC_11,AMP(45))
C     Amplitude(s) for diagram number 46
      CALL VVV1_0(W(1,9),W(1,18),W(1,33),GC_10,AMP(46))
      CALL FFV1_2(W(1,7),W(1,18),GC_11,ZERO,ZERO,W(1,35))
C     Amplitude(s) for diagram number 47
      CALL FFV1_0(W(1,35),W(1,10),W(1,9),GC_11,AMP(47))
      CALL FFV2_1(W(1,6),W(1,5),GC_100,ZERO,ZERO,W(1,36))
      CALL FFV1P0_3(W(1,7),W(1,36),GC_11,ZERO,ZERO,W(1,37))
C     Amplitude(s) for diagram number 48
      CALL FFV1_0(W(1,2),W(1,14),W(1,37),GC_11,AMP(48))
C     Amplitude(s) for diagram number 49
      CALL FFV1_0(W(1,12),W(1,3),W(1,37),GC_11,AMP(49))
C     Amplitude(s) for diagram number 50
      CALL FFV1_0(W(1,34),W(1,36),W(1,15),GC_11,AMP(50))
C     Amplitude(s) for diagram number 51
      CALL VVV1_0(W(1,9),W(1,15),W(1,37),GC_10,AMP(51))
      CALL FFV1_2(W(1,7),W(1,15),GC_11,ZERO,ZERO,W(1,12))
C     Amplitude(s) for diagram number 52
      CALL FFV1_0(W(1,12),W(1,36),W(1,9),GC_11,AMP(52))
C     Amplitude(s) for diagram number 53
      CALL FFV2_0(W(1,34),W(1,30),W(1,5),GC_100,AMP(53))
C     Amplitude(s) for diagram number 54
      CALL FFV2_0(W(1,12),W(1,11),W(1,5),GC_100,AMP(54))
C     Amplitude(s) for diagram number 55
      CALL FFV2_0(W(1,34),W(1,32),W(1,5),GC_100,AMP(55))
C     Amplitude(s) for diagram number 56
      CALL FFV2_0(W(1,35),W(1,14),W(1,5),GC_100,AMP(56))
      CALL FFV2_1(W(1,4),W(1,5),GC_100,ZERO,ZERO,W(1,14))
      CALL FFV1P0_3(W(1,7),W(1,14),GC_11,ZERO,ZERO,W(1,34))
C     Amplitude(s) for diagram number 57
      CALL FFV1_0(W(1,2),W(1,21),W(1,34),GC_11,AMP(57))
C     Amplitude(s) for diagram number 58
      CALL FFV1_0(W(1,22),W(1,6),W(1,34),GC_11,AMP(58))
      CALL FFV1_2(W(1,7),W(1,19),GC_11,ZERO,ZERO,W(1,11))
C     Amplitude(s) for diagram number 59
      CALL FFV1_0(W(1,11),W(1,14),W(1,18),GC_11,AMP(59))
C     Amplitude(s) for diagram number 60
      CALL VVV1_0(W(1,19),W(1,18),W(1,34),GC_10,AMP(60))
C     Amplitude(s) for diagram number 61
      CALL FFV1_0(W(1,35),W(1,14),W(1,19),GC_11,AMP(61))
C     Amplitude(s) for diagram number 62
      CALL FFV1_0(W(1,2),W(1,23),W(1,37),GC_11,AMP(62))
C     Amplitude(s) for diagram number 63
      CALL FFV1_0(W(1,22),W(1,4),W(1,37),GC_11,AMP(63))
C     Amplitude(s) for diagram number 64
      CALL FFV1_0(W(1,11),W(1,36),W(1,24),GC_11,AMP(64))
C     Amplitude(s) for diagram number 65
      CALL VVV1_0(W(1,19),W(1,24),W(1,37),GC_10,AMP(65))
      CALL FFV1_2(W(1,7),W(1,24),GC_11,ZERO,ZERO,W(1,22))
C     Amplitude(s) for diagram number 66
      CALL FFV1_0(W(1,22),W(1,36),W(1,19),GC_11,AMP(66))
C     Amplitude(s) for diagram number 67
      CALL FFV2_0(W(1,11),W(1,29),W(1,5),GC_100,AMP(67))
C     Amplitude(s) for diagram number 68
      CALL FFV2_0(W(1,22),W(1,21),W(1,5),GC_100,AMP(68))
C     Amplitude(s) for diagram number 69
      CALL FFV2_0(W(1,11),W(1,20),W(1,5),GC_100,AMP(69))
C     Amplitude(s) for diagram number 70
      CALL FFV2_0(W(1,35),W(1,23),W(1,5),GC_100,AMP(70))
C     Amplitude(s) for diagram number 71
      CALL FFV1_0(W(1,2),W(1,25),W(1,34),GC_11,AMP(71))
C     Amplitude(s) for diagram number 72
      CALL FFV1_0(W(1,27),W(1,3),W(1,34),GC_11,AMP(72))
      CALL FFV1_2(W(1,7),W(1,16),GC_11,ZERO,ZERO,W(1,35))
C     Amplitude(s) for diagram number 73
      CALL FFV1_0(W(1,35),W(1,14),W(1,15),GC_11,AMP(73))
C     Amplitude(s) for diagram number 74
      CALL VVV1_0(W(1,16),W(1,15),W(1,34),GC_10,AMP(74))
C     Amplitude(s) for diagram number 75
      CALL FFV1_0(W(1,12),W(1,14),W(1,16),GC_11,AMP(75))
C     Amplitude(s) for diagram number 76
      CALL FFV1_0(W(1,2),W(1,28),W(1,33),GC_11,AMP(76))
C     Amplitude(s) for diagram number 77
      CALL FFV1_0(W(1,27),W(1,4),W(1,33),GC_11,AMP(77))
C     Amplitude(s) for diagram number 78
      CALL FFV1_0(W(1,35),W(1,10),W(1,24),GC_11,AMP(78))
C     Amplitude(s) for diagram number 79
      CALL VVV1_0(W(1,16),W(1,24),W(1,33),GC_10,AMP(79))
C     Amplitude(s) for diagram number 80
      CALL FFV1_0(W(1,22),W(1,10),W(1,16),GC_11,AMP(80))
C     Amplitude(s) for diagram number 81
      CALL FFV2_0(W(1,35),W(1,26),W(1,5),GC_100,AMP(81))
C     Amplitude(s) for diagram number 82
      CALL FFV2_0(W(1,22),W(1,25),W(1,5),GC_100,AMP(82))
C     Amplitude(s) for diagram number 83
      CALL FFV2_0(W(1,35),W(1,31),W(1,5),GC_100,AMP(83))
C     Amplitude(s) for diagram number 84
      CALL FFV2_0(W(1,12),W(1,28),W(1,5),GC_100,AMP(84))
C     Amplitude(s) for diagram number 85
      CALL FFV1_0(W(1,8),W(1,6),W(1,34),GC_11,AMP(85))
C     Amplitude(s) for diagram number 86
      CALL FFV1_0(W(1,1),W(1,30),W(1,34),GC_11,AMP(86))
C     Amplitude(s) for diagram number 87
      CALL FFV1_0(W(1,13),W(1,3),W(1,34),GC_11,AMP(87))
C     Amplitude(s) for diagram number 88
      CALL FFV1_0(W(1,1),W(1,32),W(1,34),GC_11,AMP(88))
C     Amplitude(s) for diagram number 89
      CALL FFV1_0(W(1,17),W(1,6),W(1,33),GC_11,AMP(89))
C     Amplitude(s) for diagram number 90
      CALL FFV1_0(W(1,1),W(1,29),W(1,33),GC_11,AMP(90))
C     Amplitude(s) for diagram number 91
      CALL FFV1_0(W(1,13),W(1,4),W(1,33),GC_11,AMP(91))
C     Amplitude(s) for diagram number 92
      CALL FFV1_0(W(1,1),W(1,20),W(1,33),GC_11,AMP(92))
C     Amplitude(s) for diagram number 93
      CALL FFV1_0(W(1,17),W(1,3),W(1,37),GC_11,AMP(93))
C     Amplitude(s) for diagram number 94
      CALL FFV1_0(W(1,1),W(1,26),W(1,37),GC_11,AMP(94))
C     Amplitude(s) for diagram number 95
      CALL FFV1_0(W(1,8),W(1,4),W(1,37),GC_11,AMP(95))
C     Amplitude(s) for diagram number 96
      CALL FFV1_0(W(1,1),W(1,31),W(1,37),GC_11,AMP(96))
      JAMP(1)=+1D0/4D0*(+1D0/3D0*AMP(1)+1D0/3D0*AMP(2)+1D0/9D0*AMP(3)
     $ +1D0/9D0*AMP(4)+1D0/9D0*AMP(5)+1D0/9D0*AMP(6)+1D0/3D0*AMP(8)
     $ +1D0/3D0*AMP(10)+AMP(12)+1D0/3D0*AMP(13)+1D0/3D0*AMP(14)
     $ -IMAG1*AMP(19)+AMP(20)+AMP(23)+AMP(25)+IMAG1*AMP(27)+1D0/3D0
     $ *AMP(28)+1D0/3D0*AMP(30)+AMP(31)+1D0/3D0*AMP(33)+1D0/3D0
     $ *AMP(34)+1D0/3D0*AMP(35)+1D0/3D0*AMP(36)+1D0/9D0*AMP(37)
     $ +1D0/9D0*AMP(38)+AMP(40)+1D0/3D0*AMP(43)+1D0/3D0*AMP(44)
     $ +1D0/3D0*AMP(45)+1D0/3D0*AMP(47)+1D0/9D0*AMP(48)+1D0/9D0
     $ *AMP(49)+1D0/9D0*AMP(50)+1D0/9D0*AMP(52)+1D0/9D0*AMP(53)
     $ +1D0/9D0*AMP(54)+1D0/3D0*AMP(55)+1D0/3D0*AMP(56)+AMP(58)
     $ -IMAG1*AMP(60)+AMP(61)+1D0/3D0*AMP(62)+1D0/3D0*AMP(63)+AMP(70)
     $ +1D0/3D0*AMP(73)+1D0/3D0*AMP(75)+AMP(76)+AMP(78)+IMAG1*AMP(79)
     $ +AMP(81)+1D0/3D0*AMP(83)+1D0/3D0*AMP(84)+1D0/3D0*AMP(85)
     $ +1D0/3D0*AMP(86)+AMP(88)+AMP(89)+1D0/3D0*AMP(93)+1D0/3D0
     $ *AMP(94)+1D0/9D0*AMP(95)+1D0/9D0*AMP(96))
      JAMP(2)=+1D0/4D0*(-1D0/9D0*AMP(1)-1D0/9D0*AMP(2)-1D0/3D0*AMP(3)
     $ -1D0/3D0*AMP(4)-1D0/3D0*AMP(5)-1D0/3D0*AMP(6)-1D0/9D0*AMP(8)
     $ -1D0/9D0*AMP(10)-AMP(13)-AMP(15)-IMAG1*AMP(17)-1D0/3D0*AMP(18)
     $ -1D0/3D0*AMP(20)-AMP(22)-1D0/3D0*AMP(23)-1D0/3D0*AMP(24)
     $ +IMAG1*AMP(29)-AMP(30)-1D0/3D0*AMP(31)-1D0/3D0*AMP(32)-AMP(33)
     $ -AMP(36)-1D0/3D0*AMP(39)-1D0/3D0*AMP(40)-1D0/9D0*AMP(41)
     $ -1D0/9D0*AMP(42)-1D0/9D0*AMP(43)-1D0/9D0*AMP(44)-1D0/9D0
     $ *AMP(45)-1D0/9D0*AMP(47)-1D0/3D0*AMP(48)-1D0/3D0*AMP(49)
     $ -1D0/3D0*AMP(50)-1D0/3D0*AMP(52)-1D0/3D0*AMP(53)-1D0/3D0
     $ *AMP(54)-1D0/9D0*AMP(55)-1D0/9D0*AMP(56)-1D0/3D0*AMP(59)
     $ -1D0/3D0*AMP(61)-AMP(62)-AMP(64)-IMAG1*AMP(65)-AMP(67)
     $ -1D0/3D0*AMP(69)-1D0/3D0*AMP(70)-AMP(72)+IMAG1*AMP(74)-AMP(75)
     $ -1D0/3D0*AMP(76)-1D0/3D0*AMP(77)-AMP(84)-AMP(86)-1D0/3D0
     $ *AMP(87)-1D0/3D0*AMP(88)-1D0/3D0*AMP(89)-1D0/3D0*AMP(90)
     $ -1D0/9D0*AMP(91)-1D0/9D0*AMP(92)-AMP(93))
      JAMP(3)=+1D0/4D0*(-AMP(2)-1D0/3D0*AMP(3)-1D0/3D0*AMP(4)
     $ +IMAG1*AMP(9)-AMP(10)-1D0/3D0*AMP(11)-1D0/3D0*AMP(12)-1D0/9D0
     $ *AMP(13)-1D0/9D0*AMP(14)-1D0/9D0*AMP(15)-1D0/9D0*AMP(16)
     $ -1D0/3D0*AMP(18)-1D0/3D0*AMP(20)-AMP(21)-1D0/3D0*AMP(25)
     $ -1D0/3D0*AMP(26)-AMP(28)-IMAG1*AMP(29)-1D0/3D0*AMP(31)
     $ -1D0/3D0*AMP(32)-1D0/9D0*AMP(33)-1D0/9D0*AMP(34)-AMP(35)
     $ -1D0/3D0*AMP(37)-1D0/3D0*AMP(38)-AMP(42)-AMP(44)+IMAG1*AMP(46)
     $ -AMP(47)-1D0/3D0*AMP(48)-1D0/3D0*AMP(49)-AMP(56)-1D0/3D0
     $ *AMP(57)-1D0/3D0*AMP(58)-1D0/3D0*AMP(59)-1D0/3D0*AMP(61)
     $ -1D0/9D0*AMP(62)-1D0/9D0*AMP(63)-1D0/9D0*AMP(64)-1D0/9D0
     $ *AMP(66)-1D0/9D0*AMP(67)-1D0/9D0*AMP(68)-1D0/3D0*AMP(69)
     $ -1D0/3D0*AMP(70)-AMP(71)-AMP(73)-IMAG1*AMP(74)-1D0/3D0*AMP(78)
     $ -1D0/3D0*AMP(80)-1D0/3D0*AMP(81)-1D0/3D0*AMP(82)-AMP(83)
     $ -AMP(85)-1D0/3D0*AMP(89)-1D0/3D0*AMP(90)-AMP(92)-1D0/9D0
     $ *AMP(93)-1D0/9D0*AMP(94)-1D0/3D0*AMP(95)-1D0/3D0*AMP(96))
      JAMP(4)=+1D0/4D0*(+1D0/3D0*AMP(1)+1D0/3D0*AMP(2)+AMP(4)
     $ +AMP(6)-IMAG1*AMP(7)+AMP(11)+1D0/3D0*AMP(15)+1D0/3D0*AMP(16)
     $ +AMP(18)+IMAG1*AMP(19)+1D0/3D0*AMP(21)+1D0/3D0*AMP(22)
     $ +1D0/9D0*AMP(23)+1D0/9D0*AMP(24)+1D0/9D0*AMP(25)+1D0/9D0
     $ *AMP(26)+1D0/3D0*AMP(28)+1D0/3D0*AMP(30)+1D0/9D0*AMP(31)
     $ +1D0/9D0*AMP(32)+1D0/3D0*AMP(33)+1D0/3D0*AMP(34)+AMP(38)
     $ +AMP(39)+1D0/3D0*AMP(41)+1D0/3D0*AMP(42)+1D0/3D0*AMP(43)
     $ +1D0/3D0*AMP(44)+AMP(49)-IMAG1*AMP(51)+AMP(52)+AMP(54)+AMP(57)
     $ +AMP(59)+IMAG1*AMP(60)+1D0/3D0*AMP(64)+1D0/3D0*AMP(66)
     $ +1D0/3D0*AMP(67)+1D0/3D0*AMP(68)+AMP(69)+1D0/3D0*AMP(71)
     $ +1D0/3D0*AMP(72)+1D0/3D0*AMP(73)+1D0/3D0*AMP(75)+1D0/9D0
     $ *AMP(76)+1D0/9D0*AMP(77)+1D0/9D0*AMP(78)+1D0/9D0*AMP(80)
     $ +1D0/9D0*AMP(81)+1D0/9D0*AMP(82)+1D0/3D0*AMP(83)+1D0/3D0
     $ *AMP(84)+AMP(87)+1D0/9D0*AMP(89)+1D0/9D0*AMP(90)+1D0/3D0
     $ *AMP(91)+1D0/3D0*AMP(92)+1D0/3D0*AMP(93)+1D0/3D0*AMP(94)
     $ +AMP(96))
      JAMP(5)=+1D0/4D0*(+AMP(3)+AMP(5)+IMAG1*AMP(7)+1D0/3D0*AMP(8)
     $ +1D0/3D0*AMP(10)+1D0/9D0*AMP(11)+1D0/9D0*AMP(12)+1D0/3D0
     $ *AMP(13)+1D0/3D0*AMP(14)+1D0/3D0*AMP(15)+1D0/3D0*AMP(16)
     $ +1D0/9D0*AMP(18)+1D0/9D0*AMP(20)+1D0/3D0*AMP(21)+1D0/3D0
     $ *AMP(22)+AMP(24)+AMP(26)-IMAG1*AMP(27)+AMP(32)+1D0/3D0*AMP(35)
     $ +1D0/3D0*AMP(36)+AMP(37)+1D0/9D0*AMP(39)+1D0/9D0*AMP(40)
     $ +1D0/3D0*AMP(41)+1D0/3D0*AMP(42)+1D0/3D0*AMP(45)+1D0/3D0
     $ *AMP(47)+AMP(48)+AMP(50)+IMAG1*AMP(51)+AMP(53)+1D0/3D0*AMP(55)
     $ +1D0/3D0*AMP(56)+1D0/9D0*AMP(57)+1D0/9D0*AMP(58)+1D0/9D0
     $ *AMP(59)+1D0/9D0*AMP(61)+1D0/3D0*AMP(62)+1D0/3D0*AMP(63)
     $ +1D0/3D0*AMP(64)+1D0/3D0*AMP(66)+1D0/3D0*AMP(67)+1D0/3D0
     $ *AMP(68)+1D0/9D0*AMP(69)+1D0/9D0*AMP(70)+1D0/3D0*AMP(71)
     $ +1D0/3D0*AMP(72)+AMP(77)-IMAG1*AMP(79)+AMP(80)+AMP(82)
     $ +1D0/3D0*AMP(85)+1D0/3D0*AMP(86)+1D0/9D0*AMP(87)+1D0/9D0
     $ *AMP(88)+AMP(90)+1D0/3D0*AMP(91)+1D0/3D0*AMP(92)+AMP(95))
      JAMP(6)=+1D0/4D0*(-AMP(1)-1D0/3D0*AMP(5)-1D0/3D0*AMP(6)-AMP(8)
     $ -IMAG1*AMP(9)-1D0/3D0*AMP(11)-1D0/3D0*AMP(12)-AMP(14)-AMP(16)
     $ +IMAG1*AMP(17)-1D0/9D0*AMP(21)-1D0/9D0*AMP(22)-1D0/3D0*AMP(23)
     $ -1D0/3D0*AMP(24)-1D0/3D0*AMP(25)-1D0/3D0*AMP(26)-1D0/9D0
     $ *AMP(28)-1D0/9D0*AMP(30)-AMP(34)-1D0/9D0*AMP(35)-1D0/9D0
     $ *AMP(36)-1D0/3D0*AMP(37)-1D0/3D0*AMP(38)-1D0/3D0*AMP(39)
     $ -1D0/3D0*AMP(40)-AMP(41)-AMP(43)-AMP(45)-IMAG1*AMP(46)
     $ -1D0/3D0*AMP(50)-1D0/3D0*AMP(52)-1D0/3D0*AMP(53)-1D0/3D0
     $ *AMP(54)-AMP(55)-1D0/3D0*AMP(57)-1D0/3D0*AMP(58)-AMP(63)
     $ +IMAG1*AMP(65)-AMP(66)-AMP(68)-1D0/9D0*AMP(71)-1D0/9D0*AMP(72)
     $ -1D0/9D0*AMP(73)-1D0/9D0*AMP(75)-1D0/3D0*AMP(76)-1D0/3D0
     $ *AMP(77)-1D0/3D0*AMP(78)-1D0/3D0*AMP(80)-1D0/3D0*AMP(81)
     $ -1D0/3D0*AMP(82)-1D0/9D0*AMP(83)-1D0/9D0*AMP(84)-1D0/9D0
     $ *AMP(85)-1D0/9D0*AMP(86)-1D0/3D0*AMP(87)-1D0/3D0*AMP(88)
     $ -AMP(91)-AMP(94)-1D0/3D0*AMP(95)-1D0/3D0*AMP(96))

      P0_MATRIX72 = 0.D0
      DO I = 1, NCOLOR
        ZTEMP = (0.D0,0.D0)
        DO J = 1, NCOLOR
          ZTEMP = ZTEMP + CF(J,I)*JAMP(J)
        ENDDO
        P0_MATRIX72 = P0_MATRIX72+ZTEMP*DCONJG(JAMP(I))/DENOM(I)
      ENDDO
      END

