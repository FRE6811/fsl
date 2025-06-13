// xll_bsm.cpp - Black-Scholes/Merton functions for Excel
#include "fsl_bsm.h"
#include "xll_fsl.h"

using namespace xll;

AddIn xai_bsm_put_value(
	Function(XLL_DOUBLE, L"?xll_bsm_put_value", L"BSM.PUT.VALUE")
	.Arguments({
		Arg(XLL_DOUBLE, L"r", L"is interest rate.", .1),
		Arg(XLL_DOUBLE, L"s0", L"is spot stock price.", 100),
		Arg(XLL_DOUBLE, L"sigma", L"is the volatility of the stock.", .2),
		Arg(XLL_DOUBLE, L"t", L"is the time to maturity in years.", 1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the Black-Merton/Scholes put value of an option.")
);
double WINAPI xll_bsm_put_value(double r, double s0, double sigma, double t, double k)
{
#pragma XLLEXPORT
	return fsl::bsm_put_value(r, s0, sigma, t, k);
}


// TODO: Implement add-ins BSM.PUT.DELTA/GAMMA/VEGA/IMPLIED.