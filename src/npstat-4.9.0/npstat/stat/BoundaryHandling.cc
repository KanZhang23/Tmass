#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <cfloat>

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

#include "npstat/stat/BoundaryMethod.hh"
#include "npstat/stat/BoundaryHandling.hh"

//
// Adding a new boundary handling method to the factory function:
//
// 1) Add an enum value in BoundaryMethod.hh.
// 2) Add elements here in the arrays "names" and "n_expected_params".
// 3) Add the relevant call in "Filter1DBuilders.cc".
//

static const char* names[] = {
    "BM_TRUNCATE",
    "BM_STRETCH",
    "BM_FOLD",
    "BM_CONSTSQ",
    "BM_FOLDSQ",
    "BM_CONSTVAR",
    "BM_FOLDVAR",
    "BM_CONSTBW",
    "BM_FOLDBW"
};

static const unsigned n_boundary_methods = sizeof(names)/sizeof(names[0]);

// The number of parameters here should be in sync with
// the parameter usage in file "Filter1DBuilders.cc"
static const unsigned n_expected_params[n_boundary_methods] = {
    0, // BM_TRUNCATE
    0, // BM_STRETCH
    0, // BM_FOLD
    0, // BM_CONSTSQ
    0, // BM_FOLDSQ
    0, // BM_CONSTVAR
    0, // BM_FOLDVAR
    2, // BM_CONSTBW
    2  // BM_FOLDBW
};

static unsigned expectedNPars(const unsigned i)
{
    assert(i < n_boundary_methods);
    return n_expected_params[i];
}

static unsigned parseBoundaryMethod(const char* methodName)
{
    if (!methodName)
        return 0U;
    for (unsigned i=0; i<n_boundary_methods; ++i)
        if (strcmp(names[i], methodName) == 0)
            return i;
    std::ostringstream os;
    os << "In parseBoundaryMethod: invalid argument \""
       << methodName << "\". Must be one of "
       << npstat::BoundaryHandling::validMethodNames() << '.';
    throw std::invalid_argument(os.str());
}

static const char* boundaryMethodName(const unsigned i)
{
    assert(i < n_boundary_methods);
    return names[i];
}

namespace npstat {
    std::string BoundaryHandling::validMethodNames()
    {
        std::ostringstream os;
        for (unsigned i=0; i<n_boundary_methods; ++i)
        {
            if (i == 0)
                os << names[i];
            else if (i == n_boundary_methods - 1)
                os << " or " << names[i];
            else
                os << ", " << names[i];
        }
        return os.str();
    }

    std::string BoundaryHandling::parameterFreeNames()
    {
        unsigned n_such_names = 0;
        for (unsigned i=0; i<n_boundary_methods; ++i)
            if (!n_expected_params[i])
                ++n_such_names;

        std::ostringstream os;
        for (unsigned i=0, cnt=0; i<n_boundary_methods; ++i)
        {
            if (n_expected_params[i])
                continue;
            if (cnt == 0)
                os << names[i];
            else if (cnt == n_such_names - 1)
                os << " or " << names[i];
            else
                os << ", " << names[i];
            ++cnt;
        }
        return os.str();
    }

    bool BoundaryHandling::isValidMethodName(const char* methodName)
    {
        if (!methodName)
            return true;
        for (unsigned i=0; i<n_boundary_methods; ++i)
            if (strcmp(names[i], methodName) == 0)
                return true;
        return false;
    }

    bool BoundaryHandling::isParameterFree(const char* methodName)
    {
        if (!methodName)
            return !expectedNPars(0);
        for (unsigned i=0; i<n_boundary_methods; ++i)
            if (strcmp(names[i], methodName) == 0)
                return !expectedNPars(i);
        return false;
    }

    bool BoundaryHandling::operator==(const BoundaryHandling& r) const
    {
        if (id_ != r.id_)
            return false;
        assert(nParams_ == r.nParams_);
        for (unsigned i=0; i<nParams_; ++i)
            if (params_[i] != r.params_[i])
                return false;
        return true;
    }

    bool BoundaryHandling::operator<(const BoundaryHandling& r) const
    {
        if (id_ < r.id_)
            return true;
        if (id_ > r.id_)
            return false;
        assert(nParams_ == r.nParams_);
        for (unsigned i=0; i<nParams_; ++i)
        {
            if (params_[i] < r.params_[i])
                return true;
            if (params_[i] > r.params_[i])
                return false;
        }
        return false;
    }

    const char* BoundaryHandling::methodName() const
    {
        return boundaryMethodName(id_);
    }

    void BoundaryHandling::initialize(const char* i_methodName,
                                      const double* params,
                                      const unsigned nParams)
    {
        id_ = parseBoundaryMethod(i_methodName);
        if (expectedNPars(id_) != nParams) throw std::invalid_argument(
            "In npstat::BoundaryHandling::initialize: "
            "unexpected number of parameters");
        nParams_ = nParams;
        assert(nParams_ <= BM_MAXPARAMS);
        for (unsigned i=0; i<nParams; ++i)
            params_[i] = params[i];
        for (unsigned i=nParams; i<BM_MAXPARAMS; ++i)
            params_[i] = -DBL_MAX;
    }

    unsigned BoundaryHandling::expectedNParameters(const char* name)
    {
        return expectedNPars(parseBoundaryMethod(name));
    }

    bool BoundaryHandling::write(std::ostream& of) const
    {
        const std::string name(methodName());
        gs::write_pod(of, name);
        gs::write_pod(of, nParams_);
        if (nParams_)
            gs::write_pod_array(of, &params_[0], nParams_);
        return !of.fail();
    }

    void BoundaryHandling::restore(const gs::ClassId& id, std::istream& in,
                                   BoundaryHandling* ptr)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<BoundaryHandling>());
        myClassId.ensureSameId(id);

        std::string name;
        gs::read_pod(in, &name);
        unsigned npars;
        gs::read_pod(in, &npars);
        if (npars > BM_MAXPARAMS) throw gs::IOInvalidData(
            "In npstat::BoundaryHandling::restore: invalid data");
        double data[BM_MAXPARAMS];
        double* pdata = &data[0];
        if (npars)
            gs::read_pod_array(in, pdata, npars);
        else
            pdata = 0;
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::BoundaryHandling::restore: input stream failure");
        *ptr = BoundaryHandling(name.c_str(), pdata, npars);
    }
}
