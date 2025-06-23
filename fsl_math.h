// fsl_math.h - Constexpr math functions
#pragma once

namespace fsl {
	template<class X>
	constexpr X NaN = std::numeric_limits<X>::quiet_NaN();
	template<class X>
	constexpr X epsilon = std::numeric_limits<X>::epsilon();
	template<class X>
	constexpr X infinity = std::numeric_limits<X>::infinity();

	template<class X>
	constexpr bool is_nan(X x)
	{
		return x != x;
	}

	template<class X>
	constexpr X ldexp(X x, int exp)
	{
		if (exp < 0) {
			while (exp++) {
				x /= X(2);
			}
		}
		else if (exp > 0) {
			while (exp--) {
				x *= X(2);
			}
		}

		return x;
	}
#ifdef _DEBUG
	constexpr void test_ldexp()
	{
		static_assert(ldexp<double>(1, 0) == 1);
		static_assert(ldexp<double>(1, 2) == 4);
		static_assert(ldexp<double>(1, -2) == 0.25);
	}
#endif // _DEBUG

	template<class X>
	constexpr X sqrt_epsilon = ldexp(1., -std::numeric_limits<X>::digits / 2);

	// Sign of x.
	template<class X>
	constexpr X sgn(X x)
	{
		return x > 0 ? X(1) : x < 0 ? X(-1) : X(0);
	}
#ifdef _DEBUG
	static_assert(sgn(-2) == -1);
	static_assert(sgn(0) == 0);
	static_assert(sgn(2) == 1);
#endif // _DEBUG

	template<class X>
	constexpr bool samesign(X x, X y)
	{
		//return x = std::copysign(x, y);
		return sgn(x) == sgn(y);
	}
#ifdef _DEBUG
	static_assert(samesign(-2, -3));
	static_assert(samesign(0, 0));
	static_assert(samesign(2, 3));
	static_assert(!samesign(-2, 3));
	static_assert(!samesign(2, -3));
#endif // _DEBUG

	template<class X>
	constexpr X fabs(X x)
	{
		return x >= 0 ? x : -x;
	}
#ifdef _DEBUG
	static_assert(fabs(-1) == 1);
	static_assert(fabs(0) == 0);
	static_assert(fabs(1) == 1);
#endif // _DEBUG

	template<class X>
	constexpr X exp(X x, X epsilon = sqrt_epsilon<X>) {
		X ex = 1.0;
		X xn = 1.0;
		int n = 1;

		while (fabs(xn) > epsilon) {
			xn *= x / n;
			ex += xn;
			++n;
		}

		return ex;
	}
#ifdef _DEBUG
	constexpr void test_exp()
	{
		static_assert(exp<double>(0) == 1);
		static_assert(fabs(exp<double>(1) - 2.718281828459045) < sqrt_epsilon<double>);
		static_assert(fabs(exp<double>(-1) - 0.3678794411714423) < sqrt_epsilon<double>);
	}
#endif // _DEBUG

} // namespace fsl
