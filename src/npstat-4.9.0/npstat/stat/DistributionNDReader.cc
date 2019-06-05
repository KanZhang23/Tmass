#include "npstat/stat/DistributionNDReader.hh"

#include "npstat/stat/DistributionsND.hh"
#include "npstat/stat/GridInterpolatedDistribution.hh"
#include "npstat/stat/ScalableGaussND.hh"
#include "npstat/stat/CompositeDistributionND.hh"
#include "npstat/stat/Copulas.hh"

//
// If you copy the "add_reader" macro below, do not forget to change
// "AbsDistributionND" into the name of your base class
//
#define add_reader(Derived) do {\
     const gs::ClassId& id(gs::ClassId::makeId<Derived >());\
     (*this)[id.name()] = new gs::ConcreteReader<AbsDistributionND,Derived >();\
} while(0);

namespace npstat {
    DistributionNDReader::DistributionNDReader()
    {
        add_reader(ProductDistributionND);
        add_reader(RadialProfileND);
        add_reader(UniformND);
        add_reader(ScalableGaussND);
        add_reader(BinnedDensityND);
        add_reader(GridInterpolatedDistribution);
        add_reader(CompositeDistributionND);
        add_reader(GaussianCopula);
        add_reader(FGMCopula);
        add_reader(TCopula);
    }
}
