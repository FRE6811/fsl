// fsl_pwflat.h - piecewise flat right-continuous forward curve determined by points (t[i], f[i]) and extrapolated value _f.
/*
The _discount_ D(t) is the price of a zero coupon bond paying 1 unit at time t.
The _continuously compounded forward_ rate f(t) is defined by D(t) = exp(-int_0^t f(s) ds).
The _continuously compounded spot_ rate r(t) is defined by D(t) = exp(-t r(t)).
Note the spot r(t) = (1/t) int_0^t f(s) ds is the average of the foward rate.
Taking a derivative with respect to t gives f(t) = r(t) + t r'(t).
Discount is represented using a piecewise flat right-continuous forward curve.

		   { f[i] if t[i-1] < t <= t[i];
	f(t) = { _f   if t > t[n-1];
		   { NaN  if t < 0
	F                                  _f
	|       f[1]             f[n-1]  o--------
	| f[0] o-----           o--------x
	x------x      ... ------x
	|
	0------|----- ... ------|--------|--- T
		 t[0]            t[n-2]   t[n-1]

	Note f(t[i]) = f[i].
*/
#include <cassert>
#include <cmath>
#include <limits> 
#include <stdexcept>
#include <span>
#include <utility>
#include <vector>
#include "fsl_math.h"

namespace fsl::pwflat {

	// Forward at time u. Assumes t entries are increasing.
	template<class T = double, class F = double> // time, forward rate
	constexpr F forward(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		if (u < 0) {
			return NaN<F>;
		}
		if (n == 0) 
			return _f;

		auto ti = std::lower_bound(t, t + n, u); // greatest i with t[i] <= u

		return ti == t + n ? _f : f[ti - t];
	}
#ifdef _DEBUG
	constexpr void test_forward() {
		static_assert(is_nan(forward<double, double>(0, 0, nullptr, nullptr)));

		constexpr double t[] = { 1, 2, 3 };
		constexpr double f[] = { .1, .2, .3 };
		static_assert(forward<double, double>(0, 3, t, f) == .1);
		static_assert(forward<double, double>(1, 3, t, f) == .1);
		static_assert(forward<double, double>(1.1, 3, t, f) == .2);
		static_assert(forward<double, double>(2, 3, t, f) == .2);
		static_assert(forward<double, double>(3, 3, t, f) == .3);
		static_assert(is_nan(forward<double, double>(3.1, 3, t, f)));
		static_assert(is_nan(forward<double, double>(-1, 3, t, f)));
	}
#endif

	// Integral of the piecewise flat right-continuous forward curve from 0 to u.
	template<class T = double, class F = double>
	constexpr F integral(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		if (u < 0)  return NaN<F>;
		if (u == 0) return 0;
		if (n == 0 || t == nullptr || f == nullptr) return u * _f;

		F I = 0;
		T t_ = 0;

		size_t i;
		for (i = 0; i < n && t[i] <= u; ++i) {
			I += f[i] * (t[i] - t_);
			t_ = t[i];
		}
		if (u > t_) {
			I += (i == n ? _f : f[i]) * (u - t_);
		}

		return I;
	}
#ifdef _DEBUG
	constexpr void test_integral() {
		static_assert(integral<double, double>(0, 0, nullptr, nullptr) == 0);
		static_assert(is_nan(integral<double, double>(1, 0, nullptr, nullptr)));
		static_assert(is_nan(integral<double, double>(-1, 0, nullptr, nullptr)));

		constexpr double t[] = { 1, 2, 3 };
		constexpr double f[] = { .1, .2, .3 };
		static_assert(integral<double, double>(0, 3, t, f) == 0);
		static_assert(integral<double, double>(1, 3, t, f) == .1);
		static_assert(integral<double, double>(1.5, 3, t, f) == .2);
		static_assert(integral<double, double>(2, 3, t, f) == .1 + .2);
		static_assert(integral<double, double>(3, 3, t, f) == .1 + .2 + .3);
		static_assert(is_nan(forward<double, double>(3.1, 3, t, f)));
	}
#endif

	// discount D(u) = exponential(-int_0^u f(t) dt)
	template<class T = double, class F = double>
	constexpr F discount(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		return exp(-integral(u, n, t, f, _f));
	}

	// spot r(u) = (int_0^u f(t) dt)/u
	// r(u) = f(u) if u <= t[0]
	template<class T = double, class F = double>
	constexpr F spot(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		if (n == 0 || t == nullptr || f == nullptr) return _f;

		return u <= t[0] ? f[0] : integral(u, n, t, f, _f) / u;
	}

	// Non-owning view of curve
	template<class T = double, class F = double>
	class curve_view {
	protected:
		size_t n_; // number of points
		const T* t_; // time points
		const F* f_; // forward rates
		F _f; // extrapolated value
	public:
		constexpr curve_view() = default;
		constexpr curve_view(const curve_view&) = default;
		constexpr curve_view& operator=(const curve_view&) = default;
		constexpr ~curve_view() = default;
		// constant curve
		constexpr curve_view(F _f)
			: n_(0), t_(nullptr), f_(nullptr), _f(_f) 
		{ }
		// Construct a curve_view from time and forward rate arrays
		constexpr curve_view(size_t n, const T* t, const F* f, F _f = NaN<F>)
			: n_(n), t_(t), f_(f), _f(_f) 
		{ }

		constexpr size_t size() const
		{
			return n_;
		}
		constexpr const T* time() const
		{
			return t_;
		}
		constexpr const F* rate() const
		{
			return f_;
		}
		constexpr F extrapolate() const
		{
			return _f;
		}

		// Last point on curve_view.
		constexpr std::pair<T, F> back() const
		{
			if (size() == 0) {
				return { 0, 0 };
			}

			return { t_[n_ - 1], f_[n_ - 1] };

		}

		constexpr F forward(T u) const
		{
			return pwflat::forward(u, n_, t_, f_, _f);
		}
		constexpr F operator()(T u) const
		{
			return forward(u);
		}
		constexpr F integral(T u) const
		{
			return pwflat::integral(u, n_, t_, f_, _f);
		}
		constexpr F discount(T u) const
		{
			return pwflat::discount(u, n_, t_, f_, _f);
		}
		constexpr F spot(T u) const
		{
			return pwflat::spot(u, n_, t_, f_, _f);
		}
	};

	// Curve view with new extrapolated value.
	template<class T = double, class F = double>
	constexpr curve_view<T, F> extrapolate(curve_view<T,F> f, F _f)
	{
		return curve_view<T, F>(f.size(), f.time(), f.rate(), _f);
	}
#ifdef _DEBUG
	constexpr void test_curve_view()
	{
		constexpr double t[] = { 1, 2, 3 };
		constexpr double f[] = { .1, .2, .3 };
		static_assert(curve_view(3, t, f, 0.4).size() == 3);
		static_assert(curve_view(3, t, f, 0.4).size() == 3);
		static_assert(curve_view(3, t, f, 0.4).time() == t);
		static_assert(curve_view(3, t, f, 0.4).rate() == f);
		static_assert(curve_view(3, t, f, 0.4).extrapolate() == 0.4);
		static_assert(curve_view(3, t, f, 0.4).forward(0) == .1);
		static_assert(curve_view(3, t, f, 0.4).forward(1) == .1);
		static_assert(curve_view(3, t, f, 0.4).spot(1) == .1);
		static_assert(curve_view(3, t, f, 0.4).forward(1.5) == .2);
		static_assert(curve_view(3, t, f, 0.4).spot(1.5) != .2);
		static_assert(curve_view(3, t, f, 0.4).forward(2) == .2);
		static_assert(curve_view(3, t, f, 0.4).forward(3) == .3);
		static_assert(is_nan(curve_view(3, t, f).forward(3.1)));
	}	
#endif // _DEBUG

	// Value-type curve object
	template<class T = double, class F = double>
	class curve : public curve_view<T,F> {
		std::vector<T> t; // time points
		std::vector<F> f; // forward rates
	public:

		constexpr curve(F _f = NaN<F>)
			: curve_view<T,F>(0, nullptr, nullptr, _f), t{}, f{}
		{
		}
		// Construct a curve from time and forward rate arrays
		constexpr curve(size_t n, const T* t, const F* f, F _f = NaN<F>)
			: curve_view<T,F>(_f), t(t, t + n), f(f, f + n)
		{
			curve_view<T, F>::n_ = n;
			curve_view<T, F>::t_ = t;
			curve_view<T, F>::f_ = f;
		}
		curve(curve&&) = default;
		curve& operator=(const curve&) = default;
		curve& operator=(curve&&) = default;
		~curve() = default;

		// Equal values.
		constexpr bool operator==(const curve& c) const
		{
			using curve_view<T, F>::curve_view::extrapolate;
			F e = extrapolate();
			F ce = c.extrapolate();

			return t == c.t && f == c.f && ((is_nan(e) && is_nan(ce)) || e == ce);
		} 
		
		// Extend curve by (t, f).
		constexpr curve& push_back(T t, F f)
		{
			if (t <= curve_view<T,F>::back().first) {
				throw std::invalid_argument("Time must be increasing.");
			}
			this->t.push_back(t);
			this->f.push_back(f);

			return *this;
		}
		constexpr curve& push_back(std::pair<T, F> tf)
		{
			return push_back(tf.first, tf.second);
		}
	};
#ifdef _DEBUG
	constexpr void test_curve()
	{
		//constexpr double t[] = { 1, 2, 3 };
		//constexpr double f[] = { .1, .2, .3 };
		//constexpr fsl::pwflat::curve<double,double> c(3, t, f, 0.4);
		//static_assert(c.size() == 3);
	}
#endif // _DEBUG

} // namespace fsl
