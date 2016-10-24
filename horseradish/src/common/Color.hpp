#pragma once

#include "Types.hpp"
#include "Vector.hpp"
#include "Math.hpp"
#include "Encoders.hpp"

#include <xmmintrin.h>
#include <emmintrin.h>

namespace HorseRadish
{
	class Color
	{
		static constexpr const unsigned char SRGB2Linear[] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 90, 91, 92, 93, 95, 96, 97, 99, 100, 101, 103, 104, 105, 107, 108, 109, 111, 112, 114, 115, 116, 118, 119, 121, 122, 124, 125, 127, 128, 130, 131, 133, 134, 136, 138, 139, 141, 142, 144, 146, 147, 149, 151, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 171, 173, 175, 177, 179, 181, 183, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 224, 226, 229, 231, 233, 235, 237, 239, 242, 244, 246, 248, 250, 253, 255 };

		float mRGBA[4];

	public:
		class KnownColors
		{
		public:
			static constexpr const unsigned char Transparent[] = { 255, 255, 255, 0 };
			static constexpr const unsigned char PureRed[] = { 255, 0, 0, 255 };
			static constexpr const unsigned char PureGreen[] = { 0, 255, 0, 255 };
			static constexpr const unsigned char PureBlue[] = { 0, 0, 255, 255 };
			static constexpr const unsigned char AliceBlue[] = { 240, 248, 255, 255 };
			static constexpr const unsigned char AntiqueWhite[] = { 250, 235, 215, 255 };
			static constexpr const unsigned char Aqua[] = { 0, 255, 255, 255 };
			static constexpr const unsigned char Aquamarine[] = { 127, 255, 212, 255 };
			static constexpr const unsigned char Azure[] = { 240, 255, 255, 255 };
			static constexpr const unsigned char Beige[] = { 245, 245, 220, 255 };
			static constexpr const unsigned char Bisque[] = { 255, 228, 196, 255 };
			static constexpr const unsigned char Black[] = { 0, 0, 0, 255 };
			static constexpr const unsigned char BlanchedAlmond[] = { 255, 235, 205, 255 };
			static constexpr const unsigned char Blue[] = { 0, 0, 255, 255 };
			static constexpr const unsigned char BlueViolet[] = { 138, 43, 226, 255 };
			static constexpr const unsigned char Brown[] = { 165, 42, 42, 255 };
			static constexpr const unsigned char BurlyWood[] = { 222, 184, 135, 255 };
			static constexpr const unsigned char CadetBlue[] = { 95, 158, 160, 255 };
			static constexpr const unsigned char Chartreuse[] = { 127, 255, 0, 255 };
			static constexpr const unsigned char Chocolate[] = { 210, 105, 30, 255 };
			static constexpr const unsigned char Coral[] = { 255, 127, 80, 255 };
			static constexpr const unsigned char CornflowerBlue[] = { 100, 149, 237, 255 };
			static constexpr const unsigned char Cornsilk[] = { 255, 248, 220, 255 };
			static constexpr const unsigned char Crimson[] = { 220, 20, 60, 255 };
			static constexpr const unsigned char Cyan[] = { 0, 255, 255, 255 };
			static constexpr const unsigned char DarkBlue[] = { 0, 0, 139, 255 };
			static constexpr const unsigned char DarkCyan[] = { 0, 139, 139, 255 };
			static constexpr const unsigned char DarkGoldenrod[] = { 184, 134, 11, 255 };
			static constexpr const unsigned char DarkGray[] = { 169, 169, 169, 255 };
			static constexpr const unsigned char DarkGreen[] = { 0, 100, 0, 255 };
			static constexpr const unsigned char DarkKhaki[] = { 189, 183, 107, 255 };
			static constexpr const unsigned char DarkMagenta[] = { 139, 0, 139, 255 };
			static constexpr const unsigned char DarkOliveGreen[] = { 85, 107, 47, 255 };
			static constexpr const unsigned char DarkOrange[] = { 255, 140, 0, 255 };
			static constexpr const unsigned char DarkOrchid[] = { 153, 50, 204, 255 };
			static constexpr const unsigned char DarkRed[] = { 139, 0, 0, 255 };
			static constexpr const unsigned char DarkSalmon[] = { 233, 150, 122, 255 };
			static constexpr const unsigned char DarkSeaGreen[] = { 143, 188, 139, 255 };
			static constexpr const unsigned char DarkSlateBlue[] = { 72, 61, 139, 255 };
			static constexpr const unsigned char DarkSlateGray[] = { 47, 79, 79, 255 };
			static constexpr const unsigned char DarkTurquoise[] = { 0, 206, 209, 255 };
			static constexpr const unsigned char DarkViolet[] = { 148, 0, 211, 255 };
			static constexpr const unsigned char DeepPink[] = { 255, 20, 147, 255 };
			static constexpr const unsigned char DeepSkyBlue[] = { 0, 191, 255, 255 };
			static constexpr const unsigned char DimGray[] = { 105, 105, 105, 255 };
			static constexpr const unsigned char DodgerBlue[] = { 30, 144, 255, 255 };
			static constexpr const unsigned char Firebrick[] = { 178, 34, 34, 255 };
			static constexpr const unsigned char FloralWhite[] = { 255, 250, 240, 255 };
			static constexpr const unsigned char ForestGreen[] = { 34, 139, 34, 255 };
			static constexpr const unsigned char Fuchsia[] = { 255, 0, 255, 255 };
			static constexpr const unsigned char Gainsboro[] = { 220, 220, 220, 255 };
			static constexpr const unsigned char GhostWhite[] = { 248, 248, 255, 255 };
			static constexpr const unsigned char Gold[] = { 255, 215, 0, 255 };
			static constexpr const unsigned char Goldenrod[] = { 218, 165, 32, 255 };
			static constexpr const unsigned char Gray[] = { 128, 128, 128, 255 };
			static constexpr const unsigned char Green[] = { 0, 128, 0, 255 };
			static constexpr const unsigned char GreenYellow[] = { 173, 255, 47, 255 };
			static constexpr const unsigned char Honeydew[] = { 240, 255, 240, 255 };
			static constexpr const unsigned char HotPink[] = { 255, 105, 180, 255 };
			static constexpr const unsigned char IndianRed[] = { 205, 92, 92, 255 };
			static constexpr const unsigned char Indigo[] = { 75, 0, 130, 255 };
			static constexpr const unsigned char Ivory[] = { 255, 255, 240, 255 };
			static constexpr const unsigned char Khaki[] = { 240, 230, 140, 255 };
			static constexpr const unsigned char Lavender[] = { 230, 230, 250, 255 };
			static constexpr const unsigned char LavenderBlush[] = { 255, 240, 245, 255 };
			static constexpr const unsigned char LawnGreen[] = { 124, 252, 0, 255 };
			static constexpr const unsigned char LemonChiffon[] = { 255, 250, 205, 255 };
			static constexpr const unsigned char LightBlue[] = { 173, 216, 230, 255 };
			static constexpr const unsigned char LightCoral[] = { 240, 128, 128, 255 };
			static constexpr const unsigned char LightCyan[] = { 224, 255, 255, 255 };
			static constexpr const unsigned char LightGoldenrodYellow[] = { 250, 250, 210, 255 };
			static constexpr const unsigned char LightGreen[] = { 144, 238, 144, 255 };
			static constexpr const unsigned char LightGray[] = { 211, 211, 211, 255 };
			static constexpr const unsigned char LightPink[] = { 255, 182, 193, 255 };
			static constexpr const unsigned char LightSalmon[] = { 255, 160, 122, 255 };
			static constexpr const unsigned char LightSeaGreen[] = { 32, 178, 170, 255 };
			static constexpr const unsigned char LightSkyBlue[] = { 135, 206, 250, 255 };
			static constexpr const unsigned char LightSlateGray[] = { 119, 136, 153, 255 };
			static constexpr const unsigned char LightSteelBlue[] = { 176, 196, 222, 255 };
			static constexpr const unsigned char LightYellow[] = { 255, 255, 224, 255 };
			static constexpr const unsigned char Lime[] = { 0, 255, 0, 255 };
			static constexpr const unsigned char LimeGreen[] = { 50, 205, 50, 255 };
			static constexpr const unsigned char Linen[] = { 250, 240, 230, 255 };
			static constexpr const unsigned char Magenta[] = { 255, 0, 255, 255 };
			static constexpr const unsigned char Maroon[] = { 128, 0, 0, 255 };
			static constexpr const unsigned char MediumAquamarine[] = { 102, 205, 170, 255 };
			static constexpr const unsigned char MediumBlue[] = { 0, 0, 205, 255 };
			static constexpr const unsigned char MediumOrchid[] = { 186, 85, 211, 255 };
			static constexpr const unsigned char MediumPurple[] = { 147, 112, 219, 255 };
			static constexpr const unsigned char MediumSeaGreen[] = { 60, 179, 113, 255 };
			static constexpr const unsigned char MediumSlateBlue[] = { 123, 104, 238, 255 };
			static constexpr const unsigned char MediumSpringGreen[] = { 0, 250, 154, 255 };
			static constexpr const unsigned char MediumTurquoise[] = { 72, 209, 204, 255 };
			static constexpr const unsigned char MediumVioletRed[] = { 199, 21, 133, 255 };
			static constexpr const unsigned char MidnightBlue[] = { 25, 25, 112, 255 };
			static constexpr const unsigned char MintCream[] = { 245, 255, 250, 255 };
			static constexpr const unsigned char MistyRose[] = { 255, 228, 225, 255 };
			static constexpr const unsigned char Moccasin[] = { 255, 228, 181, 255 };
			static constexpr const unsigned char NavajoWhite[] = { 255, 222, 173, 255 };
			static constexpr const unsigned char Navy[] = { 0, 0, 128, 255 };
			static constexpr const unsigned char OldLace[] = { 253, 245, 230, 255 };
			static constexpr const unsigned char Olive[] = { 128, 128, 0, 255 };
			static constexpr const unsigned char OliveDrab[] = { 107, 142, 35, 255 };
			static constexpr const unsigned char Orange[] = { 255, 165, 0, 255 };
			static constexpr const unsigned char OrangeRed[] = { 255, 69, 0, 255 };
			static constexpr const unsigned char Orchid[] = { 218, 112, 214, 255 };
			static constexpr const unsigned char PaleGoldenrod[] = { 238, 232, 170, 255 };
			static constexpr const unsigned char PaleGreen[] = { 152, 251, 152, 255 };
			static constexpr const unsigned char PaleTurquoise[] = { 175, 238, 238, 255 };
			static constexpr const unsigned char PaleVioletRed[] = { 219, 112, 147, 255 };
			static constexpr const unsigned char PapayaWhip[] = { 255, 239, 213, 255 };
			static constexpr const unsigned char PeachPuff[] = { 255, 218, 185, 255 };
			static constexpr const unsigned char Peru[] = { 205, 133, 63, 255 };
			static constexpr const unsigned char Pink[] = { 255, 192, 203, 255 };
			static constexpr const unsigned char Plum[] = { 221, 160, 221, 255 };
			static constexpr const unsigned char PowderBlue[] = { 176, 224, 230, 255 };
			static constexpr const unsigned char Purple[] = { 128, 0, 128, 255 };
			static constexpr const unsigned char Red[] = { 255, 0, 0, 255 };
			static constexpr const unsigned char RosyBrown[] = { 188, 143, 143, 255 };
			static constexpr const unsigned char RoyalBlue[] = { 65, 105, 225, 255 };
			static constexpr const unsigned char SaddleBrown[] = { 139, 69, 19, 255 };
			static constexpr const unsigned char Salmon[] = { 250, 128, 114, 255 };
			static constexpr const unsigned char SandyBrown[] = { 244, 164, 96, 255 };
			static constexpr const unsigned char SeaGreen[] = { 46, 139, 87, 255 };
			static constexpr const unsigned char SeaShell[] = { 255, 245, 238, 255 };
			static constexpr const unsigned char Sienna[] = { 160, 82, 45, 255 };
			static constexpr const unsigned char Silver[] = { 192, 192, 192, 255 };
			static constexpr const unsigned char SkyBlue[] = { 135, 206, 235, 255 };
			static constexpr const unsigned char SlateBlue[] = { 106, 90, 205, 255 };
			static constexpr const unsigned char SlateGray[] = { 112, 128, 144, 255 };
			static constexpr const unsigned char Snow[] = { 255, 250, 250, 255 };
			static constexpr const unsigned char SpringGreen[] = { 0, 255, 127, 255 };
			static constexpr const unsigned char SteelBlue[] = { 70, 130, 180, 255 };
			static constexpr const unsigned char Tan[] = { 210, 180, 140, 255 };
			static constexpr const unsigned char Teal[] = { 0, 128, 128, 255 };
			static constexpr const unsigned char Thistle[] = { 216, 191, 216, 255 };
			static constexpr const unsigned char Tomato[] = { 255, 99, 71, 255 };
			static constexpr const unsigned char Turquoise[] = { 64, 224, 208, 255 };
			static constexpr const unsigned char Violet[] = { 238, 130, 238, 255 };
			static constexpr const unsigned char Wheat[] = { 245, 222, 179, 255 };
			static constexpr const unsigned char White[] = { 255, 255, 255, 255 };
			static constexpr const unsigned char WhiteSmoke[] = { 245, 245, 245, 255 };
			static constexpr const unsigned char Yellow[] = { 255, 255, 0, 255 };
			static constexpr const unsigned char YellowGreen[] = { 154, 205, 50, 255 };
		};

	public:
		static unsigned char convertColor(float val)
		{
			__m128i valConvert;
			unsigned char valFinal;

			valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_set_ss(val), Math::SIMD::fUByteMax));
			valConvert = _mm_packs_epi32(valConvert, valConvert);
			valConvert = _mm_packus_epi16(valConvert, valConvert);

			_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x80), (char*)&valFinal);
			return valFinal;
		}

		static float convertColor(unsigned char val)
		{
			return (static_cast<float>(val) * Math::SIMD::fUByteMaxInv.m128_f32[0]);
		}

		static void convertColor(unsigned char *valB, const float * const valF, bool processAlphaChannel)
		{
			__m128i valConvert;

			if (!processAlphaChannel)
			{
				valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_set_ps(0.0f, valF[2], valF[1], valF[0]), Math::SIMD::fUByteMax));
				valConvert = _mm_packs_epi32(valConvert, valConvert);
				valConvert = _mm_packus_epi16(valConvert, valConvert);

				_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x00808080), (char*)valB);
				return;
			}

			valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_loadu_ps(valF), Math::SIMD::fUByteMax));
			valConvert = _mm_packs_epi32(valConvert, valConvert);
			valConvert = _mm_packus_epi16(valConvert, valConvert);

			_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x80808080), (char*)valB);
		}

		static void convertColor(float * const valF, const unsigned char *valB, bool processAlphaChannel)
		{
			__m128i valConvert;
			__m128 valFinal;

			if (!processAlphaChannel)
			{
				valConvert = _mm_cvtsi32_si128(((valB[2] << 16) | (valB[1] << 8) | valB[0]));
				valConvert = _mm_unpacklo_epi8(valConvert, _mm_setzero_si128());
				valConvert = _mm_unpacklo_epi16(valConvert, _mm_setzero_si128());

				valFinal = _mm_mul_ps(_mm_cvtepi32_ps(valConvert), Math::SIMD::fUByteMaxInv);

				valF[0] = valFinal.m128_f32[0];
				valF[1] = valFinal.m128_f32[1];
				valF[2] = valFinal.m128_f32[2];
				return;
			}

			valConvert = _mm_cvtsi32_si128(((valB[3] << 24) | (valB[2] << 16) | (valB[1] << 8) | valB[0]));
			valConvert = _mm_unpacklo_epi8(valConvert, _mm_setzero_si128());
			valConvert = _mm_unpacklo_epi16(valConvert, _mm_setzero_si128());

			valFinal = _mm_mul_ps(_mm_cvtepi32_ps(valConvert), Math::SIMD::fUByteMaxInv);
			_mm_storeu_ps(valF, valFinal);
		}

		static Color parseColorFromHTML(const char *hexColor, bool gammaCorrect = true)
		{
			if (*hexColor == '#')
				hexColor++;

			Color color;
			if (gammaCorrect)
			{
				color.mRGBA[0] = Color::gammaCorrect(Encoders::decodeHexByte(hexColor + 0));
				color.mRGBA[1] = Color::gammaCorrect(Encoders::decodeHexByte(hexColor + 2));
				color.mRGBA[2] = Color::gammaCorrect(Encoders::decodeHexByte(hexColor + 4));
				color.mRGBA[3] = 255.0f;
			}
			else
			{
				color.mRGBA[0] = Encoders::decodeHexByte(hexColor + 0);
				color.mRGBA[1] = Encoders::decodeHexByte(hexColor + 2);
				color.mRGBA[2] = Encoders::decodeHexByte(hexColor + 4);
				color.mRGBA[3] = 255.0f;
			}

			_mm_storeu_ps(color.mRGBA, _mm_mul_ps(_mm_loadu_ps(color.mRGBA), Math::SIMD::fUByteMaxInv));

			return color;
		}

		static unsigned char gammaCorrect(unsigned char value)
		{
			return SRGB2Linear[value];
		}

	public:
		Color()
		{
			_mm_storeu_ps(mRGBA, _mm_setzero_ps());
		}

		Color(const Color &c)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(c.mRGBA));
		}

		explicit Color(const Vector3f &v)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(v.data()));
			mRGBA[3] = 1.0f;
		}

		explicit Color(const Vector4f &v)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(v.data()));
		}

		explicit Color(float scalar)
		{
			_mm_storeu_ps(mRGBA, _mm_load_ps1(&scalar));
		}

		explicit Color(float r, float g, float b, float a)
		{
			mRGBA[0] = r;
			mRGBA[1] = g;
			mRGBA[2] = b;
			mRGBA[3] = a;
		}

		explicit Color(const float * const c)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(c));
		}

		float* data()
		{
			return mRGBA;
		}

		const float* data() const
		{
			return mRGBA;
		}

		float& operator[] (const size_t index)
		{
			return mRGBA[index % 4];
		}

		const float& operator[] (const size_t index) const
		{
			return mRGBA[index % 4];
		}

		void operator+=(const Color& c)
		{
			_mm_storeu_ps(mRGBA, _mm_add_ps(_mm_loadu_ps(mRGBA), _mm_loadu_ps(c.mRGBA)));
		}

		void operator-=(const Color& c)
		{
			_mm_storeu_ps(mRGBA, _mm_sub_ps(_mm_loadu_ps(mRGBA), _mm_loadu_ps(c.mRGBA)));
		}

		void operator*=(const Color& c)
		{
			_mm_storeu_ps(mRGBA, _mm_mul_ps(_mm_loadu_ps(mRGBA), _mm_loadu_ps(c.mRGBA)));
		}

		void operator/=(const Color& c)
		{
			_mm_storeu_ps(mRGBA, _mm_div_ps(_mm_loadu_ps(mRGBA), _mm_loadu_ps(c.mRGBA)));
		}

		void operator+=(const float &n)
		{
			_mm_storeu_ps(mRGBA, _mm_add_ps(_mm_loadu_ps(mRGBA), _mm_load_ps1(&n)));
		}

		void operator-=(const float &n)
		{
			_mm_storeu_ps(mRGBA, _mm_sub_ps(_mm_loadu_ps(mRGBA), _mm_load_ps1(&n)));
		}

		void operator*=(const float &n)
		{
			_mm_storeu_ps(mRGBA, _mm_mul_ps(_mm_loadu_ps(mRGBA), _mm_load_ps1(&n)));
		}

		void operator/=(const float &n)
		{
			_mm_storeu_ps(mRGBA, _mm_div_ps(_mm_loadu_ps(mRGBA), _mm_load_ps1(&n)));
		}

		void set(const Color &color)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(color.mRGBA));
		}

		void set(const Color& color, float a)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(color.mRGBA));
			mRGBA[3] = a;
		}

		void set(const float *color)
		{
			_mm_storeu_ps(mRGBA, _mm_loadu_ps(color));
		}

		void set(const float * const color, float a)
		{
			mRGBA[0] = color[0];
			mRGBA[1] = color[1];
			mRGBA[2] = color[2];
			mRGBA[3] = a;
		}

		void set(float crgba)
		{
			_mm_storeu_ps(mRGBA, _mm_load_ps1(&crgba));
		}

		void set(float crgb, float a)
		{
			mRGBA[0] = mRGBA[1] = mRGBA[2] = crgb;
			mRGBA[3] = a;
		}

		void set(float r, float g, float b, float a)
		{
			mRGBA[0] = r;
			mRGBA[1] = g;
			mRGBA[2] = b;
			mRGBA[3] = a;
		}

		void set(const unsigned char *color)
		{
			Color::convertColor(mRGBA, color, true);
		}

		void set(const unsigned char *color, unsigned char a)
		{
			mRGBA[0] = static_cast<float>(color[0]);
			mRGBA[1] = static_cast<float>(color[1]);
			mRGBA[2] = static_cast<float>(color[2]);
			mRGBA[3] = static_cast<float>(a);
			_mm_storeu_ps(mRGBA, _mm_mul_ps(_mm_loadu_ps(mRGBA), Math::SIMD::fUByteMaxInv));
		}

		void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
		{
			mRGBA[0] = static_cast<float>(r);
			mRGBA[1] = static_cast<float>(g);
			mRGBA[2] = static_cast<float>(b);
			mRGBA[3] = static_cast<float>(a);
			_mm_storeu_ps(mRGBA, _mm_mul_ps(_mm_loadu_ps(mRGBA), Math::SIMD::fUByteMaxInv));
		}

		void set(unsigned char r, unsigned char g, unsigned char b)
		{
			mRGBA[0] = static_cast<float>(r);
			mRGBA[1] = static_cast<float>(g);
			mRGBA[2] = static_cast<float>(b);
			mRGBA[3] = 255.0f;
			_mm_storeu_ps(mRGBA, _mm_mul_ps(_mm_loadu_ps(mRGBA), Math::SIMD::fUByteMaxInv));
		}

		void setWeight(const Color& c, const float &weight)
		{
			_mm_storeu_ps(mRGBA, _mm_mul_ps(_mm_loadu_ps(c.mRGBA), _mm_load_ps1(&weight)));
		}

		void setWeight(const Color& c1, const float &weight1, const Color& c2, const float &weight2, const Color& c3, const float &weight3, const Color& c4, const float &weight4)
		{
			__m128 temp;

			temp = _mm_mul_ps(_mm_loadu_ps(c1.mRGBA), _mm_load_ps1(&weight1));
			temp = _mm_add_ps(temp, _mm_mul_ps(_mm_loadu_ps(c2.mRGBA), _mm_load_ps1(&weight2)));
			temp = _mm_add_ps(temp, _mm_mul_ps(_mm_loadu_ps(c3.mRGBA), _mm_load_ps1(&weight3)));
			_mm_storeu_ps(mRGBA, _mm_add_ps(temp, _mm_mul_ps(_mm_loadu_ps(c4.mRGBA), _mm_load_ps1(&weight4))));
		}

		void setYUV(const float * const yuv)
		{
			setYUV(yuv[0], yuv[1], yuv[2]);
		}

		void setYUV(float y, float u, float v)
		{
			mRGBA[0] = y + (v * 1.140f);
			mRGBA[1] = y - ((u * 0.395f) + (v * 0.581f));
			mRGBA[2] = y + (u * 2.032f);
			mRGBA[3] = 1.0f;
		}

		void setYUV(const unsigned char * const yuv)
		{
			setYUV(yuv[0], yuv[1], yuv[2]);
		}

		void setYUV(unsigned char y, unsigned char u, unsigned char v)
		{
			HALIGN_16BYTES float pixelAux[4];

			pixelAux[0] = y;
			pixelAux[1] = u;
			pixelAux[2] = v;
			pixelAux[3] = 0.0;
			_mm_storeu_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), Math::SIMD::fUByteMaxInv));

			setYUV(pixelAux[0], pixelAux[1], pixelAux[2]);
		}

		void setYCbCr(const float * const ycbcr, bool fullRange)
		{
			setYCbCr(ycbcr[0], ycbcr[1], ycbcr[2], fullRange);
		}

		void setYCbCr(float y, float cb, float cr, bool fullRange)
		{
			float auxCb = cb - 0.5f;
			float auxCr = cr - 0.5f;

			if (fullRange == true)
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
		}

		void setYCbCr(const unsigned char * const ycbcr, bool fullRange)
		{
			setYCbCr(ycbcr[0], ycbcr[1], ycbcr[2], fullRange);
		}

		void setYCbCr(unsigned char y, unsigned char cb, unsigned char cr, bool fullRange)
		{
			HALIGN_16BYTES float pixelAux[4];

			pixelAux[0] = y;
			pixelAux[1] = cb;
			pixelAux[2] = cr;
			pixelAux[3] = 1.0;
			_mm_storeu_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), Math::SIMD::fUByteMaxInv));

			setYCbCr(pixelAux[0], pixelAux[1], pixelAux[2], fullRange);
		}

		void setYPbPr(const float * const ypbpr, bool coefficientsSDTV)
		{
			setYPbPr(ypbpr[0], ypbpr[1], ypbpr[2], coefficientsSDTV);
		}

		void setYPbPr(float y, float pb, float pr, bool coefficientsSDTV)
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
		}

		void setYPbPr(const unsigned char * const ypbpr, bool coefficientsSDTV)
		{
			setYPbPr(ypbpr[0], ypbpr[1], ypbpr[2], coefficientsSDTV);
		}

		void setYPbPr(unsigned char y, unsigned char pb, unsigned char pr, bool coefficientsSDTV)
		{
			HALIGN_16BYTES float pixelAux[4];

			pixelAux[0] = y;
			pixelAux[1] = pb;
			pixelAux[2] = pr;
			pixelAux[3] = 1.0;
			_mm_storeu_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), Math::SIMD::fUByteMaxInv));

			setYPbPr(pixelAux[0], pixelAux[1], pixelAux[2], coefficientsSDTV);
		}

		void setCMYK(const float * const cmyk)
		{
			setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
		}

		void setCMYK(float c, float m, float y, float k)
		{
			mRGBA[0] = 1.0f - (c * (1.0f - k)) + k;
			mRGBA[1] = 1.0f - (m * (1.0f - k)) + k;
			mRGBA[2] = 1.0f - (y * (1.0f - k)) + k;
			mRGBA[3] = 1.0f;
		}

		void setCMYK(const unsigned char * const cmyk)
		{
			setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
		}

		void setCMYK(unsigned char c, unsigned char m, unsigned char y, unsigned char k)
		{
			HALIGN_16BYTES float pixelAux[4];

			pixelAux[0] = c;
			pixelAux[1] = m;
			pixelAux[2] = y;
			pixelAux[3] = k;
			_mm_storeu_ps(pixelAux, _mm_mul_ps(_mm_load_ps(pixelAux), Math::SIMD::fUByteMaxInv));

			setCMYK(pixelAux[0], pixelAux[1], pixelAux[2], pixelAux[3]);
		}

		void write(float * const dest) const
		{
			std::memcpy(dest, mRGBA, sizeof(float) * 4);
		}

		void write(unsigned char * const dest) const
		{
			Color::convertColor(dest, mRGBA, true);
		}

		void writeRGB(float * const dest) const
		{
			dest[0] = mRGBA[0];
			dest[1] = mRGBA[1];
			dest[2] = mRGBA[2];
		}

		void writeRGB(unsigned char * const dest) const
		{
			Color::convertColor(dest, mRGBA, false);
		}

		void writeYUV(float * const dest) const
		{
			dest[0] = (mRGBA[0] * 0.299f) + (mRGBA[1] * 0.587f) + (mRGBA[2] * 0.114f);
			dest[1] = -(mRGBA[0] * 0.147f) - (mRGBA[1] * 0.289f) + (mRGBA[2] * 0.436f);
			dest[2] = (mRGBA[0] * 0.615f) - (mRGBA[1] * 0.515f) - (mRGBA[2] * 0.100f);
		}

		void writeYUV(unsigned char * const dest) const
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

		void writeYCbCr(unsigned char * const dest, bool fullRange) const
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
		void writeYPbPr(unsigned char * const dest, bool coefficientsSDTV) const
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

		void writeCMYK(unsigned char * const dest) const
		{
			float bufferAux[4];

			writeCMYK(bufferAux);
			Color::convertColor(dest, bufferAux, true);
		}

		void addColorWeighted(const Color &color, float weight)
		{
			__m128 temp;

			temp = _mm_mul_ps(_mm_loadu_ps(color.mRGBA), _mm_load_ps1(&weight));
			_mm_storeu_ps(mRGBA, _mm_add_ps(_mm_loadu_ps(mRGBA), temp));
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
			_mm_storeu_ps(mRGBA, _mm_sub_ps(Math::SIMD::fOne, _mm_loadu_ps(mRGBA)));
			mRGBA[3] = alphaAux;
		}

		void negativeRGB(float value)
		{
			float alphaAux = mRGBA[3];
			_mm_storeu_ps(mRGBA, _mm_sub_ps(_mm_set_ps1(value), _mm_loadu_ps(mRGBA)));
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

			pixelVal = _mm_loadu_ps(mRGBA);
			valMul = _mm_load_ps1(&opMul);
			valAdd = _mm_load_ps1(&opAdd);

			pixelVal = _mm_mul_ps(pixelVal, valMul);
			_mm_storeu_ps(mRGBA, _mm_add_ps(pixelVal, valAdd));
		}

		void clamp()
		{
			_mm_storeu_ps(mRGBA, _mm_min_ps(_mm_max_ps(_mm_loadu_ps(mRGBA), _mm_setzero_ps()), Math::SIMD::fOne));
		}

		void clamp(float min, float max)
		{
			_mm_storeu_ps(mRGBA, _mm_min_ps(_mm_max_ps(_mm_loadu_ps(mRGBA), _mm_load_ps1(&min)), _mm_load_ps1(&max)));
		}

		void calcInterpolate(const Color &from, const Color &to, float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(from.mRGBA), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to.mRGBA), _mm_load_ps1(&t)));
			_mm_storeu_ps(mRGBA, tmp);
		}

		void calcInterpolate(const float * const from, const float * const to, float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(from), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to), _mm_load_ps1(&t)));
			_mm_storeu_ps(mRGBA, tmp);
		}

		void calcInterpolate(const Color &to, float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(mRGBA), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to.mRGBA), _mm_load_ps1(&t)));
			_mm_storeu_ps(mRGBA, tmp);
		}
	};

} //HorseRadish
