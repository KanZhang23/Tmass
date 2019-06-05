#ifndef NPSTAT_GRIDRANDOMIZER_HH_
#define NPSTAT_GRIDRANDOMIZER_HH_

/*!
// \file GridRandomizer.hh
//
// \brief Generate random numbers according to a multivariate distribution
//        tabulated on a grid 
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/BoxND.hh"

namespace npstat {
    /**
    // Class for generating random numbers according to a multivariate
    // distribution tabulated on a rectangular grid.
    */
    class GridRandomizer
    {
    public:
        /**
        // Main constructor. "interpolationDegree" may be either 0 or 1.
        // The function values are assumed to be given in the centers
        // of the bins. The density represented by "gridData" will be
        // internally normalized using grid boundaries to determine the
        // support of the density.
        */
        template <typename Num1, unsigned Len1, unsigned Dim1>
        inline GridRandomizer(const ArrayND<Num1,Len1,Dim1>& gridData,
                              const BoxND<double>& gridBoundary,
                              const unsigned interpolationDegree)
            : grid_(gridData), boundary_(gridBoundary),
              iDeg_(interpolationDegree), dim_(gridData.rank()) {initialize();}

        GridRandomizer(const GridRandomizer&);
        GridRandomizer& operator=(const GridRandomizer&);

        inline ~GridRandomizer() {clearCdfs();}

        bool operator==(const GridRandomizer& r) const;
        inline bool operator!=(const GridRandomizer& r) const
            {return !(*this == r);}

        //@{
        /** Inspect the properties of this object */
        inline const ArrayND<double>& gridData() const {return grid_;}
        inline const BoxND<double>& gridBoundary() const {return boundary_;}
        inline unsigned interpolationDegree() const {return iDeg_;}
        inline unsigned dim() const {return dim_;}
        //@}

        /** Probability density */
        double density(const double* x, unsigned xLen) const;

        /**
        // Generate random numbers according to the density provided
        // in the constructor.
        //
        // In this method, "bufLen" should not necessarily be equal
        // to the grid dimensionality. When "bufLen" is smaller,
        // the random numbers are efficiently generated for dimensions
        // 0, 1, ..., bufLen - 1. When "bufLen" is larger, only the
        // first "dim()" elements of the "resultBuf" array are filled.
        */
        void generate(const double* uniformRandomInput, unsigned bufLen,
                      double* resultBuf) const;
    private:
        GridRandomizer();

        void initialize();
        void clearCdfs();
        void copyCdfs(const GridRandomizer&);

        void generateFlat(const double* randNum, unsigned bufLen,
                          double* result) const;
        void generateInterpolated(const double* randNum, unsigned bufLen,
                                  double* result) const;
        ArrayND<double> grid_;
        BoxND<double> boundary_;
        unsigned iDeg_;
        unsigned dim_;

        std::vector<ArrayND<double>*> cdfs_;
        std::vector<ArrayND<double>*> norms_;
        std::vector<double> binwidth_;
        double densityNorm_;

        mutable std::vector<unsigned> genBins_;
        mutable std::vector<double> work_;
        mutable std::vector<double> memoizeIn_;
        mutable std::vector<double> memoizeOut_;
    };
}

#endif // NPSTAT_GRIDRANDOMIZER_HH_
