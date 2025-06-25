// xll_pwflat.cpp - Piecewise flat forward curve.
#include "fsl_pwflat.h"
#include "xll_fsl.h"

using namespace xll;
using namespace fsl::pwflat;

AddIn xai_pwflat_curve_(
	Function(XLL_HANDLEX, L"?xll_pwflat_curve_", L"\\PWFLAT.CURVE")
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
HANDLEX WINAPI xll_pwflat_curve_(const _FP12* pt, const _FP12* pf, double _f)
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

AddIn xai_pwflat_curve(
	Function(XLL_FP, L"?xll_pwflat_curve", L"PWFLAT.CURVE")
	.Arguments({
		Arg(XLL_HANDLEX, L"h", L"is a handle to a piecewise flat forward curve."),
	})
	.Category(CATEGORY)
	.FunctionHelp("Return a two row array of time points t and forward rates f.")
);
_FP12* WINAPI xll_pwflat_curve(HANDLEX h)
{
#pragma XLLEXPORT
	static FPX tf;
	try {
		tf.resize(0, 0); // Reset the array
		handle<curve<>> h_(h);
		ensure(h_);
		int n = static_cast<int>(h_->size());
		tf.resize(2, n + 1);
		for (int j = 0; j < n; ++j) {
			tf(0, j) = h_->time()[j];  // Time point
			tf(1, j) = h_->rate()[j];  // Forward rate
		}
		tf(0, n) = h_->back().first;  // Last time point
		tf(1, n) = h_->back().second; // Last forward rate
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
	return tf.get();
}

AddIn xai_pwflat_extrapolate(
	Function(XLL_DOUBLE, L"?xll_pwflat_extrapolate", L"PWFLAT.EXTRAPOLATE")
	.Arguments({
		Arg(XLL_HANDLEX, L"h", L"is a handle to a piecewise flat forward curve."),
		})
		.Category(CATEGORY)
	.FunctionHelp(
		L"Return extrapolated value _f."
	)
);
double WINAPI xll_pwflat_extrapolate(HANDLEX h)
{
#pragma XLLEXPORT
	double result = fsl::NaN<double>;

	try {
		handle<curve<double, double>> h_(h);
		if (h_) {
			result = h_->extrapolate();
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

AddIn xai_pwflat_forward(
	Function(XLL_DOUBLE, L"?xll_pwflat_forward", L"PWFLAT.FORWARD")
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

AddIn xai_pwflat_spot(
	Function(XLL_DOUBLE, L"?xll_pwflat_spot", L"PWFLAT.SPOT")
	.Arguments({
		Arg(XLL_HANDLEX, L"h", L"is a handle to a piecewise flat curve."),
		Arg(XLL_DOUBLE, L"u", L"is the time at which to evaluate the spot rate."),
		})
		.Category(CATEGORY)
	.FunctionHelp(
		L"Return the spot rate at time u for the piecewise flat curve."
	)
);
double WINAPI xll_pwflat_spot(HANDLEX h, double u)
{
#pragma XLLEXPORT
	double result = fsl::NaN<double>;

	try {
		handle<curve<double, double>> h_(h);
		if (h_) {
			result = h_->spot(u);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

AddIn xai_pwflat_discount(
	Function(XLL_DOUBLE, L"?xll_pwflat_discount", L"PWFLAT.DISCOUNT")
	.Arguments({
		Arg(XLL_HANDLEX, L"h", L"is a handle to a piecewise flat curve."),
		Arg(XLL_DOUBLE, L"u", L"is the time at which to evaluate the discount rate."),
		})
		.Category(CATEGORY)
	.FunctionHelp(
		L"Return the discount rate at time u for the piecewise flat discount curve."
	)
);
double WINAPI xll_pwflat_discount(HANDLEX h, double u)
{
#pragma XLLEXPORT
	double result = fsl::NaN<double>;

	try {
		handle<curve<double, double>> h_(h);
		if (h_) {
			result = h_->discount(u);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}