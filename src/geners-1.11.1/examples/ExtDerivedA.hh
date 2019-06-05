#ifndef GSEXAMPLES_EXTDERIVEDA_HH_
#define GSEXAMPLES_EXTDERIVEDA_HH_

#include "ExtBase.hh"

class ExtDerivedA : public ExtBase
{
public:
    inline explicit ExtDerivedA(const int value) : value_(value) {}
    inline virtual ~ExtDerivedA() {}

    virtual int getValue() const {return value_;}

private:
    int value_;
};

#endif // GSEXAMPLES_EXTDERIVEDA_HH_
