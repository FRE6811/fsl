// fsl_bootstrap.h - Bootstrap a piecewise flat forward curve.
#pragma once
#include "fsl_instrument.h"
#include "fsl_pwflat.h"

namespace fsl {

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

	// Bootstrap a piecewise flat forward curve from an instrument with price 0.
	template<class U = double, class C = double, class T = double, class F = double>
	std::pair<T, F> bootstrap0(const instrument<U, C>& uc, pwflat::curve<T, F>& f, C eps = 1e-8, size_t iter = 100)
	{
		if (uc.empty()) {
			throw std::runtime_error("Instrument cash flows are empty");
		}
		auto [u_, c_] = uc.back(); // Last cash flow.
		auto [t_, f_] = f.back(); // Last point on curve.

		if (u_ <= t_) {
			throw std::runtime_error("Last cash flow must be past end of curve");
		}

		f.extrapolate(f_);
		while (iter-- && std::fabs(present_value(uc, f)) > eps) {
			f_ = f_ - present_value(uc, f) / duration(uc, f);
			f.extrapolate(f_);
		}
		
		return { u_, f_ };
	}

	// TODO: bootstrap1 cash deposit (one cash flow)
	// f = -log((p - pn)/c D(tn))/(u - tn)
	template<class T = double, class F = double>
	std::pair<T, F> bootstrap1(
		const cash_deposit<T, F>& cd, pwflat::curve<T, F>& f,
		double eps = 1e-8, size_t iter = 100)
	{
		// Will have cd[0] = {0, -1}.
		auto [u_, c_] = cd[1]; // Cash flow at maturity.
		auto [t_, f_] = f.back(); // Last point on curve.
		auto pv = present_value(cd, f);

		return { u_, f.extrapolate() };
	}

	// TODO: bootstrap2 forward rate agreement (two cash flows, price 0)

	// TODO: implement generic bootstrap function
	template<class U = double, class C = double, class T = double, class F = double>
	inline pwflat::curve<> bootstrap(const std::vector<instrument<>>& uc,
		C eps = 1e-8, size_t iter = 100)
	{
		pwflat::curve<T, F> f;
		// call bootstrap0 for each instrument
		return f;
	}

	// TODO: implement add-in and using in hw5.xlsx

} // namespace fsl