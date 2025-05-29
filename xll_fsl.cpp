#include <cassert>
#include "fsl.h"

using namespace xll;

// Standard normal cumulative distribution function P(Z <= z).
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
			double cdf_ = Num(Excel(xlfNormsdist, z));
			assert(fabs(cdf - cdf_) <= 1e-8);
		}
	}

	return 0;
}

// Factor out platform-independent code.
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
		double data[][4] = {
			// f, s, k, z
			{100, .1, 100, 0.05},
			{100, .1, 90, -1.0036051565782627},
			{100, .1, 110, 1.0031017980432493},
			// ...more tests here...
		};
		for (auto [f, s, k, z] : data) {
			double z_ = fsl_black_moneyness(f, s, k);
			assert(fabs(z - z_) <= 1e-8);
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
		.Category(L"FSL")
	.FunctionHelp(L"Return the Black moneyness of an option, defined as (log(k/f) + s^2/2)/s.")
);
double WINAPI xll_black_moneyness(double f, double s, double k)
{
#pragma XLLEXPORT
	// Return #NUM! on error.
	double result = std::numeric_limits<double>::quiet_NaN();

	try {
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
	if (f <= 0 || s <= 0 || k <= 0) {
		throw std::runtime_error("f, s, and k must be positive");
	}
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
			assert(fabs(p - p_) <= 1e-8);
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
		.Category(L"FSL")
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
double fsl_black_put_delta(double f, double s, double k)
{
	if (f <= 0 || s <= 0 || k <= 0) {
		throw std::runtime_error("f, s, and k must be positive");
	}
	double z = fsl_black_moneyness(f, s, k);

	return - f * normal_cdf(z - s);
}
int test_fsl_black_put_delta()
{
	{
		double data[][4] = {
			// f, s, k, p 
			{100, .1, 100, 3.9877611676744920},
			// ...more tests here...
		};
		for (auto [f, s, k, p] : data) {
			double p_ = fsl_black_put_delta(f, s, k);
			assert(fabs(p - p_) <= 1e-8);
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
		.Category(L"FSL")
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

// Black-Scholes/Merton S_t = S exp((r - s^2/2) t + sigma B_t) where B_t is a standard Brownian motion.
// Put value is exp(-r t) E[max{k - S_t, 0}]
// If F = S_t then f = S exp(r t) and s = sigma sqrt(t).

// Convert Black-Scholes/Merton parameters to Black model.
std::tuple<double, double, double> fsl_bsm_black(double r, double S, double sigma, double t)
{
	if (S <= 0 || sigma <= 0 || t <= 0) {
		throw std::runtime_error("S, sigma, and t must be positive");
	}
	double D = exp(-r * t); // Discount factor
	double f = S / D; // Forward price
	double s = sigma * sqrt(t); // Volatility for Black model

	return { D, f, s };
}
double fsl_bsm_put_value(double r, double S, double sigma, double t, double k)
{
	auto [D, f, s] = fsl_bsm_black(r, S, sigma, t);

	return D * fsl_black_put_value(f, s, k);
}

// Run tests on xlAutoOpen
Auto<Open> xao_fsl_test([]() {
	try {
		test_normal_cdf();
		test_fsl_black_moneyness();
		test_fsl_black_put_value();
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