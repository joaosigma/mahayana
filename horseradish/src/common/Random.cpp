#include "Random.hpp"

namespace HorseRadish
{
	Random::Random()
	{
		std::random_device rd;
		mtwister.seed(rd());
	}

	int Random::nextInteger()
	{
		std::uniform_int_distribution<> dist;
		return dist(mtwister);
	}

	int Random::nextInteger(int max)
	{
		std::uniform_int_distribution<> dist(0, max);
		return dist(mtwister);
	}

	int Random::nextInteger(int min, int max)
	{
		std::uniform_int_distribution<> dist(min, max);
		return dist(mtwister);
	}

	double Random::nextDouble()
	{
		std::uniform_real_distribution<> dist;
		return dist(mtwister);
	}

	double Random::nextDouble(double max)
	{
		std::uniform_real_distribution<> dist(0.0, max);
		return dist(mtwister);
	}

	double Random::nextDouble(double min, double max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return dist(mtwister);
	}

	double Random::distributionNormal(double mean, double standardDeviation)
	{
		std::normal_distribution<> dist(mean, standardDeviation);
		return dist(mtwister);
	}

	bool Random::distributionBernoulli(double pSuccess)
	{
		std::bernoulli_distribution dist(pSuccess);
		return dist(mtwister);
	}

	int Random::distributionPoisson(double mean)
	{
		std::poisson_distribution<> dist(mean);
		return dist(mtwister);
	}

	int Random::distributionBinomial(int nDraw, double pChange)
	{
		std::binomial_distribution<> dist(nDraw, pChange);
		return dist(mtwister);
	}

} //HorseRadish