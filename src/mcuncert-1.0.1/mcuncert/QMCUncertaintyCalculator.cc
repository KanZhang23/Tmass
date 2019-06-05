#include <cmath>
#include <cfloat>
#include <new>          // std::nothrow

#include <iostream>

#include "QMCUncertaintyCalculator.hh"

namespace mcuncert {
    QMCUncertaintyCalculator::QMCUncertaintyCalculator(const unsigned long mmin, const unsigned l_star, const unsigned r_lag)
    {
        clear();
        if (mmin < r_lag + l_star) throw std::invalid_argument(
            "In mcuncert::QMCUncertaintyCalculator::QMCUncertaintyCalculator: "
            "Parametrization input error, mmin should be >= than lstar+rlag.");
            if (r_lag < 1 || l_star < 1) throw std::invalid_argument(
                "In mcuncert::QMCUncertaintyCalculator::QMCUncertaintyCalculator: "
                "Parametrization input error, rlag and lstar should be strictly positive integers.");
                cone_.lstar = l_star;
            cone_.rlag = r_lag;
            mmin_ = mmin;
        }

        void QMCUncertaintyCalculator::clear()
        {
            sum_ = 0.0L;
            sumsq_ = 0.0L;
            runningMean_ = 0.0L;
            min_ = DBL_MAX;
            max_ = -DBL_MAX;
            count_ = 0UL;
            count_exp_ = 0UL;
            nextRecenter_ = 0UL;
            mmin_ = 10UL;
            abs_error_bound_ = LDBL_MAX;
            ffwt_.clear();
            kappanumap_.clear();
            Stilde_.clear();
            nec_cond_lower_.clear(); for (unsigned i = cone_.lstar; i < mmin_; i++) {nec_cond_lower_.push_back(0.0L);}
            nec_cond_upper_.clear(); for (unsigned i = cone_.lstar; i < mmin_; i++) {nec_cond_upper_.push_back(LDBL_MAX);}
            nec_cond_fail_ = false;
        }

        void QMCUncertaintyCalculator::addPoint(const long double functionValue)
        {
            const long double ldv = functionValue - runningMean_;
            sum_ += ldv;
            sumsq_ += ldv*ldv;
            ffwt_.push_back(functionValue);
            if (functionValue < min_)
                min_ = functionValue;
            if (functionValue > max_)
                max_ = functionValue;
        if (++count_ >= nextRecenter_){ // Updated every power of 2
            if (nextRecenter_ >= 1) { // We start updating when nextRecenter > 0
                ++count_exp_;
                this->update_transform();
                if (count_exp_ == mmin_){
                    this->init_kappanumap();
                    this->update_error_bound();
                    // this->print();
                }
                else if (count_exp_ > mmin_){
                    this->update_kappanumap();
                    this->update_error_bound();
                    // this->print();
                }
            }
            recenter();
        }
    }

    void QMCUncertaintyCalculator::recenter()
    {
        if (count_)
        {
            const long double m = sum_/count_;
            sumsq_ -= m*sum_;
            if (sumsq_ < 0.0L)
                sumsq_ = 0.0L;
            runningMean_ += m;
            sum_ = 0.0L;
            nextRecenter_ = count_*2UL;
        }
    }

    unsigned long QMCUncertaintyCalculator::count() const
    {
        return count_;
    }

    long double QMCUncertaintyCalculator::mean() const
    {
        if (!count_) throw std::runtime_error(
            "In mcuncert::QMCUncertaintyCalculator::mean: "
            "no data accumulated");
            return sum_/count_ + runningMean_;
    }

    long double QMCUncertaintyCalculator::meanUncertainty() const
    {
        if (!count_) throw std::runtime_error(
            "In mcuncert::QMCUncertaintyCalculator::meanUncertainty: "
            "no data accumulated");
            return this->abs_error_bound_;
    }

    long double QMCUncertaintyCalculator::min() const
    {
        return min_;
    }

    long double QMCUncertaintyCalculator::max() const
    {
        return max_;
    }

    long double QMCUncertaintyCalculator::sum() const
    {
        return sum_ + runningMean_*count_;
    }

    long double QMCUncertaintyCalculator::sumsq() const
    {
        long double s = sumsq_ + runningMean_*(runningMean_*count_ + 2.0*sum_);
        if (s < 0.0L)
            s = 0.0L;
        return s;
    }

    void QMCUncertaintyCalculator::print() const
    {
        // for (unsigned long i = 0; i < count_; i++){
        //     std::cout << ffwt_[i] << "   " << kappanumap_[i] << std::endl;
        //     std::cout << ffwt_[kappanumap_[i]] << std::endl;
        // }
        // std::cout << std::endl;

        // std::cout << cone_.lstar << "  " << cone_.rlag << "  " << mmin_ << std::endl;

        std::cout << "Nec conditions:\n";
        std::cout << "---------------\n"; 
        for (unsigned n = 0; n < nec_cond_upper_.size(); n++){ // Printing necessary conditions < 1
            std::cout << nec_cond_lower_[n] << " <= " << nec_cond_upper_[n] << " -- ";
        }
        std::cout << "end \n";
        std::cout << "Error bound: " << abs_error_bound_ << std::endl;

        // for (unsigned long l = cone_.lstar; l <= count_exp_; ++l) {
        //     std::cout << 1.0L/(1.0L+cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)) << "  ";
        // }
        // std::cout << "\n";

        // for (unsigned n = 0; n < Stilde_.size(); n++){ // Printing sums
        //     std::cout << Stilde_[n] << " -- ";
        // }
        // std::cout << " end \n";
        //         std::cout << "Factors "; // Printing necessary conditions factors
        // for (unsigned long l = cone_.lstar; l <= count_exp_; ++l) {
        //     std::cout << 1.0L/(1.0L-cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)) << "  ";
        // }
        // std::cout << "\n";
    }

    bool QMCUncertaintyCalculator::nec_cond_fail() const
    {
        return nec_cond_fail_;
    }

    void QMCUncertaintyCalculator::update_transform()
    {
        long double * even_oddval_ = new (std::nothrow) long double; if (even_oddval_ == 0) { std::cout << "Memory error: even_oddval_ not allocated.\n";} // Auxiliary variable

        // We compute the FFWT on last count_/2 points
        for (unsigned long l = 0; l < count_exp_-1; l++) {
            for (unsigned long n = count_/2UL; n < count_; n++){
                *even_oddval_ = ffwt_[n];
                ffwt_[n] = (*even_oddval_+ffwt_[n+pow(2,l)])/2;
                ffwt_[n+pow(2,l)] = (*even_oddval_-ffwt_[n+pow(2,l)])/2;
                if ((n+1) % (int)pow(2,l) == 0) {
                    n = n+pow(2,l);
                }
            }
        }

        // We compute the FFWT on all points
        for (unsigned long n = 0; n < count_/2UL; n++){
            *even_oddval_ = ffwt_[n];
            ffwt_[n] = (*even_oddval_+ffwt_[n+pow(2,count_exp_-1)])/2;
            ffwt_[n+pow(2,count_exp_-1)] = (*even_oddval_-ffwt_[n+pow(2,count_exp_-1)])/2;
            if ((n+1) % (int)pow(2,count_exp_-1) == 0) {
                n = n+pow(2,count_exp_-1);
            }
        }
        delete even_oddval_;
    }

    void QMCUncertaintyCalculator::init_kappanumap()
    {
        for (unsigned long i = 0; i < count_; i++) {
            kappanumap_.push_back(i);
        }
        for (unsigned long l = mmin_-1; l > 0; l--) {
            for (unsigned long n = 1; n < pow(2,l); n++){
                if (std::abs(ffwt_[kappanumap_[n]]) < std::abs(ffwt_[kappanumap_[n+pow(2,l)]])) {
                    for (unsigned long p = 0; p < count_; p = p+pow(2,l+1)) {
                        std::swap(kappanumap_[n+p],kappanumap_[n+pow(2,l)+p]);
                    }
                }
            }
        }
    }

    void QMCUncertaintyCalculator::update_kappanumap()
    {
        for (unsigned long i = 0; i < count_/2UL; i++) {
            kappanumap_.push_back(count_/2UL + kappanumap_[i]);
        }
        for (unsigned long l = count_exp_-1; l > count_exp_-cone_.rlag-1; l--) {
            for (unsigned long n = 1; n < pow(2,l); n++){
                if (std::abs(ffwt_[kappanumap_[n]]) < std::abs(ffwt_[kappanumap_[n+pow(2,l)]])) {
                    for (unsigned long p = 0; p < count_; p = p+pow(2,l+1)) {
                        std::swap(kappanumap_[n+p],kappanumap_[n+pow(2,l)+p]);
                    }
                }
            }
        }
    }

    void QMCUncertaintyCalculator::update_error_bound()
    {
        Stilde_.clear();
        for (unsigned long l = cone_.lstar; l <= count_exp_; ++l) {
            abs_error_bound_ = 0.0L;
            for (unsigned long n = pow(2,l-1); n < pow(2,l); n++) {
                abs_error_bound_ += std::abs(ffwt_[kappanumap_[n]]);
            }
            Stilde_.push_back(abs_error_bound_);
            if (l < count_exp_){
                nec_cond_aux_ = nec_cond_lower_[l-cone_.lstar];
                nec_cond_lower_[l-cone_.lstar] = std::max(abs_error_bound_/(1.0L+cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)),nec_cond_aux_);
                if ((cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)) < 1.0L){
                    nec_cond_aux_ = nec_cond_upper_[l-cone_.lstar];
                    nec_cond_upper_[l-cone_.lstar] = std::min(abs_error_bound_/(1.0L-cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)),nec_cond_aux_);
                }
            }
            else {
                nec_cond_lower_.push_back(abs_error_bound_/(1.0L+cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)));
                if ((cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)) < 1.0L) {
                    nec_cond_upper_.push_back(abs_error_bound_/(1.0L-cone_.omgcirc(count_exp_-l)*cone_.omghat(count_exp_-l)));
                }
                else{
                    nec_cond_upper_.push_back(LDBL_MAX);
                }
            }
        }

        // Checking necessay conditions
        for (unsigned long k = cone_.lstar; k <= count_exp_; k++){
            if (nec_cond_lower_[k-cone_.lstar] > nec_cond_upper_[k-cone_.lstar] && !nec_cond_fail_){
                nec_cond_fail_ = true;
                this->enlarge_cone();
            }
        }

        // Assigning error bound
        abs_error_bound_ = Stilde_[count_exp_-cone_.rlag-cone_.lstar]*cone_.cfrag(count_exp_);
    }

    void QMCUncertaintyCalculator::enlarge_cone()
    {
        // This part is still in progress
        // this->cone_.beta *= 2.0L;
        // this->cone_.q *= 0.9L;
        
        // this->cone_.gamma *= 2.0L;
        // this->cone_.p *= 0.9L;
    }
}
