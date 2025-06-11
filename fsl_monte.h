// fsl_monte.h - Header file for Monte Carlo methods
#pragma once
#include <functional>
#include <initializer_list>
#include <tuple>

namespace fsl {

	// Return sample mean and variance of a variate.
	// m_n = (x_1 + ... + x_n) / n
	// s2_n = (x_1^2 + ... + x_n^2) / n
	inline std::tuple<double, double> monte(const std::function<double()>& f, int N)
	{
		double m = 0.0; // mean
		double s2 = 0.0; // variance
		for (int n = 1; n <= N; ++n) {
			double xn = f();
			m += (xn - m) / n; // Welford's method for mean
			s2 += (xn * xn - s2) / n;
		}

		return { m, s2 - m * m }; // {E[X}, Var[X] = E[X^2] - E[X]^2
	}

	constexpr std::tuple<double, double> monte(const std::initializer_list<double>& x)
	{
		double m = 0.0; // mean
		double s2 = 0.0; // variance
		int n = 1;
		for (double xn : x) {
			m += (xn - m) / n; // Welford's method for mean
			s2 += (xn * xn - s2) / n;
			++n;
		}

		return { m, s2 - m * m };
	}
	static_assert(monte({ -1, 1 }) == std::make_tuple(0, 1));
	static_assert(monte({ 1, 2, 3 }) == std::make_tuple(2, .6666666666666661));
}