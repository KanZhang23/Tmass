#include <cassert>
#include <algorithm>

#include "JesIntegResult.hh"

#include "geners/GenericIO.hh"
#include "geners/IOException.hh"

using namespace gs;

void JesIntegResult::clear()
{
    uid = 0;
    nDeltaJes = 0;
    minDeltaJes = 0.0;
    maxDeltaJes = 0.0;
    history.clear();
    qmcRelUncertainties.clear();
    timeElapsed = -1.0;
    status = CS_CONTINUE;
}

bool JesIntegResult::operator==(const JesIntegResult& r) const
{
    return uid                 == r.uid         &&
           minDeltaJes         == r.minDeltaJes &&
           maxDeltaJes         == r.maxDeltaJes &&
           nDeltaJes           == r.nDeltaJes   &&
           history             == r.history     &&
           qmcRelUncertainties == r.qmcRelUncertainties &&
           timeElapsed         == r.timeElapsed &&
           status              == r.status;
}

bool JesIntegResult::write(std::ostream& of) const
{
    write_pod(of, uid);
    write_pod(of, nDeltaJes);
    write_pod(of, minDeltaJes);
    write_pod(of, maxDeltaJes);
    write_pod(of, timeElapsed);
    unsigned st = status;
    write_pod(of, st);
    return write_item(of, history) && write_item(of, qmcRelUncertainties);
}

void JesIntegResult::restore(const gs::ClassId& id, std::istream& in,
                             JesIntegResult* ptr)
{
    static const ClassId myClassId(ClassId::makeId<JesIntegResult>());
    myClassId.ensureSameName(id);

    assert(ptr);
    read_pod(in, &ptr->uid);
    read_pod(in, &ptr->nDeltaJes);
    read_pod(in, &ptr->minDeltaJes);
    read_pod(in, &ptr->maxDeltaJes);
    read_pod(in, &ptr->timeElapsed);
    unsigned st;
    read_pod(in, &st);
    ptr->status = static_cast<ConvergenceStatus>(st);
    restore_item(in, &ptr->history);

    // Version-dependent code
    if (id.version() > 1)
        restore_item(in, &ptr->qmcRelUncertainties);
    else
        ptr->qmcRelUncertainties.clear();
    if (id.version() > 2)
        throw IOReadFailure("In JesIntegResult::restore: stored class version "
                            "number is too high; please update your code");

    if (in.fail()) throw IOReadFailure("In JesIntegResult::restore: "
                                       "input stream failure");
}

JesIntegResult JesIntegResult::changeDutyCycle(const unsigned factor) const
{
    if (factor < 2U)
        return *this;

    // Figure out which cells will be used
    std::vector<unsigned> indices;
    indices.reserve(nDeltaJes/factor + 2);
    const int delta = factor;
    for (int i = nDeltaJes/2; i >= 0; i -= delta)
        indices.push_back(i);
    for (unsigned u=nDeltaJes/2+factor; u<nDeltaJes; u+=factor)
        indices.push_back(u);
    std::sort(indices.begin(), indices.end());
    const double bw = (maxDeltaJes - minDeltaJes)/nDeltaJes;

    JesIntegResult result;
    result.uid = uid;
    result.nDeltaJes = indices.size();
    if (result.nDeltaJes == 1U)
    {
        result.minDeltaJes = -1.0;
        result.maxDeltaJes = 1.0;
    }
    else
    {
        const double left = minDeltaJes + (indices[0] + 0.5)*bw;
        const double right = minDeltaJes + (indices.back() + 0.5)*bw;
        const double newbw = (right - left)/(result.nDeltaJes - 1U);
        result.minDeltaJes = left - newbw/2.0;
        result.maxDeltaJes = right + newbw/2.0;
    }

    const unsigned historyLen = history.size();
    result.history.resize(historyLen);
    for (unsigned h=0; h<historyLen; ++h)
    {
        result.history[h].reserve(result.nDeltaJes);
        for (unsigned ijes=0; ijes<result.nDeltaJes; ++ijes)
            result.history[h].push_back(history[h][indices[ijes]]);
    }

    const unsigned uncertLen = qmcRelUncertainties.size();
    if (uncertLen)
    {
        assert(historyLen == uncertLen);
        result.qmcRelUncertainties.resize(uncertLen);
        for (unsigned h=0; h<uncertLen; ++h)
        {
            result.qmcRelUncertainties[h].reserve(result.nDeltaJes);
            for (unsigned ijes=0; ijes<result.nDeltaJes; ++ijes)
                result.qmcRelUncertainties[h].push_back(
                    qmcRelUncertainties[h][indices[ijes]]);
        }
    }

    result.timeElapsed = timeElapsed;
    result.status = status;
    return result;
}
