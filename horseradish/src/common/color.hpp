#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "math.hpp"
#include "encoders.hpp"

#include <array>
#include <type_traits>
#include <xmmintrin.h>
#include <emmintrin.h>

namespace hr
{
	/******
	* Implementation notes:
	*	- unless otherwise specified, everything is linear (not SRGB)
	*	- don't assume that SRGB is a linear pow(x, 2.2), because it isn't
	*	- don't convert from uint8 SRGB to linear and then store it in uint8. This will lose precision in the blacks (because SRGB is not linear)
	******/

	template<typename TDataType = uint8_t>
	class Color
	{
		TDataType mRGBA[4];

		static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, uint8_t, etc.)");

	public:
		using DataType = TDataType;

	public:
		constexpr Color() = default;

		explicit constexpr Color(TDataType rgba) noexcept
			: mRGBA{ rgba, rgba, rgba, rgba }
		{ }

		explicit constexpr Color(TDataType rgb, TDataType a) noexcept
			: mRGBA{ rgb, rgb, rgb, a }
		{ }

		constexpr Color(TDataType r, TDataType g, TDataType b, TDataType a) noexcept
			: mRGBA{ r, g, b, a }
		{ }

		explicit constexpr Color(const TDataType* const rgba) noexcept
			: mRGBA{ rgba[0], rgba[1], rgba[2], rgba[3] }
		{ }

		constexpr TDataType* data() noexcept
		{
			return mRGBA;
		}

		constexpr const TDataType* data() const noexcept
		{
			return mRGBA;
		}

		constexpr TDataType& operator[] (const size_t index)
		{
			return mRGBA[index % 4];
		}

		constexpr const TDataType& operator[] (const size_t index) const
		{
			return mRGBA[index % 4];
		}

		void operator+=(const Color& c)
		{
			mRGBA[0] += c.mRGBA[0];
			mRGBA[1] += c.mRGBA[1];
			mRGBA[2] += c.mRGBA[2];
			mRGBA[3] += c.mRGBA[3];
		}

		void operator-=(const Color& c)
		{
			mRGBA[0] -= c.mRGBA[0];
			mRGBA[1] -= c.mRGBA[1];
			mRGBA[2] -= c.mRGBA[2];
			mRGBA[3] -= c.mRGBA[3];
		}

		void operator*=(const Color& c)
		{
			mRGBA[0] *= c.mRGBA[0];
			mRGBA[1] *= c.mRGBA[1];
			mRGBA[2] *= c.mRGBA[2];
			mRGBA[3] *= c.mRGBA[3];
		}

		void operator/=(const Color& c)
		{
			mRGBA[0] /= c.mRGBA[0];
			mRGBA[1] /= c.mRGBA[1];
			mRGBA[2] /= c.mRGBA[2];
			mRGBA[3] /= c.mRGBA[3];
		}

		void operator+=(const TDataType n)
		{
			mRGBA[0] += n;
			mRGBA[1] += n;
			mRGBA[2] += n;
			mRGBA[3] += n;
		}

		void operator-=(const TDataType n)
		{
			mRGBA[0] -= n;
			mRGBA[1] -= n;
			mRGBA[2] -= n;
			mRGBA[3] -= n;
		}

		void operator*=(const TDataType n)
		{
			mRGBA[0] *= n;
			mRGBA[1] *= n;
			mRGBA[2] *= n;
			mRGBA[3] *= n;
		}

		void operator/=(const TDataType n)
		{
			mRGBA[0] /= n;
			mRGBA[1] /= n;
			mRGBA[2] /= n;
			mRGBA[3] /= n;
		}

		Color operator+(const Color& c) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] + c.mRGBA[0];
			result.mRGBA[1] = mRGBA[1] + c.mRGBA[1];
			result.mRGBA[2] = mRGBA[2] + c.mRGBA[2];
			result.mRGBA[3] = mRGBA[3] + c.mRGBA[3];

			return result;
		}

		Color operator-(const Color& c) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] - c.mRGBA[0];
			result.mRGBA[1] = mRGBA[1] - c.mRGBA[1];
			result.mRGBA[2] = mRGBA[2] - c.mRGBA[2];
			result.mRGBA[3] = mRGBA[3] - c.mRGBA[3];

			return result;
		}

		Color operator*(const Color& c) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] * c.mRGBA[0];
			result.mRGBA[1] = mRGBA[1] * c.mRGBA[1];
			result.mRGBA[2] = mRGBA[2] * c.mRGBA[2];
			result.mRGBA[3] = mRGBA[3] * c.mRGBA[3];

			return result;
		}

		Color operator/(const Color& c) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] / c.mRGBA[0];
			result.mRGBA[1] = mRGBA[1] / c.mRGBA[1];
			result.mRGBA[2] = mRGBA[2] / c.mRGBA[2];
			result.mRGBA[3] = mRGBA[3] / c.mRGBA[3];

			return result;
		}

		Color operator+(const TDataType n) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] + n;
			result.mRGBA[1] = mRGBA[1] + n;
			result.mRGBA[2] = mRGBA[2] + n;
			result.mRGBA[3] = mRGBA[3] + n;

			return result;
		}

		Color operator-(const TDataType n) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] - n;
			result.mRGBA[1] = mRGBA[1] - n;
			result.mRGBA[2] = mRGBA[2] - n;
			result.mRGBA[3] = mRGBA[3] - n;

			return result;
		}

		Color operator*(const TDataType n) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] * n;
			result.mRGBA[1] = mRGBA[1] * n;
			result.mRGBA[2] = mRGBA[2] * n;
			result.mRGBA[3] = mRGBA[3] * n;

			return result;
		}

		Color operator/(const TDataType n) const
		{
			Color result;
			result.mRGBA[0] = mRGBA[0] / n;
			result.mRGBA[1] = mRGBA[1] / n;
			result.mRGBA[2] = mRGBA[2] / n;
			result.mRGBA[3] = mRGBA[3] / n;

			return result;
		}

		Color& set(const Color& color)
		{
			mRGBA[0] = color.mRGBA[0];
			mRGBA[1] = color.mRGBA[1];
			mRGBA[2] = color.mRGBA[2];
			mRGBA[3] = color.mRGBA[3];
			return *this;
		}

		Color& set(const Color& color, const TDataType a)
		{
			mRGBA[0] = color.mRGBA[0];
			mRGBA[1] = color.mRGBA[1];
			mRGBA[2] = color.mRGBA[2];
			mRGBA[3] = a;
			return *this;
		}

		Color& set(const TDataType* rgba)
		{
			mRGBA[0] = rgba[0];
			mRGBA[1] = rgba[1];
			mRGBA[2] = rgba[2];
			mRGBA[3] = rgba[3];
			return *this;
		}

		Color& set(const TDataType* const rgb, const TDataType a)
		{
			mRGBA[0] = rgb[0];
			mRGBA[1] = rgb[1];
			mRGBA[2] = rgb[2];
			mRGBA[3] = a;
			return *this;
		}

		Color& set(TDataType rgba)
		{
			mRGBA[0] = rgba;
			mRGBA[1] = rgba;
			mRGBA[2] = rgba;
			mRGBA[3] = rgba;
			return *this;
		}

		Color& set(TDataType rgb, TDataType a)
		{
			mRGBA[0] = mRGBA[1] = mRGBA[2] = rgb;
			mRGBA[3] = a;
			return *this;
		}

		Color& set(TDataType r, TDataType g, TDataType b, TDataType a)
		{
			mRGBA[0] = r;
			mRGBA[1] = g;
			mRGBA[2] = b;
			mRGBA[3] = a;
			return *this;
		}

		void write(TDataType* const dest) const
		{
			std::memcpy(dest, mRGBA, sizeof(TDataType) * 4);
		}

		void writeRGB(TDataType* const dest) const
		{
			dest[0] = mRGBA[0];
			dest[1] = mRGBA[1];
			dest[2] = mRGBA[2];
		}

		void swapRB()
		{
			std::swap(mRGBA[0], mRGBA[2]);
		}
	};

	using Colorf = Color<float>;
	using Colord = Color<double>;

	template<>
	class alignas(16) Color<float>
	{
		float mRGBA[4];

		static constexpr std::array<float, 256> TableSRGB2Linear{ 0.0, 0.00030f, 0.00061f, 0.00091f, 0.00121f, 0.00152f, 0.00182f, 0.00212f, 0.00243f, 0.00273f, 0.00304f, 0.00335f, 0.00368f, 0.00402f, 0.00439f, 0.00478f, 0.00518f, 0.00561f, 0.00605f, 0.00651f, 0.00700f, 0.00750f, 0.00802f, 0.00857f, 0.00913f, 0.00972f, 0.01033f, 0.01096f, 0.01161f, 0.01229f, 0.01298f, 0.01370f, 0.01444f, 0.01521f, 0.01600f, 0.01681f, 0.01764f, 0.01850f, 0.01938f, 0.02029f, 0.02122f, 0.02217f, 0.02315f, 0.02416f, 0.02519f, 0.02624f, 0.02732f, 0.02843f, 0.02956f, 0.03071f, 0.03190f, 0.03310f, 0.03434f, 0.03560f, 0.03689f, 0.03820f, 0.03955f, 0.04092f, 0.04231f, 0.04374f, 0.04519f, 0.04667f, 0.04817f, 0.04971f, 0.05127f, 0.05286f, 0.05448f, 0.05613f, 0.05781f, 0.05951f, 0.06125f, 0.06301f, 0.06480f, 0.06663f, 0.06848f, 0.07036f, 0.07227f, 0.07421f, 0.07619f, 0.07819f, 0.08022f, 0.08228f, 0.08438f, 0.08650f, 0.08866f, 0.09084f, 0.09306f, 0.09531f, 0.09759f, 0.09990f, 0.10224f, 0.10462f, 0.10702f, 0.10946f, 0.11193f, 0.11444f, 0.11697f, 0.11954f, 0.12214f, 0.12477f, 0.12744f, 0.13014f, 0.13287f, 0.13563f, 0.13843f, 0.14126f, 0.14413f, 0.14703f, 0.14996f, 0.15293f, 0.15593f, 0.15896f, 0.16203f, 0.16513f, 0.16827f, 0.17144f, 0.17465f, 0.17789f, 0.18116f, 0.18447f, 0.18782f, 0.19120f, 0.19462f, 0.19807f, 0.20156f, 0.20508f, 0.20864f, 0.21223f, 0.21586f, 0.21953f, 0.22323f, 0.22697f, 0.23074f, 0.23455f, 0.23840f, 0.24228f, 0.24620f, 0.25016f, 0.25415f, 0.25818f, 0.26225f, 0.26636f, 0.27050f, 0.27468f, 0.27889f, 0.28315f, 0.28744f, 0.29177f, 0.29614f, 0.30054f, 0.30499f, 0.30947f, 0.31399f, 0.31855f, 0.32314f, 0.32778f, 0.33245f, 0.33716f, 0.34191f, 0.34670f, 0.35153f, 0.35640f, 0.36131f, 0.36625f, 0.37124f, 0.37626f, 0.38133f, 0.38643f, 0.39157f, 0.39676f, 0.40198f, 0.40724f, 0.41254f, 0.41789f, 0.42327f, 0.42869f, 0.43415f, 0.43966f, 0.44520f, 0.45079f, 0.45641f, 0.46208f, 0.46778f, 0.47353f, 0.47932f, 0.48515f, 0.49102f, 0.49693f, 0.50289f, 0.50888f, 0.51492f, 0.52100f, 0.52712f, 0.53328f, 0.53948f, 0.54572f, 0.55201f, 0.55834f, 0.56471f, 0.57112f, 0.57758f, 0.58408f, 0.59062f, 0.59720f, 0.60383f, 0.61050f, 0.61721f, 0.62396f, 0.63076f, 0.63760f, 0.64448f, 0.65141f, 0.65837f, 0.66539f, 0.67244f, 0.67954f, 0.68669f, 0.69387f, 0.70110f, 0.70838f, 0.71569f, 0.72306f, 0.73046f, 0.73791f, 0.74540f, 0.75294f, 0.76052f, 0.76815f, 0.77582f, 0.78354f, 0.79130f, 0.79910f, 0.80695f, 0.81485f, 0.82279f, 0.83077f, 0.83880f, 0.84687f, 0.85499f, 0.86316f, 0.87137f, 0.87962f, 0.88792f, 0.89627f, 0.90466f, 0.91310f, 0.92158f, 0.93011f, 0.93869f, 0.94731f, 0.95597f, 0.96469f, 0.97345f, 0.98225f, 0.99110f, 1.0f };

	public:
		using DataType = float;

		class KnownColors
		{
		public:
			static constexpr uint8_t Transparent[] = { 255, 255, 255, 0 };
			static constexpr uint8_t PureRed[] = { 255, 0, 0, 255 };
			static constexpr uint8_t PureGreen[] = { 0, 255, 0, 255 };
			static constexpr uint8_t PureBlue[] = { 0, 0, 255, 255 };
			static constexpr uint8_t AliceBlue[] = { 240, 248, 255, 255 };
			static constexpr uint8_t AntiqueWhite[] = { 250, 235, 215, 255 };
			static constexpr uint8_t Aqua[] = { 0, 255, 255, 255 };
			static constexpr uint8_t Aquamarine[] = { 127, 255, 212, 255 };
			static constexpr uint8_t Azure[] = { 240, 255, 255, 255 };
			static constexpr uint8_t Beige[] = { 245, 245, 220, 255 };
			static constexpr uint8_t Bisque[] = { 255, 228, 196, 255 };
			static constexpr uint8_t Black[] = { 0, 0, 0, 255 };
			static constexpr uint8_t BlanchedAlmond[] = { 255, 235, 205, 255 };
			static constexpr uint8_t Blue[] = { 0, 0, 255, 255 };
			static constexpr uint8_t BlueViolet[] = { 138, 43, 226, 255 };
			static constexpr uint8_t Brown[] = { 165, 42, 42, 255 };
			static constexpr uint8_t BurlyWood[] = { 222, 184, 135, 255 };
			static constexpr uint8_t CadetBlue[] = { 95, 158, 160, 255 };
			static constexpr uint8_t Chartreuse[] = { 127, 255, 0, 255 };
			static constexpr uint8_t Chocolate[] = { 210, 105, 30, 255 };
			static constexpr uint8_t Coral[] = { 255, 127, 80, 255 };
			static constexpr uint8_t CornflowerBlue[] = { 100, 149, 237, 255 };
			static constexpr uint8_t Cornsilk[] = { 255, 248, 220, 255 };
			static constexpr uint8_t Crimson[] = { 220, 20, 60, 255 };
			static constexpr uint8_t Cyan[] = { 0, 255, 255, 255 };
			static constexpr uint8_t DarkBlue[] = { 0, 0, 139, 255 };
			static constexpr uint8_t DarkCyan[] = { 0, 139, 139, 255 };
			static constexpr uint8_t DarkGoldenrod[] = { 184, 134, 11, 255 };
			static constexpr uint8_t DarkGray[] = { 169, 169, 169, 255 };
			static constexpr uint8_t DarkGreen[] = { 0, 100, 0, 255 };
			static constexpr uint8_t DarkKhaki[] = { 189, 183, 107, 255 };
			static constexpr uint8_t DarkMagenta[] = { 139, 0, 139, 255 };
			static constexpr uint8_t DarkOliveGreen[] = { 85, 107, 47, 255 };
			static constexpr uint8_t DarkOrange[] = { 255, 140, 0, 255 };
			static constexpr uint8_t DarkOrchid[] = { 153, 50, 204, 255 };
			static constexpr uint8_t DarkRed[] = { 139, 0, 0, 255 };
			static constexpr uint8_t DarkSalmon[] = { 233, 150, 122, 255 };
			static constexpr uint8_t DarkSeaGreen[] = { 143, 188, 139, 255 };
			static constexpr uint8_t DarkSlateBlue[] = { 72, 61, 139, 255 };
			static constexpr uint8_t DarkSlateGray[] = { 47, 79, 79, 255 };
			static constexpr uint8_t DarkTurquoise[] = { 0, 206, 209, 255 };
			static constexpr uint8_t DarkViolet[] = { 148, 0, 211, 255 };
			static constexpr uint8_t DeepPink[] = { 255, 20, 147, 255 };
			static constexpr uint8_t DeepSkyBlue[] = { 0, 191, 255, 255 };
			static constexpr uint8_t DimGray[] = { 105, 105, 105, 255 };
			static constexpr uint8_t DodgerBlue[] = { 30, 144, 255, 255 };
			static constexpr uint8_t Firebrick[] = { 178, 34, 34, 255 };
			static constexpr uint8_t FloralWhite[] = { 255, 250, 240, 255 };
			static constexpr uint8_t ForestGreen[] = { 34, 139, 34, 255 };
			static constexpr uint8_t Fuchsia[] = { 255, 0, 255, 255 };
			static constexpr uint8_t Gainsboro[] = { 220, 220, 220, 255 };
			static constexpr uint8_t GhostWhite[] = { 248, 248, 255, 255 };
			static constexpr uint8_t Gold[] = { 255, 215, 0, 255 };
			static constexpr uint8_t Goldenrod[] = { 218, 165, 32, 255 };
			static constexpr uint8_t Gray[] = { 128, 128, 128, 255 };
			static constexpr uint8_t Green[] = { 0, 128, 0, 255 };
			static constexpr uint8_t GreenYellow[] = { 173, 255, 47, 255 };
			static constexpr uint8_t Honeydew[] = { 240, 255, 240, 255 };
			static constexpr uint8_t HotPink[] = { 255, 105, 180, 255 };
			static constexpr uint8_t IndianRed[] = { 205, 92, 92, 255 };
			static constexpr uint8_t Indigo[] = { 75, 0, 130, 255 };
			static constexpr uint8_t Ivory[] = { 255, 255, 240, 255 };
			static constexpr uint8_t Khaki[] = { 240, 230, 140, 255 };
			static constexpr uint8_t Lavender[] = { 230, 230, 250, 255 };
			static constexpr uint8_t LavenderBlush[] = { 255, 240, 245, 255 };
			static constexpr uint8_t LawnGreen[] = { 124, 252, 0, 255 };
			static constexpr uint8_t LemonChiffon[] = { 255, 250, 205, 255 };
			static constexpr uint8_t LightBlue[] = { 173, 216, 230, 255 };
			static constexpr uint8_t LightCoral[] = { 240, 128, 128, 255 };
			static constexpr uint8_t LightCyan[] = { 224, 255, 255, 255 };
			static constexpr uint8_t LightGoldenrodYellow[] = { 250, 250, 210, 255 };
			static constexpr uint8_t LightGreen[] = { 144, 238, 144, 255 };
			static constexpr uint8_t LightGray[] = { 211, 211, 211, 255 };
			static constexpr uint8_t LightPink[] = { 255, 182, 193, 255 };
			static constexpr uint8_t LightSalmon[] = { 255, 160, 122, 255 };
			static constexpr uint8_t LightSeaGreen[] = { 32, 178, 170, 255 };
			static constexpr uint8_t LightSkyBlue[] = { 135, 206, 250, 255 };
			static constexpr uint8_t LightSlateGray[] = { 119, 136, 153, 255 };
			static constexpr uint8_t LightSteelBlue[] = { 176, 196, 222, 255 };
			static constexpr uint8_t LightYellow[] = { 255, 255, 224, 255 };
			static constexpr uint8_t Lime[] = { 0, 255, 0, 255 };
			static constexpr uint8_t LimeGreen[] = { 50, 205, 50, 255 };
			static constexpr uint8_t Linen[] = { 250, 240, 230, 255 };
			static constexpr uint8_t Magenta[] = { 255, 0, 255, 255 };
			static constexpr uint8_t Maroon[] = { 128, 0, 0, 255 };
			static constexpr uint8_t MediumAquamarine[] = { 102, 205, 170, 255 };
			static constexpr uint8_t MediumBlue[] = { 0, 0, 205, 255 };
			static constexpr uint8_t MediumOrchid[] = { 186, 85, 211, 255 };
			static constexpr uint8_t MediumPurple[] = { 147, 112, 219, 255 };
			static constexpr uint8_t MediumSeaGreen[] = { 60, 179, 113, 255 };
			static constexpr uint8_t MediumSlateBlue[] = { 123, 104, 238, 255 };
			static constexpr uint8_t MediumSpringGreen[] = { 0, 250, 154, 255 };
			static constexpr uint8_t MediumTurquoise[] = { 72, 209, 204, 255 };
			static constexpr uint8_t MediumVioletRed[] = { 199, 21, 133, 255 };
			static constexpr uint8_t MidnightBlue[] = { 25, 25, 112, 255 };
			static constexpr uint8_t MintCream[] = { 245, 255, 250, 255 };
			static constexpr uint8_t MistyRose[] = { 255, 228, 225, 255 };
			static constexpr uint8_t Moccasin[] = { 255, 228, 181, 255 };
			static constexpr uint8_t NavajoWhite[] = { 255, 222, 173, 255 };
			static constexpr uint8_t Navy[] = { 0, 0, 128, 255 };
			static constexpr uint8_t OldLace[] = { 253, 245, 230, 255 };
			static constexpr uint8_t Olive[] = { 128, 128, 0, 255 };
			static constexpr uint8_t OliveDrab[] = { 107, 142, 35, 255 };
			static constexpr uint8_t Orange[] = { 255, 165, 0, 255 };
			static constexpr uint8_t OrangeRed[] = { 255, 69, 0, 255 };
			static constexpr uint8_t Orchid[] = { 218, 112, 214, 255 };
			static constexpr uint8_t PaleGoldenrod[] = { 238, 232, 170, 255 };
			static constexpr uint8_t PaleGreen[] = { 152, 251, 152, 255 };
			static constexpr uint8_t PaleTurquoise[] = { 175, 238, 238, 255 };
			static constexpr uint8_t PaleVioletRed[] = { 219, 112, 147, 255 };
			static constexpr uint8_t PapayaWhip[] = { 255, 239, 213, 255 };
			static constexpr uint8_t PeachPuff[] = { 255, 218, 185, 255 };
			static constexpr uint8_t Peru[] = { 205, 133, 63, 255 };
			static constexpr uint8_t Pink[] = { 255, 192, 203, 255 };
			static constexpr uint8_t Plum[] = { 221, 160, 221, 255 };
			static constexpr uint8_t PowderBlue[] = { 176, 224, 230, 255 };
			static constexpr uint8_t Purple[] = { 128, 0, 128, 255 };
			static constexpr uint8_t Red[] = { 255, 0, 0, 255 };
			static constexpr uint8_t RosyBrown[] = { 188, 143, 143, 255 };
			static constexpr uint8_t RoyalBlue[] = { 65, 105, 225, 255 };
			static constexpr uint8_t SaddleBrown[] = { 139, 69, 19, 255 };
			static constexpr uint8_t Salmon[] = { 250, 128, 114, 255 };
			static constexpr uint8_t SandyBrown[] = { 244, 164, 96, 255 };
			static constexpr uint8_t SeaGreen[] = { 46, 139, 87, 255 };
			static constexpr uint8_t SeaShell[] = { 255, 245, 238, 255 };
			static constexpr uint8_t Sienna[] = { 160, 82, 45, 255 };
			static constexpr uint8_t Silver[] = { 192, 192, 192, 255 };
			static constexpr uint8_t SkyBlue[] = { 135, 206, 235, 255 };
			static constexpr uint8_t SlateBlue[] = { 106, 90, 205, 255 };
			static constexpr uint8_t SlateGray[] = { 112, 128, 144, 255 };
			static constexpr uint8_t Snow[] = { 255, 250, 250, 255 };
			static constexpr uint8_t SpringGreen[] = { 0, 255, 127, 255 };
			static constexpr uint8_t SteelBlue[] = { 70, 130, 180, 255 };
			static constexpr uint8_t Tan[] = { 210, 180, 140, 255 };
			static constexpr uint8_t Teal[] = { 0, 128, 128, 255 };
			static constexpr uint8_t Thistle[] = { 216, 191, 216, 255 };
			static constexpr uint8_t Tomato[] = { 255, 99, 71, 255 };
			static constexpr uint8_t Turquoise[] = { 64, 224, 208, 255 };
			static constexpr uint8_t Violet[] = { 238, 130, 238, 255 };
			static constexpr uint8_t Wheat[] = { 245, 222, 179, 255 };
			static constexpr uint8_t White[] = { 255, 255, 255, 255 };
			static constexpr uint8_t WhiteSmoke[] = { 245, 245, 245, 255 };
			static constexpr uint8_t Yellow[] = { 255, 255, 0, 255 };
			static constexpr uint8_t YellowGreen[] = { 154, 205, 50, 255 };
		};

	public:
		static uint8_t convertColor(float val)
		{
			__m128i valConvert;
			uint8_t valFinal;

			valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_set_ss(val), _mm_set_ps1(Math::UByteMax<float>)));
			valConvert = _mm_packs_epi32(valConvert, valConvert);
			valConvert = _mm_packus_epi16(valConvert, valConvert);

			_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x80), (char*)&valFinal);
			return valFinal;
		}

		static float convertColor(uint8_t val)
		{
			return (static_cast<float>(val) * Math::UByteMaxInv<float>);
		}

		static void convertColor(uint8_t* valB, const float* const valF, bool processAlphaChannel)
		{
			__m128i valConvert;

			if (!processAlphaChannel)
			{
				valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_set_ps(0.0f, valF[2], valF[1], valF[0]), _mm_set_ps1(Math::UByteMax<float>)));
				valConvert = _mm_packs_epi32(valConvert, valConvert);
				valConvert = _mm_packus_epi16(valConvert, valConvert);

				_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x00808080), (char*)valB);
				return;
			}

			valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(valF), _mm_set_ps1(Math::UByteMax<float>)));
			valConvert = _mm_packs_epi32(valConvert, valConvert);
			valConvert = _mm_packus_epi16(valConvert, valConvert);

			_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x80808080), (char*)valB);
		}

		static void convertColor(float* const valF, const uint8_t* valB, bool processAlphaChannel)
		{
			__m128i valConvert;
			__m128 valFinal;

			if (!processAlphaChannel)
			{
				valConvert = _mm_cvtsi32_si128(((valB[2] << 16) | (valB[1] << 8) | valB[0]));
				valConvert = _mm_unpacklo_epi8(valConvert, _mm_setzero_si128());
				valConvert = _mm_unpacklo_epi16(valConvert, _mm_setzero_si128());

				valFinal = _mm_mul_ps(_mm_cvtepi32_ps(valConvert), _mm_set_ps1(Math::UByteMaxInv<float>));

				valF[0] = valFinal.m128_f32[0];
				valF[1] = valFinal.m128_f32[1];
				valF[2] = valFinal.m128_f32[2];
				return;
			}

			valConvert = _mm_cvtsi32_si128(((valB[3] << 24) | (valB[2] << 16) | (valB[1] << 8) | valB[0]));
			valConvert = _mm_unpacklo_epi8(valConvert, _mm_setzero_si128());
			valConvert = _mm_unpacklo_epi16(valConvert, _mm_setzero_si128());

			valFinal = _mm_mul_ps(_mm_cvtepi32_ps(valConvert), _mm_set_ps1(Math::UByteMaxInv<float>));
			_mm_store_ps(valF, valFinal);
		}

		static constexpr float convertSRGB2Linear(uint8_t value) noexcept
		{
			//we could do the math here, but since we only have 256 possible values, we use a table, built like this:
			//for (int i = 0; i < 256; i++)
			//{
			//	float c = (double)i * (1.0 / 255.0);
			//	if (c <= 0.04045)
			//		table[i] = c / 12.92;
			//	else
			//		table[i] = pow((c + 0.055) / 1.055, 2.4);
			//}

			return TableSRGB2Linear[value];
		}

		static constexpr float convertLinear2SRGB(float value) noexcept
		{
			if (!(value > 0.0f)) // also covers NaNs
				return 0.0f;

			if (value <= 0.0031308f)
				return (12.92f * value);
			if (value < 1.0f)
				return (1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f);

			return 1.0f;
		}

		static Color calcInterpolate(const Color& from, const Color& to, float t) noexcept
		{
			Color color;

			__m128 tmp;
			tmp = _mm_mul_ps(_mm_load_ps(from.mRGBA), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mRGBA), _mm_set_ps1(t)));
			_mm_store_ps(color.mRGBA, tmp);

			return color;
		}

		static Color parseFromHTML(const char* hexColor, bool gammaCorrect = true) noexcept
		{
			if (*hexColor == '#')
				hexColor++;

			Color color;
			if (gammaCorrect)
			{
				color.mRGBA[0] = convertSRGB2Linear(Encoders::decodeHexByte(hexColor + 0));
				color.mRGBA[1] = convertSRGB2Linear(Encoders::decodeHexByte(hexColor + 2));
				color.mRGBA[2] = convertSRGB2Linear(Encoders::decodeHexByte(hexColor + 4));
				color.mRGBA[3] = 255.0f;
			}
			else
			{
				color.mRGBA[0] = Encoders::decodeHexByte(hexColor + 0);
				color.mRGBA[1] = Encoders::decodeHexByte(hexColor + 2);
				color.mRGBA[2] = Encoders::decodeHexByte(hexColor + 4);
				color.mRGBA[3] = 255.0f;
			}

			_mm_store_ps(color.mRGBA, _mm_mul_ps(_mm_load_ps(color.mRGBA), _mm_set_ps1(Math::UByteMaxInv<float>)));

			return color;
		}

	public:
		constexpr Color() = default;

		explicit constexpr Color(float rgba) noexcept
			: mRGBA{ rgba, rgba, rgba, rgba }
		{ }

		explicit constexpr Color(float rgb, float a) noexcept
			: mRGBA{ rgb, rgb, rgb, a }
		{ }

		constexpr Color(float r, float g, float b, float a = 1.0f) noexcept
			: mRGBA{ r, g, b, a }
		{ }

		explicit constexpr Color(const float* const rgba) noexcept
			: mRGBA{ rgba[0], rgba[1], rgba[2], rgba[3] }
		{ }

		explicit Color(const uint8_t* const rgba) noexcept
		{
			Color::convertColor(mRGBA, rgba, true);
		}

		explicit Color(const Vector3f& v) noexcept
		{
			_mm_store_ps(mRGBA, _mm_load_ps(v.data()));
			mRGBA[3] = 1.0f;
		}

		explicit Color(const Vector4f& v) noexcept
		{
			_mm_store_ps(mRGBA, _mm_load_ps(v.data()));
		}

		constexpr float* data() noexcept
		{
			return mRGBA;
		}

		constexpr const float* data() const noexcept
		{
			return mRGBA;
		}

		constexpr float& operator[] (const size_t index) noexcept
		{
			return mRGBA[index % 4];
		}

		constexpr const float& operator[] (const size_t index) const noexcept
		{
			return mRGBA[index % 4];
		}

		void operator+=(const Color& c)
		{
			_mm_store_ps(mRGBA, _mm_add_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));
		}

		void operator-=(const Color& c)
		{
			_mm_store_ps(mRGBA, _mm_sub_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));
		}

		void operator*=(const Color& c)
		{
			_mm_store_ps(mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));
		}

		void operator/=(const Color& c)
		{
			_mm_store_ps(mRGBA, _mm_div_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));
		}

		void operator+=(const float n)
		{
			_mm_store_ps(mRGBA, _mm_add_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));
		}

		void operator-=(const float n)
		{
			_mm_store_ps(mRGBA, _mm_sub_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));
		}

		void operator*=(const float n)
		{
			_mm_store_ps(mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));
		}

		void operator/=(const float n)
		{
			_mm_store_ps(mRGBA, _mm_div_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));
		}

		Color operator+(const Color& c) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_add_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));

			return result;
		}

		Color operator-(const Color& c) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_sub_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));

			return result;
		}

		Color operator*(const Color& c) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));

			return result;
		}

		Color operator/(const Color& c) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_div_ps(_mm_load_ps(mRGBA), _mm_load_ps(c.mRGBA)));

			return result;
		}

		Color operator+(const float n) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_add_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));

			return result;
		}

		Color operator-(const float n) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_sub_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));

			return result;
		}

		Color operator*(const float n) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));

			return result;
		}

		Color operator/(const float n) const
		{
			Color result;
			_mm_store_ps(result.mRGBA, _mm_div_ps(_mm_load_ps(mRGBA), _mm_set_ps1(n)));

			return result;
		}

		Color& set(const Color& color)
		{
			_mm_store_ps(mRGBA, _mm_load_ps(color.mRGBA));
			return *this;
		}

		Color& set(const Color& color, float a)
		{
			_mm_store_ps(mRGBA, _mm_load_ps(color.mRGBA));
			mRGBA[3] = a;

			return *this;
		}

		Color& set(const float* color)
		{
			_mm_store_ps(mRGBA, _mm_load_ps(color));
			return *this;
		}

		Color& set(const float* const color, float a)
		{
			mRGBA[0] = color[0];
			mRGBA[1] = color[1];
			mRGBA[2] = color[2];
			mRGBA[3] = a;

			return *this;
		}

		Color& set(float crgba)
		{
			_mm_store_ps(mRGBA, _mm_set_ps1(crgba));
			return *this;
		}

		Color& set(float crgb, float a)
		{
			mRGBA[0] = mRGBA[1] = mRGBA[2] = crgb;
			mRGBA[3] = a;
			return *this;
		}

		Color& set(float r, float g, float b, float a)
		{
			mRGBA[0] = r;
			mRGBA[1] = g;
			mRGBA[2] = b;
			mRGBA[3] = a;

			return *this;
		}

		Color& set(const uint8_t* color)
		{
			Color::convertColor(mRGBA, color, true);
			return *this;
		}

		Color& set(const uint8_t* color, uint8_t a)
		{
			mRGBA[0] = static_cast<float>(color[0]);
			mRGBA[1] = static_cast<float>(color[1]);
			mRGBA[2] = static_cast<float>(color[2]);
			mRGBA[3] = static_cast<float>(a);
			_mm_store_ps(mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_set_ps1(Math::UByteMaxInv<float>)));

			return *this;
		}

		Color& set(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			mRGBA[0] = static_cast<float>(r);
			mRGBA[1] = static_cast<float>(g);
			mRGBA[2] = static_cast<float>(b);
			mRGBA[3] = static_cast<float>(a);
			_mm_store_ps(mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_set_ps1(Math::UByteMaxInv<float>)));

			return *this;
		}

		Color& set(uint8_t r, uint8_t g, uint8_t b)
		{
			mRGBA[0] = static_cast<float>(r);
			mRGBA[1] = static_cast<float>(g);
			mRGBA[2] = static_cast<float>(b);
			mRGBA[3] = 255.0f;
			_mm_store_ps(mRGBA, _mm_mul_ps(_mm_load_ps(mRGBA), _mm_set_ps1(Math::UByteMaxInv<float>)));

			return *this;
		}

		Color& setWeight(const Color& c, const float weight)
		{
			_mm_store_ps(mRGBA, _mm_mul_ps(_mm_load_ps(c.mRGBA), _mm_set_ps1(weight)));
			return *this;
		}

		Color& setWeight(const Color& c1, const float weight1, const Color& c2, const float weight2, const Color& c3, const float weight3, const Color& c4, const float weight4)
		{
			__m128 temp;

			temp = _mm_mul_ps(_mm_load_ps(c1.mRGBA), _mm_set_ps1(weight1));
			temp = _mm_add_ps(temp, _mm_mul_ps(_mm_load_ps(c2.mRGBA), _mm_set_ps1(weight2)));
			temp = _mm_add_ps(temp, _mm_mul_ps(_mm_load_ps(c3.mRGBA), _mm_set_ps1(weight3)));
			_mm_store_ps(mRGBA, _mm_add_ps(temp, _mm_mul_ps(_mm_load_ps(c4.mRGBA), _mm_set_ps1(weight4))));

			return *this;
		}

		Color& setYUV(const float * const yuv)
		{
			setYUV(yuv[0], yuv[1], yuv[2]);
			return *this;
		}

		Color& setYUV(float y, float u, float v)
		{
			mRGBA[0] = y + (v * 1.140f);
			mRGBA[1] = y - ((u * 0.395f) + (v * 0.581f));
			mRGBA[2] = y + (u * 2.032f);
			mRGBA[3] = 1.0f;

			return *this;
		}

		Color& setYUV(const uint8_t* const yuv)
		{
			setYUV(yuv[0], yuv[1], yuv[2]);
			return *this;
		}

		Color& setYUV(uint8_t y, uint8_t u, uint8_t v)
		{
			alignas(16) float pixelAux[4];

			pixelAux[0] = y;
			pixelAux[1] = u;
			pixelAux[2] = v;
			pixelAux[3] = 0.0;
			_mm_store_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), _mm_set_ps1(Math::UByteMaxInv<float>)));

			setYUV(pixelAux[0], pixelAux[1], pixelAux[2]);

			return *this;
		}

		Color& setYCbCr(const float * const ycbcr, bool fullRange)
		{
			setYCbCr(ycbcr[0], ycbcr[1], ycbcr[2], fullRange);
			return *this;
		}

		Color& setYCbCr(float y, float cb, float cr, bool fullRange)
		{
			float auxCb = cb - 0.5f;
			float auxCr = cr - 0.5f;

			if (fullRange)
			{
				mRGBA[0] = y + (auxCr * 1.400f);
				mRGBA[1] = y - ((auxCb * 0.343f) + (auxCr * 0.711f));
				mRGBA[2] = y + (auxCb * 1.765f);
			}
			else
			{
				float auxY = y - (16.0f / 255.0f);
				mRGBA[0] = (auxY * 1.164f) + (auxCr * 1.596f);
				mRGBA[1] = (auxY * 1.164f) - ((auxCb * 0.392f) + (auxCr * 0.813f));
				mRGBA[2] = (auxY * 1.164f) + (auxCb * 2.017f);
			}

			mRGBA[0] = 1.0f;

			return *this;
		}

		Color& setYCbCr(const uint8_t* const ycbcr, bool fullRange)
		{
			setYCbCr(ycbcr[0], ycbcr[1], ycbcr[2], fullRange);
			return *this;
		}

		Color& setYCbCr(uint8_t y, uint8_t cb, uint8_t cr, bool fullRange)
		{
			alignas(16) float pixelAux[4];

			pixelAux[0] = y;
			pixelAux[1] = cb;
			pixelAux[2] = cr;
			pixelAux[3] = 1.0;
			_mm_store_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), _mm_set_ps1(Math::UByteMaxInv<float>)));

			setYCbCr(pixelAux[0], pixelAux[1], pixelAux[2], fullRange);

			return *this;
		}

		Color& setYPbPr(const float * const ypbpr, bool coefficientsSDTV)
		{
			setYPbPr(ypbpr[0], ypbpr[1], ypbpr[2], coefficientsSDTV);
			return *this;
		}

		Color& setYPbPr(float y, float pb, float pr, bool coefficientsSDTV)
		{
			if (coefficientsSDTV)
			{
				mRGBA[0] = y + (pr * 1.402f);
				mRGBA[1] = y - ((pb * 0.344f) + (pr * 0.714f));
				mRGBA[2] = y + (pb * 1.772f);
			}
			else
			{
				mRGBA[0] = y + (pr * 1.575f);
				mRGBA[1] = y - ((pb * 0.187f) + (pr * 0.468f));
				mRGBA[2] = y + (pb * 1.856f);
			}

			mRGBA[3] = 1.0f;

			return *this;
		}

		Color& setYPbPr(const uint8_t* const ypbpr, bool coefficientsSDTV)
		{
			setYPbPr(ypbpr[0], ypbpr[1], ypbpr[2], coefficientsSDTV);
			return *this;
		}

		Color& setYPbPr(uint8_t y, uint8_t pb, uint8_t pr, bool coefficientsSDTV)
		{
			alignas(16) float pixelAux[4];

			pixelAux[0] = y;
			pixelAux[1] = pb;
			pixelAux[2] = pr;
			pixelAux[3] = 1.0;
			_mm_store_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), _mm_set_ps1(Math::UByteMaxInv<float>)));

			setYPbPr(pixelAux[0], pixelAux[1], pixelAux[2], coefficientsSDTV);

			return *this;
		}

		Color& setCMYK(const float * const cmyk)
		{
			setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
			return *this;
		}

		Color& setCMYK(float c, float m, float y, float k)
		{
			mRGBA[0] = 1.0f - (c * (1.0f - k)) + k;
			mRGBA[1] = 1.0f - (m * (1.0f - k)) + k;
			mRGBA[2] = 1.0f - (y * (1.0f - k)) + k;
			mRGBA[3] = 1.0f;

			return *this;
		}

		Color& setCMYK(const uint8_t* const cmyk)
		{
			setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
			return *this;
		}

		Color& setCMYK(uint8_t c, uint8_t m, uint8_t y, uint8_t k)
		{
			alignas(16) float pixelAux[4];

			pixelAux[0] = c;
			pixelAux[1] = m;
			pixelAux[2] = y;
			pixelAux[3] = k;
			_mm_store_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), _mm_set_ps1(Math::UByteMaxInv<float>)));

			setCMYK(pixelAux[0], pixelAux[1], pixelAux[2], pixelAux[3]);

			return *this;
		}

		template<typename TTargetType>
		Color<TTargetType> convert() const;

		void write(float * const dest) const
		{
			std::memcpy(dest, mRGBA, sizeof(float) * 4);
		}

		void write(uint8_t* const dest) const
		{
			Color::convertColor(dest, mRGBA, true);
		}

		void writeRGB(float * const dest) const
		{
			dest[0] = mRGBA[0];
			dest[1] = mRGBA[1];
			dest[2] = mRGBA[2];
		}

		void writeRGB(uint8_t* const dest) const
		{
			Color::convertColor(dest, mRGBA, false);
		}

		void writeYUV(float * const dest) const
		{
			dest[0] = (mRGBA[0] * 0.299f) + (mRGBA[1] * 0.587f) + (mRGBA[2] * 0.114f);
			dest[1] = -(mRGBA[0] * 0.147f) - (mRGBA[1] * 0.289f) + (mRGBA[2] * 0.436f);
			dest[2] = (mRGBA[0] * 0.615f) - (mRGBA[1] * 0.515f) - (mRGBA[2] * 0.100f);
		}

		void writeYUV(uint8_t* const dest) const
		{
			float bufferAux[3];

			writeYUV(bufferAux);
			Color::convertColor(dest, bufferAux, false);
		}

		void writeYCbCr(float * const dest, bool fullRange) const
		{
			if (fullRange)
			{
				dest[0] = (mRGBA[0] * 0.299f) + (mRGBA[1] * 0.587f) + (mRGBA[2] * 0.114f);
				dest[1] = -(mRGBA[0] * 0.169f) - (mRGBA[1] * 0.331f) + (mRGBA[2] * 0.500f);
				dest[2] = (mRGBA[0] * 0.500f) - (mRGBA[1] * 0.419f) - (mRGBA[2] * 0.081f);
			}
			else
			{
				dest[0] = (mRGBA[0] * 0.257f) + (mRGBA[1] * 0.504f) + (mRGBA[2] * 0.098f);
				dest[1] = -(mRGBA[0] * 0.148f) - (mRGBA[1] * 0.291f) + (mRGBA[2] * 0.439f);
				dest[2] = (mRGBA[0] * 0.439f) - (mRGBA[1] * 0.368f) - (mRGBA[2] * 0.071f);
			}
		}

		void writeYCbCr(uint8_t* const dest, bool fullRange) const
		{
			float bufferAux[3];

			writeYCbCr(bufferAux, fullRange);
			Color::convertColor(dest, bufferAux, false);
		}

		void writeYPbPr(float * const dest, bool coefficientsSDTV) const
		{
			if (coefficientsSDTV)
			{
				dest[0] = (mRGBA[0] * 0.299f) + (mRGBA[1] * 0.587f) + (mRGBA[2] * 0.114f);
				dest[1] = -(mRGBA[0] * 0.169f) - (mRGBA[1] * 0.331f) + (mRGBA[2] * 0.500f);
				dest[2] = (mRGBA[0] * 0.500f) - (mRGBA[1] * 0.419f) - (mRGBA[2] * 0.081f);
			}
			else
			{
				dest[0] = (mRGBA[0] * 0.213f) + (mRGBA[1] * 0.715f) + (mRGBA[2] * 0.072f);
				dest[1] = -(mRGBA[0] * 0.115f) - (mRGBA[1] * 0.385f) + (mRGBA[2] * 0.500f);
				dest[2] = (mRGBA[0] * 0.500f) - (mRGBA[1] * 0.454f) - (mRGBA[2] * 0.046f);
			}
		}
		void writeYPbPr(uint8_t* const dest, bool coefficientsSDTV) const
		{
			float bufferAux[3];

			writeYPbPr(bufferAux, coefficientsSDTV);
			Color::convertColor(dest, bufferAux, false);
		}

		void writeCMYK(float * const dest) const
		{
			if (Math::isZero(mRGBA[0]) && Math::isZero(mRGBA[1]) && Math::isZero(mRGBA[2]))
			{
				dest[0] = dest[1] = dest[2] = 0.0f;
				dest[3] = 1.0f;
				return;
			}

			float w = std::fmax(mRGBA[0], std::fmax(mRGBA[1], mRGBA[2]));
			float wInv = 1.0f / w;

			dest[0] = (w - mRGBA[0]) * wInv;
			dest[1] = (w - mRGBA[1]) * wInv;
			dest[2] = (w - mRGBA[2]) * wInv;
			dest[3] = 1.0f - w;
		}

		void writeCMYK(uint8_t* const dest) const
		{
			float bufferAux[4];

			writeCMYK(bufferAux);
			Color::convertColor(dest, bufferAux, true);
		}

		void addColorWeighted(const Color &color, float weight)
		{
			__m128 temp;

			temp = _mm_mul_ps(_mm_load_ps(color.mRGBA), _mm_set_ps1(weight));
			_mm_store_ps(mRGBA, _mm_add_ps(_mm_load_ps(mRGBA), temp));
		}

		void toGrayscale()
		{
			float newLum = mRGBA[0] * 0.3019607843f;
			newLum += mRGBA[1] * 0.5921568627f;
			newLum += mRGBA[2] * 0.1098039215f;

			mRGBA[0] = newLum;
			mRGBA[1] = newLum;
			mRGBA[2] = newLum;
		}

		void toGrayscale(float rWeight, float gWeight, float bWeight)
		{
			float newLum = mRGBA[0] * rWeight;
			newLum += mRGBA[1] * gWeight;
			newLum += mRGBA[2] * bWeight;

			mRGBA[0] = newLum;
			mRGBA[1] = newLum;
			mRGBA[2] = newLum;
		}

		void negativeRGB()
		{
			float alphaAux = mRGBA[3];
			_mm_store_ps(mRGBA, _mm_sub_ps(_mm_set_ps1(1.0f), _mm_load_ps(mRGBA)));
			mRGBA[3] = alphaAux;
		}

		void negativeRGB(float value)
		{
			float alphaAux = mRGBA[3];
			_mm_store_ps(mRGBA, _mm_sub_ps(_mm_set_ps1(value), _mm_load_ps(mRGBA)));
			mRGBA[3] = alphaAux;
		}

		void swapRB()
		{
			std::swap(mRGBA[0], mRGBA[2]);
		}

		void weightRGB(float weight)
		{
			mRGBA[0] *= weight;
			mRGBA[1] *= weight;
			mRGBA[2] *= weight;
		}

		void mad(float opMul, float opAdd)
		{
			__m128 pixelVal, valMul, valAdd;

			pixelVal = _mm_load_ps(mRGBA);
			valMul = _mm_set_ps1(opMul);
			valAdd = _mm_set_ps1(opAdd);

			pixelVal = _mm_mul_ps(pixelVal, valMul);
			_mm_store_ps(mRGBA, _mm_add_ps(pixelVal, valAdd));
		}

		void clamp()
		{
			_mm_store_ps(mRGBA, _mm_min_ps(_mm_max_ps(_mm_load_ps(mRGBA), _mm_setzero_ps()), _mm_set_ps1(1.0f)));
		}

		void clamp(float min, float max)
		{
			_mm_store_ps(mRGBA, _mm_min_ps(_mm_max_ps(_mm_load_ps(mRGBA), _mm_set_ps1(min)), _mm_set_ps1(max)));
		}

		void interpolate(const Color& to, float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(mRGBA), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mRGBA), _mm_set_ps1(t)));
			_mm_store_ps(mRGBA, tmp);
		}

		void setInterpolate(const Color &from, const Color &to, float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(from.mRGBA), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mRGBA), _mm_set_ps1(t)));
			_mm_store_ps(mRGBA, tmp);
		}

		void setInterpolate(const float * const from, const float * const to, float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(from), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to), _mm_set_ps1(t)));
			_mm_store_ps(mRGBA, tmp);
		}
	};

	template<>
	class alignas(32) Color<double>
	{
		double mRGBA[4];

	public:
		using DataType = double;

	public:
		static Color calcInterpolate(const Color& from, const Color& to, double t) noexcept
		{
			Color color;

			__m256d tmp;
			tmp = _mm256_mul_pd(_mm256_load_pd(from.mRGBA), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mRGBA), _mm256_set1_pd(t)));
			_mm256_store_pd(color.mRGBA, tmp);

			return color;
		}

	public:
		constexpr Color() = default;

		explicit constexpr Color(double rgb, double a = 1.0) noexcept
			: mRGBA{ rgb, rgb , rgb , a }
		{ }

		explicit constexpr Color(double r, double g, double b, double a = 1.0) noexcept
			: mRGBA{ r, g, b, a }
		{ }

		explicit constexpr Color(const double* const rgba) noexcept
			: mRGBA{ rgba[0], rgba[1], rgba[2], rgba[3] }
		{ }

		explicit Color(const Vector3f& v) noexcept
		{
			_mm256_store_pd(mRGBA, _mm256_cvtps_pd(_mm_load_ps(v.data())));
			mRGBA[3] = 1.0;
		}

		explicit Color(const Vector3d& v) noexcept
		{
			_mm256_store_pd(mRGBA, _mm256_load_pd(v.data()));
			mRGBA[3] = 1.0;
		}

		explicit Color(const Vector4f& v) noexcept
		{
			_mm256_store_pd(mRGBA, _mm256_cvtps_pd(_mm_load_ps(v.data())));
		}

		constexpr double* data() noexcept
		{
			return mRGBA;
		}

		constexpr const double* data() const noexcept
		{
			return mRGBA;
		}

		constexpr double& operator[] (const size_t index) noexcept
		{
			return mRGBA[index % 4];
		}

		constexpr const double& operator[] (const size_t index) const noexcept
		{
			return mRGBA[index % 4];
		}

		void operator+=(const Color& c)
		{
			_mm256_store_pd(mRGBA, _mm256_add_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));
		}

		void operator-=(const Color& c)
		{
			_mm256_store_pd(mRGBA, _mm256_sub_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));
		}

		void operator*=(const Color& c)
		{
			_mm256_store_pd(mRGBA, _mm256_mul_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));
		}

		void operator/=(const Color& c)
		{
			_mm256_store_pd(mRGBA, _mm256_div_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));
		}

		void operator+=(const double n)
		{
			_mm256_store_pd(mRGBA, _mm256_add_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));
		}

		void operator-=(const double n)
		{
			_mm256_store_pd(mRGBA, _mm256_sub_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));
		}

		void operator*=(const double n)
		{
			_mm256_store_pd(mRGBA, _mm256_mul_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));
		}

		void operator/=(const double n)
		{
			_mm256_store_pd(mRGBA, _mm256_div_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));
		}

		Color operator+(const Color& c) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_add_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));

			return result;
		}

		Color operator-(const Color& c) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_sub_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));

			return result;
		}

		Color operator*(const Color& c) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_mul_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));

			return result;
		}

		Color operator/(const Color& c) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_div_pd(_mm256_load_pd(mRGBA), _mm256_load_pd(c.mRGBA)));

			return result;
		}

		Color operator+(const double n) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_add_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));

			return result;
		}

		Color operator-(const double n) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_sub_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));

			return result;
		}

		Color operator*(const double n) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_mul_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));

			return result;
		}

		Color operator/(const double n) const
		{
			Color result;
			_mm256_store_pd(result.mRGBA, _mm256_div_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(n)));

			return result;
		}

		Color& set(const Color& color)
		{
			_mm256_store_pd(mRGBA, _mm256_load_pd(color.mRGBA));
			return *this;
		}

		Color& set(const Color& color, double a)
		{
			_mm256_store_pd(mRGBA, _mm256_load_pd(color.mRGBA));
			mRGBA[3] = a;

			return *this;
		}

		Color& set(const double* color)
		{
			_mm256_store_pd(mRGBA, _mm256_load_pd(color));
			return *this;
		}

		Color& set(const double* const color, double a)
		{
			mRGBA[0] = color[0];
			mRGBA[1] = color[1];
			mRGBA[2] = color[2];
			mRGBA[3] = a;

			return *this;
		}

		Color& set(double rgba)
		{
			_mm256_store_pd(mRGBA, _mm256_set1_pd(rgba));
			return *this;
		}

		Color& set(double rgb, double a)
		{
			mRGBA[0] = mRGBA[1] = mRGBA[2] = rgb;
			mRGBA[3] = a;
			return *this;
		}

		Color& set(double r, double g, double b, double a)
		{
			mRGBA[0] = r;
			mRGBA[1] = g;
			mRGBA[2] = b;
			mRGBA[3] = a;

			return *this;
		}

		Color& setWeight(const Color& c, const double weight)
		{
			_mm256_store_pd(mRGBA, _mm256_mul_pd(_mm256_load_pd(c.mRGBA), _mm256_set1_pd(weight)));
			return *this;
		}

		Color& setWeight(const Color& c1, const double weight1, const Color& c2, const double weight2, const Color& c3, const double weight3, const Color& c4, const double weight4)
		{
			__m256d temp;

			temp = _mm256_mul_pd(_mm256_load_pd(c1.mRGBA), _mm256_set1_pd(weight1));
			temp = _mm256_add_pd(temp, _mm256_mul_pd(_mm256_load_pd(c2.mRGBA), _mm256_set1_pd(weight2)));
			temp = _mm256_add_pd(temp, _mm256_mul_pd(_mm256_load_pd(c3.mRGBA), _mm256_set1_pd(weight3)));
			_mm256_store_pd(mRGBA, _mm256_add_pd(temp, _mm256_mul_pd(_mm256_load_pd(c4.mRGBA), _mm256_set1_pd(weight4))));

			return *this;
		}

		Color& setYCbCr(const double* const ycbcr, bool fullRange)
		{
			setYCbCr(ycbcr[0], ycbcr[1], ycbcr[2], fullRange);
			return *this;
		}

		Color& setYCbCr(double y, double cb, double cr, bool fullRange)
		{
			double auxCb = cb - 0.5;
			double auxCr = cr - 0.5;

			if (fullRange)
			{
				mRGBA[0] = y + (auxCr * 1.400);
				mRGBA[1] = y - ((auxCb * 0.343) + (auxCr * 0.711));
				mRGBA[2] = y + (auxCb * 1.765);
			}
			else
			{
				double auxY = y - (16.0 / 255.0);
				mRGBA[0] = (auxY * 1.164) + (auxCr * 1.596);
				mRGBA[1] = (auxY * 1.164) - ((auxCb * 0.392) + (auxCr * 0.813));
				mRGBA[2] = (auxY * 1.164) + (auxCb * 2.017);
			}

			mRGBA[0] = 1.0;

			return *this;
		}

		Color& setCMYK(const double* const cmyk)
		{
			setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
			return *this;
		}

		Color& setCMYK(double c, double m, double y, double k)
		{
			mRGBA[0] = 1.0 - (c * (1.0 - k)) + k;
			mRGBA[1] = 1.0 - (m * (1.0 - k)) + k;
			mRGBA[2] = 1.0 - (y * (1.0 - k)) + k;
			mRGBA[3] = 1.0;

			return *this;
		}

		template<typename TTargetType>
		Color<TTargetType> convert() const;

		void write(double* const dest) const
		{
			std::memcpy(dest, mRGBA, sizeof(double) * 4);
		}

		void writeRGB(double* const dest) const
		{
			dest[0] = mRGBA[0];
			dest[1] = mRGBA[1];
			dest[2] = mRGBA[2];
		}

		void writeYCbCr(double* const dest, bool fullRange) const
		{
			if (fullRange)
			{
				dest[0] = (mRGBA[0] * 0.299) + (mRGBA[1] * 0.587) + (mRGBA[2] * 0.114);
				dest[1] = -(mRGBA[0] * 0.169) - (mRGBA[1] * 0.331) + (mRGBA[2] * 0.500);
				dest[2] = (mRGBA[0] * 0.500) - (mRGBA[1] * 0.419) - (mRGBA[2] * 0.081);
			}
			else
			{
				dest[0] = (mRGBA[0] * 0.257) + (mRGBA[1] * 0.504) + (mRGBA[2] * 0.098);
				dest[1] = -(mRGBA[0] * 0.148) - (mRGBA[1] * 0.291) + (mRGBA[2] * 0.439);
				dest[2] = (mRGBA[0] * 0.439) - (mRGBA[1] * 0.368) - (mRGBA[2] * 0.071);
			}
		}

		void writeCMYK(double* const dest) const
		{
			if (Math::isZero(mRGBA[0]) && Math::isZero(mRGBA[1]) && Math::isZero(mRGBA[2]))
			{
				dest[0] = dest[1] = dest[2] = 0.0;
				dest[3] = 1.0;
				return;
			}

			double w = std::fmax(mRGBA[0], std::fmax(mRGBA[1], mRGBA[2]));
			double wInv = 1.0 / w;

			dest[0] = (w - mRGBA[0]) * wInv;
			dest[1] = (w - mRGBA[1]) * wInv;
			dest[2] = (w - mRGBA[2]) * wInv;
			dest[3] = 1.0 - w;
		}

		void addColorWeighted(const Color& color, double weight)
		{
			__m256d temp;

			temp = _mm256_mul_pd(_mm256_load_pd(color.mRGBA), _mm256_set1_pd(weight));
			_mm256_store_pd(mRGBA, _mm256_add_pd(_mm256_load_pd(mRGBA), temp));
		}

		void toGrayscale()
		{
			double newLum = mRGBA[0] * 0.3019607843;
			newLum += mRGBA[1] * 0.5921568627;
			newLum += mRGBA[2] * 0.1098039215;

			mRGBA[0] = newLum;
			mRGBA[1] = newLum;
			mRGBA[2] = newLum;
		}

		void toGrayscale(double rWeight, double gWeight, double bWeight)
		{
			double newLum = mRGBA[0] * rWeight;
			newLum += mRGBA[1] * gWeight;
			newLum += mRGBA[2] * bWeight;

			mRGBA[0] = newLum;
			mRGBA[1] = newLum;
			mRGBA[2] = newLum;
		}

		void negativeRGB()
		{
			double alphaAux = mRGBA[3];
			_mm256_store_pd(mRGBA, _mm256_sub_pd(_mm256_set1_pd(1.0), _mm256_load_pd(mRGBA)));
			mRGBA[3] = alphaAux;
		}

		void negativeRGB(double value)
		{
			double alphaAux = mRGBA[3];
			_mm256_store_pd(mRGBA, _mm256_sub_pd(_mm256_set1_pd(value), _mm256_load_pd(mRGBA)));
			mRGBA[3] = alphaAux;
		}

		void swapRB()
		{
			std::swap(mRGBA[0], mRGBA[2]);
		}

		void weightRGB(double weight)
		{
			mRGBA[0] *= weight;
			mRGBA[1] *= weight;
			mRGBA[2] *= weight;
		}

		void mad(double opMul, double opAdd)
		{
			__m256d valMul, valAdd;

			valMul = _mm256_set1_pd(opMul);
			valAdd = _mm256_set1_pd(opAdd);

			_mm256_store_pd(mRGBA, _mm256_add_pd(_mm256_mul_pd(_mm256_load_pd(mRGBA), valMul), valAdd));
		}

		void clamp()
		{
			_mm256_store_pd(mRGBA, _mm256_min_pd(_mm256_max_pd(_mm256_load_pd(mRGBA), _mm256_setzero_pd()), _mm256_set1_pd(1.0)));
		}

		void clamp(double min, double max)
		{
			_mm256_store_pd(mRGBA, _mm256_min_pd(_mm256_max_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(min)), _mm256_set1_pd(max)));
		}

		void interpolate(const Color& to, double t)
		{
			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(mRGBA), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mRGBA), _mm256_set1_pd(t)));
			_mm256_store_pd(mRGBA, tmp);
		}

		void setInterpolate(const Color& from, const Color& to, double t)
		{
			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(from.mRGBA), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mRGBA), _mm256_set1_pd(t)));
			_mm256_store_pd(mRGBA, tmp);
		}

		void setInterpolate(const double* const from, const double* const to, double t)
		{
			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(from), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to), _mm256_set1_pd(t)));
			_mm256_store_pd(mRGBA, tmp);
		}
	};

	static_assert(alignof(Color<float>) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
	static_assert(alignof(Color<double>) == 32, "For performance reasons, this class must be aligned to a 32 byte boundary");
	static_assert(std::is_trivially_copyable_v<Color<float>>, "For performance reasons, this class should be trivially copyable");
	static_assert(std::is_trivially_copyable_v<Color<double>>, "For performance reasons, this class should be trivially copyable");

	template<typename TTargetType>
	Color<TTargetType> Color<float>::convert() const
	{
		if constexpr (std::is_same_v<TTargetType, float>)
		{
			Color<float> newColor;
			_mm_store_ps(newColor.data(), _mm_load_ps(mRGBA));

			return newColor;
		}
		else if constexpr (std::is_same_v<TTargetType, double>)
		{
			Color<double> newColor;
			_mm256_store_pd(newColor.data(), _mm256_cvtps_pd(_mm_load_ps(mRGBA)));

			return newColor;
		}
		else
		{
			static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
			return {};
		}
	}

	template<typename TTargetType>
	Color<TTargetType> Color<double>::convert() const
	{
		if constexpr (std::is_same_v<TTargetType, double>)
		{
			Color<double> newColor;
			_mm256_store_pd(newColor.data(), _mm256_load_pd(mRGBA));

			return newColor;
		}
		else if constexpr (std::is_same_v<TTargetType, float>)
		{
			Color<float> newColor;
			_mm_store_ps(newColor.data(), _mm256_cvtpd_ps(_mm256_load_pd(mRGBA)));

			return newColor;
		}
		else
		{
			static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
			return {};
		}
	}
}
