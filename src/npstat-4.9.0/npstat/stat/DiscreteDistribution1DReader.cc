#include "npstat/stat/DiscreteDistribution1DReader.hh"

#include "npstat/stat/DiscreteDistributions1D.hh"

#define add_reader(Derived) do {\
     const gs::ClassId& id(gs::ClassId::makeId<Derived >());\
     (*this)[id.name()] = new gs::ConcreteReader<AbsDiscreteDistribution1D,Derived >();\
} while(0);

namespace npstat {
    DiscreteDistribution1DReader::DiscreteDistribution1DReader()
    {
        add_reader(Poisson1D);
        add_reader(DiscreteTabulated1D);
    }
}
