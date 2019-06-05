#ifndef NPSTAT_TEMPLATEPARAMETERS_HH_
#define NPSTAT_TEMPLATEPARAMETERS_HH_

#include "geners/ClassId.hh"

namespace npstat {
    PyObject* templateParameters(const gs::ClassId& id)
    {
        std::vector<std::vector<gs::ClassId> > pars;
        id.templateParameters(&pars);
        const unsigned long npars = pars.size();
        PyObject* tup = PyTuple_New(npars);
        if (tup)
            for (unsigned long i=0; i<npars; ++i)
            {
                PyObject* copy = PyUnicode_FromString(pars[i][0].name().c_str());
                if (copy == NULL)
                {
                    Py_DECREF(tup);
                    return NULL;
                }
                PyTuple_SET_ITEM(tup, i, copy);
            }
        return tup;
    }
}

#endif // NPSTAT_TEMPLATEPARAMETERS_HH_
