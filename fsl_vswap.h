// fsl_vswap.h - Variance swap
// https://keithalewis.github.io/math/vswap.html
// 
// Underlying X(t). Let X_j = X(t_j), t_0 < ... < t_n
// Payoff at t_n is σ^2 = 1/(t_n - t_0) sum_0 <= j < n (ΔX_j/X_j)^2
// 
// f(X_n) - f(X_0) = sum_0 <= j < n f(X_{j+1}) - f(X_j)
//                 = sum_0 <= j < n f'(X_j) ΔX_j + f''(X_j) ΔX_j^2/2 + O(ΔX_j^3)
// Let f''(x) = 2/x^2 so f'(x) = -2/x + c and f(x) = -2 log(x) +  cx
// sum_0 <= j < n (ΔX_j/X_j)^2 = -2 log(X_n/X_0) + 2 (X_n - X_0)/z + sum_0 <= j < n (-2/X_j) ΔX_j if c = 2/z.
//                              <static hedge is European option>  <dynamic hedge in futures>
// Carr-Madan: 
// f(x) = f(z) + f'(z) (x - z) + int_0^z f''(k) p(k) dk + int_z^infty f''(k) c(k) dk
// where p(k) = max{k - x, 0} and c(k) = max{x - k, 0}.
//
// Given strikes k[0], ..., k[n-1] use piecewise linear interpolation
// to get slopes f'[i] = (f[i+1] - f[i])/(k[i+1] - k[i]) over [k[i], k[i + 1]]
// and second derivatives f''[i] = f'[i] - f'[i - 1] at k[1], ... k[n - 2].
// No put at k[0] and no call at k[n-1] required.

#pragma once
#ifdef _DEBUG
#include <cassert>
#endif // _DEBUG
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>
#include <span>
#include "fsl_math.h"

namespace fsl {

	// (f1 - f0)/(k1 - k0), (f2 - f1)/(k2 - k1), ..., 
	template<class X = double>
	constexpr X* difference_quotient(size_t n, const X* k, X* f)
	{
		if (n >= 2) {
			for (size_t i = 0; i < n - 1; ++i) {
				f[i] = (f[i + 1] - f[i]) / (k[i + 1] - k[i]);
			}
		}

		return f;
	}
#ifdef _DEBUG
	inline void test_difference_quotient()
	{
		double f[] = { 1, 2, 3, 4 };
		const double k[] = { 1, 2, 3, 4 };
		auto result = difference_quotient(4, k, f);
		assert(result[0] == 1.0);
		assert(result[1] == 1.0);
		assert(result[2] == 1.0);
	}
#endif // _DEBUG

	// Put and call prices that are NaN or 0 are ignored.
	template<class X, class P>
	constexpr bool P_or_NaN(X a, X b)
	{
		if (is_nan(a) || a == 0 || is_nan(b) || b == 0) {
			return true;
		}

		return P()(a, b);
	}
#ifdef _DEBUG
	static_assert(P_or_NaN<double, std::less<double>>(1, 2));
	static_assert(P_or_NaN<double, std::greater<double>>(2, 1));
	static_assert(P_or_NaN<double, std::less<double>>(NaN<double>, 2));
	static_assert(P_or_NaN<double, std::greater<double>>(2, NaN<double>));
	static_assert(P_or_NaN<double, std::less<double>>(0, 2));
	static_assert(P_or_NaN<double, std::greater<double>>(2, 0));
	static_assert(P_or_NaN<double, std::equal_to<double>>(2, 2));
#endif // _DEBUG
	template<class X = double>
	constexpr bool is_increasing(const X* b, const X* e)
	{
		return std::is_sorted(b, e, P_or_NaN<X, std::less<X>>);
	}
	template<class X = double>
	constexpr bool is_decreasing(const X* b, const X* e)
	{
		return std::is_sorted(b, e, P_or_NaN<X, std::greater<X>>);
	}

	template<class X = double>
	inline X static_payoff(X x0, X z, X x)
	{
		return -2 * std::log(x / x0) + 2 * (x - x0) / z;
	}

	// {*, f''(k[1]), f''(k[2]), ..., f''(k[n-2], *)}
	template<class X = double>
	inline X* vswap_weights(X x0, X z, size_t n, const X* k, X* w)
	{
		if (n < 2 || k == nullptr || w == nullptr) {
			return nullptr;
		}

		for (size_t i = 0; i < n; ++i) {
			w[i] = static_payoff(x0, z, k[i]); // w[i] = f(k[i])
		}
		if (!difference_quotient(n, k, w)) { // w[i] = (f(k[i + 1]) - f(k[i]))/(k[i + 1] - k[i])
			return nullptr;
		}

		// Last difference quotient does not exist so n - 1 size.
		std::adjacent_difference(w, w + n - 1, w); // w[i] = Δ(fi/ki)

		// f''(k) = 2/k^2 so should be decreasing and positive
		
		if (!std::is_sorted(&w[1], &w[n - 1], std::greater<X>())) {
			return nullptr;
		}
		if (!std::all_of(&w[1], &w[n - 1], [](X x) { return x > 0; })) {
			return nullptr;
		}

		return w;
	}

	// Par variance is σ_0^2 = E[σ^2]
	// given dt = tn - t0, x0 = X(0), z = put/call seperator,
	// strikes k, put price p, and call prices c.
	template<class X = double>
	inline double par_variance(X dt, X x0, X z, size_t n, const X* k, const X* p, const X* c)
	{
		if (n < 2 || k == nullptr || p == nullptr || c == nullptr) {
			return NaN<X>;
		}
		if (!std::is_sorted(k, k + n)) return NaN<X>;
		if (!is_increasing(p, p + n)) return NaN<X>;
		if (!is_decreasing(c, c + n)) return NaN<X>;

		std::vector<X> w(n);
		// +1 to line up with strikes
		if (!vswap_weights(x0, z, n, k, w.data())) {
			return NaN<X>;
		}
	
		X s2 = 0.0; // par variance
		size_t i = 1;
		// puts
		while (i < n - 1 && k[i] < z) {
			s2 += w[i] * p[i];
			++i;
		}
		if (i == n - 1) {
			return NaN<X>; // no calls
		}
		// last put/first call
		X ki_ = k[i - 1];
		X fi_ = static_payoff(x0, z, ki_);
		X ki = k[i];
		X fi = static_payoff(x0, z, ki);
		X m = 0; // TODO: slope at z	
		// Add value of linear interpolation through (ki_, fi_) and (ki, fi) at z.
		s2 += 0; // TODO: Fix Copilot generated "(m * (ki - ki_) + fi - fi_) * w[i]; // slope at z"
		// calls
		while (i < n - 1) {
			s2 += w[i] * c[i];
			++i;
		}

		return s2 / dt;
	}

} // namespace fsl
