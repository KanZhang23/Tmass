#include <cmath>
#include <climits>
#include <sstream>
#include <numeric>

#include "npstat/nm/allocators.hh"
#include "npstat/rng/AbsRandomGenerator.hh"
#include "npstat/stat/BinnedADTest1D.hh"
#include "npstat/stat/DiscreteDistributions1D.hh"

// The code for the distribution of Anderson-Darling test statistic comes
// from "Evaluating the Anderson-Darling Distribution" by G. Marsaglia and
// J. Marsaglia, Journal of Statistical Software, vol. 9, issue 2,
// pp. 1-5 (2004).
static double ADf(const double z, const int j)
{
    const double t = (4*j+1)*(4*j+1)*1.23370055013617/z;
    if (t > 150.0)
        return 0.0;
    double a = 2.22144146907918*exp(-t)/sqrt(t);
    // double b = 3.93740248643060*2.*cPhi(sqrt(2*t)); /* requires cPhi */
    /*if you have erfc(), replace 2*cPhi(sqrt(2*t)) with erfc(sqrt(t))*/
    double b = 3.93740248643060*erfc(sqrt(t));
    double r = z*.125;
    double f = a+b*r;
    for (int i=1; i<200; i++)
    {
        const double c = ((i-.5-t)*b+t*a)/i;
        a=b; b=c;
        r *= z/(8*i+8);
        if (fabs(r) < 1.e-40 || fabs(c) < 1.e-40)
            return f;
        const double fnew = f+c*r;
        if (f == fnew)
            return f;
        f = fnew;
    }
    return f;
}

static double ADinf(const double z)
{
    if (z < 0.01)
        return 0.0; /* avoids exponent limits; ADinf(.01)=.528e-52 */
    if (z > 32.0)
        return 1.0;

    double r = 1.0/z;
    double ad = r*ADf(z, 0);
    for (int j=1; j<100; j++)
    {
        r *= (0.5 - j)/j;
        const double adnew = ad+(4*j+1)*r*ADf(z,j);
        if (ad == adnew)
            return ad;
        ad = adnew;
    }
    return ad;
}

static double AD(const double n, const double z)
{
    const double x = ADinf(z);

    /* now x=adinf(z). Next, get v=errfix(n,x) and return x+v; */
    if (x > 0.8)
    {
        const double v=(-130.2137+(745.2337-(1705.091-(1950.646-(1116.360-255.7844*x)*x)*x)*x)*x)/n;
        return x+v;
    }

    const double c=.01265+.1757/n;
    if (x < c)
    { 
        double v = x/c;
        v=sqrt(v)*(1.-v)*(49*v-102);
        return x+v*(.0037/n/n + .00078/n + .00006)/n;
    }
    else
    {
        double v = (x-c)/(.8-c);
        v=-.00022633+(6.54034-(14.6538-(14.458-(8.259-1.91864*v)*v)*v)*v)*v;
        return x+v*(.04213+.01365/n)/n;
    }
}

namespace {
    struct DiscreteADDistance : public npstat::AbsDiscreteDistribution1DDistance
    {
        double operator()(const npstat::AbsDiscreteDistribution1D& data,
                          const npstat::AbsDiscreteDistribution1D& data2,
                          const npstat::AbsDiscreteDistribution1D* pooled,
                          const long first, const long last) const
        {
            const npstat::AbsDiscreteDistribution1D& ref = 
                (pooled ? *pooled : data2);

            long double sum = 0.0L;
            double oldRefCdf = 0.0;
            double oldRefExceedance = 1.0;
            double oldDataCdf = 0.0;
            double oldDataExceedance = 1.0;
            double oldData2Cdf = 0.0;
            double oldData2Exceedance = 1.0;
            bool useExceedance = false;

            double cdfCenterData = 0.0, excCenterData = 0.0;
            double cdfCenterData2 = 0.0, excCenterData2 = 0.0;

            for (long i=first; i<last; ++i)
            {
                const double prob = ref.probability(i);

                const double refCdf = ref.cdf(i);
                const double cdfCenterRef = (refCdf + oldRefCdf)/2.0;
                oldRefCdf = refCdf;

                const double refExceedance = ref.exceedance(i);
                const double excCenterRef = (refExceedance+oldRefExceedance)/2.0;
                oldRefExceedance = refExceedance;

                // Avoid subtractive cancellation by using exceedance
                // for large values of x
                if (useExceedance)
                {
                    const double dataExceedance = data.exceedance(i);
                    excCenterData = (dataExceedance + oldDataExceedance)/2.0;
                    cdfCenterData = 1.0 - excCenterData;
                    oldDataExceedance = dataExceedance;
                    oldDataCdf = 1.0 - oldDataExceedance;

                    if (pooled)
                    {
                        const double data2Exceedance = data2.exceedance(i);
                        excCenterData2=(data2Exceedance+oldData2Exceedance)/2.0;
                        cdfCenterData2 = 1.0 - excCenterData2;
                        oldData2Exceedance = data2Exceedance;
                        oldData2Cdf = 1.0 - oldData2Exceedance;
                    }
                }
                else
                {
                    const double dataCdf = data.cdf(i);
                    cdfCenterData = (dataCdf + oldDataCdf)/2.0;
                    excCenterData = 1.0 - cdfCenterData;
                    oldDataCdf = dataCdf;
                    oldDataExceedance = 1.0 - oldDataCdf;

                    if (pooled)
                    {
                        const double data2Cdf = data2.cdf(i);
                        cdfCenterData2 = (data2Cdf + oldData2Cdf)/2.0;
                        excCenterData2 = 1.0 - cdfCenterData2;
                        oldData2Cdf = data2Cdf;
                        oldData2Exceedance = 1.0 - oldData2Cdf;
                    }

                    useExceedance = (refCdf > 0.5);
                }

                if (prob > 0.0)
                {
                    double delta;
                    if (pooled)
                    {
                        if (useExceedance)
                            delta = excCenterData - excCenterData2;
                        else
                            delta = cdfCenterData - cdfCenterData2;
                    }
                    else
                    {
                        if (useExceedance)
                            delta = excCenterData - excCenterRef;
                        else
                            delta = cdfCenterData - cdfCenterRef;
                    }
                    sum += prob*delta*delta/cdfCenterRef/excCenterRef;
                }
            }
            return sum;
        }
    };
}

namespace npstat {
    BinnedADTest1D::BinnedADTest1D(const bool useTwoSampleTest,
                                   AbsRandomGenerator* rng,
                                   const unsigned mcSamplesForPValue)
        : rng_(rng),
          mcSamples_(mcSamplesForPValue),
          twoSampleTest_(useTwoSampleTest)
    {
    }

    const char* BinnedADTest1D::name() const
    {
        if (name_.empty())
        {
            std::ostringstream os;
            if (twoSampleTest_)
                os << "Two-sample ";
            os << "Anderson-Darling";
            if (rng_ && mcSamples_)
                os << " by MC";
            name_ = os.str();
        }
        return name_.c_str();
    }

    void BinnedADTest1D::compareD(const double* pdata, const double* preference,
                                  const unsigned len, double* distance,
                                  double* pvalue) const
    {
        DiscreteTabulated1D data(0, pdata, len);
        DiscreteTabulated1D ref(0, preference, len);
        DiscreteADDistance calc;
        const double dData = std::accumulate(pdata, pdata+len, 0.0L);
        double dist = 0.0, dRef = 0.0;

        if (twoSampleTest_)
        {
            dRef = std::accumulate(preference, preference+len, 0.0L);
            const DiscreteTabulated1D& pooled = pooledDiscreteTabulated1D(
                data, dData, ref, dRef, 0L, len);
            dist = calc(data, ref, &pooled, 0L, len);
        }
        else
            dist = calc(data, ref, 0, 0L, len);
        if (distance)
            *distance = dist;

        if (pvalue)
        {
            if (rng_ && mcSamples_)
            {
                // Calculate the p-value by MC
                *pvalue = pvalueByPseudo(calc, *rng_, data, dData,
                                         ref, dRef, dist, mcSamples_);
            }
            else
            {
                const double nEff = twoSampleTest_ ? 
                    dData*dRef/(dData + dRef) : dData;
                *pvalue = 1.0 - AD(nEff, nEff*dist);
                if (*pvalue > 1.0)
                    *pvalue = 1.0;
            }
        }
    }
}
