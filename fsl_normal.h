// fsl_variate.h - Header file for random variate generation
#pragma once
#include <cassert>
#include <cmath>
#include <numbers>
#include <random>
#include "fsl_monte.h"

namespace fsl
{
	// Standard normal cumulative distribution function P(Z <= z).
	// https://en.wikipedia.org/wiki/Error_function#Cumulative_distribution_function
	double normal_cdf(double z)
	{
		// Cumulative distribution function for the standard normal distribution
		return 0.5 * (1 + std::erf(z / std::sqrt(2)));
	}
	int test_normal_cdf()
	{
		{
			std::default_random_engine dre;
			std::normal_distribution<double> nd(0, 1); // mean 0, stddev 1
			double zs[] = { -2, -1, 0, 1, 2 };
			for (double z : zs) {
				double cdf = normal_cdf(z);
				auto [m, s2] = monte([&]() { return 1*(nd(dre) <= z); }, 1'000'000);
				double err = (cdf - m) / std::sqrt(s2);
				assert(std::abs(cdf - m) < 0.001); 
			}
		}

		return 0;
	}

	// Standard normal probability density function.
	double normal_pdf(double z)
	{
		return std::exp(-0.5 * z * z) / std::sqrt(2 * std::numbers::pi);
	}
	int test_normal_pdf()
	{
		{
			assert(normal_pdf(0) == 1 / std::sqrt(2 * std::numbers::pi));
		}

		return 0;
	}
}
