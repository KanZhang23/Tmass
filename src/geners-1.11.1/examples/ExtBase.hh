#ifndef GSEXAMPLES_EXTBASE_HH_
#define GSEXAMPLES_EXTBASE_HH_

class ExtBase
{
public:
    inline ExtBase() {}
    inline virtual ~ExtBase() {}

    virtual int getValue() const = 0;
};

#endif // GSEXAMPLES_EXTBASE_HH_
