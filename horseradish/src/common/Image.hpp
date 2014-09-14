#pragma once

#include "Platform.hpp"
#include "Types.hpp"
#include "Color.hpp"
#include "Matrix.hpp"

namespace HorseRadish
{
	namespace Imaging
	{
		class Image
		{
		public:
			enum ImageType{
				Float = 0xbcd1,
				UByte = 0xbcd2,
			};

			enum ImageFormat{
				RGB = 0xbcd3,
				RGBA = 0xbcd4,
				BGR = 0xbcd5,
				BGRA = 0xbcd6,
				Red = 0xbcd7,
				Green = 0xbcd8,
				Blue = 0xbcd9,
				Alpha = 0xbcda,
				Lum = 0xbcdb,
				LumAlpha = 0xbcdc,
				CMYK = 0xbcdd,
				RedGreen = 0xbcde,
			};

			enum SampleType{
				NearestNeighbour,
				Bilinear,
				Bicubic,
				Bicubic2,
				Bspline,
				Box,
				Hermite,
				Hamming,
				Sinc,
				Blackman,
				Bessel,
				Gaussian,
				Quadratic,
				Mitchell,
				Catrom,
				Hanning,
				Lanczos
			};

			enum FunctionType{
				Add,
				Sub,
				Bus,
				Mul,
				Div,
				Vid,
				Min,
				Max,
				Pow,
				PowR,
				Exp,
				Abs,
				Sqrt,
				Sqr,
				Log,
				Log10,
				Sin,
				Cos,
				Tan,
				ASin,
				ACos,
				ATan,
				Inv,
				Neg,
			};

			enum BlendOperatorType{
				Zero,
				One,
				Op1,
				Op2,
				Op1Alpha,
				Op2Alpha,
				Op1OneMinus,
				Op2OneMinus,
				Op1OneMinusAlpha,
				Op2OneMinusAlpha,
			};

		private:
			void *data;
			int comp, larg, numComponentes;
			ImageFormat format;
			ImageType type;
			bool managePixelData;

			static int getNumComponentes(const ImageFormat format);
			static int getCorrectIndex(const ImageFormat ofWhat, const ImageFormat inFormat);

			static float kernelBSpline(const float x);
			static float kernelGeneralizedCubic(const float t, const float a);
			static float kernelCubic(const float t);
			static float kernelLinear(const float t);
			static float kernelBlackman(const float x);
			static float kernelSinc(const float x);
			static float kernelHamming(const float x);
			static float kernelHanning(const float x);
			static float kernelHermite(const float x);
			static float kernelBox(const float x);
			static float kernelLanczosSinc(const float t, const float r);
			static float kernelGaussian(const float x);
			static float kernelBessel(const float x);
			static float kernelBessel_Order1(float x);
			static float kernelBessel_Q1(const float x);
			static float kernelBessel_P1(const float x);
			static float kernelBessel_J1(const float x);
			static float kernelCatrom(const float x);
			static float kernelMitchell(const float x);
			static float kernelQuadratic(const float x);

			typedef void(*BlendFuncProc) (float * const, const __m128, const __m128);
			static void blendFuncAdd(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncSub(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncBus(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncMul(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncDiv(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncVid(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncMin(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncMax(float * const result, const __m128 op1, const __m128 op2);
			static void blendFuncNone(float * const result, const __m128 op1, const __m128 op2);
			static BlendFuncProc chooseBlendFunc(const FunctionType functionType);

			void getPixelInterpolated(const float x, const float y, const SampleType sampleType, HorseRadish::Color &pixelValue) const;
			void getAreaInterpolated(const float centerX, const float centerY, const float areaWidth, const float areaHeight, const SampleType sampleType, HorseRadish::Color &pixelValue) const;
			void getPixel(const int x, const int y, HorseRadish::Color &pixelValue) const;
			void getPixel(int pixel, HorseRadish::Color &pixelValue) const;
			void setPixel(const int x, const int y, const HorseRadish::Color &pixelValue);
			void setPixel(int pixel, const HorseRadish::Color &pixelValue);
			void reset();
			void init(const int comp, const int larg, const ImageType type, const ImageFormat format, bool createBuffer);
			bool intersects(const int startX, const int startY, const int width, const int height);

			friend class Factory;

		public:

			Image(const int comp, const int larg, const ImageType type, const ImageFormat format);
			Image(const int comp, const int larg, const ImageType type, const ImageFormat format, void *pixelData, bool managePixelData);
			~Image();

			Image* Clone() const;
			Image* Clone(const ImageType newType, const ImageFormat newFormat) const;

			bool IsValid() const;

			void Clear(const HorseRadish::Color &pixelValue);
			void Clear(const float r, const float g, const float b, const float a);
			void ClearArea(const HorseRadish::Color &pixelValue, int areaStartX, int areaStartY, int areaWidth, int areaHeight);
			void ClearArea(const float r, const float g, const float b, const float a, int areaStartX, int areaStartY, int areaWidth, int areaHeight);
			void ChangeType(const ImageType newType);
			void ChangeFormat(const ImageFormat newFormat);

			void Flip();
			void Mirror();
			void GetPixel(const int x, const int y, HorseRadish::Color &pixelValue) const;
			void GetPixel(const int x, const int y, float * const pixels) const;
			void SetPixel(const int x, const int y, const HorseRadish::Color &pixelValue);
			void SetPixel(const int x, const int y, const float *pixels);
			bool Transpose();
			float ApplySAT();
			void Crop(int startX, int startY, int newComp, int newLarg);
			void ApplyGamma(const float gamma);
			void Grayscale();
			void Negative();
			bool Scale(int newComp, int newLarg, SampleType sampleType);
			int ApplyMatrix(const Matrix &matrix);
			int ApplyKernel(const float * const kernel, const int knComp, const int knLarg);
			int ApplyGaussian(const float * const kernel, const int knComp);
			void ApplyMAD(const float op1, const float op2);
			void ApplyOperator(FunctionType functionType, const float opVal);
			void Blend(const Image * const imgOP1, const Image * const imgOP2, const FunctionType blendFunc, const BlendOperatorType blendOP1, const BlendOperatorType blendOP2);
			void BlendConst(const HorseRadish::Color &constantsOp1, const Image * const imgOP2, const FunctionType blendFunc, const BlendOperatorType blendOP1, const BlendOperatorType blendOP2);
			Image* ConvertHeightToNormal(ImageType outType, ImageFormat outFormat, const float scale) const;
			void CopyRegion(const Image * const imgSrc, int regionStartX, int regionStartY);

			void ChannelSet(const ImageFormat channel, const float val);
			void ChannelCopy(const ImageFormat channelFrom, const ImageFormat channelTo);
			void ChannelFrom(const Image * const imageSrc, const ImageFormat channel);
			void ChannelSwap(const ImageFormat channel1, const ImageFormat channel2);
			void MultiplyAlpha();

			const void* GetPixelData() const { return this->data; }
			bool Check(const ImageType type, const ImageFormat format) const { return ((this->type == type) && (this->format == format)); }
			bool Check(const ImageType type) const { return (this->type == type); }
			bool Check(const ImageFormat format) const { return (this->format == format); }
			void GetDims(int &width, int &height) const { width = this->comp; height = this->larg; }
			ImageType GetType() const { return this->type; }
			ImageFormat GetFormat() const { return this->format; }
			int GetWidth() const { return this->comp; }
			int GetHeight() const { return this->larg; }
			int GetPixelSize() const;
			int GetRowSize() const { return (this->GetPixelSize() * this->comp); }
			int GetColumnSize() const { return (this->GetPixelSize() * this->larg); }
			int GetSize() const { return (this->GetPixelSize() * this->comp * this->larg); }
			int GetPixelBytes() const;
		};

	} //Imaging
} //HorseRadish

