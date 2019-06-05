#ifndef NPSTAT_MATRIX_HH_
#define NPSTAT_MATRIX_HH_

/*!
// \file Matrix.hh
//
// \brief Template matrix class
//
// Author: I. Volobouev
//
// November 2008
*/

#include "geners/ClassId.hh"
#include "geners/CPP11_config.hh"

#include "npstat/nm/EigenMethod.hh"
#include "npstat/nm/SvdMethod.hh"
#include "npstat/nm/GeneralizedComplex.hh"

namespace npstat {
    /**
    // A simple helper class for matrix manipulations. Depending on how much
    // space is provided with the "Len" parameter, the data will be placed
    // either on the stack or on the heap. Do not set "Len" to 0, the code
    // assumes that the stack space is available for at least one element.
    //
    // C storage convention is used for internal data. In the element access
    // operations, array bounds are not checked.
    //
    // A number of operations (solving linear systems, calculating eigenvalues
    // and eigenvectors, singular value decomposition, etc) are performed
    // by calling appropriate routines from LAPACK. This usually limits
    // the Numeric template parameter types for which these operations are
    // available to float and double. If the operation is unavailable for
    // the template parameter used, std::invalid_argument exception is raised.
    //
    // Note that for simple matrix operations this class is likely to be
    // slower than matrix classes based on expression templates (such as
    // those in boost uBLAS or in Blitz++). If speed is really important
    // for your calculations, consider using a dedicated matrix library.
    */
    template<typename Numeric, unsigned Len=16>
    class Matrix
    {
        template <typename Num2, unsigned Len2>
        friend class Matrix;

    public:
        typedef Numeric value_type;

        /**
        // Default constructor creates an unitialized matrix
        // which can be assigned from other matrices
        */
        Matrix();

        /**
        // This constructor creates an unitialized matrix
        // which can be assigned element-by-element or from
        // another matrix with the same dimensions
        */
        Matrix(unsigned nrows, unsigned ncols);

        /**
        // This constructor initializes the matrix as follows:
        //
        //   initCode = 0:  all elements are initialized to 0.
        //
        //   initCode = 1:  matrix must be square; diagonal elements are
        //                  initialized to 1, all other elements to 0.
        */
        Matrix(unsigned nrows, unsigned ncols, int initCode);

        /**
        // This constructor initializes the matrix from the
        // given 1-d array using C storage conventions
        */
        Matrix(unsigned nrows, unsigned ncols, const Numeric* data);

        /** Copy constructor */
        Matrix(const Matrix&);

#ifdef CPP11_STD_AVAILABLE
        /** Move constructor */
        Matrix(Matrix&&);
#endif
        /** Converting copy constructor */
        template <typename Num2, unsigned Len2>
        Matrix(const Matrix<Num2, Len2>&);

        /**
        // Constructor from a subrange of another matrix. The minimum row
        // and column numbers are included, maximum numbers are excluded.
        */
        template <typename Num2, unsigned Len2>
        Matrix(const Matrix<Num2, Len2>&,
               unsigned rowMin, unsigned rowMax,
               unsigned columnMin, unsigned columnMax);

        ~Matrix();

        /** 
        // Assignment operator. The matrix on the left must either be
        // in an uninitialized state or have compatible dimensions with
        // the matrix on the right.
        */
        Matrix& operator=(const Matrix&);

#ifdef CPP11_STD_AVAILABLE
        /** Move assignment operator */
        Matrix& operator=(Matrix&&);
#endif
        /** Converting assignment operator */
        template <typename Num2, unsigned Len2>
        Matrix& operator=(const Matrix<Num2, Len2>&);

        /** Set from triplets (for compatibility with Eigen) */
        template<typename Iterator>
        void setFromTriplets(Iterator first, Iterator last);

        /**
        // Tag this matrix as diagonal. This may improve the speed
        // of certain operations. Of course, this should be done only
        // if you know for sure that this matrix is, indeed, diagonal.
        // Works for square matrices only.
        */
        Matrix& tagAsDiagonal();

        /**
        // Fill the main diagonal and set all other elements to zero.
        // This operation tags the matrix as diagonal.
        */
        template <typename Num2>
        Matrix& diagFill(const Num2* data, unsigned dataLen);

        //@{
        /** A simple inspection of matrix properties */
        inline unsigned nRows() const {return nrows_;}
        inline unsigned nColumns() const {return ncols_;}
        inline unsigned length() const {return nrows_*ncols_;}
        inline Numeric* data() const {return data_;}
        inline bool isSquare() const {return data_ && ncols_ == nrows_;}
        bool isSymmetric() const;
        bool isAntiSymmetric() const;
        bool isDiagonal() const;
        //@}

        /**
        // This method resets the object to an unintialized state
        // Can be applied in order to force the assignment operators to work.
        */
        Matrix& uninitialize();

        /**
        // Check if this matrix has the same number of rows and columns
        // as the other one. "false" will be returned in case any one of
        // the two matrices (or both) is not initialized.
        */
        bool isCompatible(const Matrix& other) const;

        /**
        // This method changes the object dimensions.
        // All data is lost in the process.
        */
        Matrix& resize(unsigned nrows, unsigned ncols);

        /** This method sets all elements to 0 */
        Matrix& zeroOut();

        /**
        // This method sets all diagonal elements to 0.
        // It can only be used with square matrices.
        */
        Matrix& clearMainDiagonal();

        /**
        // This method sets all non-diagonal elements to 0.
        // This operation is only valid for square matrices.
        // It tags the matrix as diagonal.
        */
        Matrix& makeDiagonal();

        /** This method sets all elements to the given value */
        Matrix& constFill(Numeric c);

        /**
        // Method for transposing this matrix. Uses std::swap for square
        // matrices but might allocate memory from the heap if the matrix
        // is not square.
        */
        Matrix& Tthis();

        //@{
        /** Compare two matrices for equality */
        bool operator==(const Matrix&) const;
        bool operator!=(const Matrix&) const;
        //@}

        /**
        // Non-const access to the data (works like 2-d array in C).
        // No bounds checking.
        */
        Numeric* operator[](unsigned);

        /**
        // Const access to the data (works like 2-d array in C).
        // No bounds checking.
        */
        const Numeric* operator[](unsigned) const;

        /** Data modification method with bounds checking */
        Matrix& set(unsigned row, unsigned column, Numeric value);

        /** Access by value without bounds checking */
        Numeric operator()(unsigned row, unsigned column) const;

        /** Access by value with bounds checking */
        Numeric at(unsigned row, unsigned column) const;

        /** Sum of the values in the given row */
        Numeric rowSum(unsigned row) const;

        /** Sum of the values in the given column */
        Numeric columnSum(unsigned column) const;

        /** Remove row and/or column. Indices out of range are ignored */
        Matrix removeRowAndColumn(unsigned row, unsigned column) const;

        /** Number of non-zero elements */
        unsigned nonZeros() const;

        /** 
        // Replace every n x m square block in the matrix by its sum.
        // The number of rows must be divisible by n. The number of
        // columns must be divisible by m.
        */
        void coarseSum(unsigned n, unsigned m, Matrix* result) const;

        /** 
        // Replace every n x m square block in the matrix by its average
        // The number of rows must be divisible by n. The number of
        // columns must be divisible by m.
        */
        void coarseAverage(unsigned n, unsigned m, Matrix* result) const;

        /** Unary plus */
        Matrix operator+() const;

        /** Unary minus */
        Matrix operator-() const;

        //@{
        /** Binary algebraic operation with matrix or scalar */
        Matrix operator*(const Matrix& r) const;
        Matrix operator*(Numeric r) const;
        Matrix operator/(Numeric r) const;
        Matrix operator+(const Matrix& r) const;
        Matrix operator-(const Matrix& r) const;
        //@}

        /** Hadamard product (a.k.a. Schur product) */
        Matrix hadamardProduct(const Matrix& r) const;

        /** Hadamard ratio. All elements of the denominator must be non-zero */
        Matrix hadamardRatio(const Matrix& denominator) const;

        //@{
        /** 
        // Binary algebraic operation which writes its result into
        // an existing matrix potentially avoiding memory allocation
        */
        void times(const Matrix& r, Matrix* result) const;
        void times(Numeric r, Matrix* result) const;
        void over(Numeric r, Matrix* result) const;
        void plus(const Matrix& r, Matrix* result) const;
        void minus(const Matrix& r, Matrix* result) const;
        //@}

        //@{
        /** In-place algebraic operations with matrices and scalars */
        Matrix& operator*=(Numeric r);
        Matrix& operator/=(Numeric r);
        Matrix& operator+=(const Matrix& r);
        Matrix& operator-=(const Matrix& r);
        //@}

        //@{
        /** Multiplication by a vector represented by a pointer and array size */
        template <typename Num2>
        Matrix timesVector(const Num2* data, unsigned dataLen) const;

        template <typename Num2, typename Num3>
        void timesVector(const Num2* data, unsigned dataLen,
                         Num3* result, unsigned resultLen) const;
        //@}

        /**
        // Multiplication by a vector represented by a pointer and array size.
        // This function is useful when only one element of the result is needed
        // (or elements are needed one at a time).
        */
        template <typename Num2>
        Numeric timesVector(unsigned rowNumber,
                            const Num2* data, unsigned dataLen) const;

        //@{
        /** Multiplication by a row on the left */
        template <typename Num2>
        Matrix rowMultiply(const Num2* data, unsigned dataLen) const;

        template <typename Num2, typename Num3>
        void rowMultiply(const Num2* data, unsigned dataLen,
                         Num3* result, unsigned resultLen) const;
        //@}

        /**
        // Multiplication by a row on the left. This function is useful when
        // only one element of the result is needed (or elements are needed
        // one at a time).
        */
        template <typename Num2>
        Numeric rowMultiply(unsigned columnNumber,
                            const Num2* data, unsigned dataLen) const;

        /** Bilinear form with a vector (v^T M v) */
        template <typename Num2>
        Numeric bilinear(const Num2* data, unsigned dataLen) const;

        /** Bilinear form with a matrix (r^T M r) */
        Matrix bilinear(const Matrix& r) const;

        /** Bilinear form with a transposed matrix (r M r^T) */
        Matrix bilinearT(const Matrix& r) const;

        /**
        // Solution of a linear system of equations M*x = rhs.
        // The matrix must be square. "false" is returned in case
        // the matrix is degenerate.
        */
        bool solveLinearSystem(const Numeric* rhs, unsigned lenRhs,
                               Numeric* solution) const;

        /**
        // Solution of linear systems of equations M*X = RHS.
        // The matrix M (this one) must be square. "false" is
        // returned in case this matrix is degenerate.
        */
        bool solveLinearSystems(const Matrix& RHS, Matrix* X) const;

        /**
        // Solution of a linear overdetermined system of equations M*x = rhs
        // in the least squares sense. The "lenRhs" parameter should be equal
        // to the number of matrix rows, and it should exceed "lenSolution"
        // ("lenSolution" should be equal to the number of columns). "false"
        // is returned in case of failure.
        */
        bool linearLeastSquares(const Numeric* rhs, unsigned lenRhs,
                                Numeric* solution, unsigned lenSolution) const;

        /**
        // Solution of a linear system of equations M*x = rhs1 in the least
        // squares sense, subject to the constraint B*x = rhs2. Number of
        // equations (i.e., dimensionality of rhs1) plus the number of
        // constraints (i.e., dimensionality of rhs2) should exceed the
        // dimensionality of x. "false" is returned in case of failure. If
        // the chi-square of the result is desired, make the "resultChiSquare" 
        // argument point to some valid location. Same goes for the result
        // covariance matrix. Note that requesting the result covariance
        // matrix switches the code to a different and slower "textbook"
        // algorithm instead of an optimized algorithm from Lapack. "false"
        // is returned in case of failure.
        */
        bool constrainedLeastSquares(const Numeric* rhs1, unsigned lenRhs1,
                                     const Matrix& B,
                                     const Numeric* rhs2, unsigned lenRhs2,
                                     Numeric* solution, unsigned lenSol,
                                     Numeric* resultChiSquare = 0,
                                     Matrix* resultCovarianceMatrix = 0) const;

        /**
        // Weighted least squares problem. The inverse covariance matrix
        // must be symmetric and positive definite. If the chi-square of the
        // result is desired, make the "resultChiSquare" argument point to
        // some valid location. Same goes for the result covariance matrix.
        // "false" is returned in case of failure.
        */
        bool weightedLeastSquares(const Numeric* rhs, unsigned lenRhs,
                                  const Matrix& inverseCovarianceMatrix,
                                  Numeric* solution, unsigned lenSoution,
                                  Numeric* resultChiSquare = 0,
                                  Matrix* resultCovarianceMatrix = 0) const;

        /** Return transposed matrix */
        Matrix T() const;

        /** Return the product of this matrix transposed with this, M^T*M */
        Matrix TtimesThis() const;

        /** Return the product of this matrix with its transpose, M*M^T */
        Matrix timesT() const;

        /** Return the product of this matrix with the transposed argument */
        Matrix timesT(const Matrix& r) const;

        /** Return the product of this matrix transposed with the argument */
        Matrix Ttimes(const Matrix& r) const;

        /**
        // "directSum" method constructs a block-diagonal matrix with
        // this matrix and the argument matrix inside the diagonal blocks.
        // Block which contains this matrix is the top left one.
        */
        Matrix directSum(const Matrix& added) const;

        /** Construct a symmetric matrix from this one (symmetrize) */
        Matrix symmetrize() const;

        /** Construct an antisymmetric matrix from this one (antisymmetrize) */
        Matrix antiSymmetrize() const;

        /**
        // Matrix outer product. The number of rows of the result equals
        // the product of the numbers of rows of this matrix and the argument
        // matrix. The number of columns of the result is the product of the
        // numbers of columns.
        */
        Matrix outer(const Matrix& r) const;

        /** Maximum absolute value for any row/column */
        Numeric maxAbsValue() const;

        /** Frobenius norm of this matrix */
        double frobeniusNorm() const;

        //@{
        /** Calculate the trace (matrix must be square) */
        Numeric tr() const;
        inline Numeric sp() const {return tr();}
        //@}

        //@{
        /**
        // Calculate the trace of a product of this matrix with another.
        // This works faster if only the trace is of interest but not the
        // product itself.
        */
        Numeric productTr(const Matrix& r) const;
        inline Numeric productSp(const Matrix& r) const {return productTr(r);}
        //@}

        /** Calculate the determinant (matrix must be square) */
        Numeric det() const;

        /**
        // Inverse of a real symmetric positive-definite matrix. Use this
        // function (it has better numerical precision than symInv()) if
        // you know that your matrix is symmetric and positive definite.
        // For example, a non-singular covariance matrix calculated for
        // some dataset has these properties.
        */
        Matrix symPDInv() const;

        /**
        // Invert real symmetric positive-definite matrix using
        // eigendecomposition. Use this function if you know that your
        // matrix is supposed to be symmetric and positive-definite but
        // might not be due to numerical round-offs. The arguments of
        // this method are as folows:
        // 
        // tol    -- Eigenvalues whose ratios to the largest eigenvalue
        //           are below this parameter will be either truncated
        //           or extended (per value of the next argument).
        //           This parameter must lie in (0, 1).
        // 
        // extend -- If "true", the inverse of small eigenvalues will
        //           be replaced by the inverse of the largest eigenvalue
        //           divided by the tolerance parameter. If "false",
        //           inverse of such eigenvalues will be set to 0.
        //
        // m      -- LAPACK method to use for calculating the
        //           eigendecomposition.
        */
        Matrix symPDEigenInv(double tol, bool extend=true,
                             EigenMethod m=EIGEN_SIMPLE) const;

        /**
        // Inverse of a real symmetric matrix. If the matrix is not symmetric,
        // its symmetrized version will be used internally. Use this function
        // (it has better numerical precision than inv()) if you know that
        // your matrix is symmetric. 
        */
        Matrix symInv() const;

        /**
        // Inverse of a general matrix (which must be square and non-singular)
        */
        Matrix inv() const;

        /**
        // Eigenvalues and eigenvectors of a real tridiagonal symmetric
        // matrix. The "eigenvectors" pointer can be NULL if only eigenvalues
        // are needed. If it is not NULL, the _rows_ of that matrix will be
        // set to the computed eigenvectors.
        //
        // The "m" parameter specifies the LAPACK method to use for
        // calculating the eigendecomposition.
        */
        void tdSymEigen(Numeric* eigenvalues, unsigned lenEigenvalues,
                        Matrix* eigenvectors=0, EigenMethod m=EIGEN_RRR) const;

        /**
        // Eigenvalues and eigenvectors of a real symmetric matrix.
        // The "eigenvectors" pointer can be NULL if only eigenvalues
        // are needed. If it is not NULL, the _rows_ of that matrix
        // will be set to the computed eigenvectors.
        //
        // The "m" parameter specifies the LAPACK method to use for
        // calculating the eigendecomposition.
        */
        void symEigen(Numeric* eigenvalues, unsigned lenEigenvalues,
                      Matrix* eigenvectors=0, EigenMethod m=EIGEN_SIMPLE) const;

        /**
        // Eigenvalues and eigenvectors of a real general matrix.
        // Either "rightEigenvectors" or "leftEigenvectors" or both
        // can be NULL if they are not needed.
        //
        // If an eigenvalue is real, the corresponding row of the
        // output eigenvector matrix is set to the computed eigenvector.
        // If an eigenvalue is complex, all such eigenvalues come in
        // complex conjugate pairs. Corresponding eigenvectors make up
        // such a pair as well. The matrix row which corresponds to the
        // first eigenvalue in the pair is set to the real part of the pair
        // and the matrix row which corresponds to the second eigenvalue
        // in the pair is set to the imaginary part of the pair. For the
        // right eigenvectors, the first complex eigenvector in the pair
        // needs to be constructed by adding the imaginary part. The second
        // eigenvector has the imaginary part subtracted. For the left
        // eigenvectors the order is reversed: the first eigenvector needs
        // to be constructed by subtracting the imaginary part, and the
        // second by adding it.
        */
        void genEigen(typename GeneralizedComplex<Numeric>::type *eigenvalues,
                      unsigned lenEigenvalues,
                      Matrix* rightEigenvectors = 0,
                      Matrix* leftEigenvectors = 0) const;

        /**
        // Singular value decomposition of a matrix. The length of the
        // singular values buffer must be at least min(nrows, ncolumns).
        //
        // On exit, matrices U and V will contain singular vectors of
        // the current matrix, row-by-row. The decomposition is then
        //
        // *this = U^T * diag(singularValues, nrows, ncolumns) * V
        */
        void svd(Numeric* singularValues, unsigned lenSingularValues,
                 Matrix* U, Matrix* V, SvdMethod m = SVD_D_AND_C) const;

        /**
        // Calculate entropy-based effective rank. It is assumed that
        // the matrix is symmetric positive semidefinite.
        //
        // The "tol" parameter specifies the eigenvalue cutoff:
        // all eigenvalues equal to tol*maxEigenvalue or smaller
        // will be ignored. This parameter must be non-negative.
        //
        // The "m" parameter specifies the LAPACK method to use for
        // calculating eigenvalues.
        //
        // If "eigenSum" pointer is not NULL, *eigenSum will be filled
        // on exit with the sum of eigenvalues above the cutoff divided
        // by the largest eigenvalue.
        */
        double symPSDefEffectiveRank(double tol = 0.0,
                                     EigenMethod m = EIGEN_D_AND_C,
                                     double* eigenSum = 0) const;

        /**
        // Calculate a function of this matrix. It is assumed that the
        // matrix is symmetric and real. The calculation is performed by
        // calculating the eignenbasis, evaluating the function of the
        // eigenvalues, and then transforming back to the original basis.
        */
        template <class Functor>
        Matrix symFcn(const Functor& fcn) const;

        /** Power of a matrix. Matrix must be square. */
        Matrix pow(unsigned degree) const;

        /**
        // The following function derives a correlation matrix
        // out of a covariance matrix. In the returned matrix
        // all diagonal elements will be set to 1.
        */
        Matrix covarToCorr() const;

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in, Matrix* m);

    private:
        void calcDiagonal();
        void makeCoarse(unsigned n, unsigned m, Matrix* result) const;
        void invertDiagonal(Matrix* result) const;

        Numeric local_[Len];
        Numeric* data_;
        unsigned nrows_;
        unsigned ncols_;
        unsigned len_;
        bool isDiagonal_;
        bool diagonalityKnown_;

#ifdef SWIG
    public:
        inline Matrix timesVector2(const Numeric* data, unsigned dataLen) const
            {return timesVector(data, dataLen);}

        inline Matrix rowMultiply2(const Numeric* data, unsigned dataLen) const
            {return rowMultiply(data, dataLen);}

        inline Numeric bilinear2(const Numeric* data, unsigned dataLen) const
            {return bilinear(data, dataLen);}

        inline std::vector<Numeric> solveLinearSystem2(
            const Numeric* data, unsigned dataLen) const
        {
            std::vector<Numeric> result(dataLen);
            if (!solveLinearSystem(data, dataLen, dataLen ? &result[0] : 0))
                result.clear();
            return result;
        }

        inline std::vector<Numeric> symEigenValues() const
        {
            std::vector<Numeric> result(nrows_);
            symEigen(&result[0], nrows_, 0);
            return result;
        }

        // Perhaps, some day SWIG will be able to wrap the following. Currently,
        // its type deduction for GeneralizedComplex<Numeric>::type fails.
        //
        // inline std::vector<typename GeneralizedComplex<Numeric>::type>
        // genEigenValues() const
        // {
        //    std::vector<typename GeneralizedComplex<Numeric>::type> r(nrows_);
        //    genEigen(&r[0], nrows_);
        //    return r;
        // }
#endif
    };

    /** Utility for making square diagonal matrices from the given array */
    template<typename Numeric>
    Matrix<Numeric> diag(const Numeric* data, unsigned dataLen);

    /**
    // Utility for making rectangular diagonal matrices from the given array.
    // The length of the array must be at least min(nrows, ncols).
    */
    template<typename Numeric>
    Matrix<Numeric> diag(const Numeric* data, unsigned nrows, unsigned ncols);
}

#include <iostream>

template<typename N, unsigned Len>
std::ostream& operator<<(std::ostream& os, const npstat::Matrix<N, Len>& m);

#include "npstat/nm/Matrix.icc"

#endif // NPSTAT_MATRIX_HH_
