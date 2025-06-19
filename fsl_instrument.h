// fsl_instrument.h - Fixed income instruments
#pragma once
#include <cmath>
#include <stdexcept>
#include <utility>	
#include <variant>
#include <vector>

namespace fsl {
	template<class U = double, class C = double>
	using cash_flow = std::pair<U, C>;

	template<class U = double, class C = double>
	using instrument = std::vector<cash_flow<U, C>>;

	// Zero coupon bond given maturity and price.
	template<class U = double, class C = double>
	struct zero_coupon_bond : public instrument<U, C>
	{
		// Single cash flow.
		zero_coupon_bond(U u, C D)
			: instrument<U, C>({ {U(0), -D}, { u, C(1)}})
		{
		}
		// Default constructor.
		zero_coupon_bond() = default;
		using instrument<U, C>::operator=; // Inherit assignment operator.
	};

	// Spot starting forward rate agreement
	template<class U = double, class C = double>
	struct cash_deposit : public instrument<U, C>
	{
		// Construct from maturity and simple interest rate.
		cash_deposit(U u, C r)
			: instrument<U, C>({ {U(0), C(-1)}, { u, std::exp(r * u) } })
		{
		}
		// Default constructor.
		cash_deposit() = default;
		using instrument<U, C>::operator=; // Inherit assignment operator.
	};

	template<class U = double, class C = double>
	struct forward_rate_agreement : public instrument<U, C>
	{
		forward_rate_agreement(U u, U v, C f)
			: instrument<U, C>({ {u, C(-1)}, { v, std::exp(f * (v - u)) } })
		{
		}
		// Default constructor.
		forward_rate_agreement() = default;
		using instrument<U, C>::operator=; // Inherit assignment operator.
	};

	enum class frequency {
		annually = 1, // Annual payments.
		semiannually = 2, // Semi-annual payments.
		quarterly = 4, // Quarterly payments.
		monthly = 12 // Monthly payments.
	};

	// Cash flow of -1 at 0, coupon c/n at ju/n, 1 + c/n at u.
	template<class U = double, class C = double>
	struct interest_rate_swap : public instrument<U, C>
	{
		// Construct from maturity, par coupon and payments per year.
		interest_rate_swap(U u, C c, frequency n = frequency::annually)
			: instrument<U, C>(size_t(n * u) + 1)
		{
			this->at(0) = { U(0), C(-1) }; // Initial cash flow of -1 at time 0.
			U du = 1/static_cast<U>(n); // Period length.
			for (size_t i = 1; i < this->size(); ++i) {
				this->at(i) = { du * i, c * du }; // Cash flow at each period.
			}
			this->back() = {u, 1 + c * du}; // Final cash flow.
		}
		// Default constructor.
		interest_rate_swap() = default;
		using instrument<U, C>::operator=; // Inherit assignment operator.

	};

	template<class U = double, class C = double>
	using fixed_income_instrument = std::variant <
		zero_coupon_bond<U, C>,
		cash_deposit<U, C>,
		forward_rate_agreement<U, C>,
		interest_rate_swap<U, C>
	>;

} // namespace fsl
