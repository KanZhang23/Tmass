#include <cmath>
#include <vector>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include "npstat/nm/truncatedInverseSqrt.hh"

namespace npstat {
    double truncatedInverseSqrt(const Matrix<double>& covmat,
                                const unsigned nEigenvectorsToKeep,
                                const double eigenvaluePrecision,
                                Matrix<double>* result,
                                unsigned* numEigenvaluesAdjusted,
                                const EigenMethod m)
    {
        const unsigned nRows = covmat.nRows();
        if (nRows == 0) throw std::invalid_argument(
            "In npstat::truncatedInverseSqrt: uninitialized input matrix");
        if (nEigenvectorsToKeep > nRows) throw std::invalid_argument(
            "In npstat::truncatedInverseSqrt: too many eigenvectors requested");
        if (eigenvaluePrecision < 0.0) throw std::invalid_argument(
            "In npstat::truncatedInverseSqrt: precision can not be negative");
        assert(result);

        std::vector<double> eigenvalues(nRows);
        Matrix<double> eigenvectors(nRows, nRows);
        covmat.symEigen(&eigenvalues[0], nRows, &eigenvectors, m);

        // Lapack-based code should return eigenvalues in the increasing order.
        // Make sure that this is indeed the case.
        const unsigned first = nRows - nEigenvectorsToKeep;
        for (unsigned i=first; i<nRows; ++i)
            if (i && eigenvalues[i] < eigenvalues[i-1]) throw std::runtime_error(
                "In npstat::truncatedInverseSqrt: unexpected eigenvalue order");

        // Maximum and minimum eigenvalues
        const double maxEigen = eigenvalues.back();
        if (maxEigen <= 0.0) throw std::invalid_argument(
            "In npstat::truncatedInverseSqrt: the largest eigenvalue is not positive");
        const double minEigen = maxEigen*eigenvaluePrecision;

        // Figure out the fraction of variance dropped
        long double total = 0.0L, dropped = 0.0L;
        for (unsigned i=0; i<nRows; ++i)
        {
            const double evalue = std::max(eigenvalues[i], 0.0);
            total += evalue;
            if (i < first)
                dropped += evalue;
        }

        Matrix<double> keep(nEigenvectorsToKeep, nRows);
        unsigned iKeep = 0, nAdj = 0;
        for (unsigned row=first; row<nRows; ++row, ++iKeep)
        {
            double evalue = eigenvalues[row];
            if (evalue < minEigen)
            {
                evalue = minEigen;
                ++nAdj;
            }
            if (evalue <= 0.0)
            {
                std::ostringstream os;
                os << "In npstat::truncatedInverseSqrt: an eigenvector to keep (#"
                   << row << ") has a non-positive adjusted eigenvalue. Matrix eigenvalues are:";
                for (unsigned k=0; k<nRows; ++k)
                    os << ' ' << eigenvalues[k];
                throw std::invalid_argument(os.str());
            }
            const double factor = 1.0/sqrt(evalue);
            const double* from = eigenvectors[row];
            double* to = keep[iKeep];
            for (unsigned col=0; col<nRows; ++col)
                *to++ = factor * *from++;
        }

        if (!keep.isCompatible(*result))
            result->uninitialize();
        *result = keep;

        if (numEigenvaluesAdjusted)
            *numEigenvaluesAdjusted = nAdj;

        return dropped/total;
    }
}
