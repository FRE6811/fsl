// xll_bootstrap.cpp - Bootstrap a piecewise flat forward curve.
#include "fsl_bootstrap.h"
#include "xll_fsl.h"

using namespace xll;
using namespace fsl;

AddIn xai_fsl_bootstrap0_(
	Function(XLL_FP, L"?xll_fsl_bootstrap_", L"\\BOOTSTRAP")
	.Arguments({
		Arg(XLL_FP, "instruments", "is an array of intsrument handles."),
		})
		.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return the last cash flow time and forward rate of a piecewise flat forward curve.")
);
HANDLEX WINAPI xll_fsl_bootstrap_(const _FP12* ph)
{
#pragma XLLEXPORT
	HANDLEX h = INVALID_HANDLEX;

	try {
		// Vector of pointers to instruments.
		std::vector<const instrument<>*> is(size(*ph));
		for (int i = 0; i < size(*ph); ++i) {
			handle<instrument<>> i_(ph->array[i]);
			ensure(i_);
			is[i] = i_.ptr();
		}
		handle<pwflat::curve<>> h_(new pwflat::curve<>(fsl::bootstrap<>(is)));
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}	

	return h;
}