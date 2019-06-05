#ifndef NPSTAT_PYCLASSID_HH_
#define NPSTAT_PYCLASSID_HH_

#include "Python.h"

#include "geners/ClassId.hh"
#include "geners/IOIsExternal.hh"

gs_specialize_class_id(PyObject, 1)
gs_declare_type_external(PyObject)

#endif // NPSTAT_PYCLASSID_HH_
