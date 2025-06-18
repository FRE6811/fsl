// fsl_bootstrap.h - Bootstrap a piecewise flat forward curve.
#pragma once
#include "fsl_pwflat.h"

namespace fsl {

	template<class U = double, class C = double>
	using cash_flow = std::pair<U, C>;

	template<class U = double, class C = double>
	using instrument = std::vector<cash_flow<U, C>>;

	template<class U = double, class C = double>
	struct zero_coupon_bond : public instrument<U, C>
	{
		// Construct from a single cash flow.
		zero_coupon_bond(U u, C c = 1)
			: instrument<U, C>({ { u, c } })
		{ }
		// Default constructor.
		zero_coupon_bond() = default;
		using instrument<U, C>::operator=; // Inherit assignment operator.
	};

	template<class U = double, class C = double>
	struct forward_rate_agreement : public instrument<U, C>
	{
		// Construct from maturity and simple interest rate.
		forward_rate_agreement(double u, double r)
			: instrument<double, double>({ {U(0), C(-1)}, { u, 1 + r*u } })
		{
		}
		// Default constructor.
		forward_rate_agreement() = default;
		using instrument<double, double>::operator=; // Inherit assignment operator.
	};

	template<class U = double, class C = double>
	struct forward_rate_agreement : public instrument<U, C>
	{
		// Construct from maturity and simple interest rate.
		forward_rate_agreement(double u, double v, double f)
			: instrument<double, double>({ {u, C(-1)}, { vu, 1 + v * (v - u) } })
		{
		}
		// Default constructor.
		forward_rate_agreement() = default;
		using instrument<double, double>::operator=; // Inherit assignment operator.
	};


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
	std::pair<T, F> bootstrap0(const instrument<U, C>& uc, pwflat::curve<T, F>& f, C eps = 1e-8, size_t iter = 100)
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
		while (iter-- && std::fabs(present_value(uc, f)) > eps) {
			f_ = f_ - present_value(uc, f) / duration(uc, f);
			f.extrapolate(f_);
		}
		
		return { u_, f_ };
	}
	// TODO: !!!Add comments containing formulas
	// TODO: bootstrap1 cash deposit (one cash flow)

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