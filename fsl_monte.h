// fsl_monte.h - Header file for Monte Carlo methods
#pragma once
#include <functional>
#include <initializer_list>
#include <tuple>

namespace fsl {

	// Update mean and variance given a sample.
	constexpr std::pair<double, double> monte_step(double x, int n, double m, double s2)
	{
		return { m + (x - m) / n, s2 + (x * x - s2) / n };
	}

	// Return sample mean and variance of a variate.
	// m_n = (x_1 + ... + x_n) / n
	// s2_n = (x_1^2 + ... + x_n^2) / n
	inline std::pair<double, double> monte(const std::function<double()>& f, int N)
	{
		double m = 0.0; // mean
		double s2 = 0.0; // variance
		for (int n = 1; n <= N; ++n) {
			std::tie(m, s2) = monte_step(f(), n, m, s2);
		}

		return { m, s2 - m * m }; // {E[X}, Var[X] = E[X^2] - E[X]^2
	}

	constexpr std::pair<double, double> monte(const std::initializer_list<double>& x)
	{
		double m = 0.0; // mean
		double s2 = 0.0; // variance
		int n = 1;
		for (double xn : x) {
			std::tie(m, s2) = monte_step(xn, n, m, s2);
			++n;
		}

		return { m, s2 - m * m };
	}
#ifdef _DEBUG
	static_assert(monte({ -1, 1 }) == std::make_tuple(0, 1));
	static_assert(monte({ 1, 2, 3 }) == std::make_tuple(2, .6666666666666661));
#endif // _DEBUG
}