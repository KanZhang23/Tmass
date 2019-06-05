#include <cassert>
#include <cmath>

#include "makeResultHisto.hh"
#include "histoscope.h"

static int make_1d_h(const int uid, const char* title, const char* category,
                     const JesIntegResult& result, const double scale,
                     const bool useQmcUncertainties)
{
    static std::vector<float> bufVec;

    const unsigned nbins = result.nDeltaJes;
    const int id = hs_create_1d_hist(
        uid, title, category, "Delta JES", "Weight",
        nbins, result.minDeltaJes, result.maxDeltaJes);
    if (id > 0)
    {
        const std::vector<npstat::StatAccumulator>& vec = result.history.back();
        const std::vector<double>* qmcUncert = 0;
        if (!result.qmcRelUncertainties.empty())
            qmcUncert = &result.qmcRelUncertainties.back();
        assert(vec.size() == nbins);
        if (bufVec.size() < 2*nbins)
            bufVec.resize(2*nbins);
        float* bins = &bufVec[0];
        float* errs = bins + nbins;
        for (unsigned i=0; i<nbins; ++i)
        {
            const double value = vec[i].noThrowMean()*scale;
            bins[i] = value;
            if (useQmcUncertainties)
            {
                if (qmcUncert)
                    errs[i] = (*qmcUncert)[i]*value;
                else
                    errs[i] = -1.0*value;
            }
            else
                errs[i] = vec[i].noThrowMeanUncertainty()*scale;
        }
        hs_1d_hist_block_fill(id, bins, errs, NULL);
    }
    return id;
}

static int make_2d_h(const int uid, const char* title, const char* category,
                     const JesIntegResult& result, const double scale,
                     const bool useQmcUncertainties)
{
    static std::vector<float> bufVec;

    const unsigned npows = result.history.size();
    const double firstCount = result.history[0][0].count();
    assert(firstCount > 0.0);
    const double powmin = round(log(firstCount)/log(2));
    const double powmax = round(log(result.history[npows-1][0].count())/log(2));
    assert(powmax - powmin == static_cast<double>(npows - 1));
    const unsigned nbins = result.nDeltaJes;
    const int id = hs_create_2d_hist(
        uid, title, category, "Power of 2", "Delta JES", "Weight",
        npows, nbins, powmin-0.5, powmax+0.5,
        result.minDeltaJes, result.maxDeltaJes);
    if (id > 0)
    {
        const bool haveQmcUncert = !result.qmcRelUncertainties.empty();
        if (haveQmcUncert)
            assert(result.qmcRelUncertainties.size() == npows);
        if (bufVec.size() < 2*npows*nbins)
            bufVec.resize(2*npows*nbins);
        float* bins = &bufVec[0];
        float* errs = bins + npows*nbins;
        for (unsigned ipow=0; ipow<npows; ++ipow)
        {
            const std::vector<npstat::StatAccumulator>& vec = result.history[ipow];
            assert(vec.size() == nbins);
            if (haveQmcUncert)
                assert(result.qmcRelUncertainties[ipow].size() == nbins);
            for (unsigned i=0; i<nbins; ++i)
            {
                const double value = vec[i].noThrowMean()*scale;
                *bins++ = value;
                if (useQmcUncertainties)
                {
                    if (haveQmcUncert)
                        *errs++ = result.qmcRelUncertainties[ipow][i]*value;
                    else
                        *errs++ = -1.0*value;
                }
                else
                    *errs++ = vec[i].noThrowMeanUncertainty()*scale;
            }
        }
        bins = &bufVec[0];
        errs = bins + npows*nbins;
        hs_2d_hist_block_fill(id, bins, errs, NULL);
    }
    return id;
}

int makeResultHisto(const int uid, const char* title, const char* category,
                    const JesIntegResult& result, const bool lastSetOnly,
                    const double scaleFactor, const bool useQmcU)
{
    assert(title);
    assert(category);
    assert(!result.history.empty());

    if (lastSetOnly)
        return make_1d_h(uid, title, category, result, scaleFactor, useQmcU);
    else
        return make_2d_h(uid, title, category, result, scaleFactor, useQmcU);
}
