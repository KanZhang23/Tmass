      SUBROUTINE SP0_MATRIX22(P,ANS)
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
C     Process: u c > w+ g g u s WEIGHTED=6
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
      REAL*8 P0_MATRIX22
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
      DATA IDEN/72/
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
          T=P0_MATRIX22(P ,NHEL(1,IHEL),JC(1))
          ANS=ANS+T
          IF (T .NE. 0D0 .AND. .NOT.    GOODHEL(IHEL)) THEN
            GOODHEL(IHEL)=.TRUE.
          ENDIF
        ENDIF
      ENDDO
      ANS=ANS/DBLE(IDEN)
      END


      REAL*8 FUNCTION P0_MATRIX22(P,NHEL,IC)
C     
C     Generated by MadGraph5_aMC@NLO v. 2.2.2, 2014-11-06
C     By the MadGraph5_aMC@NLO Development Team
C     Visit launchpad.net/madgraph5 and amcatnlo.web.cern.ch
C     
C     Returns amplitude squared summed/avg over colors
C     for the point with external lines W(0:6,NEXTERNAL)
C     
C     Process: u c > w+ g g u s WEIGHTED=6
C     *   Decay: w+ > e+ ve WEIGHTED=2
C     
      IMPLICIT NONE
C     
C     CONSTANTS
C     
      INTEGER    NGRAPHS
      PARAMETER (NGRAPHS=102)
      INTEGER    NEXTERNAL
      PARAMETER (NEXTERNAL=8)
      INTEGER    NWAVEFUNCS, NCOLOR
      PARAMETER (NWAVEFUNCS=63, NCOLOR=12)
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
      DATA DENOM(1)/3/
      DATA (CF(I,  1),I=  1,  6) /   48,   16,   16,    6,    0,   16/
      DATA (CF(I,  1),I=  7, 12) /   -2,    0,   -6,   -2,   -2,    6/
C     1 T(5,6,7,1) T(8,2)
      DATA DENOM(2)/3/
      DATA (CF(I,  2),I=  1,  6) /   16,   48,    6,   16,   16,    0/
      DATA (CF(I,  2),I=  7, 12) /    0,   -2,   -2,   -6,    6,   -2/
C     1 T(5,6,7,2) T(8,1)
      DATA DENOM(3)/3/
      DATA (CF(I,  3),I=  1,  6) /   16,    6,   48,   16,   -2,    0/
      DATA (CF(I,  3),I=  7, 12) /    0,   16,   -2,    6,   -6,   -2/
C     1 T(5,6,8,1) T(7,2)
      DATA DENOM(4)/3/
      DATA (CF(I,  4),I=  1,  6) /    6,   16,   16,   48,    0,   -2/
      DATA (CF(I,  4),I=  7, 12) /   16,    0,    6,   -2,   -2,   -6/
C     1 T(5,6,8,2) T(7,1)
      DATA DENOM(5)/3/
      DATA (CF(I,  5),I=  1,  6) /    0,   16,   -2,    0,   48,   16/
      DATA (CF(I,  5),I=  7, 12) /   16,    6,    0,   -2,   16,    0/
C     1 T(5,7,1) T(6,8,2)
      DATA DENOM(6)/3/
      DATA (CF(I,  6),I=  1,  6) /   16,    0,    0,   -2,   16,   48/
      DATA (CF(I,  6),I=  7, 12) /    6,   16,   -2,    0,    0,   16/
C     1 T(5,7,2) T(6,8,1)
      DATA DENOM(7)/3/
      DATA (CF(I,  7),I=  1,  6) /   -2,    0,    0,   16,   16,    6/
      DATA (CF(I,  7),I=  7, 12) /   48,   16,   16,    0,    0,   -2/
C     1 T(5,8,1) T(6,7,2)
      DATA DENOM(8)/3/
      DATA (CF(I,  8),I=  1,  6) /    0,   -2,   16,    0,    6,   16/
      DATA (CF(I,  8),I=  7, 12) /   16,   48,    0,   16,   -2,    0/
C     1 T(5,8,2) T(6,7,1)
      DATA DENOM(9)/3/
      DATA (CF(I,  9),I=  1,  6) /   -6,   -2,   -2,    6,    0,   -2/
      DATA (CF(I,  9),I=  7, 12) /   16,    0,   48,   16,   16,    6/
C     1 T(6,5,7,1) T(8,2)
      DATA DENOM(10)/3/
      DATA (CF(I, 10),I=  1,  6) /   -2,   -6,    6,   -2,   -2,    0/
      DATA (CF(I, 10),I=  7, 12) /    0,   16,   16,   48,    6,   16/
C     1 T(6,5,7,2) T(8,1)
      DATA DENOM(11)/3/
      DATA (CF(I, 11),I=  1,  6) /   -2,    6,   -6,   -2,   16,    0/
      DATA (CF(I, 11),I=  7, 12) /    0,   -2,   16,    6,   48,   16/
C     1 T(6,5,8,1) T(7,2)
      DATA DENOM(12)/3/
      DATA (CF(I, 12),I=  1,  6) /    6,   -2,   -2,   -6,    0,   16/
      DATA (CF(I, 12),I=  7, 12) /   -2,    0,    6,   16,   16,   48/
C     1 T(6,5,8,2) T(7,1)
C     ----------
C     BEGIN CODE
C     ----------
      CALL IXXXXX(P(0,1),ZERO,NHEL(1),+1*IC(1),W(1,1))
      CALL IXXXXX(P(0,2),ZERO,NHEL(2),+1*IC(2),W(1,2))
      CALL IXXXXX(P(0,3),ZERO,NHEL(3),-1*IC(3),W(1,3))
      CALL OXXXXX(P(0,4),ZERO,NHEL(4),+1*IC(4),W(1,4))
      CALL FFV2_3(W(1,3),W(1,4),GC_100,MDL_MW,MDL_WW,W(1,5))
      CALL VXXXXX(P(0,5),ZERO,NHEL(5),+1*IC(5),W(1,4))
      CALL VXXXXX(P(0,6),ZERO,NHEL(6),+1*IC(6),W(1,3))
      CALL OXXXXX(P(0,7),ZERO,NHEL(7),+1*IC(7),W(1,6))
      CALL OXXXXX(P(0,8),ZERO,NHEL(8),+1*IC(8),W(1,7))
      CALL VVV1P0_1(W(1,4),W(1,3),GC_10,ZERO,ZERO,W(1,8))
      CALL FFV2_1(W(1,7),W(1,5),GC_100,ZERO,ZERO,W(1,9))
      CALL FFV1_1(W(1,6),W(1,8),GC_11,ZERO,ZERO,W(1,10))
      CALL FFV1P0_3(W(1,2),W(1,9),GC_11,ZERO,ZERO,W(1,11))
C     Amplitude(s) for diagram number 1
      CALL FFV1_0(W(1,1),W(1,10),W(1,11),GC_11,AMP(1))
      CALL FFV1_2(W(1,1),W(1,8),GC_11,ZERO,ZERO,W(1,12))
C     Amplitude(s) for diagram number 2
      CALL FFV1_0(W(1,12),W(1,6),W(1,11),GC_11,AMP(2))
      CALL FFV1P0_3(W(1,1),W(1,6),GC_11,ZERO,ZERO,W(1,13))
      CALL FFV1_1(W(1,9),W(1,8),GC_11,ZERO,ZERO,W(1,14))
C     Amplitude(s) for diagram number 3
      CALL FFV1_0(W(1,2),W(1,14),W(1,13),GC_11,AMP(3))
      CALL VVV1P0_1(W(1,8),W(1,13),GC_10,ZERO,ZERO,W(1,14))
C     Amplitude(s) for diagram number 4
      CALL FFV1_0(W(1,2),W(1,9),W(1,14),GC_11,AMP(4))
      CALL FFV1_2(W(1,2),W(1,8),GC_11,ZERO,ZERO,W(1,15))
C     Amplitude(s) for diagram number 5
      CALL FFV1_0(W(1,15),W(1,9),W(1,13),GC_11,AMP(5))
      CALL FFV2_2(W(1,2),W(1,5),GC_100,ZERO,ZERO,W(1,16))
      CALL FFV1P0_3(W(1,16),W(1,7),GC_11,ZERO,ZERO,W(1,17))
C     Amplitude(s) for diagram number 6
      CALL FFV1_0(W(1,1),W(1,10),W(1,17),GC_11,AMP(6))
C     Amplitude(s) for diagram number 7
      CALL FFV1_0(W(1,12),W(1,6),W(1,17),GC_11,AMP(7))
      CALL FFV1_2(W(1,16),W(1,8),GC_11,ZERO,ZERO,W(1,12))
C     Amplitude(s) for diagram number 8
      CALL FFV1_0(W(1,12),W(1,7),W(1,13),GC_11,AMP(8))
C     Amplitude(s) for diagram number 9
      CALL FFV1_0(W(1,16),W(1,7),W(1,14),GC_11,AMP(9))
      CALL FFV1_1(W(1,7),W(1,8),GC_11,ZERO,ZERO,W(1,14))
C     Amplitude(s) for diagram number 10
      CALL FFV1_0(W(1,16),W(1,14),W(1,13),GC_11,AMP(10))
      CALL FFV1_2(W(1,2),W(1,13),GC_11,ZERO,ZERO,W(1,8))
C     Amplitude(s) for diagram number 11
      CALL FFV2_0(W(1,8),W(1,14),W(1,5),GC_100,AMP(11))
      CALL FFV1_1(W(1,7),W(1,13),GC_11,ZERO,ZERO,W(1,14))
C     Amplitude(s) for diagram number 12
      CALL FFV2_0(W(1,15),W(1,14),W(1,5),GC_100,AMP(12))
      CALL FFV1_1(W(1,6),W(1,4),GC_11,ZERO,ZERO,W(1,15))
      CALL FFV1_1(W(1,7),W(1,3),GC_11,ZERO,ZERO,W(1,12))
      CALL FFV1P0_3(W(1,1),W(1,15),GC_11,ZERO,ZERO,W(1,10))
      CALL FFV2_1(W(1,12),W(1,5),GC_100,ZERO,ZERO,W(1,18))
C     Amplitude(s) for diagram number 13
      CALL FFV1_0(W(1,2),W(1,18),W(1,10),GC_11,AMP(13))
C     Amplitude(s) for diagram number 14
      CALL FFV1_0(W(1,16),W(1,12),W(1,10),GC_11,AMP(14))
      CALL FFV1_2(W(1,1),W(1,3),GC_11,ZERO,ZERO,W(1,19))
      CALL FFV1P0_3(W(1,19),W(1,15),GC_11,ZERO,ZERO,W(1,20))
C     Amplitude(s) for diagram number 15
      CALL FFV1_0(W(1,2),W(1,9),W(1,20),GC_11,AMP(15))
C     Amplitude(s) for diagram number 16
      CALL FFV1_0(W(1,16),W(1,7),W(1,20),GC_11,AMP(16))
      CALL FFV1_2(W(1,2),W(1,3),GC_11,ZERO,ZERO,W(1,20))
      CALL FFV2_2(W(1,20),W(1,5),GC_100,ZERO,ZERO,W(1,21))
C     Amplitude(s) for diagram number 17
      CALL FFV1_0(W(1,21),W(1,7),W(1,10),GC_11,AMP(17))
C     Amplitude(s) for diagram number 18
      CALL FFV1_0(W(1,20),W(1,9),W(1,10),GC_11,AMP(18))
      CALL FFV1_1(W(1,15),W(1,3),GC_11,ZERO,ZERO,W(1,22))
C     Amplitude(s) for diagram number 19
      CALL FFV1_0(W(1,1),W(1,22),W(1,11),GC_11,AMP(19))
      CALL FFV1_1(W(1,9),W(1,3),GC_11,ZERO,ZERO,W(1,15))
C     Amplitude(s) for diagram number 20
      CALL FFV1_0(W(1,2),W(1,15),W(1,10),GC_11,AMP(20))
C     Amplitude(s) for diagram number 21
      CALL VVV1_0(W(1,10),W(1,3),W(1,11),GC_10,AMP(21))
C     Amplitude(s) for diagram number 22
      CALL FFV1_0(W(1,1),W(1,22),W(1,17),GC_11,AMP(22))
      CALL FFV1_2(W(1,16),W(1,3),GC_11,ZERO,ZERO,W(1,22))
C     Amplitude(s) for diagram number 23
      CALL FFV1_0(W(1,22),W(1,7),W(1,10),GC_11,AMP(23))
C     Amplitude(s) for diagram number 24
      CALL VVV1_0(W(1,10),W(1,3),W(1,17),GC_10,AMP(24))
      CALL FFV1_1(W(1,7),W(1,4),GC_11,ZERO,ZERO,W(1,10))
      CALL FFV1_1(W(1,6),W(1,3),GC_11,ZERO,ZERO,W(1,23))
      CALL FFV2_1(W(1,10),W(1,5),GC_100,ZERO,ZERO,W(1,24))
      CALL FFV1P0_3(W(1,1),W(1,23),GC_11,ZERO,ZERO,W(1,25))
C     Amplitude(s) for diagram number 25
      CALL FFV1_0(W(1,2),W(1,24),W(1,25),GC_11,AMP(25))
      CALL FFV1P0_3(W(1,16),W(1,10),GC_11,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 26
      CALL FFV1_0(W(1,1),W(1,23),W(1,26),GC_11,AMP(26))
      CALL FFV1P0_3(W(1,19),W(1,6),GC_11,ZERO,ZERO,W(1,27))
C     Amplitude(s) for diagram number 27
      CALL FFV1_0(W(1,2),W(1,24),W(1,27),GC_11,AMP(27))
C     Amplitude(s) for diagram number 28
      CALL FFV1_0(W(1,19),W(1,6),W(1,26),GC_11,AMP(28))
C     Amplitude(s) for diagram number 29
      CALL FFV1_0(W(1,20),W(1,24),W(1,13),GC_11,AMP(29))
      CALL FFV1_1(W(1,10),W(1,13),GC_11,ZERO,ZERO,W(1,28))
C     Amplitude(s) for diagram number 30
      CALL FFV2_0(W(1,20),W(1,28),W(1,5),GC_100,AMP(30))
      CALL FFV1_1(W(1,10),W(1,3),GC_11,ZERO,ZERO,W(1,29))
C     Amplitude(s) for diagram number 31
      CALL FFV1_0(W(1,16),W(1,29),W(1,13),GC_11,AMP(31))
C     Amplitude(s) for diagram number 32
      CALL VVV1_0(W(1,3),W(1,13),W(1,26),GC_10,AMP(32))
C     Amplitude(s) for diagram number 33
      CALL FFV1_0(W(1,16),W(1,28),W(1,3),GC_11,AMP(33))
C     Amplitude(s) for diagram number 34
      CALL FFV2_0(W(1,8),W(1,29),W(1,5),GC_100,AMP(34))
      CALL VVV1P0_1(W(1,3),W(1,13),GC_10,ZERO,ZERO,W(1,29))
C     Amplitude(s) for diagram number 35
      CALL FFV1_0(W(1,2),W(1,24),W(1,29),GC_11,AMP(35))
C     Amplitude(s) for diagram number 36
      CALL FFV1_0(W(1,8),W(1,24),W(1,3),GC_11,AMP(36))
      CALL FFV1_2(W(1,1),W(1,4),GC_11,ZERO,ZERO,W(1,24))
      CALL FFV1P0_3(W(1,24),W(1,23),GC_11,ZERO,ZERO,W(1,28))
C     Amplitude(s) for diagram number 37
      CALL FFV1_0(W(1,2),W(1,9),W(1,28),GC_11,AMP(37))
C     Amplitude(s) for diagram number 38
      CALL FFV1_0(W(1,16),W(1,7),W(1,28),GC_11,AMP(38))
      CALL FFV1P0_3(W(1,24),W(1,6),GC_11,ZERO,ZERO,W(1,28))
C     Amplitude(s) for diagram number 39
      CALL FFV1_0(W(1,2),W(1,18),W(1,28),GC_11,AMP(39))
C     Amplitude(s) for diagram number 40
      CALL FFV1_0(W(1,16),W(1,12),W(1,28),GC_11,AMP(40))
C     Amplitude(s) for diagram number 41
      CALL FFV1_0(W(1,21),W(1,7),W(1,28),GC_11,AMP(41))
C     Amplitude(s) for diagram number 42
      CALL FFV1_0(W(1,20),W(1,9),W(1,28),GC_11,AMP(42))
      CALL FFV1_2(W(1,24),W(1,3),GC_11,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 43
      CALL FFV1_0(W(1,26),W(1,6),W(1,11),GC_11,AMP(43))
C     Amplitude(s) for diagram number 44
      CALL FFV1_0(W(1,2),W(1,15),W(1,28),GC_11,AMP(44))
C     Amplitude(s) for diagram number 45
      CALL VVV1_0(W(1,28),W(1,3),W(1,11),GC_10,AMP(45))
C     Amplitude(s) for diagram number 46
      CALL FFV1_0(W(1,26),W(1,6),W(1,17),GC_11,AMP(46))
C     Amplitude(s) for diagram number 47
      CALL FFV1_0(W(1,22),W(1,7),W(1,28),GC_11,AMP(47))
C     Amplitude(s) for diagram number 48
      CALL VVV1_0(W(1,28),W(1,3),W(1,17),GC_10,AMP(48))
      CALL FFV1_2(W(1,2),W(1,4),GC_11,ZERO,ZERO,W(1,28))
      CALL FFV2_2(W(1,28),W(1,5),GC_100,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 49
      CALL FFV1_0(W(1,26),W(1,7),W(1,25),GC_11,AMP(49))
      CALL FFV1P0_3(W(1,28),W(1,9),GC_11,ZERO,ZERO,W(1,24))
C     Amplitude(s) for diagram number 50
      CALL FFV1_0(W(1,1),W(1,23),W(1,24),GC_11,AMP(50))
C     Amplitude(s) for diagram number 51
      CALL FFV1_0(W(1,26),W(1,12),W(1,13),GC_11,AMP(51))
      CALL FFV1_2(W(1,28),W(1,13),GC_11,ZERO,ZERO,W(1,10))
C     Amplitude(s) for diagram number 52
      CALL FFV2_0(W(1,10),W(1,12),W(1,5),GC_100,AMP(52))
C     Amplitude(s) for diagram number 53
      CALL FFV1_0(W(1,26),W(1,7),W(1,27),GC_11,AMP(53))
C     Amplitude(s) for diagram number 54
      CALL FFV1_0(W(1,19),W(1,6),W(1,24),GC_11,AMP(54))
      CALL FFV1_2(W(1,28),W(1,3),GC_11,ZERO,ZERO,W(1,30))
C     Amplitude(s) for diagram number 55
      CALL FFV1_0(W(1,30),W(1,9),W(1,13),GC_11,AMP(55))
C     Amplitude(s) for diagram number 56
      CALL VVV1_0(W(1,3),W(1,13),W(1,24),GC_10,AMP(56))
C     Amplitude(s) for diagram number 57
      CALL FFV1_0(W(1,10),W(1,9),W(1,3),GC_11,AMP(57))
C     Amplitude(s) for diagram number 58
      CALL FFV2_0(W(1,30),W(1,14),W(1,5),GC_100,AMP(58))
C     Amplitude(s) for diagram number 59
      CALL FFV1_0(W(1,26),W(1,7),W(1,29),GC_11,AMP(59))
C     Amplitude(s) for diagram number 60
      CALL FFV1_0(W(1,26),W(1,14),W(1,3),GC_11,AMP(60))
      CALL FFV1_1(W(1,23),W(1,4),GC_11,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 61
      CALL FFV1_0(W(1,1),W(1,26),W(1,11),GC_11,AMP(61))
      CALL FFV1_1(W(1,9),W(1,4),GC_11,ZERO,ZERO,W(1,23))
C     Amplitude(s) for diagram number 62
      CALL FFV1_0(W(1,2),W(1,23),W(1,25),GC_11,AMP(62))
C     Amplitude(s) for diagram number 63
      CALL VVV1_0(W(1,4),W(1,25),W(1,11),GC_10,AMP(63))
C     Amplitude(s) for diagram number 64
      CALL FFV1_0(W(1,1),W(1,26),W(1,17),GC_11,AMP(64))
      CALL FFV1_2(W(1,16),W(1,4),GC_11,ZERO,ZERO,W(1,26))
C     Amplitude(s) for diagram number 65
      CALL FFV1_0(W(1,26),W(1,7),W(1,25),GC_11,AMP(65))
C     Amplitude(s) for diagram number 66
      CALL VVV1_0(W(1,4),W(1,25),W(1,17),GC_10,AMP(66))
      CALL FFV1_1(W(1,12),W(1,4),GC_11,ZERO,ZERO,W(1,25))
C     Amplitude(s) for diagram number 67
      CALL FFV1_0(W(1,16),W(1,25),W(1,13),GC_11,AMP(67))
C     Amplitude(s) for diagram number 68
      CALL FFV1_0(W(1,26),W(1,12),W(1,13),GC_11,AMP(68))
      CALL VVV1P0_1(W(1,4),W(1,13),GC_10,ZERO,ZERO,W(1,1))
C     Amplitude(s) for diagram number 69
      CALL FFV1_0(W(1,16),W(1,12),W(1,1),GC_11,AMP(69))
C     Amplitude(s) for diagram number 70
      CALL FFV2_0(W(1,8),W(1,25),W(1,5),GC_100,AMP(70))
C     Amplitude(s) for diagram number 71
      CALL FFV1_0(W(1,2),W(1,18),W(1,1),GC_11,AMP(71))
C     Amplitude(s) for diagram number 72
      CALL FFV1_0(W(1,8),W(1,18),W(1,4),GC_11,AMP(72))
      CALL FFV1_2(W(1,19),W(1,4),GC_11,ZERO,ZERO,W(1,18))
C     Amplitude(s) for diagram number 73
      CALL FFV1_0(W(1,18),W(1,6),W(1,11),GC_11,AMP(73))
C     Amplitude(s) for diagram number 74
      CALL FFV1_0(W(1,2),W(1,23),W(1,27),GC_11,AMP(74))
C     Amplitude(s) for diagram number 75
      CALL VVV1_0(W(1,4),W(1,27),W(1,11),GC_10,AMP(75))
C     Amplitude(s) for diagram number 76
      CALL FFV1_0(W(1,18),W(1,6),W(1,17),GC_11,AMP(76))
C     Amplitude(s) for diagram number 77
      CALL FFV1_0(W(1,26),W(1,7),W(1,27),GC_11,AMP(77))
C     Amplitude(s) for diagram number 78
      CALL VVV1_0(W(1,4),W(1,27),W(1,17),GC_10,AMP(78))
      CALL FFV1_2(W(1,20),W(1,4),GC_11,ZERO,ZERO,W(1,27))
C     Amplitude(s) for diagram number 79
      CALL FFV1_0(W(1,27),W(1,9),W(1,13),GC_11,AMP(79))
C     Amplitude(s) for diagram number 80
      CALL FFV1_0(W(1,20),W(1,23),W(1,13),GC_11,AMP(80))
C     Amplitude(s) for diagram number 81
      CALL FFV1_0(W(1,20),W(1,9),W(1,1),GC_11,AMP(81))
C     Amplitude(s) for diagram number 82
      CALL FFV2_0(W(1,27),W(1,14),W(1,5),GC_100,AMP(82))
C     Amplitude(s) for diagram number 83
      CALL FFV1_0(W(1,21),W(1,7),W(1,1),GC_11,AMP(83))
C     Amplitude(s) for diagram number 84
      CALL FFV1_0(W(1,21),W(1,14),W(1,4),GC_11,AMP(84))
C     Amplitude(s) for diagram number 85
      CALL FFV1_0(W(1,2),W(1,23),W(1,29),GC_11,AMP(85))
C     Amplitude(s) for diagram number 86
      CALL FFV1_0(W(1,8),W(1,23),W(1,3),GC_11,AMP(86))
C     Amplitude(s) for diagram number 87
      CALL FFV1_0(W(1,2),W(1,15),W(1,1),GC_11,AMP(87))
C     Amplitude(s) for diagram number 88
      CALL VVV1_0(W(1,1),W(1,3),W(1,11),GC_10,AMP(88))
C     Amplitude(s) for diagram number 89
      CALL FFV1_0(W(1,8),W(1,15),W(1,4),GC_11,AMP(89))
C     Amplitude(s) for diagram number 90
      CALL VVV1_0(W(1,4),W(1,29),W(1,11),GC_10,AMP(90))
      CALL VVVV1P0_1(W(1,4),W(1,3),W(1,13),GC_12,ZERO,ZERO,W(1,11))
      CALL VVVV3P0_1(W(1,4),W(1,3),W(1,13),GC_12,ZERO,ZERO,W(1,15))
      CALL VVVV4P0_1(W(1,4),W(1,3),W(1,13),GC_12,ZERO,ZERO,W(1,8))
C     Amplitude(s) for diagram number 91
      CALL FFV1_0(W(1,2),W(1,9),W(1,11),GC_11,AMP(91))
      CALL FFV1_0(W(1,2),W(1,9),W(1,15),GC_11,AMP(92))
      CALL FFV1_0(W(1,2),W(1,9),W(1,8),GC_11,AMP(93))
C     Amplitude(s) for diagram number 92
      CALL FFV1_0(W(1,26),W(1,7),W(1,29),GC_11,AMP(94))
C     Amplitude(s) for diagram number 93
      CALL FFV1_0(W(1,26),W(1,14),W(1,3),GC_11,AMP(95))
C     Amplitude(s) for diagram number 94
      CALL FFV1_0(W(1,22),W(1,7),W(1,1),GC_11,AMP(96))
C     Amplitude(s) for diagram number 95
      CALL VVV1_0(W(1,1),W(1,3),W(1,17),GC_10,AMP(97))
C     Amplitude(s) for diagram number 96
      CALL FFV1_0(W(1,22),W(1,14),W(1,4),GC_11,AMP(98))
C     Amplitude(s) for diagram number 97
      CALL VVV1_0(W(1,4),W(1,29),W(1,17),GC_10,AMP(99))
C     Amplitude(s) for diagram number 98
      CALL FFV1_0(W(1,16),W(1,7),W(1,11),GC_11,AMP(100))
      CALL FFV1_0(W(1,16),W(1,7),W(1,15),GC_11,AMP(101))
      CALL FFV1_0(W(1,16),W(1,7),W(1,8),GC_11,AMP(102))
      JAMP(1)=+1D0/2D0*(-1D0/3D0*IMAG1*AMP(1)-1D0/3D0*IMAG1*AMP(2)
     $ -1D0/3D0*IMAG1*AMP(6)-1D0/3D0*IMAG1*AMP(7)+1D0/3D0*AMP(15)
     $ +1D0/3D0*AMP(16)+1D0/3D0*AMP(19)+1D0/3D0*AMP(22)+1D0/3D0
     $ *AMP(73)+1D0/3D0*AMP(76))
      JAMP(2)=+1D0/2D0*(+IMAG1*AMP(1)-AMP(4)+IMAG1*AMP(5)+IMAG1*AMP(6)
     $ +IMAG1*AMP(8)-AMP(9)+IMAG1*AMP(12)-AMP(17)-AMP(18)-AMP(19)
     $ +IMAG1*AMP(21)-AMP(22)-AMP(23)+IMAG1*AMP(24)-AMP(79)-IMAG1
     $ *AMP(81)-AMP(82)-IMAG1*AMP(83)-AMP(84)-AMP(88)+AMP(93)
     $ +AMP(92)-IMAG1*AMP(96)-AMP(97)-AMP(98)+AMP(102)+AMP(101))
      JAMP(3)=+1D0/2D0*(+IMAG1*AMP(2)+IMAG1*AMP(3)+AMP(4)+IMAG1*AMP(7)
     $ +AMP(9)+IMAG1*AMP(10)+IMAG1*AMP(11)-AMP(27)-AMP(28)-AMP(31)
     $ +IMAG1*AMP(32)-AMP(34)+IMAG1*AMP(35)-AMP(36)-AMP(73)-AMP(74)
     $ +IMAG1*AMP(75)-AMP(76)+IMAG1*AMP(78)+IMAG1*AMP(85)-AMP(86)
     $ +AMP(90)-AMP(93)+AMP(91)+AMP(99)-AMP(102)+AMP(100))
      JAMP(4)=+1D0/2D0*(-1D0/3D0*IMAG1*AMP(3)-1D0/3D0*IMAG1*AMP(5)
     $ -1D0/3D0*IMAG1*AMP(8)-1D0/3D0*IMAG1*AMP(10)-1D0/3D0*IMAG1
     $ *AMP(11)-1D0/3D0*IMAG1*AMP(12)+1D0/3D0*AMP(29)+1D0/3D0*AMP(30)
     $ +1D0/3D0*AMP(31)+1D0/3D0*AMP(33)+1D0/3D0*AMP(34)+1D0/3D0
     $ *AMP(36)+1D0/3D0*AMP(79)+1D0/3D0*AMP(80)+1D0/3D0*AMP(82)
     $ +1D0/3D0*AMP(84)+1D0/3D0*AMP(86)+1D0/3D0*AMP(98))
      JAMP(5)=+1D0/2D0*(+1D0/3D0*AMP(13)+1D0/3D0*AMP(14)+1D0/3D0
     $ *AMP(17)+1D0/3D0*AMP(18)+1D0/3D0*AMP(20)+1D0/3D0*AMP(23)
     $ +1D0/3D0*AMP(39)+1D0/3D0*AMP(40)+1D0/3D0*AMP(41)+1D0/3D0
     $ *AMP(42)+1D0/3D0*AMP(44)+1D0/3D0*AMP(47))
      JAMP(6)=+1D0/2D0*(-AMP(13)-AMP(14)-AMP(15)-AMP(16)-AMP(20)
     $ -IMAG1*AMP(21)-IMAG1*AMP(24)-AMP(51)-AMP(52)-AMP(53)-AMP(54)
     $ +IMAG1*AMP(56)-AMP(57)+IMAG1*AMP(59)-AMP(68)-IMAG1*AMP(69)
     $ -IMAG1*AMP(71)-IMAG1*AMP(75)-AMP(77)-IMAG1*AMP(78)-IMAG1
     $ *AMP(87)+AMP(88)-AMP(90)-AMP(91)-AMP(92)+IMAG1*AMP(94)
     $ +AMP(97)-AMP(99)-AMP(100)-AMP(101))
      JAMP(7)=+1D0/2D0*(-AMP(25)-AMP(26)-AMP(29)-AMP(30)-IMAG1*AMP(32)
     $ -AMP(33)-IMAG1*AMP(35)-AMP(37)-AMP(38)-AMP(41)-AMP(42)
     $ +IMAG1*AMP(45)-AMP(47)+IMAG1*AMP(48)-AMP(62)+IMAG1*AMP(63)
     $ +IMAG1*AMP(66)-AMP(80)+IMAG1*AMP(81)+IMAG1*AMP(83)-IMAG1
     $ *AMP(85)+AMP(88)-AMP(90)-AMP(91)-AMP(92)+IMAG1*AMP(96)
     $ +AMP(97)-AMP(99)-AMP(100)-AMP(101))
      JAMP(8)=+1D0/2D0*(+1D0/3D0*AMP(25)+1D0/3D0*AMP(26)+1D0/3D0
     $ *AMP(27)+1D0/3D0*AMP(28)+1D0/3D0*AMP(49)+1D0/3D0*AMP(50)
     $ +1D0/3D0*AMP(53)+1D0/3D0*AMP(54)+1D0/3D0*AMP(62)+1D0/3D0
     $ *AMP(65)+1D0/3D0*AMP(74)+1D0/3D0*AMP(77))
      JAMP(9)=+1D0/2D0*(+1D0/3D0*IMAG1*AMP(1)+1D0/3D0*IMAG1*AMP(2)
     $ +1D0/3D0*IMAG1*AMP(6)+1D0/3D0*IMAG1*AMP(7)+1D0/3D0*AMP(37)
     $ +1D0/3D0*AMP(38)+1D0/3D0*AMP(43)+1D0/3D0*AMP(46)+1D0/3D0
     $ *AMP(61)+1D0/3D0*AMP(64))
      JAMP(10)=+1D0/2D0*(-IMAG1*AMP(1)+AMP(4)-IMAG1*AMP(5)-IMAG1
     $ *AMP(6)-IMAG1*AMP(8)+AMP(9)-IMAG1*AMP(12)-AMP(49)-AMP(50)
     $ -AMP(55)-IMAG1*AMP(56)-AMP(58)-IMAG1*AMP(59)-AMP(60)-AMP(61)
     $ -IMAG1*AMP(63)-AMP(64)-AMP(65)-IMAG1*AMP(66)+AMP(90)-AMP(93)
     $ +AMP(91)-IMAG1*AMP(94)-AMP(95)+AMP(99)-AMP(102)+AMP(100))
      JAMP(11)=+1D0/2D0*(-IMAG1*AMP(2)-IMAG1*AMP(3)-AMP(4)-IMAG1
     $ *AMP(7)-AMP(9)-IMAG1*AMP(10)-IMAG1*AMP(11)-AMP(39)-AMP(40)
     $ -AMP(43)-AMP(44)-IMAG1*AMP(45)-AMP(46)-IMAG1*AMP(48)-AMP(67)
     $ +IMAG1*AMP(69)-AMP(70)+IMAG1*AMP(71)-AMP(72)+IMAG1*AMP(87)
     $ -AMP(88)-AMP(89)+AMP(93)+AMP(92)-AMP(97)+AMP(102)+AMP(101))
      JAMP(12)=+1D0/2D0*(+1D0/3D0*IMAG1*AMP(3)+1D0/3D0*IMAG1*AMP(5)
     $ +1D0/3D0*IMAG1*AMP(8)+1D0/3D0*IMAG1*AMP(10)+1D0/3D0*IMAG1
     $ *AMP(11)+1D0/3D0*IMAG1*AMP(12)+1D0/3D0*AMP(51)+1D0/3D0*AMP(52)
     $ +1D0/3D0*AMP(55)+1D0/3D0*AMP(57)+1D0/3D0*AMP(58)+1D0/3D0
     $ *AMP(60)+1D0/3D0*AMP(67)+1D0/3D0*AMP(68)+1D0/3D0*AMP(70)
     $ +1D0/3D0*AMP(72)+1D0/3D0*AMP(89)+1D0/3D0*AMP(95))

      P0_MATRIX22 = 0.D0
      DO I = 1, NCOLOR
        ZTEMP = (0.D0,0.D0)
        DO J = 1, NCOLOR
          ZTEMP = ZTEMP + CF(J,I)*JAMP(J)
        ENDDO
        P0_MATRIX22 = P0_MATRIX22+ZTEMP*DCONJG(JAMP(I))/DENOM(I)
      ENDDO
      END

