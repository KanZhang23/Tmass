#include <cmath>

#include "cone.hh"

namespace mcuncert {
	namespace priv {

		long double cone::cfrag(unsigned long m) const
		{
                    return this->omghat(m)*this->omgcirc(rlag)/(1.0L-this->omghat(rlag)*this->omgcirc(rlag));
		}

		long double cone::omgcirc(unsigned long m) const
		{
                    return gamma/pow(2.0L,p*m);
		}

		long double cone::omghat(unsigned long m) const
		{
                    return beta/pow(2.0L,q*m);
		}

	}
}
