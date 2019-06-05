#ifndef NPSTAT_DISTRIBUTION1DFACTORY_HH_
#define NPSTAT_DISTRIBUTION1DFACTORY_HH_

/*!
// \file Distribution1DFactory.hh
//
// \brief Factories for 1-d distributions for use in interpretive language 
//        environments
//
// Author: I. Volobouev
//
// April 2010
*/

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class AbsScalableDistribution1DFactory
    {
    public:
        inline virtual ~AbsScalableDistribution1DFactory() {}
        virtual AbsScalableDistribution1D* create(
            double location, double scale,
            const std::vector<double>&) const = 0;
        virtual int nParameters() const = 0;
    };

    template <typename T>
    class ScalableDistribution1DFactory: public AbsScalableDistribution1DFactory
    {
    public:
        inline virtual ~ScalableDistribution1DFactory() {}

        inline int nParameters() const {return T::nParameters();}

        T* create(const double location, const double scale,
                  const std::vector<double>& params) const
        {
            if (nParameters() >= 0)
                if (params.size() != static_cast<unsigned>(nParameters()))
                    throw std::invalid_argument(
                        "In npstat::ScalableDistribution1DFactory constructor:"
                        " wrong number of distribution parameters");
            return new T(location, scale, params);
        }
    };

    /**
    // Use this factory to construct a 1-d distribution from
    // its name, location, scale, and the vector of other parameters.
    //
    // Not all distributions support this construction method. Iterate
    // over the keys in this map (or look at the Distribution1DFactory.cc
    // code) for the list of supported distributions. Of course,
    // user-developed distributions can be added to this factory
    // as well if they implement a compatible constructor.
    */
    class DefaultScalableDistribution1DFactory :
        public std::map<std::string, AbsScalableDistribution1DFactory *>
    {
    public:
        DefaultScalableDistribution1DFactory();
        virtual ~DefaultScalableDistribution1DFactory();

    private:
        DefaultScalableDistribution1DFactory(
            const DefaultScalableDistribution1DFactory&);
        DefaultScalableDistribution1DFactory& operator=(
            const DefaultScalableDistribution1DFactory&);
    };
}

#endif // NPSTAT_DISTRIBUTION1DFACTORY_HH_
