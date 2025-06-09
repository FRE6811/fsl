#include "fsl_black.h"	
#include "xll_fsl.h"

using namespace xll;

AddIn xai_black_moneyness(
	Function(XLL_DOUBLE, L"?xll_black_moneyness", L"BLACK.MONEYNESS")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the Black moneyness of an option, defined as (log(k/f) + s^2/2)/s.")
);
double WINAPI xll_black_moneyness(double f, double s, double k)
{
#pragma XLLEXPORT
	return fsl::black_moneyness(f, s, k);
}

AddIn xai_black_put_value(
	Function(XLL_DOUBLE, L"?xll_black_put_value", L"BLACK.PUT.VALUE")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the forward Black put value of an option E[max{k - F, 0}]"
		" where F = f exp(s Z - s^2/2) and Z is a standard normal random variable.")
);
double WINAPI xll_black_put_value(double f, double s, double k)
{
#pragma XLLEXPORT
	return fsl::black_put_value(f, s, k);
}

AddIn xai_black_put_delta(
	Function(XLL_DOUBLE, L"?xll_black_put_delta", L"BLACK.PUT.DELTA")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the forward Black put delta of an option (d/df) E[max{k - F, 0}]"
		" where F = f exp(s Z - s^2/2) and Z is a standard normal random variable.")
);
double WINAPI xll_black_put_delta(double f, double s, double k)
{
#pragma XLLEXPORT
	return fsl::black_put_delta(f, s, k);
}

// TODO: Implement Black put vega

// Black-Scholes/Merton S_t = s0 exp((r - sigma^2/2) t + sigma B_t) where B_t is a standard Brownian motion.
// Put value is exp(-r t) E[max{k - S_t, 0}]
// If F = S_t then f = s0 exp(r t) and s = sigma sqrt(t).
// Convert Black-Scholes/Merton parameters to Black model.
std::tuple<double, double, double> fsl_black_bsm(double r, double s0, double sigma, double t)
{
	if (s0 <= 0 || sigma <= 0 || t <= 0) {
		throw std::runtime_error("s0, sigma, and t must be positive");
	}
	double D = exp(-r * t); // Discount factor
	double f = s0 / D; // Forward price
	double s = sigma * sqrt(t); // Volatility for Black model

	return { D, f, s };
}
int test_fsl_black_bsm()
{
	{
		double data[][7] = {
			// r, s0, sigma, t, D, f, s
			{0.05, 100, 0.2, 1, exp(-0.05*1), 100/exp(-0.05*1), 0.2*sqrt(1)},
			{0.03, 50, 0.1, 2, 0.94176453358424872, 53.091827327267978, 0.14142135623730953},
			// ...more tests here...
		};
		for (auto [r, s0, sigma, t, D, f, s] : data) {
			auto [D_, f_, s_] = fsl_black_bsm(r, s0, sigma, t);
			assert(D == D_);
			assert(f == f_);
			assert(s == s_);
		}
	}
	return 0;
}

// dS_t/ds0 = exp((r - sigma^2/2) t + sigma B_t) ~ exp(r t) exp(s Z - s^2/2)
// so d/ds0 exp(-r t) E[max(k - S_t, 0)] = d/df E[max(k - F, 0)]
// and B-S/M delta is the same as Black delta.

// Run tests on xlAutoOpen
Auto<Open> xao_fsl_test([]() {
	try {
		using namespace fsl;
		test_black_moneyness();
		test_black_put_value();
		test_black_put_delta();
		test_black_put_vega();
		test_black_put_implied();
		//test_fsl_black_bsm();
		//test_fsl_bsm_put_value();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
		return FALSE;
	}
	catch (...) {
		XLL_ERROR(L"Unknown exception in xao_fsl_test");
		return FALSE;
	}

	return TRUE;
});

// TODO: Implement spreadsheet to test BSM.PUT.DELTA and BSM.PUT.VEGA using symmetric difference quotient.