// xll_instrument.cpp - Fixed income instruments
#include "fsl_instrument.h"
#include "xll_fsl.h"

using namespace fsl;
using namespace xll;

// Enumeration constants.
XLL_CONST(INT, FREQUENCY_ANNUALLY, (int)frequency::annually, "One payment per year.", CATEGORY, "");
XLL_CONST(INT, FREQUENCY_SENIANNUALLY, (int)frequency::semiannually, "Two payments per year.", CATEGORY, "");
XLL_CONST(INT, FREQUENCY_QUARTERLY, (int)frequency::quarterly, "Four payment per year.", CATEGORY, "");
XLL_CONST(INT, FREQUENCY_MONTLY, (int)frequency::monthly, "Twelve payment per year.", CATEGORY, "");

AddIn xai_fsl_instrument_(
	Function(XLL_HANDLEX, L"?xll_fsl_instrument_", L"\\INSTRUMENT")
	.Arguments({
		Arg(XLL_FP, "u", "is an array of cash flow times in years."),
		Arg(XLL_FP, "c", "is an array of cash flows."),
		})
		.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to an instrument with cash flows at times u and amounts c.")
);
HANDLEX WINAPI xll_fsl_instrument_(const _FP12* pu, const _FP12* pc)
{
#pragma XLLEXPORT
	HANDLEX h = INVALID_HANDLEX;

	try {
		ensure(size(*pu) == size(*pc) || !"Cash flow times and amounts must have the same size");
		handle<instrument<>> i_(new instrument<>(size(*pu)));
		for (size_t i = 0; i < i_->size(); ++i) {
			(*i_)[i] = cash_flow(pu->array[i], pc->array[i]);
		}
		h = i_.get();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return h;
}

AddIn xai_fsl_instrument(
	Function(XLL_FP, L"?xll_fsl_instrument", L"INSTRUMENT")
	.Arguments({
		Arg(XLL_HANDLEX, "instrument", "is a handle to an instrument."),
		})
	.Category(CATEGORY)
	.FunctionHelp("Return a two row array of times u and amounts c.")
);
_FP12* WINAPI xll_fsl_instrument(HANDLEX i)
{
#pragma XLLEXPORT
	static FPX uc;

	try {
		uc.resize(0, 0); // Reset the array
		handle<instrument<>> i_(i);
		ensure(i_);
		int n = static_cast<int>(i_->size());
		uc.resize(2, n);
		for (int j = 0; j < n; ++j) {
			uc(0, j) = (*i_)[j].first;  // Cash flow time
			uc(1, j) = (*i_)[j].second; // Cash flow amount
		}	
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return uc.get();
}

AddIn xai_fsl_instrument_zero_coupon_bond_(
	Function(XLL_HANDLEX, L"?xll_fsl_instrument_zero_coupon_bond_", L"\\INSTRUMENT.ZERO_COUPON_BOND")
	.Arguments({
		Arg(XLL_DOUBLE, "u", "is the maturity of the bond in years."),
		Arg(XLL_DOUBLE, "D", "is the price of the bond."),
		})
		.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to a zero coupon bond with maturity u and price D.")
);
HANDLEX WINAPI xll_fsl_instrument_zero_coupon_bond_(double u, double D)
{
#pragma XLLEXPORT
	HANDLEX h = INVALID_HANDLEX;
	try {
		handle<instrument<>> zcb(new zero_coupon_bond<>(u, D));
		ensure(zcb);
		h = zcb.get();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
	return h;
}

AddIn xai_fsl_instrument_cash_deposit_(
	Function(XLL_HANDLEX, L"?xll_fsl_instrument_cash_deposit_", L"\\INSTRUMENT.CASH_DEPOSIT")
	.Arguments({
		Arg(XLL_DOUBLE, "u", "is the maturity of the cash deposit in years."),
		Arg(XLL_DOUBLE, "r", "is the continuously compounded rate."),
		})
		.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to a cash deposit with maturity u and rate r.")
);
HANDLEX WINAPI xll_fsl_instrument_cash_deposit_(double u, double r)
{
#pragma XLLEXPORT
	HANDLEX h = INVALID_HANDLEX;
	try {
		handle<instrument<>> zcb(new cash_deposit<>(u, r));
		ensure(zcb);
		h = zcb.get();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
	return h;
}

// TODO: Implement \\INSTRUMENT.FORWARD_RATE_AGREEMENT and INTEREST_RATE_SWAP

// TODO: Add instruments to hw5.xlsx and insert a plot of the curve from 0 to 10 years at steps of 0.1 years.