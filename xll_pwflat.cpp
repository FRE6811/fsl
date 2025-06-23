// xll_pwflat.cpp - Piecewise flat forward curve.
#include "fsl_pwflat.h"
#include "xll_fsl.h"

using namespace xll;
using namespace fsl::pwflat;

AddIn xai_pwflat_curve(
	Function(XLL_HANDLEX, L"?xll_pwflat_curve", L"FSL.PWFLAT.CURVE")
	.Arguments({
		Arg(XLL_FP, L"t", L"is a vector of time points."),
		Arg(XLL_FP, L"f", L"is a vector of forward rates."),
		Arg(XLL_DOUBLE, L"_f", L"is the extrapolated value (default is NaN)."),
		})
		.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp(
		L"Return a handle to a piecewise flat forward curve determined by points (t[i], f[i]) and extrapolated value _f."
	)
);
HANDLEX WINAPI xll_pwflat_curve(const _FP12* pt, const _FP12* pf, double _f)
{
#pragma XLLEXPORT
	HANDLEX h = INVALID_HANDLEX;

	try {
		ensure(size(*pt) == size(*pf));
		if (_f == 0) {
			_f = fsl::NaN<double>;
		}
		handle<curve<>> h_(new curve(size(*pt), pt->array, pf->array, _f));
		ensure(h_);
		h = h_.get();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return h;
}

AddIn xai_pwflat_forward(
	Function(XLL_DOUBLE, L"?xll_pwflat_forward", L"FSL.PWFLAT.FORWARD")
	.Arguments({
		Arg(XLL_HANDLEX, L"h", L"is a handle to a piecewise flat forward curve."),
		Arg(XLL_DOUBLE, L"u", L"is the time at which to evaluate the forward rate."),
		})
		.Category(CATEGORY)
	.FunctionHelp(
		L"Return the forward rate at time u for the piecewise flat forward curve."
	)
);
double WINAPI xll_pwflat_forward(HANDLEX h, double u)
{
#pragma XLLEXPORT
	double result = fsl::NaN<double>;

	try {
		handle<curve<double, double>> h_(h);
		if (h_) {
			result = h_->forward(u);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}