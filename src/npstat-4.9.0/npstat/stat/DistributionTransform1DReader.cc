#include "npstat/stat/DistributionTransform1DReader.hh"

#include "npstat/stat/IdentityTransform1D.hh"
#include "npstat/stat/AsinhTransform1D.hh"
#include "npstat/stat/LogRatioTransform1D.hh"
#include "npstat/stat/LogTransform1D.hh"
#include "npstat/stat/SinhAsinhTransform1D.hh"

#define add_reader(Derived) do {\
     const gs::ClassId& id(gs::ClassId::makeId<Derived >());\
     (*this)[id.name()] = new gs::ConcreteReader<AbsDistributionTransform1D,Derived >();\
} while(0);

namespace npstat {
    DistributionTransform1DReader::DistributionTransform1DReader()
    {
        add_reader(IdentityTransform1D);
        add_reader(AsinhTransform1D);
        add_reader(LogTransform1D);
        add_reader(LogRatioTransform1D);
        add_reader(SinhAsinhTransform1D);
    }
}
