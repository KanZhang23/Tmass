#include <vector>
#include <cassert>
#include <climits>

#include "packStringToHSItem.hh"
#include "histoscope.h"

int packStringToHSItem(const int uid, const char* title,
                       const char* category, const std::string& str)
{
    const unsigned fsize = sizeof(float);
    assert(fsize == sizeof(unsigned));
    assert(str.size() < UINT_MAX);

    const char* names[] = {"c0"};
    const int id = hs_create_ntuple(uid, title, category, 1, (char**)names);
    if (id <= 0)
        return id;
    unsigned sz = str.size();
    assert(hs_fill_ntuple(id, (float*)(&sz)) == id);
    const char* s = &str[0];
    for (; sz >= fsize; sz -= fsize, s += fsize)
        assert(hs_fill_ntuple(id, (float*)(s)) == id);
    if (sz)
    {
        float tmp = 0.f;
        char* ctmp = (char*)(&tmp);
        for (; sz>0; --sz)
            *ctmp++ = *s++;
        assert(hs_fill_ntuple(id, &tmp) == id);
    }

    return id;
}

std::string unpackStringFromHSItem(const int id)
{
    static std::vector<float> bufVec;

    const unsigned fsize = sizeof(float);
    assert(fsize == sizeof(unsigned));
    assert(hs_type(id) == HS_NTUPLE);
    assert(hs_num_variables(id) == 1);
    const unsigned nrows = hs_num_entries(id);
    assert(nrows);
    if (bufVec.size() < nrows)
        bufVec.resize(nrows);
    float* buf = &bufVec[0];
    hs_ntuple_contents(id, buf);
    const unsigned sz = *((unsigned*)buf);
    assert(sz <= (nrows-1U)*fsize);
    char* start = (char*)(buf + 1);
    return std::string(start, start+sz);
}
