// xll_array.cpp - Array functions for the Financial Software Library
#include "xll24/include/xll.h"

using namespace xll;

AddIn xai_array_sequence(
	Function(XLL_FP, L"?xll_array_sequence", L"ARRAY.SEQUENCE")
	.Arguments({
		Arg(XLL_DOUBLE, L"start", L"is the first number in the sequence.", 1),
		Arg(XLL_DOUBLE, L"stop", L"is the last number in the sequence.", 10),
		Arg(XLL_DOUBLE, L"_step", L"is the amount to increment. The default value is 1.", 1),
		})
	.Category(L"XLL")
	.FunctionHelp(L"Array having one column from start to stop in step increments.")
	.Documentation(
		L"For example, <code>ARRAY.SEQUENCE(1, 3)</code> "
		L"is the array <code>{1;2;3}</code>."
	)
);
_FP12* WINAPI
xll_array_sequence(double a, double b, double da)
{
#pragma XLLEXPORT
	static FPX x;

	try {

		if (da > 0) {
			ensure(da > 0 && a < b);
		}
		else if (da < 0) {
			ensure(da < 0 && a > b);
		}

		if (da == 0)
			da = 1;
		double n_ = 1 + (b - a) / da;

		int n = static_cast<int>(n_);

		if (n == 0)
			return 0;

		x.resize(n, 1);

		for (int i = 0; i < n; ++i) {
			x[i] = a + i * da;
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	return x.get();
}