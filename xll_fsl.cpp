#include <cassert>
#include "fsl.h"

using namespace xll;

// Standard normal cumulative distribution function P(Z <= z).
// https://en.wikipedia.org/wiki/Error_function#Cumulative_distribution_function
double normal_cdf(double z)
{
	// Cumulative distribution function for the standard normal distribution
	return 0.5 * (1 + erf(z / sqrt(2)));
}
int test_normal_cdf()
{
	{
		double zs[] = { -2, -1, 0, 1, 2 };
		for (double z : zs) {
			double cdf = normal_cdf(z);
			// compare with Excel builtin
			double cdf_ = asNum(Excel(xlfNormsdist, z));
			assert(fabs(cdf - cdf_) <= std::numeric_limits<double>::epsilon());
		}
	}

	return 0;
}

// Factor out code independent of Excel.
// F = f exp(sZ - s^2/2) <= k if and only if Z <= (log(k/f) + s^2/2)/s
double fsl_black_moneyness(double f, double s, double k)
{
	if (f <= 0 || s <= 0 || k <= 0) {
		// Use C++ exceptions for error handling.
		throw std::runtime_error("f, s, and k must be positive");
	}

	return (log(k / f) + s * s / 2) / s;
}
int test_fsl_black_moneyness()
{
	{
		// Test exception.
		double data[][3] = {
			// f, s, k
			{-1, 1, 1},
			{1, -1, 1},
			{1, 1, -1}
		};
		for (auto [f, s, k] : data) {
			bool thrown = false;
			try {
				fsl_black_moneyness(f, s, k);
			}
			catch (const std::exception&) {
				thrown = true;
			}
			assert(thrown);
		}
	}
	{
		double data[][4] = {
			// f, s, k, z
			{100, .1, 100, 0.05000000000000001},
			{100, .1, 90, -1.0036051565782627},
			{100, .1, 110, 1.0031017980432493},
			// ...more tests here...
		};
		for (auto [f, s, k, z] : data) {
			double z_ = fsl_black_moneyness(f, s, k);
			assert(z == z_);
		}
	}

	return 0;
}

AddIn xai_black_moneyness(
	Function(XLL_DOUBLE, L"?xll_black_moneyness", L"BLACK.MONEYNESS")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset."),
		Arg(XLL_DOUBLE, L"s", L"is the vol."),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option."),
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
		result = fsl_black_moneyness(f, s, k);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return result;
}

// E[max(k - F, 0)] = k P(Z <= z) - f P(Z + s <= z)
double fsl_black_put_value(double f, double s, double k)
{
	double z = fsl_black_moneyness(f, s, k);

	return k * normal_cdf(z) - f * normal_cdf(z - s);
}
int test_fsl_black_put_value()
{
	{
		double data[][4] = {
			// f, s, k, p 
			{100, .1, 100, 3.9877611676744920},
			// ...more tests here...
		};
		for (auto [f, s, k, p] : data) {
			double p_ = fsl_black_put_value(f, s, k);
			assert(p == p_);
		}
	}

	return 0;
}

AddIn xai_black_put_value(
	Function(XLL_DOUBLE, L"?xll_black_put_value", L"BLACK.PUT.VALUE")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset."),
		Arg(XLL_DOUBLE, L"s", L"is the vol."),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option."),
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
		result = fsl_black_put_value(f, s, k);
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
	double z = fsl_black_moneyness(f, s, k);

	return - f * normal_cdf(z - s);
}
int test_fsl_black_put_delta()
{
	{
		double data[][4] = {
			// f, s, k, p 
			{100, .1, 100, -48.006119416162754},
			// ...more tests here...
		};
		for (auto [f, s, k, p] : data) {
			double p_ = fsl_black_put_delta(f, s, k);
			assert(p == p_);
		}
	}

	return 0;
}

AddIn xai_black_put_delta(
	Function(XLL_DOUBLE, L"?xll_black_put_delta", L"BLACK.PUT.DELTA")
	.Arguments({
		Arg(XLL_DOUBLE, L"f", L"is the forward price of the underlying asset."),
		Arg(XLL_DOUBLE, L"s", L"is the vol."),
		Arg(XLL_DOUBLE, L"k", L"is the strike price of the option."),
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
std::tuple<double, double, double> fsl_bsm_black(double r, double s0, double sigma, double t)
{
	if (s0 <= 0 || sigma <= 0 || t <= 0) {
		throw std::runtime_error("s0, sigma, and t must be positive");
	}
	double D = exp(-r * t); // Discount factor
	double f = s0 / D; // Forward price
	double s = sigma * sqrt(t); // Volatility for Black model

	return { D, f, s };
}
int test_fsl_bsm_black()
{
	{
		double data[][7] = {
			// r, s0, sigma, t, D, f, s
			{0.05, 100, 0.2, 1, exp(-0.05*1), 100/exp(-0.05*1), 0.2*sqrt(1)},
			{0.03, 50, 0.1, 2, 0.94176453358424872, 53.091827327267978, 0.14142135623730953},
			// ...more tests here...
		};
		for (auto [r, s0, sigma, t, D, f, s] : data) {
			auto [D_, f_, s_] = fsl_bsm_black(r, s0, sigma, t);
			assert(D == D_);
			assert(f == f_);
			assert(s == s_);
		}
	}
	return 0;
}

double fsl_bsm_put_value(double r, double s0, double sigma, double t, double k)
{
	auto [D, f, s] = fsl_bsm_black(r, s0, sigma, t);

	return D * fsl_black_put_value(f, s, k);
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
//
// TODO: Implement BSM.PUT.VALUE
// 

// B-S/M delta is d/dS exp(-r t) E[max(k - S_t, 0)] = E[-1(S_t <= k) dS_t/dS]
// dS_t/dS = exp((r - sigma^2/2) t + sigma B_t) ~ exp(r t) exp(s Z - s^2/2)

//
// TODO: Implement fsl_bsm_put_delta in C++
// 
// TODO: Implement test_fsl_bsm_put_delta in C++
//
// TODO: Implement the BSM.PUT.DELTA add-in
//

// Run tests on xlAutoOpen
Auto<Open> xao_fsl_test([]() {
	try {
		test_normal_cdf();
		test_fsl_black_moneyness();
		test_fsl_black_put_value();
		test_fsl_black_put_delta();
		test_fsl_bsm_black();
		test_fsl_bsm_put_value();
		//test_fsl_bsm_put_delta();
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