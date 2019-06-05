The code in this directory needs the "Eigen" linear algebra package
for its implementation of algorithms with sparse matrices. See, in
particular, http://eigen.tuxfamily.org/

It also needs the TRLAN package for calculating eigenvalues/eigenvectors
of symmetric matrices using the Lanczos algorithm (with restarting). The
TRLAN code can be downloaded from 
https://codeforge.lbl.gov/frs/download.php/210/trlan-201009.tar.gz
The TRLAN manual which explains the algorithm parameters can be found
at http://crd-legacy.lbl.gov/~kewu/ps/trlan-ug.html (HTML version) and
at http://crd-legacy.lbl.gov/~kewu/ps/trlan-ug.ps (PostScript version).
It is assumed that TRLAN is compiled without MPI support.

The following functionality is provided:

AbsSparseUnfoldingFilterND.hh        -- Base class for multivariate smoothers
                                        that can be represented with sparse
                                        matrices. Also includes a wrapper
                                        for building such smoothers from
                                        npstat classes SequentialPolyFilterND
                                        and LocalPolyFilterND.

AbsSparseUnfoldND.hh                 -- Base class for multivariate unfolding
                                        algorithms that use sparse matrices.

EigenParameters.hh                   -- Parameters for calculating eigenspectra
                                        with TRLAN.

pruneCovariance.hh                   -- Prune small correlation coefficients
                                        in a sparse covariance matrix.

SmoothedEMSparseUnfoldND.hh          -- Expectation-maximization unfolding
                                        with smoothing for multivariate
                                        distributions. The functionality
                                        is similar to the SmoothedEMUnfoldND
                                        class from npstat but the code is
                                        using sparse matrices.

SparseUnfoldingBandwidthScannerND.hh -- Class which gets various information
                                        from multivariate unfolding results
                                        in a convenient form. Similar to
                                        UnfoldingBandwidthScannerND from
                                        npstat but uses sparse matrices.

trlan77.h                            -- C/C++ interface to TRLAN.

trlanEigensystem.hh                  -- Calculates eigenvalues/vectors of
                                        symmetric sparse matrices using TRLAN.

The classes defined in this directory (except those in the header file
AbsSparseUnfoldingFilterND.hh) are templated upon the matrix class. It is
expected that this matrix class will be either Eigen::SparseMatrix<double>
(which is equivalent to Eigen::SparseMatrix<double,Eigen::ColMajor,int>) or
Eigen::SparseMatrix<double,Eigen::ColMajor,long>.
