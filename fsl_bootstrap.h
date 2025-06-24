// fsl_bootstrap.h - Bootstrap a piecewise flat forward curve.
#pragma once
#include "fsl_instrument.h"
#include "fsl_pwflat.h"
#include "fsl_root1d.h"

namespace fsl {

	template<class U = double, class C = double, class T = double, class F = double>
	constexpr C present_value(const instrument<U, C>& uc, const pwflat::curve_view<T, F>& D)
	{
		C pv = 0;

		for (const auto& [u, c] : uc) {
			pv += c * D.discount(u);
		}
		
		return pv;
	}

	// Derivative of present value with respect to forward rate.
	template<class U = double, class C = double, class T = double, class F = double>
	constexpr C duration(const instrument<U, C>& uc, const pwflat::curve_view<T, F>& f)
	{
		C dur = 0;

		for (const auto& [u, c] : uc) {
			dur += u * c * f.discount(u);
		}

		return dur;
	}

	// Bootstrap a piecewise flat forward curve from an instrument with price 0.
	template<class U = double, class C = double, class T = double, class F = double>
	std::pair<T, F> bootstrap0(const instrument<U, C>& uc, const pwflat::curve_view<T, F>& f, C eps = 1e-8, size_t iter = 100)
	{
		if (uc.empty()) {
			throw std::runtime_error("Instrument cash flows must be non- empty");
		}
		auto [u_, c_] = uc.back(); // Last cash flow.
		auto [t_, f_] = f.back(); // Last point on curve.

		if (u_ <= t_) {
			throw std::runtime_error("Last cash flow must be past end of curve");
		}

		const auto pv = [&uc, &f](F _f) { return present_value(uc, extrapolate(f, _f)); };
		f_ = std::get<0>(root1d::secant(f_, f_ + 0.01).solve(pv));

		return { u_, f_ };
	}

	template<class U = double, class C = double, class T = double, class F = double>
	inline pwflat::curve<T,F> bootstrap(const std::vector<const instrument<U,C>*>& uc)
	{
		pwflat::curve<T, F> f;

 		// call bootstrap0 for each instrument
		for (size_t i = 0; i < uc.size(); ++i) {
			if (uc[i] == nullptr) {
				throw std::runtime_error("Null instrument pointer in bootstrap");
			}
			auto [u, f_] = bootstrap0(*uc[i], f);
			f.push_back(u, f_);
		}

		return f;
	}

} // namespace fsl