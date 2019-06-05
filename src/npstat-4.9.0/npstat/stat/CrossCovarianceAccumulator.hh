#ifndef NPSTAT_CROSSCOVARIANCEACCUMULATOR_HH_
#define NPSTAT_CROSSCOVARIANCEACCUMULATOR_HH_

/*!
// \file CrossCovarianceAccumulator.hh
//
// \brief Accumulator of covariances between two arrays of random quantities
//
// Author: I. Volobouev
//
// July 2011
*/

#include <vector>

#include "geners/ClassId.hh"
#include "npstat/nm/Matrix.hh"

namespace npstat {
    /**
    // Simple single-pass (which means imprecise) accumulator of covariances
    // between two arrays. Correlations are between all possible pairs
    // in which one element is taken from the first array and the other from
    // the second array.
    */
    class CrossCovarianceAccumulator
    {
    public:
        /** n1 and n2 are the expected array sizes */
        CrossCovarianceAccumulator(unsigned n1, unsigned n2);

        //@{
        /** Basic inspector of accumulated quantities */
        inline unsigned dim1() const {return dim1_;}
        inline unsigned dim2() const {return dim2_;}
        inline unsigned long count() const {return count_;}
        inline const std::vector<long double>& sum1() const {return sum1_;}
        inline const std::vector<long double>& sum2() const {return sum2_;}
        inline const std::vector<long double>& sumsq1() const {return sum1_;}
        inline const std::vector<long double>& sumsq2() const {return sum2_;}
        long double sumsq(unsigned i, unsigned j) const;
        //@}

        //@{
        /** Inspect mean, standard deviation, etc */
        double mean1(unsigned i) const;
        double mean2(unsigned i) const;
        double stdev1(unsigned i) const;
        double stdev2(unsigned i) const;
        double meanUncertainty1(unsigned i) const;
        double meanUncertainty2(unsigned i) const;
        //@}

        /** Covariance between the given variables */
        double cov(unsigned i, unsigned j) const;

        /** Correlation coefficient between the given variables */
        double corr(unsigned i, unsigned j) const;

        /**
        // Covariance matrix (of dimensions n1 x n2,
        // where n1 and n2 are given in the constructor)
        */
        Matrix<double> crossCovMat() const;

        /**
        // Correlation matrix (of dimensions n1 x n2,
        // where n1 and n2 are given in the constructor)
        */
        Matrix<double> crossCorrMat() const;

        /**
        // Accumulate the statistical summaries.
        //
        // The length of array data1 should be n1 (as in the constructor).
        // The length of array data2 should be n2 (as in the constructor).
        */
        template<typename T1, typename T2>
        void accumulate(const T1* data1, unsigned len1,
                        const T2* data2, unsigned len2);

        /** Accumulate summaries from a pair of vectors or arrays */
        template<typename ArrayPair>
        inline void accumulate(const ArrayPair& pair)
        {
            accumulate(&pair.first[0], pair.first.size(),
                       &pair.second[0], pair.second.size());
        }

        /** Add the statistical summaries from another accumulator */
        void accumulate(const CrossCovarianceAccumulator&);

        /** Accumulate summaries from a pair of vectors or arrays */
        template<typename ArrayPair>
        inline CrossCovarianceAccumulator& operator+=(const ArrayPair& pair)
            {accumulate(pair); return *this;}

        /** Add the statistical summaries from another accumulator */
        inline CrossCovarianceAccumulator& operator+=(
            const CrossCovarianceAccumulator& r)
            {accumulate(r); return *this;}

        /** Reset all accumulators and counters */
        void reset();

        /** Comparison for equality */
        bool operator==(const CrossCovarianceAccumulator& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const CrossCovarianceAccumulator& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::CrossCovarianceAccumulator";}
        static inline unsigned version() {return 1;}
        static CrossCovarianceAccumulator* read(const gs::ClassId&, std::istream&);

    private:
        CrossCovarianceAccumulator();

        unsigned dim1_;
        unsigned dim2_;
        unsigned long count_;
        std::vector<long double> sum1_;
        std::vector<long double> sum2_;
        std::vector<long double> sumsq1_;
        std::vector<long double> sumsq2_;
        std::vector<long double> sumsq_;
    };
}

#include "npstat/stat/CrossCovarianceAccumulator.icc"

#endif // NPSTAT_CROSSCOVARIANCEACCUMULATOR_HH_
