#pragma once
#ifndef __HRANDOM__
#define __HRANDOM__

#include <random>

namespace HorseRadish
{
	class Random
	{
	private:
		std::mt19937_64 mtwister;

	public:
		Random();

		int NextInteger();
		int NextInteger(int max);
		int NextInteger(int min, int max);

		double NextDouble();
		double NextDouble(double max);
		double NextDouble(double min, double max);

		double GetDistributionNormal(const double mean, const double standardDeviation);
		bool GetDistributionBernoulli(const double pSuccess);
		int GetDistributionPoisson(const double mean);
		int GetDistributionBinomial(const int nDraw, const double pChange);
	};

} //namespace HorseRadish

#endif
