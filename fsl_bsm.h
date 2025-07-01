// fsl_bsm.h - Black-Scholes/Merton header file
#pragma once
#include "fsl_black.h"

namespace fsl {

	// Black-Scholes/Merton S_t = s0 exp((r - sigma^2/2) t + sigma B_t) where B_t is a standard Brownian motion.
	// Put value is exp(-r t) E[max{k - S_t, 0}]
	// If F = S_t then f = s0 exp(r t) and s = sigma sqrt(t).
	// Convert Black-Scholes/Merton parameters to Black model.
	inline std::tuple<double, double, double> black_bsm(double r, double s0, double sigma, double t)
	{
		if (s0 <= 0 || sigma <= 0 || t <= 0) {
			throw std::runtime_error("s0, sigma, and t must be positive");
		}
		double D = exp(-r * t); // Discount factor
		double f = s0 / D; // Forward price
		double s = sigma * sqrt(t); // Vol for Black model

		return { D, f, s };
	}
	inline int test_black_bsm()
	{
		{
			double data[][7] = {
				// r, s0, sigma, t, D, f, s
				{0.05, 100, 0.2, 1, exp(-0.05 * 1), 100 / exp(-0.05 * 1), 0.2 * sqrt(1)},
				{0.03, 50, 0.1, 2, 0.94176453358424872, 53.091827327267978, 0.14142135623730953},
				// ...more tests here...
			};
			for (auto [r, s0, sigma, t, D, f, s] : data) {
				auto [D_, f_, s_] = black_bsm(r, s0, sigma, t);
				assert(D == D_);
				assert(f == f_);
				assert(s == s_);
			}
		}
		return 0;
	}

	// exp(-r t) E[max{k - S_t, 0}] = exp(-r t) E[max{k - F, 0}]
	inline double bsm_put_value(double r, double s0, double sigma, double t, double k)
	{
		auto [D, f, s] = black_bsm(r, s0, sigma, t);
		
		return D * black_put_value(f, s, k);
	}
	inline int test_bsm_put_value()
	{
		double data[][6] = {
			// r, s0, sigma, t, k, p
			{0.05, 100, 0.2, 1, 100, 5.5735260222569671},
			// ...more tests here...
		};
		for (auto [r, s0, sigma, t, k, p] : data) {
			double p_ = bsm_put_value(r, s0, sigma, t, k);
			assert(p == p_);
		}

		return 0;
	}

	// (d/ds0) exp(-r t) E[max{k - S_t, 0}] = exp(-r t) (d/df) E[max{k - F, 0}] dF/ds0
	// dF/ds0 = exp(r t)
	// (d/ds0) exp(-r t) E[max{k - S_t, 0}] = (d/df) E[max{k - F, 0}]
	inline double bsm_put_delta(double r, double s0, double sigma, double t, double k)
	{
		auto [D, f, s] = black_bsm(r, s0, sigma, t);

		// BSM delta = Black delta
		return black_put_delta(f, s, k);
	}

	// (d/ds0) bsm_put_delta(r, s0, sigma, t, k) = (d/df) bsm_put_delta(f, s, k) dF/ds0
	inline double bsm_put_gamma(double r, double s0, double sigma, double t, double k)
	{
		auto [D, f, s] = black_bsm(r, s0, sigma, t);
		return fsl::black_put_gamma(f, s, k) / D;
	}

	// (d/dsigma) exp(-r t) E[max{k - S_t, 0}] = exp(-r t) (d/ds) E[max{k - F, 0}] ds/dsigma
	// ds/dsigma = sqrt(t)
	// (d/dsigma) exp(-r t) E[max{k - S_t, 0}] = exp(-r t) (d/ds) E[max{k - F, 0}] sqrt(t) 
	inline double bsm_put_vega(double r, double s0, double sigma, double t, double k)
	{
		auto [D, f, s] = black_bsm(r, s0, sigma, t);

		// BSM vega = D * Black vega * sqrt(t)
		return D * black_put_vega(f, s, k) * std::sqrt(t);
	}

	inline double bsm_put_implied(double r, double s0, double p, double t, double k,
		double sigma = 0.2, double eps = 1e-8, unsigned iter = 100)
	{
		auto [D, f, s] = black_bsm(r, s0, sigma, t);
		double p_D = p / D; // Forward put value

		// Use Black implied volatility, then convert back
		double implied = fsl::black_put_implied(f, p_D, k, s, eps, iter);
		return implied / sqrt(t);
	}

} // namespace fsl