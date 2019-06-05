#ifndef GSEXAMPLES_EXTDERIVEDB_HH_
#define GSEXAMPLES_EXTDERIVEDB_HH_

#include "ExtDerivedA.hh"

class ExtDerivedB : public ExtDerivedA
{
public:
    inline ExtDerivedB(const int i, const double d) : ExtDerivedA(i), d_(d) {}
    inline virtual ~ExtDerivedB() {}

    inline double getAnother() const {return d_;}

private:
    double d_;
};

#endif // GSEXAMPLES_EXTDERIVEDB_HH_
