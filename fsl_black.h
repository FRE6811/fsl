// fsl_black.h - Header file for the Black model.
#pragma once
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace fsl {

	// Factor out code independent of Excel.
	// F = f exp(sZ - s^2/2) <= k if and only if Z <= (log(k/f) + s^2/2)/s
	double black_moneyness(double f, double s, double k)
	{
		if (f <= 0 || s <= 0 || k <= 0) {
			// Use C++ exceptions for error handling.
			throw std::runtime_error("f, s, and k must be positive");
		}

		return (std::log(k / f) + s * s / 2) / s;
	}
	int test_black_moneyness()
	{
		{
			// Test exception.
			double data[][3] = {
				// f, s, k
				{-1, 1, 1},
				{1, -1, 1},
				{1, 1, -1}
			};
			for (auto [f, s, k] : data) {
				bool thrown = false;
				try {
					fsl::black_moneyness(f, s, k);
				}
				catch (const std::exception&) {
					thrown = true;
				}
				assert(thrown);
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

	// TODO: implement Black put vega.
	// (d/ds) E[max(k - F, 0)] = f normal_pdf(z - s)
	// TODO: int test_black_put_vega();
}
