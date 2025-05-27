#include "fsl.h"

using namespace xll;

AddIn xai_black_moneyness(
	Function(XLL_DOUBLE, L"?xll_black_moneyness", L"BLACK.MONEYNESS")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset."),
		Arg(XLL_DOUBLE, L"s", L"is the vol."),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option."),
		})
		.Category(L"FSL")
	.FunctionHelp(L"Return the Black moneyness of an option, defined as (log(k/f) + s^2/2)/s.")
);
double WINAPI xll_black_moneyness(double f, double s, double k)
{
#pragma XLLEXPORT
	if (f <= 0 || s <= 0 || k <= 0) {
		XLL_ERROR("f, s, and k must be positive");
		return 0;
	}
	return (log(k / f) + s * s / 2) / s;
}