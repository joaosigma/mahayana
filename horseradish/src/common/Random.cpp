/*****************************   sfmt.cpp   ***********************************
* Authors:
* Mutsuo Saito (Hiroshima University)
* Makoto Matsumoto (Hiroshima University)
* Agner Fog (Copenhagen University College of Engineering)
* Date created:  2006
* Last modified: 2009-02-08
* Project:       randomc
* Platform:      This C++ version requires an x86 family microprocessor 
*                with the SSE2 or later instruction set and a compiler 
*                that supports intrinsic functions.
* Source URL:    www.agner.org/random
* Source URL for original C language implementation:
*                www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html
*
* Description:
* "SIMD-oriented Fast Mersenne Twister" (SFMT) random number generator.
* The SFMT random number generator is a modification of the Mersenne Twister 
* with improved randomness and speed, adapted to the SSE2 instruction set.
* The SFMT was invented by Mutsuo Saito and Makoto Matsumoto.
* The present C++ implementation is by Agner Fog.
*
* Class description and member functions: See sfmt.h
*
* Example:
* ========
* The file EX-RAN.CPP contains an example of how to generate random numbers.
*
* Library version:
* ================
* An optimized version of this random number generator is provided as function
* libraries in randoma.zip. These function libraries are coded in assembly
* language and support only x86 platforms, including 32-bit and 64-bit
* Windows, Linux, BSD, Mac OS-X (Intel based). Use randoma.h from randoma.zip
*
*
* Further documentation:
* ======================
* See the file ran-instructions.pdf for detailed instructions and documentation
*
*
* Copyright notice
* ================
* GNU General Public License http://www.gnu.org/licenses/gpl.html
* This C++ implementation of SFMT contains parts of the original C code
* which was published under the following BSD license, which is therefore
* in effect in addition to the GNU General Public License.
*
Copyright (c) 2006, 2007 by Mutsuo Saito, Makoto Matsumoto and Hiroshima University.
Copyright (c) 2008 by Agner Fog.
All rights reserved.
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:
    > Redistributions of source code must retain the above copyright notice, 
      this list of conditions and the following disclaimer.
    > Redistributions in binary form must reproduce the above copyright notice, 
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    > Neither the name of the Hiroshima University nor the names of its 
      contributors may be used to endorse or promote products derived from 
      this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "Random.hpp"
#include "Math.hpp"

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Funções auxiliares locais		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

// length of factorial table
static const int FAK_LEN = 1024;

// = 8/e
static const double SHAT1 = 2.943035529371538573;

// = 3-sqrt(12/e)
static const double SHAT2 = 0.8989161620588987408;

static
inline __m128i sfmt_recursion(__m128i const &a, __m128i const &b, __m128i const &c, __m128i const &d, __m128i const &mask)
{
    __m128i a1, b1, c1, d1, z1, z2;

    b1 = _mm_srli_epi32(b, SFMT_SR1);
    a1 = _mm_slli_si128(a, SFMT_SL2);
    c1 = _mm_srli_si128(c, SFMT_SR2);
    d1 = _mm_slli_epi32(d, SFMT_SL1);
    b1 = _mm_and_si128(b1, mask);
    z1 = _mm_xor_si128(a, a1);
    z2 = _mm_xor_si128(b1, d1);
    z1 = _mm_xor_si128(z1, c1);
    z2 = _mm_xor_si128(z1, z2);
    return z2;
}

// log factorial function. gives natural logarithm of n!
static
double lnFac(const HorseRadish::hInt32 &n)
{
	// define constants
	static const double        // coefficients in Stirling approximation     
		C0 =  0.918938533204672722,   // ln(sqrt(2*pi))
		C1 =  1.0/12.0, 
		C3 = -1.0/360.0;
	static double fac_table[FAK_LEN]; // table of ln(n!):
	static bool initialized = false;   // remember if fac_table has been initialized

	if (n < FAK_LEN)
	{
		if (n <= 1)
			return (n < 0) ? -1.0f : 0.0f;

		// first time. Must initialize table
		if (!initialized)
		{
			// make table of ln(n!)
			double sum = fac_table[0] = 0.0;
			for(int i=1; i<FAK_LEN; i++)
				{
				sum += log((double)i);
				fac_table[i] = sum;
				}
			initialized = true;
		}

		//prontos, posso sair
		return fac_table[n];
	}

	// not found in table. use Stirling approximation
	double  n1, r;
	n1 = n;
	r = 1.0/n1;
	return (n1 + 0.5)*log(n1) - n1 + C0 + r*(C1 + r*r*C3);
}

// subfunction used by hypergeometric and extended hypergeometric distribution
static
double fc_lnpk(const HorseRadish::hInt32 &k, const HorseRadish::hInt32 &L, const HorseRadish::hInt32 &m, const HorseRadish::hInt32 &n)
{	return (lnFac(k) + lnFac(m - k) + lnFac(n - k) + lnFac(L + k));}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ A classe propriamente dita		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
namespace HorseRadish
{

void Random::init()
{
   // Various initializations and period certification
   hUInt32 i, j, temp;

   // Initialize mask
   static const hUInt32 maskinit[4] = {SFMT_MASK};
   mask = _mm_loadu_si128((__m128i*)maskinit);

   // Period certification
   // Define period certification vector
   static const hUInt32 parityvec[4] = {SFMT_PARITY};

   // Check if parityvec & state[0] has odd parity
   temp = 0;
   for (i = 0; i < 4; i++) {
      temp ^= parityvec[i] & ((hUInt32*)state)[i];
   }
   for (i = 16; i > 0; i >>= 1) temp ^= temp >> i;
   if (!(temp & 1)) {
      // parity is even. Certification failed
      // Find a nonzero bit in period certification vector
      for (i = 0; i < 4; i++) {
         if (parityvec[i]) {
            for (j = 1; j; j <<= 1) {
               if (parityvec[i] & j) {
                  // Flip the corresponding bit in state[0] to change parity
                  ((hUInt32*)state)[i] ^= j;
                  // Done. Exit i and j loops
                  i = 5;  break;
               }
            }
         }
      }
   }

   // Generate first random numbers and set iState = 0
   generate();
}

void Random::generate()
{
   // Fill state array with new random numbers
   int i;
   __m128i r, r1, r2;

   r1 = state[SFMT_N - 2];
   r2 = state[SFMT_N - 1];
   for (i = 0; i < SFMT_N - SFMT_M; i++) {
      r = sfmt_recursion(state[i], state[i + SFMT_M], r1, r2, mask);
      state[i] = r;
      r1 = r2;
      r2 = r;
   }
   for (; i < SFMT_N; i++) {
      r = sfmt_recursion(state[i], state[i + SFMT_M - SFMT_N], r1, r2, mask);
      state[i] = r;
      r1 = r2;
      r2 = r;
   }
   iState = 0;
}

hInt32 Random::dPoissonLow(const double &lambda)
{
	/*
	This subfunction generates a random variate with the poisson 
		distribution for extremely low values of L.
	The method is a simple calculation of the probabilities of x = 1
		and x = 2. Higher values are ignored.
	The reason for using this method is to avoid the numerical inaccuracies 
		in other methods.
	*/

	double d,r;

	d = sqrt(lambda);
	if (this->NextDouble() >= d)
		return 0;
	r = this->NextDouble() * d;
	if (r > lambda * (1.0-lambda))
		return 0;
	if (r > 0.5 * lambda*lambda * (1.0 - lambda))
		return 1;
	return 2;
}

hInt32 Random::dPoissonInver(const double &lambda)
{
	/*
	This subfunction generates a random variate with the poisson 
		distribution using inversion by the chop down method (PIN).
	Execution time grows with L. Gives overflow for L > 80.
	The value of bound must be adjusted to the maximal value of L.
	*/

	const hInt32 bound = 130;		// safety bound. Must be > L + 8*sqrt(L).
	static double p_L_last = -1.0;	// previous value of L
	static double p_f0;				// value at x=0
	double r;						// uniform random number
	double f;						// function value
	hInt32 x;						// return value

	//ao principio
	if (lambda != p_L_last)
	{
		p_L_last = lambda;
		p_f0 = exp(-lambda);
	}// f(0) = probability of x=0

	//nunca saio até
	while (1)
	{  
		r=this->NextDouble();
		x=0;
		f=p_f0;

		// recursive calculation: f(x) = f(x-1) * L / x
		do{
			r -= f;
			if (r <= 0.0)
				return x;
			x++;
			f *= lambda;
			r *= x;
		}while (x <= bound);// instead of f /= x
	}
}

hInt32 Random::dPoissonRatioUniforms(const double &lambda)
{
	/*
	This subfunction generates a random variate with the poisson 
		distribution using the ratio-of-uniforms rejection method (PRUAt).
	Execution time does not depend on L, except that it matters whether L
		is within the range where ln(n!) is tabulated.
	Reference: E. Stadlober: "The ratio of uniforms approach for generating
		discrete random variates". Journal of Computational and Applied Mathematics,
		vol. 31, no. 1, 1990, pp. 181-189.
	*/

	static double p_L_last = -1.0;	// previous L
	static double p_a;				// hat center
	static double p_h;				// hat width
	static double p_g;				// ln(L)
	static double p_q;				// value at mode
	static hInt32 p_bound;			// upper bound
	hInt32 mode;					// mode
	double u;						// uniform random
	double lf;						// ln(f(x))
	double x;						// real sample
	hInt32 k;						// integer sample


	if (p_L_last != lambda)
	{
		p_L_last = lambda;							// Set-up
		p_a = lambda + 0.5;							// hat center
		mode = (hInt32)lambda;						// mode
		p_g  = log(lambda);
		p_q = mode * p_g - lnFac(mode);				// value at mode
		p_h = sqrt(SHAT1 * (lambda+0.5)) + SHAT2;	// hat width
		p_bound = (hInt32)(p_a + 6.0 * p_h);
	}	// safety-bound

	while(1)
	{
		// avoid division by 0
		u = this->NextDouble();
		if (u == 0.0)
			continue;

		// reject if outside valid range
		x = p_a + p_h * (this->NextDouble() - 0.5) / u;
		if (x < 0.0 || x >= p_bound)
			continue;

		k = (hInt32)(x);
		lf = k * p_g - lnFac(k) - p_q;

		// quick acceptance
		if (lf >= u * (4.0 - u) - 3.0)
			break;

		// quick rejection
		if (u * (u - lf) > 1.0)
			continue;

		// final acceptance
		if (2.0 * log(u) <= lf)
			break;
	}

	//valor final
	return(k);
}

hInt32 Random::dBinomialInver(const hInt32 &n, const double &p)
{
	/*
	Subfunction for Binomial distribution. Assumes p < 0.5.
	Uses inversion method by search starting at 0.
	Gives overflow for n*p > 60.
	This method is fast when n*p is low. 
	*/

	double f0, f, q, pn, r, rc; 
	hInt32 bound,x, n1, i;

	// f(0) = probability of x=0 is (1-p)^n
	// fast calculation of (1-p)^n
	f0 = 1.0;
	pn = 1.0-p;
	n1 = n;
	while (n1)
	{
		if ( (n1 & 1) )
			f0 *= pn;
		pn *= pn;
		n1 >>= 1;
	}

	// calculate safety bound
	rc = (n + 1) * p;
	bound = (hInt32)(rc + 11.0*(sqrt(rc) + 1.0));
	if (bound > n)
		bound = n;
	q = p / (1.0 - p);

	while (1)
	{
		r = this->NextDouble();

		// recursive calculation: f(x) = f(x-1) * (n-x+1)/x*p/(1-p)
		f = f0;
		x = 0;
		i = n;
		do {
			r -= f;
			if (r <= 0.0)
				return x;
			x++;
			f *= q * i;
			r *= x;       // it is faster to multiply r by x than dividing f by x
			i--;
		}while (x <= bound);
	}
}

hInt32 Random::dBinomialRatioOfUniforms(const hInt32 &n, const double &p)
{
	/* 
	Subfunction for Binomial distribution. Assumes p < 0.5.
	Uses the Ratio-of-Uniforms rejection method (BRUAt).
	The computation time hardly depends on the parameters, except that it matters
		a lot whether parameters are within the range where the LnFac function is 
		tabulated.
	Reference: E. Stadlober: "The ratio of uniforms approach for generating
		discrete random variates". Journal of Computational and Applied Mathematics,
		vol. 31, no. 1, 1990, pp. 181-189.
	*/

	static hInt32 b_n_last = -1;               // last n
	static double b_p_last = -1.;             // last p
	static hInt32 b_mode;                      // mode
	static hInt32 b_bound;                     // upper bound
	static double b_a;                        // hat center
	static double b_h;                        // hat width
	static double b_g;                        // value at mode
	static double b_r1;                       // ln(p/(1-p))
	double u;                                 // uniform random
	double q1;                                // 1-p
	double np;                                // n*p
	double var;                               // variance
	double lf;                                // ln(f(x))
	double x;                                 // real sample
	hInt32 k;                                  // integer sample

	if(b_n_last != n || b_p_last != p)
	{      // Set_up
		b_n_last = n;
		b_p_last = p;
		q1 = 1.0 - p;
		np = n * p;
		b_mode = (hInt32)(np + p);               // mode
		b_a = np + 0.5;                         // hat center
		b_r1 = log(p / q1);
		b_g = lnFac(b_mode) + lnFac(n-b_mode);
		var = np * q1;                          // variance
		b_h = sqrt(SHAT1 * (var+0.5)) + SHAT2;  // hat width
		b_bound = (hInt32)(b_a + 6.0 * b_h);     // safety-bound
		if (b_bound > n)
			b_bound = n;
	}
    
	// rejection loop
	while (1)
	{
		// avoid division by 0
		u = this->NextDouble();
		if (u == 0.0)
			continue;

		// reject, avoid overflow
		x = b_a + b_h * (this->NextDouble() - 0.5) / u;
		if (x < 0.0 || x > b_bound)
			continue;

		// truncate and the = ln(f(k))
		k = (hInt32)x;
		lf = (k-b_mode)*b_r1+b_g-lnFac(k)-lnFac(n-k);

		// lower squeeze accept
		if (u * (4.0 - u) - 3.0 <= lf)
			break;
		// upper squeeze reject
		if (u * (u - lf) > 1.0)
			continue;
		// final acceptance
		if (2.0 * log(u) <= lf)
			break;
	}

	return k;
}

hInt32 Random::dHypRatioOfUnifoms(const hInt32 &n, const hInt32 &m, const hInt32 &N)
{
	/* 
	Subfunction for Hypergeometric distribution using the ratio-of-uniforms
		rejection method.
	This code is valid for 0 < n <= m <= N/2.
	The computation time hardly depends on the parameters, except that it matters
		a lot whether parameters are within the range where the LnFac function is 
		tabulated.
	Reference: E. Stadlober: "The ratio of uniforms approach for generating
		discrete random variates". Journal of Computational and Applied Mathematics,
		vol. 31, no. 1, 1990, pp. 181-189.
	*/

	static hInt32 h_N_last = -1, h_m_last = -1, h_n_last = -1;// previous parameters
	static hInt32 h_bound;                                    // upper bound
	static double h_a;                                       // hat center
	static double h_h;                                       // hat width
	static double h_g;                                       // value at mode
	hInt32 L;                                                 // N-m-n
	hInt32 mode;                                              // mode
	hInt32 k;                                                 // integer sample
	double x;                                                // real sample
	double rNN;                                              // 1/(N*(N+2))
	double my;                                               // mean
	double var;                                              // variance
	double u;                                                // uniform random
	double lf;                                               // ln(f(x))

	L = N - m - n;
	if (h_N_last != N || h_m_last != m || h_n_last != n)
	{
		h_N_last = N;
		h_m_last = m;
		h_n_last = n;
		rNN = 1. / ((double)N*(N+2));                          // make two divisions in one
		my = (double)n * m * rNN * (N+2);                      // mean = n*m/N
		mode = (hInt32)(double(n+1) * double(m+1) * rNN * N);   // mode = floor((n+1)*(m+1)/(N+2))
		var = (double)n * m * (N-m) * (N-n) / ((double)N*N*(N-1)); // variance
		h_h = sqrt(SHAT1 * (var+0.5)) + SHAT2;                 // hat width
		h_a = my + 0.5;                                        // hat center
		h_g = fc_lnpk(mode, L, m, n);                          // maximum
		h_bound = (hInt32)(h_a + 4.0 * h_h);                    // safety-bound
		if (h_bound > n)
			h_bound = n;
	}

	while(1)
	{
		// uniform random number and avoid division by 0
		u = this->NextDouble();                                          
		if (u == 0)
			continue;

		// generate hat distribution and reject to avoid overflow
		x = h_a + h_h * (this->NextDouble()-0.5) / u;
		if (x < 0.0 || x > 2e9)
			continue;

		// reject if outside range
		k = (hInt32)x;
		if (k > h_bound)
			continue;
		
		// ln(f(k))
		lf = h_g - fc_lnpk(k,L,m,n);

		// lower squeeze accept
		if (u * (4.0 - u) - 3.0 <= lf)
			break;
		// upper squeeze reject
		if (u * (u-lf) > 1.0)
			continue;
		// final acceptance
		if (2.0 * log(u) <= lf)
			break;
	}

	return k;
}

hInt32 Random::dHypInversionMod(const hInt32 &n, const hInt32 &m, const hInt32 &N)
{
	/* 
	Subfunction for Hypergeometric distribution. Assumes 0 <= n <= m <= N/2.
	Overflow protection is needed when N > 680 or n > 75.
	Hypergeometric distribution by inversion method, using down-up 
		search starting at the mode using the chop-down technique.
	This method is faster than the rejection method when the variance is low.
	*/   

	static hInt32  h_n_last = -1, h_m_last = -1, h_N_last = -1;
	static hInt32  h_mode, h_mp, h_bound;
	static double h_fm;
	hInt32 L, I, K;
	double modef, Mp, np, p, c, d, U, divisor;

	Mp = (double)(m + 1);
	np = (double)(n + 1);
	L = N - m - n;
  
	if (N != h_N_last || m != h_m_last || n != h_n_last)
	{
		// set-up when parameters have changed
		h_N_last = N;
		h_m_last = m;
		h_n_last = n;

		p  = Mp / (N + 2.0);
		modef = np * p;                                   // mode, real
		h_mode = (hInt32)modef;                            // mode, integer
		if (h_mode == modef && p == 0.5)   
			h_mp = h_mode--;
		else
			h_mp = h_mode + 1;

		// mode probability, using log factorial function
		// (may read directly from fac_table if N < FAK_LEN)
		h_fm = exp(lnFac(N-m) - lnFac(L+h_mode) - lnFac(n-h_mode)
				 + lnFac(m)   - lnFac(m-h_mode) - lnFac(h_mode)
				 - lnFac(N)   + lnFac(N-n)      + lnFac(n)        );

		// safety bound - guarantees at least 17 significant decimal digits
		// bound = min(n, (hInt32)(modef + k*c'))
		h_bound = (hInt32)(modef + 11.0 * sqrt(modef * (1.0-p) * (1.0-n/(double)N)+1.0));
		if (h_bound > n)
			h_bound = n;
	}

	// loop until accepted
	while(1)
	{
		// uniform random number to be converted
		U = this->NextDouble();

		if ((U -= h_fm) <= 0.0)
			return(h_mode);
		c = d = h_fm;

		// alternating down- and upward search from the mode
		for(I=1; I<=h_mode; I++)
		{
			K  = h_mp - I;
			divisor = (np - K)*(Mp - K);
			// Instead of dividing c with divisor, we multiply U and d because 
			// multiplication is faster. This will give overflow if N > 800
			U *= divisor;
			d *= divisor;
			c *= ((double)K) * ((double)(L + K));
			if ((U -= c) <= 0.0)
				return (K - 1);

			K  = h_mode + I;                                // upward search
			divisor = ((double)K) * ((double)(L + K));
			U *= divisor;  c *= divisor; // re-scale parameters to avoid time-consuming division
			d *= (np - K) * (Mp - K);
			if ((U -= d) <= 0.0)
				return(K);
			// Values of n > 75 or N > 680 may give overflow if you leave out this..
			// overflow protection
			// if (U > 1.E100) {U *= 1.E-100; c *= 1.E-100; d *= 1.E-100;}
		}

		// upward search from K = 2*mode + 1 to K = bound
		for(K = h_mp + h_mode; K <= h_bound; K++)
		{
			divisor = ((double)K) * ((double)(L + K));
			U *= divisor;
			d *= (np - K) * (Mp - K);
			if ((U -= d) <= 0.0)
				return(K);
			// more overflow protection
			// if (U > 1.E100) {U *= 1.E-100; d *= 1.E-100;}
		}
	}
}

void Random::ReSeed(int seed)
{
   hUInt32 y = seed;                  // Temporary
   hUInt32 statesize = SFMT_N*4;      // Size of state vector

   // Fill state vector with random numbers from seed
   ((hUInt32*)state)[0] = y;
   const hUInt32 factor = 1812433253U;// Multiplication factor

   for (int i = 1; i < statesize; i++) {
      y = factor * (y ^ (y >> 30)) + i;
      ((hUInt32*)state)[i] = y;
   }

   // Further initialization and period certification
   init();
}

void Random::ReSeedByArray(const int* const seeds, int numSeeds)
{
   // Seed by more than 32 bits
   hUInt32 i, j, count, r, lag;

   if (numSeeds < 0)
	   numSeeds = 0;

   const hUInt32 size = SFMT_N*4; // number of 32-bit integers in state

   // Typecast state to hUInt32 *
   hUInt32 * sta = (hUInt32*)state;

   if (size >= 623)
      lag = 11; 
   else if (size >= 68)
      lag = 7;
   else if (size >= 39)
      lag = 5;
   else
      lag = 3;

   const hUInt32 mid = (size - lag) / 2;

   if ((hUInt32)numSeeds + 1 > size)
      count = (hUInt32)numSeeds;
   else
      count = size - 1;

   // 1. loop: Fill state vector with random numbers from numSeeds
   const hUInt32 factor = 1812433253U;// Multiplication factor
   r = (hUInt32)numSeeds;
   for (i = 0; i < SFMT_N*4; i++) {
      r = factor * (r ^ (r >> 30)) + i;
      sta[i] = r;
   }

   // 2. loop: Fill state vector with random numbers from seeds[]
   for (i = 1, j = 0; j < count; j++)
   {
      r = sta[i] ^ sta[(i + mid) % size] ^ sta[(i + size - 1) % size];
	  r = (r ^ (r >> 27)) * 1664525U;

      sta[(i + mid) % size] += r;
      if (j < (hUInt32)numSeeds) r += (hUInt32)seeds[j];
      r += i;
      sta[(i + mid + lag) % size] += r;
      sta[i] = r;
      i = (i + 1) % size;
   }

   // 3. loop: Randomize some more
   for (j = 0; j < size; j++)
   {
      r = sta[i] + sta[(i + mid) % size] + sta[(i + size - 1) % size];
	  r = (r ^ (r >> 27)) * 1566083941U;

      sta[(i + mid) % size] ^= r;
      r -= i;
      sta[(i + mid + lag) % size] ^= r;
      sta[i] = r;
      i = (i + 1) % size;
   }
   
   // Further initialization and period certification
   init();
}

int Random::NextInteger()
{
	// Output random integer in the interval min <= x <= max
	// Each output value has exactly the same probability.
	// This is obtained by rejecting certain bit values so that the number
	// of possible bit values is divisible by the interval length
	
	// Assume 64 bit integers supported. Use multiply and shift method
	hUInt32 interval;                  // Length of interval
	hUInt64 longran;                   // Random bits * interval
	hUInt32 iran;                      // Longran / 2^32
	hUInt32 remainder;                 // Longran % 2^32

	interval = (hUInt32)(42949672935);
	if (interval != lastInterval)
	{
		// Interval length has changed. Must calculate rejection limit
		// Reject when remainder = 2^32 / interval * interval
		// rLimit will be 0 if interval is a power of 2. No rejection then.
		rLimit = (hUInt32)(((hUInt64)1 << 32) / interval) * interval - 1;
		lastInterval = interval;
	}
	
	do
	{ 
		// Rejection loop
		longran  = (hUInt64)Next32Bits() * interval;
		iran = (hUInt32)(longran >> 32);
		remainder = (hUInt32)longran;
	}while (remainder > rLimit);

	// Convert back to signed and return result
	return (hInt32)iran;
}

int Random::NextInteger(int max)
{
	//basta fazer esta conta
	return this->NextInteger(0, max);
}

int Random::NextInteger(int min, int max)
{
	// Output random integer in the interval min <= x <= max
	// Each output value has exactly the same probability.
	// This is obtained by rejecting certain bit values so that the number
	// of possible bit values is divisible by the interval length
	
	if (max <= min)
		return ((max == min) ? min : 0x80000000);

	// Assume 64 bit integers supported. Use multiply and shift method
	hUInt32 interval;                  // Length of interval
	hUInt64 longran;                   // Random bits * interval
	hUInt32 iran;                      // Longran / 2^32
	hUInt32 remainder;                 // Longran % 2^32

	interval = (hUInt32)(max - min + 1);
	if (interval != lastInterval)
	{
		// Interval length has changed. Must calculate rejection limit
		// Reject when remainder = 2^32 / interval * interval
		// rLimit will be 0 if interval is a power of 2. No rejection then.
		rLimit = (hUInt32)(((hUInt64)1 << 32) / interval) * interval - 1;
		lastInterval = interval;
	}
	
	do
	{ 
		// Rejection loop
		longran  = (hUInt64)Next32Bits() * interval;
		iran = (hUInt32)(longran >> 32);
		remainder = (hUInt32)longran;
	}while (remainder > rLimit);

	// Convert back to signed and return result
	return (hInt32)iran + min;
}

double Random::NextDouble()
{
	// Output random floating point number
	if (iState >= SFMT_N*4-1)
		generate();

	hUInt64 r = *(hUInt64*)((hUInt32*)state+iState);
	iState += 2;

	// 53 bits resolution:
	// return (hInt64)(r >> 11) * (1./(67108864.0*134217728.0)); // (r >> 11)*2^(-53)
	// 52 bits resolution for compatibility with assembly version:
	return (hInt64)(r >> 12) * (1./(67108864.0*67108864.0));  // (r >> 12)*2^(-52)
}

double Random::NextDouble(double max)
{
	//basta fazer esta conta
	return (this->NextDouble() * max);
}

double Random::NextDouble(double min, double max)
{
	//basta fazer esta conta
	return (min + this->NextDouble() * (max - min));
}

hUInt32 Random::Next32Bits()
{
   // Output 32 random bits
   hUInt32 y;

   if (iState >= SFMT_N*4)
      generate();
   
   y = ((hUInt32*)state)[iState++];
   return y;
}

double Random::GetDistributionNormal(const double mean, const double standardDeviation)
{
	static bool normal_x2_valid = false;
	static double normal_x2;
	double normal_x1;          // first random coordinate (normal_x2 is member of class)
	double w;                  // radius
	
	// we have a valid result from last call
	if (normal_x2_valid)
		{
		normal_x2_valid = false;
		return (normal_x2 * standardDeviation + mean);
		}

	// make two normally distributed variates by Box-Muller transformation
	do {
		normal_x1 = 2.0 * this->NextDouble() - 1.0;
		normal_x2 = 2.0 * this->NextDouble() - 1.0;
		w = normal_x1*normal_x1 + normal_x2*normal_x2;
	}while (w >= 1.0 || w < 1e-30);

	w = sqrt(log(w)*(-2.0/w));
	normal_x1 *= w;
	normal_x2 *= w;			// normal_x1 and normal_x2 are independent normally distributed variates
	normal_x2_valid = true;	// save normal_x2 for next call
	return (normal_x1 * standardDeviation + mean);
}

bool Random::GetDistributionBernoulli(const double pSuccess)
{
	// Bernoulli distribution with parameter p. This function returns 
	// 0 or 1 with probability (1-p) and p, respectively.
	if (pSuccess<0.0 || pSuccess>1.0)
		return 0;
	return (this->NextDouble() < pSuccess);
}

hInt32 Random::GetDistributionPoisson(const double lambda)
{
	/*
	This function generates a random variate with the poisson distribution.
	Uses inversion by chop-down method for L < 17, and ratio-of-uniforms
		method for L >= 17.
	For L < 1.E-6 numerical inaccuracy is avoided by direct calculation.
	*/
 
	//se estiver abaixo de 17.0f
	if (lambda<17.0f)
	{
		//se for muito pequeno
		if (lambda<1.e-6)
		{
			if (lambda==0.0f)
				return 0;
			if (lambda<0.0f)
				return 0;

			// = calculate probabilities =
			// For extremely small L we calculate the probabilities of x = 1
			//		and x = 2 (ignoring higher x). The reason for using this 
			//		method is to prevent numerical inaccuracies in other methods.
			return dPoissonLow(lambda);
		}

		// = inversion method =
		// The computation time for this method grows with L.
		// Gives overflow for L > 80
		return dPoissonInver(lambda);
	}
	    
	//se for muito grande
	if (lambda>2.e9)
		return -1;

	// = ratio-of-uniforms method =
	// The computation time for this method does not depend on L.
	// Use where other methods would be slower.
	return dPoissonRatioUniforms(lambda);
}

hInt32 Random::GetDistributionBinomial(const long nDraw, const double pChange)
{
	/*
	This function generates a random variate with the binomial distribution.
	Uses inversion by chop-down method for n*p < 35, and ratio-of-uniforms
		method for n*p >= 35.
	For n*p < 1.E-6 numerical inaccuracy is avoided by poisson approximation.
	*/

	hInt32 x,inv;
	double np,pChange2;

	//initiate
	np=nDraw*pChange;
	pChange2=pChange;
	inv=0;

	// faster calculation by inversion
	if (pChange2>0.5)
	{
		pChange2=1.0-pChange2;
		inv=1;
	}

	//stupid bounds check
	if (nDraw<=0.0 || pChange2<=0.0)
	{
		// only one possible result
		if (nDraw==0.0 || pChange2==0.0)
			return inv * nDraw;
		return -1;
	}

	//choose method
	if (np < 35.)
	{
		// Poisson approximation for extremely low np
		if (np < 1.e-6)
			x = dPoissonLow(np);
		// inversion method, using chop-down search from 0
		else
			x = dBinomialInver(nDraw, pChange2);
	}
	else
	{
		// ratio of uniforms method
		x = dBinomialRatioOfUniforms(nDraw, pChange2);
	}

	// undo inversion
	if (inv)
		x = nDraw - x;
	return x;
}

hInt32 Random::GetDistributionHipergeometric(const long nDraw, const long nTotal, const long nUniverse)
{
	/*
	This function generates a random variate with the hypergeometric
		distribution. This is the distribution you get when drawing balls without 
		replacement from an urn with two colors. n is the number of balls you take,
		m is the number of red balls in the urn, N is the total number of balls in 
		the urn, and the return value is the number of red balls you get.
	This function uses inversion by chop-down search from the mode when
		parameters are small, and the ratio-of-uniforms method when the former
		method would be too slow or would give overflow.
	*/   

	hInt32 nDrawUse,nTotalUse;
	hInt32 fak, addd;           // used for undoing transformations
	hInt32 x;                   // result

	// check if parameters are valid
	if (nDraw > nUniverse || nTotal > nUniverse || nDraw < 0 || nTotal < 0)
		return 0;

	//posso trocar os valores
	nDrawUse=nDraw;
	nTotalUse=nTotal;

	// symmetry transformations
	fak = 1;
	addd = 0;

	// invert nTotalUse
	if (nTotalUse > nUniverse/2)
	{
		nTotalUse = nUniverse - nTotalUse;
		fak = -1;
		addd = nDrawUse;
	}

	// invert nDrawUse
	if (nDrawUse > nUniverse/2)
	{
		nDrawUse = nUniverse - nDrawUse;
		addd += fak * nTotalUse;  fak = - fak;
	}

	// swap nDrawUse and nTotalUse
	if (nDrawUse > nTotalUse)
	{
		x = nDrawUse;
		nDrawUse = nTotalUse;
		nTotalUse = x;
	}

	// cases with only one possible result end here
	if (nDrawUse == 0)
		return addd;

	// use ratio-of-uniforms method
	if (nUniverse > 680 || nDrawUse > 70)
		x = dHypRatioOfUnifoms (nDrawUse, nTotalUse, nUniverse);
	// inversion method, using chop-down search from mode
	else
		x = dHypInversionMod (nDrawUse, nTotalUse, nUniverse);

	// undo symmetry transformations  
	return x * fak + addd;
}

}//namespace HorseRadish