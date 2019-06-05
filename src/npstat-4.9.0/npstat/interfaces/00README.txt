The code in this directory needs the Minuit function minimization package
in order to be compiled. See, in particular, http://www.cern.ch/minuit/

The following functionality is supported:

fitCompositeJohnson.hh      -- A driver for 1-d nonparametric density
                               estimation using Johnson transformations
                               to approximate uniformity.

MinuitDensityFitFcn1D.hh    -- The function to minimize for Minuit for
                               fitting 1-d densities to histograms with
                               the maximum likelihood method.

MinuitLOrPEBgCVFcn1D.hh     -- The function to minimize for Minuit in order
                               to adjust LOrPE bandwidth and poly degree
                               in semiparametric signal/background fits.

minuitFitJohnsonCurves.hh   -- Fit Johnson system of curves to histogrammed
                               data using maximum likelihood.

MinuitLocalLogisticFcn.hh   -- Minuit function to minimize for local logistic
                               regression.

minuitLocalQuantileRegression1D.hh -- High-level driver functions for performing
                               local quantile regression with single predictor.

minuitLocalRegression.hh    -- Local logistic regression using Minuit. Both
                               unbinned and histogrammed variants are supported.

MinuitSemiparametricFitFcn1D.hh -- Minuit function to minimize for fitting
                               semiparametric models (parametric signal,
                               nonparametric background).

MinuitSemiparametricFitUtils.hh -- Some utilities useful for fitting
                               semiparametric models.

MinuitQuantileRegression1DFcn.hh -- Minuit function to minimize for local
                               quantile regression with single predictor.

minuitQuantileRegression.hh -- High-level driver functions for performing local
                               quantile regression with multiple predictors.

MinuitQuantileRegressionNDFcn.hh -- Minuit function to minimize for local
                               quantile regression with multiple predictors.

MinuitUnbinnedFitFcn1D.hh   -- The function to minimize for Minuit for
                               fitting 1-d densities to unbinned samples
                               with the maximum likelihood method.

ScalableDensityConstructor1D.hh  -- Constructor of location/scale densities
                               for use with Minuit fitting functions.

weightedLocalQuantileRegression1D.hh -- High-level driver functions for
                               performing local quantile regression with single
                               predictor on weighted points.
