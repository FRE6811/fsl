// fsl_black.h - Header file for the Fischer Black model.
#pragma once
#include <cassert>
#include <cmath>
#include <stdexcept>
#include "fsl_normal.h"

namespace fsl {

	// Factor out code independent of Excel.
	// F = f exp(sZ - s^2/2) <= k if and only if Z <= (log(k/f) + s^2/2)/s
	double black_moneyness(double f, double s, double k)
	{
		if (f <= 0 || s <= 0 || k <= 0) {
			return std::numeric_limits<double>::quiet_NaN(); // Use NaN for errors
		}

		return (std::log(k / f) + s * s / 2) / s;
	}
	int test_black_moneyness()
	{
		{
			// Test NaN return code.
			double data[][3] = {
				// f, s, k
				{-1, 1, 1},
				{1, -1, 1},
				{1, 1, -1}
			};
			for (auto [f, s, k] : data) {
				assert(std::isnan(fsl::black_moneyness(f, s, k)));
			}
		}
		{
			double data[][4] = {
				// f, s, k, z
				{100, .1, 100, 0.05000000000000001},
				{100, .1, 90, -1.0036051565782627},
				{100, .1, 110, 1.0031017980432493},
				// ...more tests here...
			};
			for (auto [f, s, k, z] : data) {
				double z_ = fsl::black_moneyness(f, s, k);
				assert(z == z_);
			}
		}

		return 0;
	}

	// E[max(k - F, 0)] = k P(Z <= z) - f P(Z + s <= z)
	double black_put_value(double f, double s, double k)
	{
		double z = fsl::black_moneyness(f, s, k);

		return k * fsl::normal_cdf(z) - f * fsl::normal_cdf(z - s);
	}
	int test_black_put_value()
	{
		{
			double data[][4] = {
				// f, s, k, p 
				{100, .1, 100, 3.9877611676744920},
				// ...more tests here...
			};
			for (auto [f, s, k, p] : data) {
				double p_ = fsl::black_put_value(f, s, k);
				assert(p == p_);
			}
		}

		return 0;
	}

	// (d/df) E[max(k - F, 0)] = E[-1(F <= k) dF/df]
	// dF/df = exp(s Z - s^2/2).
	double black_put_delta(double f, double s, double k)
	{
		double z = fsl::black_moneyness(f, s, k);

		return -fsl::normal_cdf(z - s);
	}

	int test_black_put_delta()
	{
		{
			double data[][4] = {
				// f, s, k, p 
				{100, .1, 100, -0.48006119416162754},
				// ...more tests here...
			};
			double eps = 1e-4;
			for (auto [f, s, k, p] : data) {
				double p_ = black_put_delta(f, s, k);
				assert(p == p_);

				// Symmetric difference quotient for numerical derivative.
				double dp = (black_put_value(f + eps, s, k) - black_put_value(f - eps, s, k)) / (2 * eps);
				double err = p_ - dp;
				assert(fabs(err) <= eps * eps);
			}
		}

		return 0;
	}

	// (d/ds) E[max(k - F, 0)] = f normal_pdf(z - s)
	double black_put_vega(double f, double s, double k)
	{
		double z = fsl::black_moneyness(f, s, k);

		return f * fsl::normal_pdf(z - s);
	}
	int test_black_put_vega()
	{
		{
			double data[][4] = {
				// f, s, k, v 
				{100, .1, 100, 39.844391409476401},
				// ...more tests here...
			};
			double eps = 1e-4;
			for (auto [f, s, k, v] : data) {
				double v_ = black_put_vega(f, s, k);
				assert(v == v_);
				// Symmetric difference quotient for numerical derivative.
				double dv = (black_put_value(f, s + eps, k) - black_put_value(f, s - eps, k)) / (2 * eps);
				double err = v_ - dv;
				assert(fabs(err) <= 2 * eps * eps);
			}
		}
		return 0;
	}

	// Black implied volatility for put option.
	double black_put_implied(double f, double p, double k, double s = 0.1, double eps = 1e-8, unsigned iter = 100)
	{
		do {
			// Newton-Raphson method.
			double s_ = s - (black_put_value(f, s, k) - p) / black_put_vega(f, s, k);
			if (s_ <= 0) {
				s_ = s / 2;
			}
			if (fabs(s_ - s) < eps) {
				s = s_;
				break;
			}
			s = s_;
		} while (--iter);

		return s;
	}
	int test_black_put_implied()
	{
		{
			double data[][3] = {
				{100, .2, 100},
				{100, .05, 100},
				{100, .1, 90},
				{100, .15, 110},
			};
			for (auto [f, s, k] : data) {
				double p = black_put_value(f, s, k);
				double s_ = black_put_implied(f, p, k);
				assert(fabs(s - s_) < 1e-7); // Check if implied volatility matches input
			}
		}

		return 0;
	}
}
