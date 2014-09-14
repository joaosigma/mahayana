#include "Random.hpp"

namespace HorseRadish
{
	Random::Random()
	{
		std::random_device rd;
		mtwister.seed(rd());
	}

	int Random::NextInteger()
	{
		std::uniform_int_distribution<> dist;
		return dist(mtwister);
	}

	int Random::NextInteger(int max)
	{
		std::uniform_int_distribution<> dist(0, max);
		return dist(mtwister);
	}

	int Random::NextInteger(int min, int max)
	{
		std::uniform_int_distribution<> dist(min, max);
		return dist(mtwister);
	}

	double Random::NextDouble()
	{
		std::uniform_real_distribution<> dist;
		return dist(mtwister);
	}

	double Random::NextDouble(double max)
	{
		std::uniform_real_distribution<> dist(0.0, max);
		return dist(mtwister);
	}

	double Random::NextDouble(double min, double max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return dist(mtwister);
	}

	double Random::GetDistributionNormal(const double mean, const double standardDeviation)
	{
		std::normal_distribution<> dist(mean, standardDeviation);
		return dist(mtwister);
	}

	bool Random::GetDistributionBernoulli(const double pSuccess)
	{
		std::bernoulli_distribution dist(pSuccess);
		return dist(mtwister);
	}

	int Random::GetDistributionPoisson(const double mean)
	{
		std::poisson_distribution<> dist(mean);
		return dist(mtwister);
	}

	int Random::GetDistributionBinomial(const int nDraw, const double pChange)
	{
		std::binomial_distribution<> dist(nDraw, pChange);
		return dist(mtwister);
	}

} //HorseRadish