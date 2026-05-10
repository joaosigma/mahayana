export module core:random;

import std;

export namespace hr
{
    class Random
    {
    private:
        std::mt19937_64 mtwister;

    public:
        Random();

        int nextInteger();
        int nextInteger(int max);
        int nextInteger(int min, int max);

        double nextDouble();
        double nextDouble(double max);
        double nextDouble(double min, double max);

        double distributionNormal(double mean, double standardDeviation);
        bool distributionBernoulli(double pSuccess);
        int distributionPoisson(double mean);
        int distributionBinomial(int nDraw, double pChange);
    };
}
