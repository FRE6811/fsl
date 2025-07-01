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
AddIn xai_bsm_put_delta(
    Function(XLL_DOUBLE, L"?xll_bsm_put_delta", L"BSM.PUT.DELTA")
    .Arguments({
        Arg(XLL_DOUBLE, L"r", L"is interest rate.", .05),
        Arg(XLL_DOUBLE, L"s0", L"is spot stock price.", 100),
        Arg(XLL_DOUBLE, L"sigma", L"is the volatility of the stock.", .2),
        Arg(XLL_DOUBLE, L"t", L"is the time to maturity in years.", 1),
        Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
        })
        .Category(CATEGORY)
    .FunctionHelp(L"Return the Black-Scholes/Merton put delta - sensitivity to underlying price.")
);

double WINAPI xll_bsm_put_delta(double r, double s0, double sigma, double t, double k)
{
#pragma XLLEXPORT
    // Validate inputs
    if (s0 <= 0 || sigma <= 0 || t <= 0) {
        return std::numeric_limits<double>::quiet_NaN(); // or use xll::xlerr::Num for Excel #NUM!
    }
    return fsl::bsm_put_delta(r, s0, sigma, t, k);
}

AddIn xai_bsm_put_gamma(
    Function(XLL_DOUBLE, L"?xll_bsm_put_gamma", L"BSM.PUT.GAMMA")
    .Arguments({
        Arg(XLL_DOUBLE, L"r", L"is interest rate.", .05),
        Arg(XLL_DOUBLE, L"s0", L"is spot stock price.", 100),
        Arg(XLL_DOUBLE, L"sigma", L"is the volatility of the stock.", .2),
        Arg(XLL_DOUBLE, L"t", L"is the time to maturity in years.", 1),
        Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
        })
        .Category(CATEGORY)
    .FunctionHelp(L"Return the Black-Scholes/Merton put gamma - convexity measure.")
);
double WINAPI xll_bsm_put_gamma(double r, double s0, double sigma, double t, double k)
{
#pragma XLLEXPORT
    // Validate inputs
    if (s0 <= 0 || sigma <= 0 || t <= 0) {
        return std::numeric_limits<double>::quiet_NaN(); // or use xll::xlerr::Num for Excel #NUM!
    }
    return fsl::bsm_put_gamma(r, s0, sigma, t, k);
}

AddIn xai_bsm_put_vega(
    Function(XLL_DOUBLE, L"?xll_bsm_put_vega", L"BSM.PUT.VEGA")
    .Arguments({
        Arg(XLL_DOUBLE, L"r", L"is interest rate.", .05),
        Arg(XLL_DOUBLE, L"s0", L"is spot stock price.", 100),
        Arg(XLL_DOUBLE, L"sigma", L"is the volatility of the stock.", .2),
        Arg(XLL_DOUBLE, L"t", L"is the time to maturity in years.", 1),
        Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
        })
        .Category(CATEGORY)
    .FunctionHelp(L"Return the Black-Scholes/Merton put vega - sensitivity to volatility.")
);

double WINAPI xll_bsm_put_vega(double r, double s0, double sigma, double t, double k)
{
#pragma XLLEXPORT
    // Validate inputs
    if (s0 <= 0 || sigma <= 0 || t <= 0) {
        return std::numeric_limits<double>::quiet_NaN(); // or use xll::xlerr::Num for Excel #NUM!
    }
    return fsl::bsm_put_vega(r, s0, sigma, t, k);
}

AddIn xai_bsm_put_implied(
    Function(XLL_DOUBLE, L"?xll_bsm_put_implied", L"BSM.PUT.IMPLIED")
    .Arguments({
        Arg(XLL_DOUBLE, L"r", L"is interest rate.", .05),
        Arg(XLL_DOUBLE, L"s0", L"is spot stock price.", 100),
        Arg(XLL_DOUBLE, L"p", L"is the put option market price.", 5),
        Arg(XLL_DOUBLE, L"t", L"is the time to maturity in years.", 1),
        Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
        })
        .Category(CATEGORY)
    .FunctionHelp(L"Return the Black-Scholes/Merton implied volatility from put price.")
);
double WINAPI xll_bsm_put_implied(double r, double s0, double p, double t, double k)
{
#pragma XLLEXPORT
    double result = std::numeric_limits<double>::quiet_NaN();

    try {
        result = fsl::bsm_put_implied(r, s0, p, t, k);
    }
    catch (const std::exception& ex) {
        XLL_ERROR(ex.what());
    }

    return result;
}

// TODO: Implement add-ins BSM.PUT.DELTA/GAMMA/VEGA/IMPLIED.