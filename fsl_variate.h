// fsl_variate.h - Header file for random variate generation
#pragma once
#include <cmath>
#include <tuple>
#include <functional>
#include <random>

namespace fsl
{
	// Return mean and standard deviation of a variate.
	std::tuple<double, double> monte(const std::function<double()>& f, int N)
	{
		double m = 0.0; // mean
		double s2 = 0.0; // variance
		for (int n = 1; n <= N; ++n) {
			double xn = f();
			m += (xn - m) / n; // Welford's method for mean
			s2 += (xn * xn - s2) / n;
		}

		return { m, std::sqrt(s2 - m * m) };

	}
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
				auto [m, s] = monte([&]() { return 1*(nd(dre) <= z); }, 1'000'000);
				//double err = (cdf - m) / s;
				assert(std::abs(cdf - m) < 0.001); 
			}
		}

		return 0;
	}

	// NVI (Non-Virtual Interface) idiom for variate generation.
	struct variate {
		~variate() = default;
		// Cumulative share distribution.
		double cdf(double x, double s) const
		{
			return cdf_(x, s);
		}
		// Share density function (PDF) for the variate.
		double pdf(double x, double s) const
		{
			return pdf_(x, s);
		}
		// E[exp(s X)] - Moment generating function.
		double mgf(double s) const
		{
			return mgf_(s); // Moment generating function	
		}
		// log(E[exp(s X)]) - Cumulant generating function.
		double cgf(double s) const
		{
			return cgf_(s); // Cumulant generating function
		}	
	private:
		virtual double cdf_(double x, double s) const = 0; // pure virtual function
		virtual double pdf_(double x, double s) const = 0; // pure virtual function
		virtual double mgf_(double s) const = 0; // moment generating function
		virtual double cgf_(double s) const = 0; // cumulant generating function
	};

	struct normal : public variate {
		double cdf_(double x, double s) const override
		{
			return normal_cdf(x / s); // TODO: fix
		}

	};
}
