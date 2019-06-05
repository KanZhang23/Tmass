#ifndef NPSTAT_KDTREE_HH_
#define NPSTAT_KDTREE_HH_

/**
// \file KDTree.hh
//
// \brief k-d tree template
//
// Author: I. Volobouev
//
// March 2010
*/

#include <stdexcept>

#include "npstat/nm/BoxND.hh"
#include "npstat/nm/AbsVisitor.hh"

namespace npstat {
    /**
    // Basic k-d tree template. See, for example,
    // http://en.wikipedia.org/wiki/K-d_tree for the
    // computational geometry ideas behind this type
    // of space partitioning.
    //
    // Intended for speeding up the construction of empirical copulas
    // or copula densities. Can also be used for "sliding window"
    // kernel density estimation, local regression, etc.
    //
    // Class Point must possess operator[] which returns an object
    // or reference to Numeric or some other type which can be
    // automatically converted to Numeric.
    */
    template <class Point, typename Numeric=double>
    class KDTree
    {
    public:
        /**
        // For efficiency reasons, this class will not make an internal
        // copy of the "points" vector. This vector must not be modified
        // after KDTree construction. Otherwise tree pointers will become
        // invalid.
        //
        // The arguments "dimsToUse" (and "nDimsToUse" which is the length
        // of the "dimsToUse" array) specify which point dimensions should
        // be used to construct the tree.
        */
        KDTree(const std::vector<Point>& points,
               const unsigned* dimsToUse, unsigned nDimsToUse);
        ~KDTree();

        /**
        // Return the reference to the vector of points
        // used to construct the tree
        */
        inline const std::vector<Point>& inputPoints() const {return points_;}

        /** The size of the vector of points used to construct the tree */
        inline unsigned nPoints() const {return nPoints_;}

        /** Total number of nodes (trunk and leaf) */
        inline unsigned nNodes() const {return nodes_.size();}

        /** 
        // The tree dimensionality (less or equal to the dimensionality
        // of points used to construct the tree)
        */
        inline unsigned dim() const {return dim_;}

        /** The point dimensions used for tree construction */
        inline unsigned pointIndex(const unsigned i) const
        {
            if (i >= dim_) throw std::out_of_range(
                "In npstat::KDTree::pointIndex: index out of range");
            return indices_[i];
        }

        /** 
        // Number of points whose coordinates are all equal to
        // or below the corresponding coordinates of the "limit" array.
        // The length of this array must be equal to the tree
        // dimensionality.
        */
        unsigned nCdf(const Numeric* limit, unsigned limitDim) const;

        /** 
        // Number of points whose coordinates are all inside
        // the given box (including the upper boundaries). The box
        // dimensionality must be equal to the tree dimensionality.
        */
        unsigned nInBox(const BoxND<Numeric>& box) const;

        /** 
        // Visit all points inside the given box. This function will not
        // call either "clear()" or "result()" methods of the visitor
        // object, only "process".
        */
        template <class Result>
        void visitInBox(AbsVisitor<Point,Result>& visitor,
                        const BoxND<Numeric>& box) const;
    private:
        // Disable the default constructors and
        // the assignment operator
        KDTree();
        KDTree(const KDTree&);
        KDTree& operator=(const KDTree&);

        struct KDTreeNode
        {
            inline KDTreeNode() : point(0), split(Numeric()), splitIndex(0), 
                                  nPoints(0), left(0), right(0) {}
            const Point* point;
            Numeric split;
            unsigned splitIndex;
            unsigned nPoints;
            unsigned left;
            unsigned right;
        };

        unsigned pointLessOrEqual(const Point* pt, const Numeric* coord) const;
        unsigned coordsLessOrEqual(const Numeric* c1, const Numeric* c2) const;
        unsigned pointInBox(const Point* pt, const BoxND<Numeric>& box) const;
        bool limitsInBox(const BoxND<Numeric>& box, const Numeric* lowerLimit,
                         const Numeric* upperLimit) const;

        unsigned addNode(std::vector<const Point*>& pointers,
                         unsigned level, unsigned ibegin, unsigned iend);

        // Number of points below or equal to the given one,
        // starting with the given inode
        unsigned countBelow(unsigned inode, const Numeric* limit,
                            Numeric* upperLimit) const;

        // Number of points in the given box, starting with the given inode
        unsigned countInBox(unsigned inode, const BoxND<Numeric>& box,
                            Numeric* lowerLimit, Numeric* upperLimit) const;

        // Recursive point lookup for the visitor
        template <class Result>
        void visitorRecursion(AbsVisitor<Point,Result>& visitor,
                              unsigned inode, const BoxND<Numeric>& box,
                              Numeric* lowerLimit, Numeric* upperLimit,
                              bool knownInside) const;

        const std::vector<Point>& points_;
        unsigned* indices_;
        std::vector<KDTreeNode> nodes_;
        unsigned nPoints_;
        unsigned dim_;

        mutable std::vector<Numeric> lolim_;
        mutable std::vector<Numeric> uplim_;
    };
}

#include "npstat/nm/KDTree.icc"

#endif // NPSTAT_KDTREE_HH_
