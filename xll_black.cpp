#include "fsl_bsm.h"	
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

AddIn xai_black_put_gamma(
	Function(XLL_DOUBLE, L"?xll_black_put_gamma", L"BLACK.PUT.GAMMA")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the forward Black put gamma of an option (d/df)^2 E[max{k - F, 0}]"
		" where F = f exp(s Z - s^2/2) and Z is a standard normal random variable.")
);
double WINAPI xll_black_put_gamma(double f, double s, double k)
{
#pragma XLLEXPORT
	return fsl::black_put_gamma(f, s, k);
}

AddIn xai_black_put_vega(
	Function(XLL_DOUBLE, L"?xll_black_put_vega", L"BLACK.PUT.VEGA")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the forward Black put vega of an option (d/ds) E[max{k - F, 0}]"
		" where F = f exp(s Z - s^2/2) and Z is a standard normal random variable.")
);
double WINAPI xll_black_put_vega(double f, double s, double k)
{
#pragma XLLEXPORT
	return fsl::black_put_vega(f, s, k);
}

AddIn xai_black_put_implied(
	Function(XLL_DOUBLE, L"?xll_black_put_implied", L"BLACK.PUT.IMPLIED")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"p", L"is the put value.", 4),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the implied volatility for a Black put option with value p.")
);
double WINAPI xll_black_put_implied(double f, double p, double k)
{
#pragma XLLEXPORT
	// Return #NUM! by default.
	double result = std::numeric_limits<double>::quiet_NaN();
	
	try {
		result = fsl::black_put_implied(p, f, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

// Run tests on xlAutoOpen
Auto<Open> xao_fsl_test([]() {
	try {
		using namespace fsl;
		test_normal_cdf();
		test_normal_pdf();
		test_black_moneyness();
		test_black_put_value();
		test_black_put_delta();
		test_black_put_gamma();
		test_black_put_vega();
		test_black_put_implied();
		test_black_bsm();
		test_bsm_put_value();
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
