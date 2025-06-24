// xll_brownian.cpp - Brownian samples
#include <random>
#include "xll_fsl.h"

using namespace xll;

std::default_random_engine dre;
std::normal_distribution<double> N;

AddIn xai_brownian(
	Function(XLL_FP, L"?xll_brownian", L"BROWNIAN")
	.Arguments({
		Arg(XLL_FP, "times", "is the sample times.")
		})
	.Volatile()
	.Category(CATEGORY)
	.FunctionHelp("Return Brownian samples at times.")
);
_FP12* WINAPI xll_brownian(_FP12* pt)
{
#pragma XLLEXPORT
	try {
		int n = size(*pt);
		double B = 0; // last Brownian sample
		double t = 0; // last time

		for (int i = 0; i < n; ++i) {
			B += N(dre) * sqrt(pt->array[i] - t);
			t = pt->array[i]; // new last time
			pt->array[i] = B; // current sample
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
	catch (...) {
		XLL_ERROR("Unknown error in " __FUNCTION__);
	}

	return pt;
}