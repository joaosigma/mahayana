#include "Image.hpp"

#include "Math.hpp"
#include "Color.hpp"
#include "BAreas.hpp"

#include <algorithm>

namespace HorseRadish
{
	namespace Imaging
	{
		int Image::getNumComponentes(const ImageFormat format)
		{
			switch (format)
			{
			case Image::RGB:
			case Image::BGR:
				return 3;

			case Image::RGBA:
			case Image::BGRA:
			case Image::CMYK:
				return 4;

			case Image::Red:
			case Image::Green:
			case Image::Blue:
			case Image::Alpha:
			case Image::Lum:
				return 1;

			case Image::LumAlpha:
			case Image::RedGreen:
				return 2;
			}

			return 0;
		}

		int Image::getCorrectIndex(const ImageFormat ofWhat, const ImageFormat inFormat)
		{
			if (ofWhat == Image::Red)
			{
				switch (inFormat){
				case Image::RGB:
				case Image::RGBA:
					return 0;
				case Image::BGR:
				case Image::BGRA:
					return 2;
				case Image::Red:
				case Image::Lum:
				case Image::LumAlpha:
				case Image::RedGreen:
					return 0;
				}

				return -1;
			}

			if (ofWhat == Image::Green)
			{
				switch (inFormat){
				case Image::RGB:
				case Image::RGBA:
				case Image::BGR:
				case Image::BGRA:
				case Image::RedGreen:
					return 1;
				case Image::Green:
				case Image::Lum:
				case Image::LumAlpha:
					return 0;
				}

				return -1;
			}

			if (ofWhat == Image::Blue)
			{
				switch (inFormat){
				case Image::RGB:
				case Image::RGBA:
					return 2;
				case Image::BGR:
				case Image::BGRA:
					return 0;
				case Image::Blue:
				case Image::Lum:
				case Image::LumAlpha:
					return 0;
				}

				return -1;
			}

			if (ofWhat == Image::Alpha)
			{
				switch (inFormat){
				case Image::RGBA:
				case Image::BGRA:
					return 3;
				case Image::Alpha:
					return 0;
				case Image::LumAlpha:
					return 1;
				}

				return -1;
			}

			return -1;
		}

		float Image::kernelBSpline(const float x)
		{
			if (x > 2.0f)
				return 0.0f;

			float a, b, c, d;
			float xm1 = x - 1.0f;
			float xp1 = x + 1.0f;
			float xp2 = x + 2.0f;

			a = b = c = d = 0.0f;

			if (xp2 > 0.0f)
				a = xp2*xp2*xp2;
			if (xp1 > 0.0f)
				b = xp1*xp1*xp1;
			if (x > 0.0f)
				c = x*x*x;
			if (xm1 > 0.0f)
				d = xm1*xm1*xm1;

			return (0.16666666666666666667f * (a - (4.0f * b) + (6.0f * c) - (4.0f * d)));
		}
		float Image::kernelLinear(const float t)
		{
			if (t < -1.0f)
				return 0.0f;
			if (t < 0.0f)
				return 1.0f + t;
			if (t < 1.0f)
				return 1.0f - t;
			return 0.0f;
		}
		float Image::kernelCubic(const float t)
		{
			float abs_t = (float)HorseRadish::Math::fAbs(t);
			float abs_t_sq = abs_t * abs_t;

			if (abs_t < 1.0f)
				return 1 - 2 * abs_t_sq + abs_t_sq*abs_t;
			if (abs_t < 2.0f)
				return 4 - 8 * abs_t + 5 * abs_t_sq - abs_t_sq*abs_t;
			return 0;
		}
		float Image::kernelGeneralizedCubic(const float t, const float a)
		{
			float abs_t = (float)HorseRadish::Math::fAbs(t);
			float abs_t_sq = abs_t * abs_t;

			if (abs_t < 1.0f)
				return (a + 2)*abs_t_sq*abs_t - (a + 3)*abs_t_sq + 1;
			if (abs_t < 2.0f)
				return a*abs_t_sq*abs_t - 5 * a*abs_t_sq + 8 * a*abs_t - 4 * a;
			return 0;
		}
		float Image::kernelLanczosSinc(const float t, const float r)
		{
			if (HorseRadish::Math::fAbs(t) > r)
				return 0;

			if (HorseRadish::Math::isZero(t) == true)
				return 1;

			float pit = HorseRadish::Math::PI*t;
			float pitd = pit / r;
			return (float)((HorseRadish::Math::sinR(pit) / pit) * (HorseRadish::Math::sinR(pitd) / pitd));
		}
		float Image::kernelBox(const float x)
		{
			if (x < -0.5f)
				return 0.0f;
			if (x < 0.5f)
				return 1.0f;
			return 0.0f;
		}
		float Image::kernelHermite(const float x)
		{
			if (x < -1.0f)
				return 0.0f;
			if (x < 0.0f)
				return (-2.0f*x - 3.0f)*x*x + 1.0f;
			if (x < 1.0f)
				return (2.0f*x - 3.0f)*x*x + 1.0f;
			return 0.0f;
		}
		float Image::kernelHanning(const float x)
		{
			if (HorseRadish::Math::fAbs(x) > 1.0f)
				return 0.0f;
			return (0.5f + 0.5f*(float)HorseRadish::Math::cosR(HorseRadish::Math::PI*x))*((float)HorseRadish::Math::sinR(HorseRadish::Math::PI*x) / (HorseRadish::Math::PI*x));
		}
		float Image::kernelHamming(const float x)
		{
			if (x < -1.0f)
				return 0.0f;
			if (x < 0.0f)
				return 0.92f*(-2.0f*x - 3.0f)*x*x + 1.0f;
			if (x < 1.0f)
				return 0.92f*(2.0f*x - 3.0f)*x*x + 1.0f;
			return 0.0f;
		}
		float Image::kernelSinc(const float x)
		{
			if (x == 0.0)
				return 1.0f;
			return((float)HorseRadish::Math::sinR(HorseRadish::Math::PI*x) / (HorseRadish::Math::PI*x));
		}
		float Image::kernelBlackman(const float x)
		{
			return (0.42f + 0.5f*(float)HorseRadish::Math::cosR(HorseRadish::Math::PI*x) + 0.08f*(float)HorseRadish::Math::cosR(2.0f*HorseRadish::Math::PI*x));
		}
		float Image::kernelBessel_J1(const float x)
		{
			double p, q;

			static const double
				Pone[] =
			{
				0.581199354001606143928050809e+21,
				-0.6672106568924916298020941484e+20,
				0.2316433580634002297931815435e+19,
				-0.3588817569910106050743641413e+17,
				0.2908795263834775409737601689e+15,
				-0.1322983480332126453125473247e+13,
				0.3413234182301700539091292655e+10,
				-0.4695753530642995859767162166e+7,
				0.270112271089232341485679099e+4
			},
			Qone[] =
				{
					0.11623987080032122878585294e+22,
					0.1185770712190320999837113348e+20,
					0.6092061398917521746105196863e+17,
					0.2081661221307607351240184229e+15,
					0.5243710262167649715406728642e+12,
					0.1013863514358673989967045588e+10,
					0.1501793594998585505921097578e+7,
					0.1606931573481487801970916749e+4,
					0.1e+1
				};

				p = Pone[8];
				q = Qone[8];
				for (int i = 7; i >= 0; i--)
				{
					p = p*x*x + Pone[i];
					q = q*x*x + Qone[i];
				}
				return (float)(p / q);
		}
		float Image::kernelBessel_P1(const float x)
		{
			double p, q;

			static const double
				Pone[] =
			{
				0.352246649133679798341724373e+5,
				0.62758845247161281269005675e+5,
				0.313539631109159574238669888e+5,
				0.49854832060594338434500455e+4,
				0.2111529182853962382105718e+3,
				0.12571716929145341558495e+1
			},
			Qone[] =
				{
					0.352246649133679798068390431e+5,
					0.626943469593560511888833731e+5,
					0.312404063819041039923015703e+5,
					0.4930396490181088979386097e+4,
					0.2030775189134759322293574e+3,
					0.1e+1
				};

				p = Pone[5];
				q = Qone[5];
				for (int i = 4; i >= 0; i--)
				{
					p = p*(8.0 / x)*(8.0 / x) + Pone[i];
					q = q*(8.0 / x)*(8.0 / x) + Qone[i];
				}
				return (float)(p / q);
		}
		float Image::kernelBessel_Q1(const float x)
		{
			double p, q;

			static const double
				Pone[] =
			{
				0.3511751914303552822533318e+3,
				0.7210391804904475039280863e+3,
				0.4259873011654442389886993e+3,
				0.831898957673850827325226e+2,
				0.45681716295512267064405e+1,
				0.3532840052740123642735e-1
			},
			Qone[] =
				{
					0.74917374171809127714519505e+4,
					0.154141773392650970499848051e+5,
					0.91522317015169922705904727e+4,
					0.18111867005523513506724158e+4,
					0.1038187585462133728776636e+3,
					0.1e+1
				};

				p = Pone[5];
				q = Qone[5];
				for (int i = 4; i >= 0; i--)
				{
					p = p*(8.0 / x)*(8.0 / x) + Pone[i];
					q = q*(8.0 / x)*(8.0 / x) + Qone[i];
				}
				return (float)(p / q);
		}
		float Image::kernelBessel_Order1(float x)
		{
			float p, q;

			if (HorseRadish::Math::isZero(x) == true)
				return (0.0f);
			p = x;
			if (x < 0.0)
				x = (-x);
			if (x < 8.0)
				return(p*Image::kernelBessel_J1(x));
			q = (float)sqrt(2.0f / (HorseRadish::Math::PI*x))*(float)(Image::kernelBessel_P1(x)*(1.0f / sqrt(2.0f)*(HorseRadish::Math::sinR(x) - cos(x))) - 8.0f / x*Image::kernelBessel_Q1(x)*
				(-1.0f / sqrt(2.0f)*(HorseRadish::Math::sinR(x) + HorseRadish::Math::cosR(x))));
			if (p < 0.0f)
				q = (-q);
			return (q);
		}
		float Image::kernelBessel(const float x)
		{
			if (x == 0.0f)
				return(HorseRadish::Math::PI / 4.0f);
			return(Image::kernelBessel_Order1(HorseRadish::Math::PI*x) / (2.0f*x));
		}
		float Image::kernelGaussian(const float x)
		{
			return (float)(exp(-2.0f*x*x)*0.79788456080287f);
		}
		float Image::kernelQuadratic(const float x)
		{
			if (x < -1.5f)
				return(0.0f);
			if (x < -0.5f)
				return(0.5f*(x + 1.5f)*(x + 1.5f));
			if (x < 0.5f)
				return(0.75f - x*x);
			if (x < 1.5f)
				return(0.5f*(x - 1.5f)*(x - 1.5f));
			return(0.0f);
		}
		float Image::kernelMitchell(const float x)
		{
#define KM_B (1.0f/3.0f)
#define KM_C (1.0f/3.0f)
#define KM_P0 ((  6.0f - 2.0f * KM_B ) / 6.0f)
#define KM_P2 ((-18.0f + 12.0f * KM_B + 6.0f * KM_C) / 6.0f)
#define KM_P3 (( 12.0f - 9.0f  * KM_B - 6.0f * KM_C) / 6.0f)
#define KM_Q0 ((  8.0f * KM_B + 24.0f * KM_C) / 6.0f)
#define KM_Q1 ((-12.0f * KM_B - 48.0f * KM_C) / 6.0f)
#define KM_Q2 ((  6.0f * KM_B + 30.0f * KM_C) / 6.0f)
#define KM_Q3 (( -1.0f * KM_B -  6.0f * KM_C) / 6.0f)

			if (x < -2.0)
				return(0.0f);
			if (x < -1.0)
				return(KM_Q0 - x*(KM_Q1 - x*(KM_Q2 - x*KM_Q3)));
			if (x < 0.0f)
				return(KM_P0 + x*x*(KM_P2 - x*KM_P3));
			if (x < 1.0f)
				return(KM_P0 + x*x*(KM_P2 + x*KM_P3));
			if (x < 2.0f)
				return(KM_Q0 + x*(KM_Q1 + x*(KM_Q2 + x*KM_Q3)));
			return(0.0f);
		}
		float Image::kernelCatrom(const float x)
		{
			if (x < -2.0)
				return 0.0f;
			if (x < -1.0)
				return (0.5f*(4.0f + x*(8.0f + x*(5.0f + x))));
			if (x < 0.0)
				return (0.5f*(2.0f + x*x*(-5.0f - 3.0f*x)));
			if (x < 1.0)
				return (0.5f*(2.0f + x*x*(-5.0f + 3.0f*x)));
			if (x < 2.0)
				return (0.5f*(4.0f + x*(-8.0f + x*(5.0f - x))));
			return 0.0f;
		}

		void Image::blendFuncAdd(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_add_ps(op1, op2));
		}
		void Image::blendFuncSub(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_sub_ps(op1, op2));
		}
		void Image::blendFuncBus(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_sub_ps(op2, op1));
		}
		void Image::blendFuncMul(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_mul_ps(op1, op2));
		}
		void Image::blendFuncDiv(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_div_ps(op1, op2));
		}
		void Image::blendFuncVid(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_div_ps(op2, op1));
		}
		void Image::blendFuncMin(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_min_ps(op1, op2));
		}
		void Image::blendFuncMax(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, _mm_max_ps(op1, op2));
		}
		void Image::blendFuncNone(float * const result, const __m128 op1, const __m128 op2)
		{
			_mm_storeu_ps(result, op1);
		}

		Image::BlendFuncProc Image::chooseBlendFunc(const FunctionType functionType)
		{
			switch (functionType){
			case HorseRadish::Imaging::Image::Add:		return blendFuncAdd;
			case HorseRadish::Imaging::Image::Sub:		return blendFuncSub;
			case HorseRadish::Imaging::Image::Bus:		return blendFuncBus;
			case HorseRadish::Imaging::Image::Mul:		return blendFuncMul;
			case HorseRadish::Imaging::Image::Div:		return blendFuncDiv;
			case HorseRadish::Imaging::Image::Vid:		return blendFuncVid;
			case HorseRadish::Imaging::Image::Min:		return blendFuncMin;
			case HorseRadish::Imaging::Image::Max:		return blendFuncMax;
			}

			return blendFuncNone;
		}

		void Image::getPixelInterpolated(const float x, const float y, const SampleType sampleType, Color &pixelValue) const
		{
			int xi = (int)(x);
			if (x < 0)
				xi--;
			int yi = (int)(y);
			if (y < 0)
				yi--;

			if (sampleType == Image::NearestNeighbour)
			{
				getPixel((int)(x + 0.5f), (int)(y + 0.5f), pixelValue);
				return;
			}

			if ((sampleType == Image::Bicubic) || (sampleType == Image::Bicubic2) || (sampleType == Image::Bspline) || (sampleType == Image::Box) ||
				(sampleType == Image::Hermite) || (sampleType == Image::Hamming) || (sampleType == Image::Sinc) || (sampleType == Image::Blackman) ||
				(sampleType == Image::Bessel) || (sampleType == Image::Gaussian) || (sampleType == Image::Quadratic) || (sampleType == Image::Mitchell) ||
				(sampleType == Image::Catrom) || (sampleType == Image::Hanning))
			{
				HorseRadish::Color finalColor, curSample;
				float kernelx[12], kernely[4];

				switch (sampleType)
				{
				case Image::Bicubic:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelCubic((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelCubic((float)(yi + i - 1 - y));
					}
					break;
				case Image::Bicubic2:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelGeneralizedCubic((float)(xi + i - 1 - x), -0.5);
						kernely[i] = Image::kernelGeneralizedCubic((float)(yi + i - 1 - y), -0.5);
					}
					break;
				case Image::Bspline:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelBSpline((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelBSpline((float)(yi + i - 1 - y));
					}
					break;
				case Image::Box:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelBox((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelBox((float)(yi + i - 1 - y));
					}
					break;
				case Image::Hermite:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelHermite((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelHermite((float)(yi + i - 1 - y));
					}
					break;
				case Image::Hamming:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelHamming((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelHamming((float)(yi + i - 1 - y));
					}
					break;
				case Image::Sinc:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelSinc((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelSinc((float)(yi + i - 1 - y));
					}
					break;
				case Image::Blackman:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelBlackman((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelBlackman((float)(yi + i - 1 - y));
					}
					break;
				case Image::Bessel:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelBessel((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelBessel((float)(yi + i - 1 - y));
					}
					break;
				case Image::Gaussian:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelGaussian((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelGaussian((float)(yi + i - 1 - y));
					}
					break;
				case Image::Quadratic:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelQuadratic((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelQuadratic((float)(yi + i - 1 - y));
					}
					break;
				case Image::Mitchell:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelMitchell((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelMitchell((float)(yi + i - 1 - y));
					}
					break;
				case Image::Catrom:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelCatrom((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelCatrom((float)(yi + i - 1 - y));
					}
					break;
				case Image::Hanning:
					for (int i = 0; i < 4; i++)
					{
						kernelx[i] = Image::kernelHanning((float)(xi + i - 1 - x));
						kernely[i] = Image::kernelHanning((float)(yi + i - 1 - y));
					}
					break;
				}

				finalColor.Set(0.0f, 0.0f, 0.0f, 0.0f);

				for (int yii = yi - 1; yii < yi + 3; yii++)
				{
					float kernelAuxY;

					kernelAuxY = kernely[yii - (yi - 1)];

					for (int xii = xi - 1; xii < xi + 3; xii++)
					{
						this->GetPixel(xii, yii, curSample);
						finalColor.AddColorWeighted(curSample, kernelAuxY * kernelx[xii - (xi - 1)]);
					}
				}

				pixelValue.Set(finalColor);
				return;
			}

			if (sampleType == Image::Lanczos)
			{
				float kernelx[12];
				HorseRadish::Color finalColor, curSample;

				for (int xii = xi - 5; xii < xi + 7; xii++)
					kernelx[xii - (xi - 5)] = Image::kernelLanczosSinc((float)(xii - x), 6.0f);

				finalColor.Set(0.0f, 0.0f, 0.0f, 0.0f);

				for (int yii = yi - 5; yii < yi + 7; yii++)
				{
					float kernely;

					kernely = Image::kernelLanczosSinc((float)(yii - y), 6.0f);

					for (int xii = xi - 5; xii < xi + 7; xii++)
					{
						this->getPixel(xii, yii, curSample);
						finalColor.AddColorWeighted(curSample, kernely*kernelx[xii - (xi - 5)]);
					}
				}

				pixelValue.Set(finalColor);
				return;
			}

	{
		HorseRadish::Color curSample11, curSample21, curSample12, curSample22;
		float t1 = x - xi, t2 = y - yi;
		float d = t1*t2;
		float b = t1 - d;
		float c = t2 - d;
		float a = 1 - t1 - c;

		this->getPixel(xi, yi, curSample11);
		this->getPixel(xi + 1, yi, curSample21);
		this->getPixel(xi, yi + 1, curSample12);
		this->getPixel(xi + 1, yi + 1, curSample22);

		pixelValue.SetWeight(curSample11, a, curSample21, b, curSample12, c, curSample22, d);
	}
		}

		void Image::getAreaInterpolated(const float centerX, const float centerY, const float areaWidth, const float areaHeight, const SampleType sampleType, Color &pixelValue) const
		{
			if ((areaWidth <= 1.0) && (areaHeight <= 1.0))
			{
				this->getPixelInterpolated(centerX, centerY, sampleType, pixelValue);
				return;
			}

			HorseRadish::Color finalColor;
			Vector areaMin, areaMax;
			int lowX, lowY, highX, highY;
			BRect area;

			area.SetExpand(centerX, centerY, areaWidth * 0.5f, areaHeight * 0.5f);
			area.GetMinMax(areaMin, areaMax);

			lowX = HorseRadish::Math::ftoi(HorseRadish::Math::floor(areaMin.x + 0.49999999f));
			lowY = HorseRadish::Math::ftoi(HorseRadish::Math::floor(areaMin.y + 0.49999999f));
			highX = HorseRadish::Math::ftoi(HorseRadish::Math::floor(areaMax.x + 0.5f));
			highY = HorseRadish::Math::ftoi(HorseRadish::Math::floor(areaMax.y + 0.5f));

			if ((areaHeight > 1.0f) && (areaWidth > 1.0f))
			{
				HorseRadish::Color curSample;
				float wBL, wTR, hBL, hTR;
				BRect intBL, intTR;

				area.CrossSection(BRect((float)lowX, (float)lowY, 0.5f), intBL);
				area.CrossSection(BRect((float)highX, (float)highY, 0.5f), intTR);

				intBL.GetDims(wBL, hBL);
				intTR.GetDims(wTR, hTR);

				this->getPixel(lowX, lowY, curSample);
				finalColor.AddColorWeighted(curSample, wBL*hBL);
				this->getPixel(highX, lowY, curSample);
				finalColor.AddColorWeighted(curSample, wTR*hBL);
				this->getPixel(lowX, highY, curSample);
				finalColor.AddColorWeighted(curSample, wBL*hTR);
				this->getPixel(highX, highY, curSample);
				finalColor.AddColorWeighted(curSample, wTR*hTR);

				for (int x = lowX + 1; x < highX; x++)
				{
					this->getPixel(x, lowY, curSample);
					finalColor.AddColorWeighted(curSample, hBL);
					this->getPixel(x, highY, curSample);
					finalColor.AddColorWeighted(curSample, hTR);
				}

				for (int y = lowY + 1; y < highY; y++)
				{
					this->getPixel(lowX, y, curSample);
					finalColor.AddColorWeighted(curSample, wBL);
					this->getPixel(highX, y, curSample);
					finalColor.AddColorWeighted(curSample, wTR);
				}

				for (int y = lowY + 1; y < highY; y++)
				{
					for (int x = lowX + 1; x < highX; x++)
					{
						this->getPixel(x, y, curSample);
						finalColor += curSample;
					}
				}
			}
			else
			{
				BRect curArea, intersect;
				HorseRadish::Color curSample;
				float centerX, centerY;

				for (int y = lowY; y <= highY; y++)
				{
					for (int x = lowX; x <= highX; x++)
					{
						curArea.SetExpand((float)x, (float)y, 0.5f);
						area.CrossSection(curArea, intersect);
						intersect.GetCenter(centerX, centerY);

						this->getPixelInterpolated(centerX, centerY, sampleType, curSample);
						finalColor.AddColorWeighted(curSample, intersect.GetArea());
					}
				}
			}

			pixelValue.SetWeight(finalColor, 1.0f / area.GetArea());
		}

		void Image::getPixel(const int x, const int y, HorseRadish::Color &pixelValue) const
		{
			getPixel(y *this->comp + x, pixelValue);
		}

		void Image::getPixel(int pixel, HorseRadish::Color &pixelValue) const
		{
			pixel = HorseRadish::Math::iClamp(pixel, 0, this->comp * this->larg);

			if (this->type == Image::UByte)
			{
				auto walkerB = reinterpret_cast<const unsigned char*>(this->data) + pixel*this->numComponentes;
				switch (this->format){
				case Image::RGBA:
					pixelValue.Set(walkerB);
					return;
				case Image::CMYK:
					pixelValue.SetCMYK(walkerB);
					return;

				case Image::BGRA:
					pixelValue.Set(walkerB);
					pixelValue.SwapRB();
					return;
				case Image::RGB:
					pixelValue.Set(walkerB[0], walkerB[1], walkerB[2]);
					return;
				case Image::BGR:
					pixelValue.Set(walkerB[2], walkerB[1], walkerB[0]);
					return;
				case Image::Red:
					pixelValue.Set(walkerB[0], 0, 0);
					return;
				case Image::Green:
					pixelValue.Set(0, walkerB[0], 0);
					return;
				case Image::Blue:
					pixelValue.Set(0, 0, walkerB[0]);
					return;
				case Image::Alpha:
					pixelValue.Set(0, 0, 0, walkerB[0]);
					return;
				case Image::Lum:
					pixelValue.Set(walkerB[0], walkerB[0], walkerB[0]);
					return;
				case Image::LumAlpha:
					pixelValue.Set(walkerB[0], walkerB[0], walkerB[0], walkerB[1]);
					return;
				case Image::RedGreen:
					pixelValue.Set(walkerB[0], walkerB[1], 0);
					return;
				}
				return;
			}

			auto walkerF = reinterpret_cast<const float*>(this->data) + pixel*this->numComponentes;
			switch (this->format){
			case Image::RGBA:
				pixelValue.Set(walkerF);
				return;
			case Image::CMYK:
				pixelValue.SetCMYK(walkerF);
				return;
			case Image::BGRA:
				pixelValue.r = walkerF[2];
				pixelValue.g = walkerF[1];
				pixelValue.b = walkerF[0];
				pixelValue.a = walkerF[3];
				return;
			case Image::RGB:
				pixelValue.Set(walkerF, 1.0f);
				return;
			case Image::BGR:
				pixelValue.r = walkerF[2];
				pixelValue.g = walkerF[1];
				pixelValue.b = walkerF[0];
				pixelValue.a = 1.0f;
				return;
			case Image::Red:
				pixelValue.r = walkerF[0];
				pixelValue.g = 0.0f;
				pixelValue.b = 0.0f;
				pixelValue.a = 1.0f;
				return;
			case Image::Green:
				pixelValue.r = 0.0f;
				pixelValue.g = walkerF[0];
				pixelValue.b = 0.0f;
				pixelValue.a = 1.0f;
				return;
			case Image::Blue:
				pixelValue.r = 0.0f;
				pixelValue.g = 0.0f;
				pixelValue.b = walkerF[0];
				pixelValue.a = 1.0f;
				return;
			case Image::Alpha:
				pixelValue.Set(0.0f, 0.0f, 0.0f, walkerF[0]);
				return;
			case Image::Lum:
				pixelValue.r = walkerF[0];
				pixelValue.g = pixelValue.r;
				pixelValue.b = pixelValue.r;
				pixelValue.a = 1.0f;
				return;
			case Image::LumAlpha:
				pixelValue.r = walkerF[0];
				pixelValue.g = pixelValue.r;
				pixelValue.b = pixelValue.r;
				pixelValue.a = walkerF[1];
				return;
			case Image::RedGreen:
				pixelValue.r = walkerF[0];
				pixelValue.g = walkerF[1];
				pixelValue.b = 0.0f;
				pixelValue.a = 1.0f;
				return;
			}
		}

		void Image::setPixel(const int x, const int y, const HorseRadish::Color &pixelValue)
		{
			setPixel(y*this->comp + x, pixelValue);
		}

		void Image::setPixel(int pixel, const HorseRadish::Color &pixelValue)
		{
			pixel = HorseRadish::Math::iClamp(pixel, 0, this->comp * this->larg);

			//tenho unsigned char
			if (this->type == UByte)
			{
				auto walkerB = reinterpret_cast<unsigned char*>(this->data) + pixel*this->numComponentes;
				switch (this->format){
				case Image::RGBA:
					pixelValue.Write(walkerB);
					return;
				case Image::CMYK:
					pixelValue.WriteCMYK(walkerB);
					return;
				case Image::BGRA:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.b);
					walkerB[1] = HorseRadish::Color::ConvertColor(pixelValue.g);
					walkerB[2] = HorseRadish::Color::ConvertColor(pixelValue.r);
					walkerB[3] = HorseRadish::Color::ConvertColor(pixelValue.a);
					return;
				case Image::RGB:
					pixelValue.WriteRGB(walkerB);
					return;
				case Image::BGR:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.b);
					walkerB[1] = HorseRadish::Color::ConvertColor(pixelValue.g);
					walkerB[2] = HorseRadish::Color::ConvertColor(pixelValue.r);
					return;
				case Image::Red:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.r);
					return;
				case Image::Green:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.g);
					return;
				case Image::Blue:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.b);
					return;
				case Image::Alpha:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.a);
					return;
				case Image::Lum:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.r);
					return;
				case Image::LumAlpha:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.r);
					walkerB[1] = HorseRadish::Color::ConvertColor(pixelValue.a);
					return;
				case Image::RedGreen:
					walkerB[0] = HorseRadish::Color::ConvertColor(pixelValue.r);
					walkerB[1] = HorseRadish::Color::ConvertColor(pixelValue.g);
					return;
				}
				return;
			}

			auto walkerF = reinterpret_cast<float*>(this->data) + pixel*this->numComponentes;
			switch (this->format){
			case Image::RGBA:
				pixelValue.Write(walkerF);
				return;
			case Image::CMYK:
				pixelValue.WriteCMYK(walkerF);
				return;
			case Image::BGRA:
				walkerF[2] = pixelValue.r;
				walkerF[1] = pixelValue.g;
				walkerF[0] = pixelValue.b;
				walkerF[3] = pixelValue.a;
				return;
			case Image::RGB:
				pixelValue.WriteRGB(walkerF);
				return;
			case Image::BGR:
				walkerF[2] = pixelValue.r;
				walkerF[1] = pixelValue.g;
				walkerF[0] = pixelValue.b;
				return;
			case Image::Red:
				walkerF[0] = pixelValue.r;
				return;
			case Image::Green:
				walkerF[0] = pixelValue.g;
				return;
			case Image::Blue:
				walkerF[0] = pixelValue.b;
				return;
			case Image::Alpha:
				walkerF[0] = pixelValue.a;
				return;
			case Image::Lum:
				walkerF[0] = pixelValue.r;
				return;
			case Image::LumAlpha:
				walkerF[0] = pixelValue.r;
				walkerF[1] = pixelValue.a;
				return;
			case Image::RedGreen:
				walkerF[0] = pixelValue.r;
				walkerF[1] = pixelValue.g;
				return;
			}
		}

		void Image::reset()
		{
			this->data = nullptr;
			this->comp = this->larg = 0;
			this->format = Image::RGBA;
			this->type = Image::UByte;
			this->numComponentes = 0;
			this->managePixelData = false;
		}

		void Image::init(const int comp, const int larg, const ImageType type, const ImageFormat format, bool createBuffer)
		{
			this->data = nullptr;
			this->comp = this->larg = 0;
			this->format = Image::RGBA;
			this->type = Image::UByte;
			this->numComponentes = 0;
			this->managePixelData = false;

			if (comp <= 0 || larg <= 0)
				return;
			if (Image::getNumComponentes(format) == 0)
				return;

			this->comp = comp;
			this->larg = larg;
			this->type = type;
			this->format = format;

			this->numComponentes = Image::getNumComponentes(this->format);

			if (createBuffer == false)
				return;

			this->managePixelData = true;

			if (type == Image::UByte)
			{
				this->data = new unsigned char[this->comp * this->larg * this->numComponentes];
				memset(this->data, 0, sizeof(unsigned char)*this->comp*this->larg*this->numComponentes);
				return;
			}

			this->data = new float[this->comp * this->larg * this->numComponentes];
			memset(this->data, 0, sizeof(float)*this->comp*this->larg*this->numComponentes);
		}

		bool Image::intersects(const int startX, const int startY, const int width, const int height)
		{
			int finalX = startX + width;
			int finalY = startY + height;

			if ((finalX < 0) || (finalY < 0))
				return false;
			if ((startX > this->comp) || (startY > this->larg))
				return false;

			return true;
		}

		Image::Image(const int comp, const int larg, const ImageType type, const ImageFormat format)
		{
			this->init(comp, larg, type, format, true);
		}

		Image::Image(const int comp, const int larg, const ImageType type, const ImageFormat format, void *pixelData, bool managePixelData)
		{
			if (pixelData == nullptr)
			{
				this->init(comp, larg, type, format, true);
				return;
			}

			this->init(comp, larg, type, format, !managePixelData);

			if (managePixelData)
				this->data = pixelData;
			else
				memcpy(this->data, pixelData, this->GetSize());

			this->managePixelData = managePixelData;
		}

		Image::~Image()
		{
			if (this->managePixelData == true)
				delete[] this->data;

			this->data = nullptr;
			this->comp = this->larg = 0;
			this->format = Image::RGBA;
			this->type = Image::UByte;
			this->numComponentes = 0;
			this->managePixelData = false;
		}

		HorseRadish::Imaging::Image* Image::Clone() const
		{
			if (this->IsValid() == false)
				return nullptr;

			auto newImage = new HorseRadish::Imaging::Image(this->comp, this->larg, this->type, this->format);
			memcpy(newImage->data, this->data, this->GetSize());

			return newImage;
		}
		HorseRadish::Imaging::Image* Image::Clone(const HorseRadish::Imaging::Image::ImageType newType, const HorseRadish::Imaging::Image::ImageFormat newFormat) const
		{
			if (this->IsValid() == false)
				return nullptr;

			if (this->Check(newType, newFormat) == true)
				return this->Clone();

			auto newImage = new HorseRadish::Imaging::Image(this->comp, this->larg, newType, newFormat);

			for (int curPixelY = 0; curPixelY < this->larg; curPixelY++)
			{
				HorseRadish::Color pixelVal;

				for (int curPixelX = 0; curPixelX < this->comp; curPixelX++)
				{
					this->GetPixel(curPixelX, curPixelY, pixelVal);
					newImage->SetPixel(curPixelX, curPixelY, pixelVal);
				}
			}

			return newImage;
		}

		bool Image::IsValid() const
		{
			if (this->data == nullptr)
				return false;

			if (this->GetPixelBytes() <= 0)
				return false;
			if (this->GetSize() <= 0)
				return false;

			return true;
		}

		void Image::Clear(const HorseRadish::Color &pixelValue)
		{
			this->Clear(pixelValue.r, pixelValue.g, pixelValue.b, pixelValue.a);
		}

		void Image::Clear(const float r, const float g, const float b, const float a)
		{
			if (this->IsValid() == false)
				return;

			HorseRadish::Color pixelValue(r, g, b, a);

			int numPixels = this->comp * this->larg;

			if ((this->format == Image::RGBA || this->format == Image::BGRA) && this->type == Image::UByte)
			{
				unsigned char colorB[4];
				unsigned int colorInt;

				HorseRadish::Color::ConvertColor(colorB, (const float *)pixelValue, true);

				if (this->format == Image::RGBA)
					colorInt = ((colorB[3] & 0xff) << 24) | ((colorB[2] & 0xff) << 16) | ((colorB[1] & 0xff) << 8) | (colorB[0] & 0xff);
				else
					colorInt = ((colorB[3] & 0xff) << 24) | ((colorB[0] & 0xff) << 16) | ((colorB[1] & 0xff) << 8) | (colorB[2] & 0xff);

				auto walker = reinterpret_cast<unsigned int*>(this->data);
				for (int i = 0; i < numPixels; i++, walker++)
					*walker = colorInt;

				return;
			}

			for (int i = 0; i < numPixels; i++)
				setPixel(i, pixelValue);
		}

		void Image::ClearArea(const HorseRadish::Color &pixelValue, int areaStartX, int areaStartY, int areaWidth, int areaHeight)
		{
			this->ClearArea(pixelValue.r, pixelValue.g, pixelValue.b, pixelValue.a, areaStartX, areaStartY, areaWidth, areaHeight);
		}

		void Image::ClearArea(const float r, const float g, const float b, const float a, int areaStartX, int areaStartY, int areaWidth, int areaHeight)
		{
			HorseRadish::Color pixelValue;
			int metaX, metaY;

			if (this->IsValid() == false)
				return;

			areaStartX = Math::iClampZero(areaStartX, this->comp);
			areaStartY = Math::iClampZero(areaStartY, this->larg);
			areaWidth = Math::iClamp(areaWidth, 0, this->comp - areaStartX);
			areaHeight = Math::iClamp(areaHeight, 0, this->larg - areaStartY);

			if ((areaWidth <= 0) || (areaHeight <= 0))
				return;

			pixelValue.Set(r, g, b, a);
			metaX = areaStartX + areaWidth;
			metaY = areaStartY + areaHeight;

			if ((this->format == Image::RGBA || this->format == Image::BGRA) && this->type == Image::UByte)
			{
				unsigned char colorB[4];
				unsigned int colorInt;

				HorseRadish::Color::ConvertColor(colorB, (const float *)pixelValue, true);

				if (this->format == Image::RGBA)
					colorInt = ((colorB[3] & 0xff) << 24) | ((colorB[2] & 0xff) << 16) | ((colorB[1] & 0xff) << 8) | (colorB[0] & 0xff);
				else
					colorInt = ((colorB[3] & 0xff) << 24) | ((colorB[0] & 0xff) << 16) | ((colorB[1] & 0xff) << 8) | (colorB[2] & 0xff);

				for (int pixelY = areaStartY; pixelY < metaY; pixelY++)
				{
					auto walker = reinterpret_cast<unsigned int*>(this->data) + (pixelY * this->comp) + areaStartX;

					for (int pixelX = areaStartX; pixelX < metaX; pixelX++, walker++)
						*walker = colorInt;
				}

				return;
			}

			for (int pixelY = areaStartY; pixelY < metaY; pixelY++)
			{
				for (int pixelX = areaStartX; pixelX < metaX; pixelX++)
					setPixel(pixelX, pixelY, pixelValue);
			}
		}

		void Image::ChangeType(const ImageType newType)
		{
			if ((this->IsValid() == false) || (this->type == newType))
				return;

			if (this->managePixelData == false)
				return;

			int numPixels = this->comp * this->larg;

			auto walkerF = reinterpret_cast<float*>(this->data);
			auto walkerUBYTE = reinterpret_cast<unsigned char*>(this->data);

			if (newType == Image::UByte)
			{
				//SAsmFloat2UByte((unsigned char*)img->data, (float*)img->data, img->comp*img->larg*numComp, 255.0f, 0.0f);
				for (int i = 0; i < numPixels; i++, walkerF++, walkerUBYTE++)
					*walkerUBYTE = HorseRadish::Color::ConvertColor(*walkerF);

				this->type = newType;
				return;
			}

			auto newBufferF = new float[this->comp * this->larg * this->numComponentes];

			//SAsmUByte2Float(newBufferF, (unsigned char*)img->data, img->comp*img->larg*numComp, 1.0f / 255.0f, 0.0f);
			walkerF = (float*)newBufferF;
			for (int i = 0; i < numPixels; i++, walkerF++, walkerUBYTE++)
				*walkerF = HorseRadish::Color::ConvertColor(*walkerUBYTE);

			delete[] this->data;
			this->data = newBufferF;
			this->type = newType;
		}

		void Image::ChangeFormat(const ImageFormat newFormat)
		{
			if ((this->IsValid() == false) || (this->format == newFormat))
				return;

			if (Image::getNumComponentes(newFormat) == 0)
				return;

			if (this->managePixelData == false)
				return;

			{
				HorseRadish::Color pixelValue;

				Image imgTmp(this->comp, this->larg, this->type, newFormat);

				int numPixels = this->comp * this->larg;

				for (int i = 0; i < numPixels; i++)
				{
					getPixel(i, pixelValue);
					imgTmp.setPixel(i, pixelValue);
				}

				delete[] this->data;
				this->data = imgTmp.data;
				this->format = imgTmp.format;
				this->numComponentes = Image::getNumComponentes(this->format);

				imgTmp.reset();
			}
		}

		void Image::Flip()
		{
			int meta, lineSize;
			unsigned char *lineAux, *walkerTop, *walkerBottom;

			if ((this->IsValid() == false) || (this->larg <= 2))
				return;

			lineSize = this->GetRowSize();
			lineAux = new unsigned char[lineSize];

			meta = this->larg / 2;
			walkerTop = (unsigned char*)this->data;
			walkerBottom = walkerTop + (this->larg - 1)*lineSize;

			if (lineAux != nullptr)
			{
				for (int i = 0; i < meta; i++)
				{
					memcpy(lineAux, walkerTop, lineSize);
					memcpy(walkerTop, walkerBottom, lineSize);
					memcpy(walkerBottom, lineAux, lineSize);

					walkerTop += lineSize;
					walkerBottom -= lineSize;
				}

				delete[] lineAux;
				return;
			}

			for (int i = 0; i < meta; i++)
			{
				unsigned char troca;

				for (int j = 0; j < lineSize; j++)
				{
					troca = walkerTop[j];
					walkerTop[j] = walkerBottom[j];
					walkerBottom[j] = troca;
				}

				walkerTop += lineSize;
				walkerBottom -= lineSize;
			}
		}

		void Image::Mirror()
		{
			if (this->IsValid() == false)
				return;

			if (this->type == Image::UByte)
			{
				int meta;
				unsigned char pixelB[4], *walkerTopB, *walkerBottomB;

				meta = this->comp / 2;
				for (int j = 0; j < this->larg; j++)
				{
					walkerTopB = ((unsigned char*)this->data) + this->comp*this->numComponentes*j;
					walkerBottomB = walkerTopB + this->numComponentes*(this->comp - 1);
					for (int i = 0; i < meta; i++, walkerTopB += this->numComponentes, walkerBottomB -= this->numComponentes)
					{
						memcpy(pixelB, walkerBottomB, sizeof(unsigned char)*this->numComponentes);
						memcpy(walkerBottomB, walkerTopB, sizeof(unsigned char)*this->numComponentes);
						memcpy(walkerTopB, pixelB, sizeof(unsigned char)*this->numComponentes);
					}
				}
				return;
			}

			if (this->type == Image::Float)
			{
				int meta;
				float pixelF[4], *walkerTopF, *walkerBottomF;

				meta = this->comp / 2;
				for (int j = 0; j < this->larg; j++)
				{
					walkerTopF = ((float*)this->data) + this->comp*this->numComponentes*j;
					walkerBottomF = walkerTopF + this->numComponentes*(this->comp - 1);
					for (int i = 0; i < meta; i++, walkerTopF += this->numComponentes, walkerBottomF -= this->numComponentes)
					{
						memcpy(pixelF, walkerBottomF, sizeof(float)*this->numComponentes);
						memcpy(walkerBottomF, walkerTopF, sizeof(float)*this->numComponentes);
						memcpy(walkerTopF, pixelF, sizeof(float)*this->numComponentes);
					}
				}
			}
		}

		void Image::GetPixel(const int x, const int y, HorseRadish::Color &pixelValue) const
		{
			if ((this->IsValid() == false) || (x < 0) || (y < 0) || (x >= this->comp) || (y >= this->larg))
				return;

			this->getPixel(x, y, pixelValue);
		}

		void Image::GetPixel(const int x, const int y, float * const pixels) const
		{
			HorseRadish::Color pixelValue;

			if ((this->IsValid() == false) || (x < 0) || (y < 0) || (x >= this->comp) || (y >= this->larg) || (pixels == nullptr))
				return;

			this->getPixel(x, y, pixelValue);
			pixelValue.Write(pixels);
		}

		void Image::SetPixel(const int x, const int y, const HorseRadish::Color &pixelValue)
		{
			if ((this->IsValid() == false) || (x < 0) || (y < 0) || (x >= this->comp) || (y >= this->larg))
				return;

			this->setPixel(x, y, pixelValue);
		}

		void Image::SetPixel(const int x, const int y, const float *pixels)
		{
			if ((this->IsValid() == false) || (x < 0) || (y < 0) || (x >= this->comp) || (y >= this->larg) || (pixels == nullptr))
				return;

			this->setPixel(x, y, HorseRadish::Color(pixels));
		}

		bool Image::Transpose()
		{
			if ((this->IsValid() == false) || (this->comp != this->larg) || (this->comp <= 1))
				return false;

			if (this->type == Image::UByte)
			{
				unsigned char pixelB[4], *dataB;
				int imPos1, imPos2;

				dataB = (unsigned char*)this->data;
				for (int j = 0; j < this->larg; j++)
				{
					for (int i = j + 1; i < this->comp; i++)
					{
						imPos1 = (j*this->comp + i)*this->numComponentes;
						imPos2 = (i*this->larg + j)*this->numComponentes;

						memcpy(pixelB, dataB + imPos1, sizeof(unsigned char)*this->numComponentes);
						memcpy(dataB + imPos1, dataB + imPos2, sizeof(unsigned char)*this->numComponentes);
						memcpy(dataB + imPos2, pixelB, sizeof(unsigned char)*this->numComponentes);
					}
				}

				return true;
			}

			if (this->type == Image::Float)
			{
				float pixelF[4], *dataF;
				int imPos1, imPos2;

				dataF = (float*)this->data;
				for (int j = 0; j < this->larg; j++)
				{
					for (int i = j + 1; i < this->comp; i++)
					{
						imPos1 = (j*this->comp + i)*this->numComponentes;
						imPos2 = (i*this->larg + j)*this->numComponentes;

						memcpy(pixelF, dataF + imPos1, sizeof(float)*this->numComponentes);
						memcpy(dataF + imPos1, dataF + imPos2, sizeof(float)*this->numComponentes);
						memcpy(dataF + imPos2, pixelF, sizeof(float)*this->numComponentes);
					}
				}

				return true;
			}

			return false;
		}

		float Image::ApplySAT()
		{
			HorseRadish::Color pixelActual, pixelAntes;
			float maxValue;

			if (this->IsValid() == false)
				return 0.0f;

			if (this->type != Image::Float)
				this->ChangeType(Image::Float);

			for (int pixelY = 1; pixelY < this->larg; pixelY++)
			{
				for (int pixelX = 1; pixelX < this->comp; pixelX++)
				{
					getPixel(pixelX, pixelY, pixelActual);
					getPixel(pixelX - 1, pixelY, pixelAntes);

					pixelActual += pixelAntes;

					setPixel(pixelX, pixelY, pixelActual);
				}
			}

			for (int pixelY = this->larg - 2; pixelY >= 0; pixelY--)
			{
				for (int pixelX = 1; pixelX < this->comp; pixelX++)
				{
					getPixel(pixelX, pixelY, pixelActual);
					getPixel(pixelX, pixelY + 1, pixelAntes);

					pixelActual += pixelAntes;

					setPixel(pixelX, pixelY, pixelActual);
				}
			}

			getPixel(this->comp - 1, 0, pixelActual);
			maxValue = 0.0f;
			maxValue = HorseRadish::Math::fMax(maxValue, pixelActual.r);
			maxValue = HorseRadish::Math::fMax(maxValue, pixelActual.g);
			maxValue = HorseRadish::Math::fMax(maxValue, pixelActual.b);
			maxValue = HorseRadish::Math::fMax(maxValue, pixelActual.a);

			return maxValue;
		}

		void Image::Crop(int startX, int startY, int newComp, int newLarg)
		{
			int tamPixel, tamLinha, pixelY;
			unsigned char *linha, *pCrop, *pWrite;

			if ((this->IsValid() == false) || (newComp <= 0) || (newLarg <= 0))
				return;

			tamPixel = this->GetPixelBytes();

			if (startX<0)
				startX = 0;
			if (startY<0)
				startY = 0;
			if (startX + newComp>this->comp)
				newComp = this->comp - startX;
			if (startY + newLarg>this->larg)
				newLarg = this->larg - startY;

			if ((startX == 0) && (startY == 0) && (newComp == this->comp) && (newLarg == this->larg))
				return;
			if ((newComp <= 0) || (newLarg <= 0))
				return;

			tamLinha = tamPixel * newComp;
			linha = new unsigned char[tamLinha];

			pWrite = (unsigned char*)this->data;

			for (pixelY = 0; pixelY < newLarg; pixelY++)
			{
				pCrop = ((unsigned char*)this->data) + ((pixelY + startY) * this->comp + startX) * tamPixel;

				memcpy(linha, pCrop, tamPixel * newComp);

				memcpy(pWrite, linha, tamPixel * newComp);

				pWrite += tamLinha;
			}

			this->comp = newComp;
			this->larg = newLarg;

			delete[] linha;
		}

		void Image::ApplyGamma(const float gamma)
		{
			HorseRadish::Color pixelValue;
			float scale, tmp;

			if (this->IsValid() == false)
				return;

			for (int walkerY = 0; walkerY < this->larg; walkerY++)
			{
				for (int walkerX = 0; walkerX<this->comp; walkerX++)
				{
					getPixel(walkerX, walkerY, pixelValue);

					pixelValue.WeightRGB(gamma);

					scale = 1.0f;
					if (pixelValue.r>1.0f && (tmp = (1.0f / pixelValue.r)) < scale) scale = tmp;
					if (pixelValue.g > 1.0f && (tmp = (1.0f / pixelValue.g)) < scale) scale = tmp;
					if (pixelValue.b > 1.0f && (tmp = (1.0f / pixelValue.b)) < scale) scale = tmp;

					pixelValue.WeightRGB(scale);

					setPixel(walkerX, walkerY, pixelValue);
				}
			}
		}

		void Image::Grayscale()
		{
			HorseRadish::Color pixelValue;

			if (this->IsValid() == false)
				return;

			for (int walkerY = 0; walkerY < this->larg; walkerY++)
			{
				for (int walkerX = 0; walkerX < this->comp; walkerX++)
				{
					getPixel(walkerX, walkerY, pixelValue);

					pixelValue.ToGrayscale(0.3019607843f, 0.5921568627f, 0.1098039215f);

					setPixel(walkerX, walkerY, pixelValue);
				}
			}
		}

		void Image::Negative()
		{
			HorseRadish::Color pixelValue;

			if (this->IsValid() == false)
				return;

			if (this->type == Image::UByte)
			{
				if (this->format == Image::RGB)
				{
					auto walkerB = reinterpret_cast<unsigned char*>(this->data);
					auto metaB = walkerB + (this->comp * this->larg * 3);
					while (walkerB < metaB)
					{
						walkerB[0] = 255 - walkerB[0];
						walkerB[1] = 255 - walkerB[1];
						walkerB[2] = 255 - walkerB[2];
						walkerB += 3;
					}

					return;
				}

				if (this->format == Image::RGBA)
				{
					auto walkerB = reinterpret_cast<unsigned char*>(this->data);
					auto metaB = walkerB + (this->comp * this->larg * 4);
					while (walkerB < metaB)
					{
						walkerB[0] = 255 - walkerB[0];
						walkerB[1] = 255 - walkerB[1];
						walkerB[2] = 255 - walkerB[2];
						walkerB += 4;
					}

					return;
				}
			}

			for (int walkerY = 0; walkerY < this->larg; walkerY++)
			{
				for (int walkerX = 0; walkerX < this->comp; walkerX++)
				{
					getPixel(walkerX, walkerY, pixelValue);
					pixelValue.NegativeRGB();
					setPixel(walkerX, walkerY, pixelValue);
				}
			}
		}

		bool Image::Scale(int newComp, int newLarg, SampleType sampleType)
		{
			if ((this->IsValid() == false) || (newComp <= 0) || (newLarg <= 0))
				return false;

			{
				Image imgTemp(newComp, newLarg, this->type, this->format);

				if (imgTemp.IsValid() == false)
					return false;

				float xScale = (float)this->comp / (float)newComp;
				float yScale = (float)this->larg / (float)newLarg;

				if ((xScale <= 1.0) && (yScale <= 1.0))
				{
					HorseRadish::Color curSample;
					float sX, sY;

					for (int dY = 0; dY < newLarg; dY++)
					{

						sY = (dY + 0.5f) * yScale - 0.5f;

						for (int dX = 0; dX < newComp; dX++)
						{
							sX = (dX + 0.5f) * xScale - 0.5f;

							this->getPixelInterpolated(sX, sY, sampleType, curSample);


							imgTemp.setPixel(dX, dY, curSample);
						}
					}
				}
				else
				{
					HorseRadish::Color curSample;
					float sX, sY;

					for (int dY = 0; dY < newLarg; dY++)
					{
						sY = (dY + 0.5f) * yScale - 0.5f;

						for (int dX = 0; dX < newComp; dX++)
						{
							sX = (dX + 0.5f) * xScale - 0.5f;

							this->getAreaInterpolated(sX, sY, xScale, yScale, sampleType, curSample);

							imgTemp.setPixel(dX, dY, curSample);
						}
					}
				}

				delete[] this->data;
				this->data = imgTemp.data;
				this->comp = imgTemp.comp;
				this->larg = imgTemp.larg;

				imgTemp.reset();
			}

			return true;
		}

		int Image::ApplyMatrix(const Matrix &matrix)
		{
			__m128 matRow1, matRow2, matRow3, matRow4, finalPixel;
			HorseRadish::Color pixelValue;
			float auxAlpha;

			if (this->IsValid() == false)
				return -1;

			matRow1 = _mm_loadu_ps(((const float *)matrix) + 0);
			matRow2 = _mm_loadu_ps(((const float *)matrix) + 4);
			matRow3 = _mm_loadu_ps(((const float *)matrix) + 8);
			matRow4 = _mm_loadu_ps(((const float *)matrix) + 12);

			for (int pixelY = 0; pixelY < this->larg; pixelY++)
			{
				for (int pixelX = 0; pixelX < this->comp; pixelX++)
				{
					getPixel(pixelX, pixelY, pixelValue);
					auxAlpha = pixelValue.a;

					finalPixel = _mm_mul_ps(_mm_load_ps1(&pixelValue.r), matRow1);
					finalPixel = _mm_add_ps(finalPixel, _mm_mul_ps(_mm_load_ps1(&pixelValue.g), matRow2));
					finalPixel = _mm_add_ps(finalPixel, _mm_mul_ps(_mm_load_ps1(&pixelValue.b), matRow3));
					finalPixel = _mm_add_ps(finalPixel, matRow4);

					_mm_storeu_ps(&pixelValue.r, finalPixel);
					pixelValue.a = auxAlpha;

					setPixel(pixelX, pixelY, pixelValue);
				}
			}

			return 1;
		}

		int Image::ApplyKernel(const float * const kernel, const int knComp, const int knLarg)
		{
			int newPixelX, newPixelY, offsetX, offsetY;
			HorseRadish::Color pixelValue, pixelConv;
			ImageType saveType;
			float invPeso;
			Image *newImg;

			if ((this->IsValid() == false) || (kernel == nullptr) || (knComp <= 0) || (knLarg <= 0))
				return -1;
			if ((knComp % 2) == 0 || (knLarg % 2) == 0)
				return -1;

			if (knComp == 1 && knLarg == 1)
				return 1;

			invPeso = 0.0f;
			for (int numComp = 0; numComp < knComp*knLarg; numComp++)
				invPeso += kernel[numComp];

			if (invPeso != 0.0f)
				invPeso = 1.0f / invPeso;
			if (invPeso < 0.0f)
				invPeso = -invPeso;
			if (invPeso == 0.0f)
				invPeso = 1.0f;

			offsetX = (knComp - 1) / 2;
			offsetY = (knLarg - 1) / 2;

			saveType = this->type;
			this->ChangeType(Image::Float);

			newImg = this->Clone();
			if (newImg == nullptr)
			{
				this->ChangeType(saveType);
				return 0;
			}

			for (int pixelY = 0; pixelY < this->larg; pixelY++)
			{
				for (int pixelX = 0; pixelX < this->comp; pixelX++)
				{
					pixelConv.Set(0.0f, 0.0f, 0.0f, 0.0f);

					for (int filtroY = 0; filtroY < knLarg; filtroY++)
					{
						newPixelY = HorseRadish::Math::iClampZero(pixelY - offsetY + filtroY, this->larg - 1);

						for (int filtroX = 0; filtroX < knComp; filtroX++)
						{
							newPixelX = HorseRadish::Math::iClampZero(pixelX - offsetX + filtroX, this->comp - 1);

							newImg->getPixel(newPixelX, newPixelY, pixelValue);

							pixelConv.AddColorWeighted(pixelValue, kernel[filtroY*knComp + filtroX]);
						}
					}

					pixelValue.SetWeight(pixelConv, invPeso);
					this->setPixel(pixelX, pixelY, pixelValue);
				}
			}

			delete newImg;
			newImg = nullptr;

			this->ChangeType(saveType);

			return 1;
		}

		int Image::ApplyGaussian(const float * const kernel, const int knComp)
		{
			HorseRadish::Color pixelValue, pixelConv;
			int newPixel, offsetX;
			ImageType saveType;
			float invPeso;
			Image *newImg;

			if ((this->IsValid() == false) || (kernel == nullptr) || (knComp <= 0))
				return -1;
			if ((knComp % 2) == 0 || knComp <= 1)
				return -1;

			invPeso = 0.0f;
			for (int numComp = 0; numComp < knComp; numComp++)
				invPeso += kernel[numComp];

			if (invPeso != 0.0f)
				invPeso = 1.0f / invPeso;
			if (invPeso < 0.0f)
				invPeso = -invPeso;

			offsetX = (knComp - 1) / 2;

			saveType = this->type;
			this->ChangeType(Image::Float);

			newImg = this->Clone();
			if (newImg == nullptr)
			{
				this->ChangeType(saveType);
				return 0;
			}

			for (int pixelY = 0; pixelY < this->larg; pixelY++)
			{
				for (int pixelX = 0; pixelX < this->comp; pixelX++)
				{
					pixelConv.Set(0.0f, 0.0f, 0.0f, 0.0f);

					for (int filtroX = 0; filtroX < knComp; filtroX++)
					{
						newPixel = HorseRadish::Math::iClampZero(pixelX - offsetX + filtroX, this->comp - 1);

						this->getPixel(newPixel, pixelY, pixelValue);

						pixelConv.AddColorWeighted(pixelValue, kernel[filtroX]);
					}


					pixelValue.SetWeight(pixelConv, invPeso);
					newImg->setPixel(pixelX, pixelY, pixelValue);
				}
			}

			for (int pixelY = 0; pixelY < this->larg; pixelY++)
			{
				for (int pixelX = 0; pixelX < this->comp; pixelX++)
				{
					pixelConv.Set(0.0f, 0.0f, 0.0f, 0.0f);

					for (int filtroX = 0; filtroX < knComp; filtroX++)
					{
						newPixel = HorseRadish::Math::iClampZero(pixelY - offsetX + filtroX, this->larg - 1);

						newImg->getPixel(pixelX, newPixel, pixelValue);

						pixelConv.AddColorWeighted(pixelValue, kernel[filtroX]);
					}

					pixelValue.SetWeight(pixelConv, invPeso);
					this->setPixel(pixelX, pixelY, pixelValue);
				}
			}

			delete newImg;
			newImg = nullptr;

			this->ChangeType(saveType);

			return 1;
		}

		void Image::ApplyMAD(const float op1, const float op2)
		{
			HorseRadish::Color pixelValue;

			if (this->IsValid() == false)
				return;

			int numPixels = this->comp * this->larg;

			for (int i = 0; i < numPixels; i++)
			{
				getPixel(i, pixelValue);
				pixelValue.MAD(op1, op2);
				setPixel(i, pixelValue);
			}
		}

		void Image::ApplyOperator(FunctionType functionType, const float opVal)
		{
			float pixel;

			if (this->IsValid() == false)
				return;

			if (this->type == HorseRadish::Imaging::Image::UByte)
			{
				auto walkerB = reinterpret_cast<unsigned char*>(this->data);
				auto metaB = walkerB + this->comp * this->larg * this->numComponentes;

				switch (functionType){
				case HorseRadish::Imaging::Image::Add:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(opVal + pixel);
					}
					break;
				case HorseRadish::Imaging::Image::Sub:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(pixel - opVal);
					}
					break;
				case HorseRadish::Imaging::Image::Bus:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(opVal - pixel);
					}
					break;
				case HorseRadish::Imaging::Image::Mul:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(opVal*pixel);
					}
					break;
				case HorseRadish::Imaging::Image::Div:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(pixel / opVal);
					}
					break;
				case HorseRadish::Imaging::Image::Vid:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(opVal / pixel);
					}
					break;
				case HorseRadish::Imaging::Image::Min:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(std::min(pixel, opVal));
					}
					break;
				case HorseRadish::Imaging::Image::Max:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(std::max(pixel, opVal));
					}
					break;
				case HorseRadish::Imaging::Image::Pow:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(pow(pixel, opVal));
					}
					break;
				case HorseRadish::Imaging::Image::PowR:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(pow(opVal, pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Exp:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(exp(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Sqrt:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(sqrt(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Sqr:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(pixel*pixel);
					}
					break;
				case HorseRadish::Imaging::Image::Log:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(log(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Log10:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(log10(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Sin:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(sin(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Cos:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(cos(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Tan:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(tan(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::ASin:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(asin(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::ACos:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(acos(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::ATan:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(atan(pixel));
					}
					break;
				case HorseRadish::Imaging::Image::Inv:
					for (; walkerB < metaB; walkerB++)
					{
						pixel = ((float)(*walkerB))*0.003921568627450980392156862f;
						*walkerB = HorseRadish::Color::ConvertColor(1.0f / pixel);
					}
					break;
				}
				return;
			}

			auto walkerF = reinterpret_cast<float*>(this->data);
			auto metaF = walkerF + this->comp * this->larg * this->numComponentes;

			switch (functionType){
			case HorseRadish::Imaging::Image::Add:
				HorseRadish::Math::fMAD((float*)this->data, this->comp * this->larg *  this->numComponentes, 1.0f, opVal);
				break;
			case HorseRadish::Imaging::Image::Sub:
				HorseRadish::Math::fMAD((float*)this->data, this->comp * this->larg * this->numComponentes, 1.0f, -opVal);
				break;
			case HorseRadish::Imaging::Image::Bus:
				HorseRadish::Math::fMAD((float*)this->data, this->comp * this->larg * this->numComponentes, -1.0f, opVal);
				break;
			case HorseRadish::Imaging::Image::Mul:
				HorseRadish::Math::fMAD((float*)this->data, this->comp * this->larg * this->numComponentes, opVal, 0.0f);
				break;
			case HorseRadish::Imaging::Image::Div:
				HorseRadish::Math::fMAD((float*)this->data, this->comp * this->larg * this->numComponentes, 1.0f / opVal, 0.0f);
				break;
			case HorseRadish::Imaging::Image::Vid:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = opVal / (*walkerF);
				}
				break;
			case HorseRadish::Imaging::Image::Min:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = std::min((*walkerF), opVal);
				}
				break;
			case HorseRadish::Imaging::Image::Max:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = std::max((*walkerF), opVal);
				}
				break;
			case HorseRadish::Imaging::Image::Pow:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = pow((*walkerF), opVal);
				}
				break;
			case HorseRadish::Imaging::Image::PowR:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = pow(opVal, (*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Exp:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = exp((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Abs:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = abs((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Sqrt:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = sqrt((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Sqr:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = (*walkerF)*(*walkerF);
				}
				break;
			case HorseRadish::Imaging::Image::Log:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = log((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Log10:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = log10((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Sin:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = sin((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Cos:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = cos((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Tan:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = tan((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::ASin:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = asin((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::ACos:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = acos((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::ATan:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = atan((*walkerF));
				}
				break;
			case HorseRadish::Imaging::Image::Inv:
				for (; walkerF < metaF; walkerF++)
				{
					*walkerF = 1.0f / (*walkerF);
				}
				break;
			case HorseRadish::Imaging::Image::Neg:
				HorseRadish::Math::fMAD((float*)this->data, this->comp * this->larg * this->numComponentes, -1.0f, 0.0f);
				break;
			}
		}

		void Image::Blend(const Image * const imgOP1, const Image * const imgOP2, const FunctionType blendFunc, const BlendOperatorType blendOP1, const BlendOperatorType blendOP2)
		{
			BlendFuncProc blendCallFunc;
			float pixel1[4], pixel2[4], result[4];
			__m128 calc1, calc2, just1s, copy1;
			HorseRadish::Color corFinal;

			if ((this->IsValid() == false) || (imgOP1 == nullptr) || (imgOP2 == nullptr) || (imgOP1->IsValid() == false) || (imgOP2->IsValid() == false))
				return;

			just1s = _mm_set_ps1(1.0f);

			blendCallFunc = Image::chooseBlendFunc(blendFunc);

			for (int walkerY = 0; walkerY < this->larg; walkerY++)
			{
				for (int walkerX = 0; walkerX < this->comp; walkerX++)
				{
					imgOP1->GetPixel(walkerX, walkerY, pixel1);
					imgOP2->GetPixel(walkerX, walkerY, pixel2);

					calc1 = copy1 = _mm_loadu_ps(pixel1);
					calc2 = _mm_loadu_ps(pixel2);

					switch (blendOP1){
					case HorseRadish::Imaging::Image::Zero:
						calc1 = _mm_setzero_ps();
						break;
					case HorseRadish::Imaging::Image::One:
						break;
					case HorseRadish::Imaging::Image::Op1:
						calc1 = _mm_mul_ps(calc1, calc1);
						break;
					case HorseRadish::Imaging::Image::Op2:
						calc1 = _mm_mul_ps(calc1, calc2);
						break;
					case HorseRadish::Imaging::Image::Op1Alpha:
						calc1 = _mm_mul_ps(calc1, _mm_shuffle_ps(calc1, calc1, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op2Alpha:
						calc1 = _mm_mul_ps(calc1, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinus:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, calc1));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinus:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, calc2));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinusAlpha:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, _mm_shuffle_ps(calc1, calc1, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinusAlpha:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					}

					switch (blendOP2){
					case HorseRadish::Imaging::Image::Zero:
						calc2 = _mm_setzero_ps();
						break;
					case HorseRadish::Imaging::Image::One:
						break;
					case HorseRadish::Imaging::Image::Op1:
						calc2 = _mm_mul_ps(calc2, copy1);
						break;
					case HorseRadish::Imaging::Image::Op2:
						calc2 = _mm_mul_ps(calc2, calc2);
						break;
					case HorseRadish::Imaging::Image::Op1Alpha:
						calc2 = _mm_mul_ps(calc2, _mm_shuffle_ps(copy1, copy1, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op2Alpha:
						calc2 = _mm_mul_ps(calc2, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinus:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, copy1));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinus:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, calc2));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinusAlpha:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, _mm_shuffle_ps(copy1, copy1, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinusAlpha:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					}

					blendCallFunc(result, calc1, calc2);

					corFinal.Set(result);

					this->setPixel(walkerX, walkerY, corFinal);
				}
			}
		}

		void Image::BlendConst(const HorseRadish::Color &constantsOp1, const Image * const imgOP2, const FunctionType blendFunc, const BlendOperatorType blendOP1, const BlendOperatorType blendOP2)
		{
			BlendFuncProc blendCallFunc;
			float pixel2[4], result[4];
			__m128 calc1, calc2, just1s, sseConst;
			HorseRadish::Color corFinal;

			if ((this->IsValid() == false) || (imgOP2 == nullptr) || (imgOP2->IsValid() == false))
				return;

			just1s = _mm_set_ps1(1.0f);
			sseConst = _mm_loadu_ps(constantsOp1);

			blendCallFunc = Image::chooseBlendFunc(blendFunc);

			for (int walkerY = 0; walkerY < this->larg; walkerY++)
			{
				for (int walkerX = 0; walkerX < this->comp; walkerX++)
				{
					imgOP2->GetPixel(walkerX, walkerY, pixel2);

					calc1 = sseConst;
					calc2 = _mm_loadu_ps(pixel2);

					switch (blendOP1){
					case HorseRadish::Imaging::Image::Zero:
						calc1 = _mm_setzero_ps();
						break;
					case HorseRadish::Imaging::Image::One:
						break;
					case HorseRadish::Imaging::Image::Op1:
						calc1 = _mm_mul_ps(calc1, sseConst);
						break;
					case HorseRadish::Imaging::Image::Op2:
						calc1 = _mm_mul_ps(calc1, calc2);
						break;
					case HorseRadish::Imaging::Image::Op1Alpha:
						calc1 = _mm_mul_ps(calc1, _mm_shuffle_ps(sseConst, sseConst, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op2Alpha:
						calc1 = _mm_mul_ps(calc1, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinus:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, sseConst));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinus:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, calc2));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinusAlpha:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, _mm_shuffle_ps(sseConst, sseConst, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinusAlpha:
						calc1 = _mm_mul_ps(calc1, _mm_sub_ps(just1s, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					}

					switch (blendOP2){
					case HorseRadish::Imaging::Image::Zero:
						calc2 = _mm_setzero_ps();
						break;
					case HorseRadish::Imaging::Image::One:
						break;
					case HorseRadish::Imaging::Image::Op1:
						calc2 = _mm_mul_ps(calc2, sseConst);
						break;
					case HorseRadish::Imaging::Image::Op2:
						calc2 = _mm_mul_ps(calc2, calc2);
						break;
					case HorseRadish::Imaging::Image::Op1Alpha:
						calc2 = _mm_mul_ps(calc2, _mm_shuffle_ps(sseConst, sseConst, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op2Alpha:
						calc2 = _mm_mul_ps(calc2, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3)));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinus:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, sseConst));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinus:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, calc2));
						break;
					case HorseRadish::Imaging::Image::Op1OneMinusAlpha:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, _mm_shuffle_ps(sseConst, sseConst, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					case HorseRadish::Imaging::Image::Op2OneMinusAlpha:
						calc2 = _mm_mul_ps(calc2, _mm_sub_ps(just1s, _mm_shuffle_ps(calc2, calc2, _MM_SHUFFLE(3, 3, 3, 3))));
						break;
					}

					blendCallFunc(result, calc1, calc2);

					corFinal.Set(result);

					this->setPixel(walkerX, walkerY, corFinal);
				}
			}
		}

		Image* Image::ConvertHeightToNormal(ImageType outType, ImageFormat outFormat, const float scale) const
		{
			HorseRadish::Color pixel1, sobelPixel;
			float invSqr, sX, sY, sZ;
			const float sobelX[5][5] = {
					{ 1, 2, 0, -2, -1 },
					{ 4, 8, 0, -8, -4 },
					{ 6, 12, 0, -12, -6 },
					{ 4, 8, 0, -8, -4 },
					{ 1, 2, 0, -2, -1 },
			};
			const float sobelY[5][5] = {
					{ 1, 4, 6, 4, 1 },
					{ 2, 8, 12, 8, 2 },
					{ 0, 0, 0, 0, 0 },
					{ -2, -8, -12, -8, -2 },
					{ -1, -4, -6, -4, -1 },
			};
			Image *imgFinal;

			if (this->IsValid() == false)
				return nullptr;

			if (Image::getNumComponentes(outFormat) < 3)
				imgFinal = new Image(this->comp, this->larg, outType, Image::RGB);
			else
				imgFinal = new Image(this->comp, this->larg, outType, outFormat);

			sZ = 256.0f;
			if (scale > 0.0)
				sZ = 256.0f / scale;

			for (int j = 0; j < this->larg; j++)
			{
				for (int i = 0; i < this->comp; i++)
				{
					sX = sY = 0.0f;
					for (int dy = 0; dy < 5; dy++)
					{
						for (int dx = 0; dx < 5; dx++)
						{
							this->getPixel(HorseRadish::Math::iClampZero(i + dx - 2, this->comp - 1), HorseRadish::Math::iClampZero(j + dy - 2, this->larg - 1), sobelPixel);
							sX += sobelX[dy][dx] * sobelPixel.r;
							sY += sobelY[dy][dx] * sobelPixel.r;
						}
					}

					this->getPixel(i, j, pixel1);
					pixel1.a = pixel1.r;

					invSqr = sX*sX + sY*sY + sZ*sZ;
					if (HorseRadish::Math::isZero(invSqr, 1.192092896e-06f))
					{
						pixel1.r = pixel1.g = 0.0f;
						pixel1.b = 1.0f;
					}
					else
					{
						invSqr = 1.0f / sqrt(invSqr);
						pixel1.r = sX*invSqr;
						pixel1.g = sY*invSqr;
						pixel1.b = sZ*invSqr;
					}

					if (outType == Image::UByte)
					{
						pixel1.r = (pixel1[0] + 1.0f)*0.5f;
						pixel1.g = (pixel1[1] + 1.0f)*0.5f;
						pixel1.b = (pixel1[2] + 1.0f)*0.5f;
					}

					imgFinal->setPixel(i, j, pixel1);
				}
			}

			return imgFinal;
		}

		void Image::CopyRegion(const Image * const imgSrc, int regionStartX, int regionStartY)
		{
			if ((this->IsValid() == false) || (imgSrc == nullptr) || (imgSrc->IsValid() == false))
				return;

			if (this->intersects(regionStartX, regionStartY, imgSrc->comp, imgSrc->larg) == false)
				return;

			if ((this->Check(imgSrc->type, imgSrc->format) == true) && (regionStartX >= 0) && (regionStartY >= 0))
			{
				unsigned char *dataBD;
				const unsigned char *dataBS;
				int	areaWidth, areaHeight, copySize, pixelSize;

				areaWidth = imgSrc->comp;
				areaHeight = imgSrc->larg;
				if ((regionStartX + areaWidth) > this->comp)
					areaWidth -= this->comp - (regionStartX + areaWidth);
				if ((regionStartY + areaHeight) > this->larg)
					areaHeight -= this->larg - (regionStartY + areaHeight);

				dataBD = (unsigned char*)this->data;
				dataBS = (unsigned char*)imgSrc->data;
				pixelSize = imgSrc->GetPixelSize();

				copySize = areaWidth * pixelSize;

				dataBD += regionStartY * (this->comp * pixelSize) + (regionStartX * pixelSize);

				for (int i = 0; i < areaHeight; i++)
				{
					memcpy(dataBD, dataBS, copySize);

					dataBD += this->comp * pixelSize;
					dataBS += imgSrc->comp * pixelSize;
				}

				return;
			}

			for (int pixelY = 0; pixelY < imgSrc->larg; pixelY++)
			{
				HorseRadish::Color corPixel;

				for (int pixelX = 0; pixelX < imgSrc->comp; pixelX++)
				{
					imgSrc->GetPixel(pixelX, pixelY, corPixel);

					this->SetPixel(regionStartX + pixelX, regionStartY + pixelY, corPixel);
				}
			}
		}

		void Image::ChannelSet(const ImageFormat channel, const float val)
		{
			if (this->IsValid() == false)
				return;

			int indexOf = Image::getCorrectIndex(channel, this->format);
			if (indexOf < 0)
				return;

			if (this->type == Image::UByte)
			{
				unsigned char valB;

				auto walkerB = reinterpret_cast<unsigned char*>(this->data);
				auto metaB = walkerB + this->comp*this->larg*this->numComponentes;

				walkerB += indexOf;
				valB = HorseRadish::Color::ConvertColor(val);
				while (walkerB < metaB)
				{
					*walkerB = valB;
					walkerB += this->numComponentes;
				}
				return;
			}

			if (this->type == Image::Float)
			{
				auto walkerF = reinterpret_cast<float*>(this->data);
				auto metaF = walkerF + this->comp*this->larg*this->numComponentes;
				walkerF += indexOf;
				while (walkerF < metaF)
				{
					*walkerF = val;
					walkerF += this->numComponentes;
				}
			}
		}

		void Image::ChannelCopy(const ImageFormat channelFrom, const ImageFormat channelTo)
		{
			if ((this->IsValid() == false) || (channelFrom == channelTo))
				return;

			int indexOfFrom = Image::getCorrectIndex(channelFrom, this->format);
			int indexOfTo = Image::getCorrectIndex(channelTo, this->format);
			if (indexOfFrom < 0 || indexOfTo < 0)
				return;

			if (this->type == Image::UByte)
			{
				auto walkerB = reinterpret_cast<unsigned char*>(this->data);
				auto metaB = walkerB + this->comp*this->larg*this->numComponentes;
				while (walkerB < metaB)
				{
					walkerB[indexOfTo] = walkerB[indexOfFrom];
					walkerB += this->numComponentes;
				}
				return;
			}

			if (this->type == Image::Float)
			{
				auto walkerF = reinterpret_cast<float*>(this->data);
				auto metaF = walkerF + this->comp*this->larg*this->numComponentes;
				while (walkerF < metaF)
				{
					walkerF[indexOfTo] = walkerF[indexOfFrom];
					walkerF += this->numComponentes;
				}
			}
		}

		void Image::ChannelFrom(const Image * const imageSrc, const ImageFormat channel)
		{
			float *metaDF, *walkerDF, *walkerSF;
			unsigned char *metaDB, *walkerDB, *walkerSB;

			if ((this->IsValid() == false) || (imageSrc->IsValid() == false) || (this->comp != imageSrc->comp) || (this->larg != imageSrc->larg))
				return;

			int indexOfDest = Image::getCorrectIndex(channel, this->format);
			int indexOfSrc = Image::getCorrectIndex(channel, imageSrc->format);
			if (indexOfDest < 0 || indexOfSrc < 0)
				return;

			if (this->type == imageSrc->type && this->type == Image::UByte)
			{
				walkerDB = (unsigned char*)this->data;
				walkerSB = (unsigned char*)imageSrc->data;
				metaDB = walkerDB + this->comp*this->larg*this->numComponentes;
				walkerDB += indexOfDest;
				walkerSB += indexOfSrc;
				while (walkerDB < metaDB)
				{
					*walkerDB = *walkerSB;
					walkerDB += this->numComponentes;
					walkerSB += imageSrc->numComponentes;
				}
				return;
			}

			if (this->type == imageSrc->type && this->type == Image::Float)
			{
				walkerDF = (float*)this->data;
				walkerSF = (float*)imageSrc->data;
				metaDF = walkerDF + this->comp*this->larg*this->numComponentes;
				walkerDF += indexOfDest;
				walkerSF += indexOfSrc;
				while (walkerDF < metaDF)
				{
					*walkerDF = *walkerSF;
					walkerDF += this->numComponentes;
					walkerSF += imageSrc->numComponentes;
				}
				return;
			}

			if (this->type == Image::UByte)
			{
				walkerDB = (unsigned char*)this->data;
				walkerSF = (float*)imageSrc->data;
				metaDB = walkerDB + this->comp*this->larg*this->numComponentes;
				walkerDB += indexOfDest;
				walkerSF += indexOfSrc;
				while (walkerDB < metaDB)
				{
					*walkerDB = HorseRadish::Color::ConvertColor(*walkerSF);
					walkerDB += this->numComponentes;
					walkerSF += imageSrc->numComponentes;
				}
				return;
			}

			walkerDF = (float*)this->data;
			walkerSB = (unsigned char*)imageSrc->data;
			metaDF = walkerDF + (this->comp * this->larg * this->numComponentes);
			walkerDF += indexOfDest;
			walkerSB += indexOfSrc;
			while (walkerDF < metaDF)
			{
				*walkerDF = HorseRadish::Color::ConvertColor(*walkerSB);
				walkerDF += this->numComponentes;
				walkerSB += imageSrc->numComponentes;
			}
		}

		void Image::ChannelSwap(const ImageFormat channel1, const ImageFormat channel2)
		{
			if ((this->IsValid() == false) || (channel1 == channel2))
				return;

			int indexOf1 = Image::getCorrectIndex(channel1, this->format);
			int indexOf2 = Image::getCorrectIndex(channel2, this->format);
			if (indexOf1 < 0 || indexOf2 < 0)
				return;

			if (indexOf1 == indexOf2)
				return;

			if (this->type == Image::UByte)
			{
				unsigned char trocaB;

				auto walkerB = reinterpret_cast<unsigned char*>(this->data);
				auto metaB = walkerB + this->comp*this->larg*this->numComponentes;
				while (walkerB < metaB)
				{
					trocaB = walkerB[indexOf1];
					walkerB[indexOf1] = walkerB[indexOf2];
					walkerB[indexOf2] = trocaB;
					walkerB += this->numComponentes;
				}

				return;
			}

			if (this->type == Image::Float)
			{
				float trocaF;

				auto walkerF = reinterpret_cast<float*>(this->data);
				auto metaF = walkerF + this->comp*this->larg*this->numComponentes;
				while (walkerF < metaF)
				{
					trocaF = walkerF[indexOf1];
					walkerF[indexOf1] = walkerF[indexOf2];
					walkerF[indexOf2] = trocaF;
					walkerF += this->numComponentes;
				}
			}
		}

		void Image::MultiplyAlpha()
		{
			float alpha;

			if (this->IsValid() == false)
				return;
			if (this->format != Image::RGBA && this->format != Image::BGRA && this->format != Image::LumAlpha)
				return;

			int numPixeis = this->comp*this->larg;

			if (this->format == Image::RGBA || this->format == Image::BGRA)
			{
				if (this->type == Image::UByte)
				{
					for (int i = 0; i < numPixeis; i++)
					{
						alpha = HorseRadish::Color::ConvertColor(((unsigned char*)this->data)[i * 4 + 3]);
						(((unsigned char*)this->data)[i * 4 + 0]) = HorseRadish::Color::ConvertColor(HorseRadish::Color::ConvertColor(((unsigned char*)this->data)[i * 4 + 0]) * alpha);
						(((unsigned char*)this->data)[i * 4 + 1]) = HorseRadish::Color::ConvertColor(HorseRadish::Color::ConvertColor(((unsigned char*)this->data)[i * 4 + 1]) * alpha);
						(((unsigned char*)this->data)[i * 4 + 2]) = HorseRadish::Color::ConvertColor(HorseRadish::Color::ConvertColor(((unsigned char*)this->data)[i * 4 + 2]) * alpha);
					}

					return;
				}
				if (this->type == Image::Float)
				{
					for (int i = 0; i < numPixeis; i++)
					{
						alpha = (((float*)this->data)[i * 4 + 3]);
						(((float*)this->data)[i * 4 + 0]) *= alpha;
						(((float*)this->data)[i * 4 + 1]) *= alpha;
						(((float*)this->data)[i * 4 + 2]) *= alpha;
					}

					return;
				}

				return;
			}

			if (this->type == Image::UByte)
			{
				for (int i = 0; i < numPixeis; i++)
				{
					alpha = HorseRadish::Color::ConvertColor(((unsigned char*)this->data)[i * 2 + 1]);
					(((unsigned char*)this->data)[i * 2 + 0]) = HorseRadish::Color::ConvertColor(HorseRadish::Color::ConvertColor(((unsigned char*)this->data)[i * 2 + 0]) * alpha);
				}
				return;
			}
			if (this->type == Image::Float)
			{
				for (int i = 0; i < numPixeis; i++)
					(((float*)this->data)[i * 2 + 0]) *= (((float*)this->data)[i * 2 + 1]);
				return;
			}
		}

		int Image::GetPixelSize() const
		{
			if (this->data == nullptr)
				return 0;

			if (this->type == Image::UByte)
				return this->numComponentes;
			if (this->type == Image::Float)
				return this->numComponentes * 4;

			return 0;
		}

		int Image::GetPixelBytes() const
		{
			if (this->type == Image::Float)
				return this->numComponentes * sizeof(float);
			if (this->type == Image::UByte)
				return this->numComponentes * sizeof(unsigned char);

			return 0;
		}

	} //Imaging
} //HorseRadish