// xll_vswap.cpp - Variance swap implementation
#include "fsl_vswap.h"
#include "xll_fsl.h"

using namespace fsl;
using namespace xll;

Auto<Open> xao_vswap_test([] {

	test_difference_quotient();
	
	return TRUE; // Indicate successful test
});

AddIn xai_vswap_static_payoff(
	Function(XLL_DOUBLE, L"?xll_vswap_static_payoff", L"VSWAP.STATIC_PAYOFF")
	.Arguments({
		Arg(XLL_DOUBLE, L"x0",L"is the initial stock price."),
		Arg(XLL_DOUBLE, L"z", L"is the put/call separator."),
		Arg(XLL_DOUBLE, L"x", L"is the underlying price."),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the variance swap payoff at x given initial price x0 and put/call separator z.")
);
double WINAPI xll_vswap_static_payoff(double x0, double z, double x)
{
#pragma XLLEXPORT
	return fsl::static_payoff(x0, z, x);
}

AddIn xai_vswap_weights(
	Function(XLL_FP, L"?xll_vswap_weights", L"VSWAP.WEIGHTS")
	.Arguments({
		Arg(XLL_DOUBLE, L"x0", L"is the initial stock price."),
		Arg(XLL_DOUBLE, L"z", L"is the put/call separator."),
		Arg(XLL_FP, L"k", L"is an array of strikes."),
		})
	.Category(CATEGORY)
	.FunctionHelp(L"Return the variance swap weights for strikes k given initial price x0 and put/call separator z.")
);
_FP12* WINAPI xll_vswap_weights(double x0, double z, _FP12* pk)
{
#pragma XLLEXPORT
	static xll::FPX w;
	try {
		w.resize(size(*pk), 1);

		ensure(nullptr != fsl::vswap_weights(x0, z, size(*pk), pk->array, w.array()));
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
		return nullptr;
	}
	return w.get();
}

AddIn xai_par_variance(
	Function(XLL_DOUBLE, L"?xll_par_variance", L"VSWAP.PAR_VARIANCE")
	.Arguments({
		Arg(XLL_DOUBLE, L"dt", L"time period of variance swap in years."),
		Arg(XLL_DOUBLE, L"x0", L"is the initial stock price."),
		Arg(XLL_DOUBLE, L"z", L"is the put/call separator."),
		Arg(XLL_FP, L"k", L"is an array of strikes."),
		Arg(XLL_FP, L"p", L"is an array of put prices."),
		Arg(XLL_FP, L"c", L"is an array of call prices."),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the par variance σ_0^2 given time period dt, initial price x0, put/call separator z, strikes k, put prices p, and call prices c.")
);
double WINAPI xll_par_variance(double dt, double x0, double z, _FP12* pk, _FP12* pp, _FP12* cp)
{
#pragma XLLEXPORT
	double result = NaN<double>;
	try {
		ensure(size(*pk) == size(*pp));
		ensure(size(*pk) == size(*cp));

		result = fsl::par_variance(dt, x0, z, size(*pk), pk->array, pp->array, cp->array);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}