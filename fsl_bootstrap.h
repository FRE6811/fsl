// fsl_bootstrap.h - Bootstrap a piecewise flat forward curve.
#pragma once
#include "fsl_pwflat.h"

namespace fsl {

	template<class U = double, class C = double>
	using cash_flow = std::pair<U, C>;

	template<class U = double, class C = double>
	using instrument = std::vector<cash_flow<U, C>>;

	template<class U = double, class C = double, class T = double, class F = double>
	constexpr C present_value(const instrument<U, C>& uc, const pwflat::curve<T, F>& D)
	{
		C pv = 0;

		for (const auto& [u, c] : uc) {
			pv += c * D.discount(u);
		}
		
		return pv;
	}

	// Derivative of present value with respect to forward rate.
	template<class U = double, class C = double, class T = double, class F = double>
	constexpr C duration(const instrument<U, C>& uc, const pwflat::curve<T, F>& f)
	{
		C dur = 0;

		for (const auto& [u, c] : uc) {
			dur += u * c * f.discount(u);
		}

		return dur;
	}

	// Bootstrap a piecewise flat forward curve from an instrument and price.
	template<class U = double, class C = double, class T = double, class F = double>
	std::pair<T, F> bootstrap1(const instrument<U, C>& uc, pwflat::curve<T, F>& f, F p = 0, F eps = 1e-8, size_t iter = 100)
	{
		if (uc.empty()) {
			throw std::runtime_error("Instrument cash flow is empty");
		}
		auto [u_, c_] = uc.back(); // Last cash flow.
		auto [t_, f_] = f.back(); // Last point on curve.

		if (u_ <= t_) {
			throw std::runtime_error("Last cash flow must be past end of curve");
		}

		f.extrapolate(f_);
		while (iter-- && std::fabs(present_value(uc, f) - p) > eps) {
			f_ = f_ - (present_value(uc, f) - p) / duration(uc, f);
			f.extrapolate(f_);
		}
		
		return { t_, f_ };
	}

} // namespace fsl