// fmx_root1d.h - 1-d root using secant method
#pragma once
#include <tuple>
#include <limits>
#include "fsl_math.h"

namespace fsl::root1d {
	
	// Move x to bracketed root in [a, b] given last bracketed guess x0.
	template<class X>
	constexpr X bracket(X x, X x0, X a = -infinity<X>, X b = infinity<X>)
	{
		if (a >= b || a >= x0 || x0 >= b) {
			return NaN<X>;
		}

		if (x < a) {
			return (x0 + a) / 2;
		}
		else if (x > b) {
			return (x0 + b) / 2;
		}

		return x;
	}
#ifdef _DEBUG
	static_assert(bracket<double>(1, 1) == 1);
	static_assert(bracket<double>(2, 1) == 2);
	static_assert(bracket<double>(1, 3, 2, 4) == 2.5);
	static_assert(bracket<double>(5, 3, 2, 4) == 3.5);
	//static_assert(fsl::isnan(bracket<double>(1, 3, 4, 2)));
	//static_assert(fsl::isnan(bracket<double>(1, 1, 2, 4)));
	//static_assert(fsl::isnan(bracket<double>(1, 5, 2, 4)));
#endif // _DEBUG

	template<class X, class Y = X>
	struct secant {
		X x0, x1;
		X tolerance;
		size_t iterations;

		secant(X x0, X x1, X tol = sqrt_epsilon<X>, size_t iter = 100)
			: x0(x0), x1(x1), tolerance(tol), iterations(iter)
		{
		}

		constexpr auto next(X x0_, Y y0, X x1_, Y y1)
		{
			return (x0_ * y1 - x1_ * y0) / (y1 - y0);
		}

		// Find root given two initial guesses.
		// Return root approximation, tolerance, and number of iterations.
		template<class F>
		constexpr std::tuple<X, X, size_t> solve(const F& f)
		{
			Y y0 = f(x0);
			Y y1 = f(x1);
			bool bounded = !samesign(y0, y1);
			size_t n = 0;

			while (++n < iterations && fabs(y1) > tolerance) {
				auto x = next(x0, y0, x1, y1);
				Y y = f(x);
				if (bounded && samesign(y, y1)) {
					x1 = x;
					y1 = y;
				}
				else {
					x0 = x1;
					y0 = y1;
					x1 = x;
					y1 = y;
					bounded = !samesign(y0, y1);
				}
			}
			if (n == iterations) {
				x1 = NaN<X>;
			}

			return { x1, y1, n };
		}
#ifdef _DEBUG
		constexpr int solve_test()
		{
			{
				constexpr double x = std::get<0>(solve([](double x) { return x * x - 4; }, 0., 1.));
				static_assert(fabs(x - 2) <= sqrt_epsilon<double>);
			}

			return 0;
		}
#endif // _DEBUG
	};

	// Find root given initial guess and derivative using Newton's method.
	// Return root approximation, tolerance, and number of iterations.
	template<class X = double, class Y = double>
	struct newton {
		X x0;
		X tolerance;
		size_t iterations = 100;

		newton(X x0, X tol = sqrt_epsilon<X>, size_t iter = 100)
			: x0(x0), tolerance(tol), iterations(iter)
		{
		}

		constexpr auto next(X x, Y y, decltype(y / x) df)
		{
			return x - y / df;
		}

		// Find positive root in [a, b] given initial guess and derivative.
		template<class F, class dF>
		constexpr std::tuple<X, X, size_t> solve(const F& f, const dF& df, X a = -infinity<X>, X b = infinity<X>)
		{
			auto y0 = f(x0);
			size_t n = 0;
			while (++n < iterations && fabs(y0) > tolerance) {
				auto x = next(x0, y0, df(x0));
				x0 = bracket(x, x0, a, b);
				y0 = f(x0);
			}
			if (n == iterations) {
				x0 = NaN<X>;
			}

			return { x0, y0, n };
		}
#ifdef _DEBUG
		constexpr int solve_test()
		{
			{
				constexpr double x = std::get<0>(solve([](double x) { return x * x - 4; }, [](double x) { return 2 * x; }, 1.));
				static_assert(fabs(x - 2) < sqrt_epsilon<double>);
			}

			return 0;
		}
#endif // _DEBUG	
	};

} // namespace fms::secant

