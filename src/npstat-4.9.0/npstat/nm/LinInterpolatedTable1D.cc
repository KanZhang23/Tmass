#include "npstat/nm/LinInterpolatedTable1D.hh"
#include "npstat/nm/isMonotonous.hh"

#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

namespace npstat {
    LinInterpolatedTable1D::LinInterpolatedTable1D()
        : data_(2, 0.0),
          xmin_(0.0),
          xmax_(1.0),
          binwidth_(1.0),
          npoints_(2U),
          leftExtrapolationLinear_(false),
          rightExtrapolationLinear_(false),
          monotonous_(false),
          monotonicityKnown_(true)
    {
    }

    LinInterpolatedTable1D::LinInterpolatedTable1D(const double c)
        : data_(2, c),
          xmin_(0.0),
          xmax_(1.0),
          binwidth_(1.0),
          npoints_(2U),
          leftExtrapolationLinear_(false),
          rightExtrapolationLinear_(false),
          monotonous_(false),
          monotonicityKnown_(true)
    {
    }

    bool LinInterpolatedTable1D::operator==(
        const LinInterpolatedTable1D& r) const
    {
        return xmin_ == r.xmin_ &&
               xmax_ == r.xmax_ &&
               binwidth_ == r.binwidth_ &&
               npoints_ == r.npoints_ &&
               leftExtrapolationLinear_ == r.leftExtrapolationLinear_ &&
               rightExtrapolationLinear_ == r.rightExtrapolationLinear_ &&
               data_ == r.data_;
    }

    bool LinInterpolatedTable1D::isMonotonous() const
    {
        if (!monotonicityKnown_)
        {
            monotonous_ = isStrictlyMonotonous(data_.begin(), data_.end());
            monotonicityKnown_ = true;
        }
        return monotonous_;
    }

    CPP11_auto_ptr<LinInterpolatedTable1D> LinInterpolatedTable1D::inverse(
        const unsigned npoints, const bool leftExtrapolationLinear,
        const bool rightExtrapolationLinear) const
    {
        if (!isMonotonous())
            return CPP11_auto_ptr<LinInterpolatedTable1D>(
                (LinInterpolatedTable1D*)0);

        std::vector<std::pair<double,double> > points;
        points.reserve(npoints_);

        if (data_[npoints_ - 1U] > data_[0])
        {
            points.push_back(std::pair<double,double>(data_[0], xmin_));
            for (unsigned i=1; i<npoints_ - 1U; ++i)
                points.push_back(std::pair<double,double>(data_[i], xmin_+i*binwidth_));
            points.push_back(std::pair<double,double>(data_[npoints_ - 1U], xmax_));
        }
        else
        {
            points.push_back(std::pair<double,double>(data_[npoints_ - 1U], xmax_));
            for (unsigned i=npoints_ - 2U; i>0; --i)
                points.push_back(std::pair<double,double>(data_[i], xmin_+i*binwidth_));
            points.push_back(std::pair<double,double>(data_[0], xmin_));
        }

        return CPP11_auto_ptr<LinInterpolatedTable1D>(
            new LinInterpolatedTable1D(points, npoints,
                                       leftExtrapolationLinear,
                                       rightExtrapolationLinear));
    }

    double LinInterpolatedTable1D::operator()(const double& x) const
    {
        if (x <= xmin_)
        {
            if (leftExtrapolationLinear_)
                return data_[0] + (data_[1]-data_[0])*((x-xmin_)/binwidth_);
            else
                return data_[0];
        }
        else if (x >= xmax_)
        {
            if (rightExtrapolationLinear_)
                return data_[npoints_ - 1U] - (
                  data_[npoints_-2U]-data_[npoints_-1U])*((x-xmax_)/binwidth_);
            else
                return data_[npoints_ - 1U];
        }
        else
        {
            const unsigned ux = static_cast<unsigned>((x - xmin_)/binwidth_);
            if (ux >= npoints_ - 1U)
                return data_[npoints_ - 1U];
            const double delta = x - (ux*binwidth_ + xmin_);
            return data_[ux] + (data_[ux+1U]-data_[ux])*delta/binwidth_;
        }
    }

    bool LinInterpolatedTable1D::write(std::ostream& of) const
    {
        assert(data_.size() == npoints_);
        gs::write_pod_vector(of, data_);
        gs::write_pod(of, xmin_);
        gs::write_pod(of, xmax_);
        gs::write_pod(of, leftExtrapolationLinear_);
        gs::write_pod(of, rightExtrapolationLinear_);
        return !of.fail();
    }

    LinInterpolatedTable1D* LinInterpolatedTable1D::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<LinInterpolatedTable1D>());
        current.ensureSameId(id);

        std::vector<double> d;
        double xmin;
        double xmax;
        bool left, right;

        gs::read_pod_vector(in, &d);
        gs::read_pod(in, &xmin);
        gs::read_pod(in, &xmax);
        gs::read_pod(in, &left);
        gs::read_pod(in, &right);
        if (in.fail())
            throw gs::IOReadFailure("In npstat::LinInterpolatedTable1D::read:"
                                    " input stream failure");
        return new LinInterpolatedTable1D(&d.at(0), d.size(),
                                          xmin, xmax, left, right);
    }
}
