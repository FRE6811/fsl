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
#include <cmath>
#include <limits> 
#include <stdexcept>
#include <span>
#include <utility>
#include <vector>

namespace fsl::pwflat {

	template<class X>
	constexpr X NaN = std::numeric_limits<X>::quiet_NaN();

	// Forward at time u.
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

	// Integral of the piecewise flat right-continuous forward curve from 0 to u.
	template<class T = double, class F = double>
	constexpr F integral(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		if (u < 0)  return NaN<F>;
		if (u == 0) return 0;
		if (n == 0) return u * _f;

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

	// discount D(u) = exponential(-int_0^u f(t) dt)
	template<class T = double, class F = double>
	constexpr F discount(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		return std::exp(-integral(u, n, t, f, _f));
	}

	// spot r(u) = (int_0^u f(t) dt)/u
	// r(u) = f(u) if u <= t[0]
	template<class T = double, class F = double>
	constexpr F spot(T u, size_t n, const T* t, const F* f, F _f = NaN<F>)
	{
		if (n == 0) return _f;

		return u <= t[0] ? f[0] : integral(u, n, t, f, _f) / u;
	}

	// Value-type curve object
	template<class T = double, class F = double>
	class curve {
		std::vector<T> t_; // time points
		std::vector<T> f_; // forward rates
		F _f; // extrapolated value
	public:
		// Construct a curve from time and forward rate arrays
		curve(size_t n, const T* t, const F* f, F _f = NaN<F>)
			: t_(t, t + n), f_(f, f + n), _f(_f) {
		}
		curve(const curve&) = default;
		curve(curve&&) = default;
		curve& operator=(const curve&) = default;
		curve& operator=(curve&&) = default;
		~curve() = default;

		size_t size() const 
		{ 
			return t_.size(); 
		}
		const T* time() const
		{
			return t_.data();
		}
		const F* rate() const
		{
			return f_.data();
		}
		// Get extrapolated value.
		F extrapolate() const
		{
			return _f;
		}
		// Set extrapolated value.
		void extrapolate(F f_)
		{
			_f = f_;
		}

		// Last point on curve.
		std::pair<T, F> back() const
		{
			return size() == 0 ? std::make_pair<T,F>(0, 0) : std::make_pair<T,F>(t_.back(), f_.back());

		}
		
		// Extend curve by (t, f).
		curve& push_back(T t, F f)
		{
			if (size() != 0 && t <= t.back()) {
				throw std::invalid_argument("Time must be increasing.");
			}
			t_.push_back(t);
			f_.push_back(f);

			return *this;
		}

		F forward(T u) const
		{
			return pwflat::forward(u, t_.size(), t_.data(), f_.data(), _f);
		}	
		F operator()(T u) const
		{
			return forward(u);
		}
		F integral(T u) const
		{
			return pwflat::integral(u, t_.size(), t_.data(), f_.data(), _f);
		}
		F discount(T u) const
		{
			return pwflat::discount(u, t_.size(), t_.data(), f_.data(), _f);
		}
		F spot(T u) const
		{
			return pwflat::spot(u, t_.size(), t_.data(), f_.data(), _f);
		}	
	};

} // namespace fsl
