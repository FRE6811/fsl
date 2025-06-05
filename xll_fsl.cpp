#include <cassert>
#include "fsl_variate.h"
#include "fsl_black.h"
#include "fsl.h"


using namespace xll;

AddIn xai_black_moneyness(
	Function(XLL_DOUBLE, L"?xll_black_moneyness", L"BLACK.MONEYNESS")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the Black moneyness of an option, defined as (log(k/f) + s^2/2)/s.")
);
double WINAPI xll_black_moneyness(double f, double s, double k)
{
#pragma XLLEXPORT
	// Return #NUM! on error.
	double result = std::numeric_limits<double>::quiet_NaN();

	try {
		// Collect arguments from Excel and call C++.
		result = fsl::black_moneyness(f, s, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

AddIn xai_black_put_value(
	Function(XLL_DOUBLE, L"?xll_black_put_value", L"BLACK.PUT.VALUE")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the forward Black put value of an option E[max{k - F, 0}]"
		" where F = f exp(s Z - s^2/2) and Z is a standard normal random variable.")
);
double WINAPI xll_black_put_value(double f, double s, double k)
{
#pragma XLLEXPORT
	double result = std::numeric_limits<double>::quiet_NaN();

	try {
		result = fsl::black_put_value(f, s, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

// (d/df) E[max(k - F, 0)] = E[-1(F <= k) dF/df]
// dF/df = exp(s Z - s^2/2).
double fsl_black_put_delta(double f, double s, double k)
{
	double z = fsl::black_moneyness(f, s, k);

	return - fsl::normal_cdf(z - s);
}
int test_fsl_black_put_delta()
{
	{
		double data[][4] = {
			// f, s, k, p 
			{100, .1, 100, -0.48006119416162754},
			// ...more tests here...
		};
		double eps = 1e-4;
		for (auto [f, s, k, p] : data) {
			double p_ = fsl_black_put_delta(f, s, k);
			assert(p == p_);

			// Symmetric difference quotient for numerical derivative.
			double dp = (fsl::black_put_value(f + eps, s, k) - fsl::black_put_value(f - eps, s, k)) / (2 * eps);
			double err = p_ - dp;
			assert(fabs(err) <= eps * eps);
		}
	}

	return 0;
}

AddIn xai_black_put_delta(
	Function(XLL_DOUBLE, L"?xll_black_put_delta", L"BLACK.PUT.DELTA")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset.", 100),
		Arg(XLL_DOUBLE, L"s", L"is the vol.", .1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the forward Black put delta of an option E[max{k - F, 0}]"
		" where F = f exp(s Z - s^2/2) and Z is a standard normal random variable.")
);
double WINAPI xll_black_put_delta(double f, double s, double k)
{
#pragma XLLEXPORT
	double result = std::numeric_limits<double>::quiet_NaN();

	try {
		result = fsl_black_put_delta(f, s, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

// Black-Scholes/Merton S_t = s0 exp((r - sigma^2/2) t + sigma B_t) where B_t is a standard Brownian motion.
// Put value is exp(-r t) E[max{k - S_t, 0}]
// If F = S_t then f = s0 exp(r t) and s = sigma sqrt(t).
// Convert Black-Scholes/Merton parameters to Black model.
std::tuple<double, double, double> fsl_black_bsm(double r, double s0, double sigma, double t)
{
	if (s0 <= 0 || sigma <= 0 || t <= 0) {
		throw std::runtime_error("s0, sigma, and t must be positive");
	}
	double D = exp(-r * t); // Discount factor
	double f = s0 / D; // Forward price
	double s = sigma * sqrt(t); // Volatility for Black model

	return { D, f, s };
}
int test_fsl_black_bsm()
{
	{
		double data[][7] = {
			// r, s0, sigma, t, D, f, s
			{0.05, 100, 0.2, 1, exp(-0.05*1), 100/exp(-0.05*1), 0.2*sqrt(1)},
			{0.03, 50, 0.1, 2, 0.94176453358424872, 53.091827327267978, 0.14142135623730953},
			// ...more tests here...
		};
		for (auto [r, s0, sigma, t, D, f, s] : data) {
			auto [D_, f_, s_] = fsl_black_bsm(r, s0, sigma, t);
			assert(D == D_);
			assert(f == f_);
			assert(s == s_);
		}
	}
	return 0;
}

double fsl_bsm_put_value(double r, double s0, double sigma, double t, double k)
{
	auto [D, f, s] = fsl_black_bsm(r, s0, sigma, t);

	return D * fsl::black_put_value(f, s, k);
}
int test_fsl_bsm_put_value()
{
	{
		double data[][6] = {
			// r, s0, sigma, t, k, p
			{0.05, 100, 0.2, 1, 100, 5.5735260222569671},
			{0.03, 50, 0.1, 2, 55, 3.8565888892016207},
			// ...more tests here...
		};
		for (auto [r, s0, sigma, t, k, p] : data) {
			double p_ = fsl_bsm_put_value(r, s0, sigma, t, k);
			assert(fabs(p - p_) <= std::numeric_limits<double>::epsilon());
		}
	}
	
	return 0;
}
AddIn xai_bsm_put_value(
	Function(XLL_DOUBLE, L"?xll_bsm_put_value", L"BSM.PUT.VALUE")
	.Arguments({
		Arg(XLL_DOUBLE, L"r", L"is the risk-free interest rate.", 0.05),
		Arg(XLL_DOUBLE, L"s0", L"is the initial stock price.", 100),
		Arg(XLL_DOUBLE, L"sigma", L"is the volatility of the stock.", 0.2),
		Arg(XLL_DOUBLE, L"t", L"is the time to expiration in years.", 1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
	.Category(CATEGORY)
	.FunctionHelp(L"Return the Black-Scholes/Merton put value of an option E[max{k - S_t, 0}]"
		" where S_t is the stock price at time t.")
);
double WINAPI xll_bsm_put_value(double r, double s0, double sigma, double t, double k)
{
#pragma XLLEXPORT
	double result = std::numeric_limits<double>::quiet_NaN();
	try {
		result = fsl_bsm_put_value(r, s0, sigma, t, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
	return result;
}

// B-S/M delta is d/ds0 exp(-r t) E[max(k - S_t, 0)] = exp(-r t) E[-1(S_t <= k) dS_t/ds0]
// dS_t/ds0 = exp((r - sigma^2/2) t + sigma B_t) ~ exp(r t) exp(s Z - s^2/2)
// so d/ds0 exp(-r t) E[max(k - S_t, 0)] = d/df E[max(k - F, 0)]
// and B-S/M delta is the same as Black delta.

double fsl_bsm_put_delta(double r, double s0, double sigma, double t, double k)
{
	auto [D, f, s] = fsl_black_bsm(r, s0, sigma, t);
	return fsl_black_put_delta(f, s, k);
}
int test_fsl_bsm_put_delta()
{
	{
		double data[][6] = {
			// r, s0, sigma, t, k, p 
			{0.05, 100, 0.2, 1, 100, -0.48006119416162754},
			{0.03, 50, 0.1, 2, 55,   -0.48006119416162754},
			// ...more tests here...
		};
		for (auto [r, s0, sigma, t, k, p] : data) {
			double p_ = fsl_bsm_put_delta(r, s0, sigma, t, k);
			//assert(fabs(p - p_) <= std::numeric_limits<double>::epsilon());
		}
	}
	return 0;
}
AddIn xai_bsm_put_delta(
	Function(XLL_DOUBLE, L"?xll_bsm_put_delta", L"BSM.PUT.DELTA")
	.Arguments({
		Arg(XLL_DOUBLE, L"r", L"is the risk-free interest rate.", 0.05),
		Arg(XLL_DOUBLE, L"s0", L"is the initial stock price.", 100),
		Arg(XLL_DOUBLE, L"sigma", L"is the volatility of the stock.", 0.2),
		Arg(XLL_DOUBLE, L"t", L"is the time to expiration in years.", 1),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option.", 100),
		})
		.Category(CATEGORY)
	.FunctionHelp(L"Return the Black-Scholes/Merton put delta of an option d/ds0 E[max{k - S_t, 0}]"
		" where S_t is the stock price at time t.")
);
double WINAPI xll_bsm_put_delta(double r, double s0, double sigma, double t, double k)
{
#pragma XLLEXPORT
	double result = std::numeric_limits<double>::quiet_NaN();
	try {
		result = fsl_bsm_put_delta(r, s0, sigma, t, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
	return result;
}


// Run tests on xlAutoOpen
Auto<Open> xao_fsl_test([]() {
	try {
		fsl::test_normal_cdf();
		fsl::test_fsl_black_moneyness();
		fsl::test_fsl_black_put_value();
		test_fsl_black_put_delta();
		test_fsl_black_bsm();
		test_fsl_bsm_put_value();
		test_fsl_bsm_put_delta();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
		return FALSE;
	}
	catch (...) {
		XLL_ERROR(L"Unknown exception in xao_fsl_test");
		return FALSE;
	}

	return TRUE;
});