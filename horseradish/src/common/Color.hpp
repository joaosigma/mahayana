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
		static const unsigned char SRGB2Linear[256];

	public:
		class KnownColors
		{
		public:
			static const unsigned char Transparent[4];
			static const unsigned char PureRed[4];
			static const unsigned char PureGreen[4];
			static const unsigned char PureBlue[4];
			static const unsigned char AliceBlue[4];
			static const unsigned char AntiqueWhite[4];
			static const unsigned char Aqua[4];
			static const unsigned char Aquamarine[4];
			static const unsigned char Azure[4];
			static const unsigned char Beige[4];
			static const unsigned char Bisque[4];
			static const unsigned char Black[4];
			static const unsigned char BlanchedAlmond[4];
			static const unsigned char Blue[4];
			static const unsigned char BlueViolet[4];
			static const unsigned char Brown[4];
			static const unsigned char BurlyWood[4];
			static const unsigned char CadetBlue[4];
			static const unsigned char Chartreuse[4];
			static const unsigned char Chocolate[4];
			static const unsigned char Coral[4];
			static const unsigned char CornflowerBlue[4];
			static const unsigned char Cornsilk[4];
			static const unsigned char Crimson[4];
			static const unsigned char Cyan[4];
			static const unsigned char DarkBlue[4];
			static const unsigned char DarkCyan[4];
			static const unsigned char DarkGoldenrod[4];
			static const unsigned char DarkGray[4];
			static const unsigned char DarkGreen[4];
			static const unsigned char DarkKhaki[4];
			static const unsigned char DarkMagenta[4];
			static const unsigned char DarkOliveGreen[4];
			static const unsigned char DarkOrange[4];
			static const unsigned char DarkOrchid[4];
			static const unsigned char DarkRed[4];
			static const unsigned char DarkSalmon[4];
			static const unsigned char DarkSeaGreen[4];
			static const unsigned char DarkSlateBlue[4];
			static const unsigned char DarkSlateGray[4];
			static const unsigned char DarkTurquoise[4];
			static const unsigned char DarkViolet[4];
			static const unsigned char DeepPink[4];
			static const unsigned char DeepSkyBlue[4];
			static const unsigned char DimGray[4];
			static const unsigned char DodgerBlue[4];
			static const unsigned char Firebrick[4];
			static const unsigned char FloralWhite[4];
			static const unsigned char ForestGreen[4];
			static const unsigned char Fuchsia[4];
			static const unsigned char Gainsboro[4];
			static const unsigned char GhostWhite[4];
			static const unsigned char Gold[4];
			static const unsigned char Goldenrod[4];
			static const unsigned char Gray[4];
			static const unsigned char Green[4];
			static const unsigned char GreenYellow[4];
			static const unsigned char Honeydew[4];
			static const unsigned char HotPink[4];
			static const unsigned char IndianRed[4];
			static const unsigned char Indigo[4];
			static const unsigned char Ivory[4];
			static const unsigned char Khaki[4];
			static const unsigned char Lavender[4];
			static const unsigned char LavenderBlush[4];
			static const unsigned char LawnGreen[4];
			static const unsigned char LemonChiffon[4];
			static const unsigned char LightBlue[4];
			static const unsigned char LightCoral[4];
			static const unsigned char LightCyan[4];
			static const unsigned char LightGoldenrodYellow[4];
			static const unsigned char LightGreen[4];
			static const unsigned char LightGray[4];
			static const unsigned char LightPink[4];
			static const unsigned char LightSalmon[4];
			static const unsigned char LightSeaGreen[4];
			static const unsigned char LightSkyBlue[4];
			static const unsigned char LightSlateGray[4];
			static const unsigned char LightSteelBlue[4];
			static const unsigned char LightYellow[4];
			static const unsigned char Lime[4];
			static const unsigned char LimeGreen[4];
			static const unsigned char Linen[4];
			static const unsigned char Magenta[4];
			static const unsigned char Maroon[4];
			static const unsigned char MediumAquamarine[4];
			static const unsigned char MediumBlue[4];
			static const unsigned char MediumOrchid[4];
			static const unsigned char MediumPurple[4];
			static const unsigned char MediumSeaGreen[4];
			static const unsigned char MediumSlateBlue[4];
			static const unsigned char MediumSpringGreen[4];
			static const unsigned char MediumTurquoise[4];
			static const unsigned char MediumVioletRed[4];
			static const unsigned char MidnightBlue[4];
			static const unsigned char MintCream[4];
			static const unsigned char MistyRose[4];
			static const unsigned char Moccasin[4];
			static const unsigned char NavajoWhite[4];
			static const unsigned char Navy[4];
			static const unsigned char OldLace[4];
			static const unsigned char Olive[4];
			static const unsigned char OliveDrab[4];
			static const unsigned char Orange[4];
			static const unsigned char OrangeRed[4];
			static const unsigned char Orchid[4];
			static const unsigned char PaleGoldenrod[4];
			static const unsigned char PaleGreen[4];
			static const unsigned char PaleTurquoise[4];
			static const unsigned char PaleVioletRed[4];
			static const unsigned char PapayaWhip[4];
			static const unsigned char PeachPuff[4];
			static const unsigned char Peru[4];
			static const unsigned char Pink[4];
			static const unsigned char Plum[4];
			static const unsigned char PowderBlue[4];
			static const unsigned char Purple[4];
			static const unsigned char Red[4];
			static const unsigned char RosyBrown[4];
			static const unsigned char RoyalBlue[4];
			static const unsigned char SaddleBrown[4];
			static const unsigned char Salmon[4];
			static const unsigned char SandyBrown[4];
			static const unsigned char SeaGreen[4];
			static const unsigned char SeaShell[4];
			static const unsigned char Sienna[4];
			static const unsigned char Silver[4];
			static const unsigned char SkyBlue[4];
			static const unsigned char SlateBlue[4];
			static const unsigned char SlateGray[4];
			static const unsigned char Snow[4];
			static const unsigned char SpringGreen[4];
			static const unsigned char SteelBlue[4];
			static const unsigned char Tan[4];
			static const unsigned char Teal[4];
			static const unsigned char Thistle[4];
			static const unsigned char Tomato[4];
			static const unsigned char Turquoise[4];
			static const unsigned char Violet[4];
			static const unsigned char Wheat[4];
			static const unsigned char White[4];
			static const unsigned char WhiteSmoke[4];
			static const unsigned char Yellow[4];
			static const unsigned char YellowGreen[4];
		};

	public:
		static unsigned char convertColor(const float val)
		{
			__m128i valConvert;
			unsigned char valFinal;

			valConvert = _mm_cvtps_epi32(_mm_mul_ps(_mm_set_ss(val), Math::SIMD::fUByteMax));
			valConvert = _mm_packs_epi32(valConvert, valConvert);
			valConvert = _mm_packus_epi16(valConvert, valConvert);

			_mm_maskmoveu_si128(valConvert, _mm_set_epi32(0x0, 0x0, 0x0, 0x80), (char*)&valFinal);
			return valFinal;
		}

		static float convertColor(const unsigned char val)
		{
			return (((float)val)*Math::SIMD::fUByteMaxInv.m128_f32[0]);
		}

		static void convertColor(unsigned char *valB, const float * const valF, const bool processAlphaChannel)
		{
			__m128i valConvert;

			if (processAlphaChannel == false)
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

		static void convertColor(float * const valF, const unsigned char *valB, const bool processAlphaChannel)
		{
			__m128i valConvert;
			__m128 valFinal;

			if (processAlphaChannel == false)
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

		static Color parseColorFromHTML(const char *hexColor, const bool gammaCorrect = true)
		{
			if (*hexColor == '#')
				hexColor++;

			Color color;
			if (gammaCorrect)
			{
				color.r = Color::gammaCorrect(Encoders::DecodeHexByte(hexColor + 0));
				color.g = Color::gammaCorrect(Encoders::DecodeHexByte(hexColor + 2));
				color.b = Color::gammaCorrect(Encoders::DecodeHexByte(hexColor + 4));
				color.a = 255.0f;
			}
			else
			{
				color.r = Encoders::DecodeHexByte(hexColor + 0);
				color.g = Encoders::DecodeHexByte(hexColor + 2);
				color.b = Encoders::DecodeHexByte(hexColor + 4);
				color.a = 255.0f;
			}

			_mm_storeu_ps(&color.r, _mm_mul_ps(_mm_loadu_ps(&color.r), Math::SIMD::fUByteMaxInv));

			return color;
		}

		static unsigned char gammaCorrect(const unsigned char value)
		{
			return SRGB2Linear[value];
		}

	public:
		float r, g, b, a;

		Color()
		{
			_mm_storeu_ps(&r, _mm_setzero_ps());
		}

		Color(const Color &c)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(&c.r));
		}

		explicit Color(const Vector3f &v)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(v.data()));
			a = 1.0f;
		}

		explicit Color(const Vector4f &v)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(v.data()));
		}

		explicit Color(const float scalar)
		{
			_mm_storeu_ps(&r, _mm_load_ps1(&scalar));
		}

		explicit Color(const float cr, const float cg, const float cb, const float ca)
		{
			r = cr; g = cg; b = cb; a = ca;
		}

		explicit Color(const float * const c)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(c));
		}

		float* data()
		{
			return &r;
		}

		const float* data() const
		{
			return &r;
		}

		void operator+=(const Color& c)
		{
			_mm_storeu_ps(&r, _mm_add_ps(_mm_loadu_ps(&r), _mm_loadu_ps(&c.r)));
		}

		void operator-=(const Color& c)
		{
			_mm_storeu_ps(&r, _mm_sub_ps(_mm_loadu_ps(&r), _mm_loadu_ps(&c.r)));
		}

		void operator*=(const Color& c)
		{
			_mm_storeu_ps(&r, _mm_mul_ps(_mm_loadu_ps(&r), _mm_loadu_ps(&c.r)));
		}

		void operator/=(const Color& c)
		{
			_mm_storeu_ps(&r, _mm_div_ps(_mm_loadu_ps(&r), _mm_loadu_ps(&c.r)));
		}

		void operator+=(const float &n)
		{
			_mm_storeu_ps(&r, _mm_add_ps(_mm_loadu_ps(&r), _mm_load_ps1(&n)));
		}

		void operator-=(const float &n)
		{
			_mm_storeu_ps(&r, _mm_sub_ps(_mm_loadu_ps(&r), _mm_load_ps1(&n)));
		}

		void operator*=(const float &n)
		{
			_mm_storeu_ps(&r, _mm_mul_ps(_mm_loadu_ps(&r), _mm_load_ps1(&n)));
		}

		void operator/=(const float &n)
		{
			_mm_storeu_ps(&r, _mm_div_ps(_mm_loadu_ps(&r), _mm_load_ps1(&n)));
		}

		void set(const Color &color)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(&color.r));
		}

		void set(const Color& color, const float &a)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(&color.r));
			this->a = a;
		}

		void set(const float *color)
		{
			_mm_storeu_ps(&r, _mm_loadu_ps(color));
		}

		void Set(const float *color, const float &a)
		{
			this->r = color[0];
			this->g = color[1];
			this->b = color[2];
			this->a = a;
		}

		void set(const float &crgba)
		{
			_mm_storeu_ps(&r, _mm_load_ps1(&crgba));
		}

		void set(const float &crgb, const float &ca)
		{
			this->r = this->g = this->b = crgb;
			this->a = ca;
		}

		void set(const float &cr, const float &cg, const float &cb, const float &ca)
		{
			this->r = cr;
			this->g = cg;
			this->b = cb;
			this->a = ca;
		}

		void set(const unsigned char *color)
		{
			Color::convertColor(&r, color, true);
		}

		void set(const unsigned char *color, const unsigned char &ca)
		{
			this->r = (float)color[0];
			this->g = (float)color[1];
			this->b = (float)color[2];
			this->a = (float)ca;
			_mm_storeu_ps(&r, _mm_mul_ps(_mm_loadu_ps(&r), Math::SIMD::fUByteMaxInv));
		}

		void set(const unsigned char &cr, const unsigned char &cg, const unsigned char &cb, const unsigned char &ca)
		{
			this->r = (float)cr;
			this->g = (float)cg;
			this->b = (float)cb;
			this->a = (float)ca;
			_mm_storeu_ps(&r, _mm_mul_ps(_mm_loadu_ps(&r), Math::SIMD::fUByteMaxInv));
		}

		void set(const unsigned char &cr, const unsigned char &cg, const unsigned char &cb)
		{
			this->r = (float)cr;
			this->g = (float)cg;
			this->b = (float)cb;
			this->a = 255.0f;
			_mm_storeu_ps(&r, _mm_mul_ps(_mm_loadu_ps(&r), Math::SIMD::fUByteMaxInv));
		}

		void setWeight(const Color& c, const float &weight)
		{
			_mm_storeu_ps(&r, _mm_mul_ps(_mm_loadu_ps(&c.r), _mm_load_ps1(&weight)));
		}

		void setWeight(const Color& c1, const float &weight1, const Color& c2, const float &weight2, const Color& c3, const float &weight3, const Color& c4, const float &weight4)
		{
			__m128 temp;

			temp = _mm_mul_ps(_mm_loadu_ps(&c1.r), _mm_load_ps1(&weight1));
			temp = _mm_add_ps(temp, _mm_mul_ps(_mm_loadu_ps(&c2.r), _mm_load_ps1(&weight2)));
			temp = _mm_add_ps(temp, _mm_mul_ps(_mm_loadu_ps(&c3.r), _mm_load_ps1(&weight3)));
			_mm_storeu_ps(&r, _mm_add_ps(temp, _mm_mul_ps(_mm_loadu_ps(&c4.r), _mm_load_ps1(&weight4))));
		}

		void setYUV(const float * const yuv)
		{
			setYUV(yuv[0], yuv[1], yuv[2]);
		}

		void setYUV(const float &y, const float &u, const float &v)
		{
			this->r = y + (v * 1.140f);
			this->g = y - ((u * 0.395f) + (v * 0.581f));
			this->r = y + (u * 2.032f);
			this->a = 1.0f;
		}

		void setYUV(const unsigned char * const yuv)
		{
			setYUV(yuv[0], yuv[1], yuv[2]);
		}

		void setYUV(const unsigned char &y, const unsigned char &u, const unsigned char &v)
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

		void setYCbCr(const float &y, const float &cb, const float &cr, bool fullRange)
		{
			float auxCb = cb - 0.5f;
			float auxCr = cr - 0.5f;

			if (fullRange == true)
			{
				this->r = y + (auxCr * 1.400f);
				this->g = y - ((auxCb * 0.343f) + (auxCr * 0.711f));
				this->r = y + (auxCb * 1.765f);
			}
			else
			{
				float auxY;

				auxY = y - (16.0f / 255.0f);
				this->r = (auxY * 1.164f) + (auxCr * 1.596f);
				this->g = (auxY * 1.164f) - ((auxCb * 0.392f) + (auxCr * 0.813f));
				this->r = (auxY * 1.164f) + (auxCb * 2.017f);
			}

			this->a = 1.0f;
		}

		void setYCbCr(const unsigned char * const ycbcr, bool fullRange)
		{
			setYCbCr(ycbcr[0], ycbcr[1], ycbcr[2], fullRange);
		}

		void setYCbCr(const unsigned char &y, const unsigned char &cb, const unsigned char &cr, bool fullRange)
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

		void setYPbPr(const float &y, const float &pb, const float &pr, bool coefficientsSDTV)
		{
			if (coefficientsSDTV == true)
			{
				this->r = y + (pr * 1.402f);
				this->g = y - ((pb * 0.344f) + (pr * 0.714f));
				this->r = y + (pb * 1.772f);
			}
			else
			{
				this->r = y + (pr * 1.575f);
				this->g = y - ((pb * 0.187f) + (pr * 0.468f));
				this->r = y + (pb * 1.856f);
			}

			this->a = 1.0f;
		}

		void setYPbPr(const unsigned char * const ypbpr, bool coefficientsSDTV)
		{
			setYPbPr(ypbpr[0], ypbpr[1], ypbpr[2], coefficientsSDTV);
		}

		void setYPbPr(const unsigned char &y, const unsigned char &pb, const unsigned char &pr, bool coefficientsSDTV)
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

		void setCMYK(const float &c, const float &m, const float &y, const float &k)
		{
			this->r = 1.0f - (c * (1.0f - k)) + k;
			this->g = 1.0f - (m * (1.0f - k)) + k;
			this->r = 1.0f - (y * (1.0f - k)) + k;
			this->a = 1.0f;
		}

		void setCMYK(const unsigned char * const cmyk)
		{
			setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
		}

		void setCMYK(const unsigned char &c, const unsigned char &m, const unsigned char &y, const unsigned char &k)
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
			dest[0] = r; dest[1] = g; dest[2] = b; dest[3] = a;
		}

		void write(unsigned char * const dest) const
		{
			Color::convertColor(dest, &r, true);
		}

		void writeRGB(float * const dest) const
		{
			dest[0] = r; dest[1] = g; dest[2] = b;
		}

		void writeRGB(unsigned char * const dest) const
		{
			Color::convertColor(dest, &r, false);
		}

		void writeYUV(float * const dest) const
		{
			dest[0] = (this->r * 0.299f) + (this->g * 0.587f) + (this->b * 0.114f);
			dest[1] = -(this->r * 0.147f) - (this->g * 0.289f) + (this->b * 0.436f);
			dest[2] = (this->r * 0.615f) - (this->g * 0.515f) - (this->b * 0.100f);
		}

		void writeYUV(unsigned char * const dest) const
		{
			float bufferAux[3];

			writeYUV(bufferAux);
			Color::convertColor(dest, bufferAux, false);
		}

		void writeYCbCr(float * const dest, bool fullRange) const
		{
			if (fullRange == true)
			{
				dest[0] = (this->r * 0.299f) + (this->g * 0.587f) + (this->b * 0.114f);
				dest[1] = -(this->r * 0.169f) - (this->g * 0.331f) + (this->b * 0.500f);
				dest[2] = (this->r * 0.500f) - (this->g * 0.419f) - (this->b * 0.081f);
			}
			else
			{
				dest[0] = (this->r * 0.257f) + (this->g * 0.504f) + (this->b * 0.098f);
				dest[1] = -(this->r * 0.148f) - (this->g * 0.291f) + (this->b * 0.439f);
				dest[2] = (this->r * 0.439f) - (this->g * 0.368f) - (this->b * 0.071f);
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
			if (coefficientsSDTV == true)
			{
				dest[0] = (this->r * 0.299f) + (this->g * 0.587f) + (this->b * 0.114f);
				dest[1] = -(this->r * 0.169f) - (this->g * 0.331f) + (this->b * 0.500f);
				dest[2] = (this->r * 0.500f) - (this->g * 0.419f) - (this->b * 0.081f);
			}
			else
			{
				dest[0] = (this->r * 0.213f) + (this->g * 0.715f) + (this->b * 0.072f);
				dest[1] = -(this->r * 0.115f) - (this->g * 0.385f) + (this->b * 0.500f);
				dest[2] = (this->r * 0.500f) - (this->g * 0.454f) - (this->b * 0.046f);
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
			if ((Math::isZero(this->r) == true) && (Math::isZero(this->g) == true) && (Math::isZero(this->b) == true))
			{
				dest[0] = dest[1] = dest[2] = 0.0f;
				dest[3] = 1.0f;
				return;
			}

			float w = std::fmax(this->r, std::fmax(this->g, this->b));
			float wInv = 1.0f / w;

			dest[0] = (w - this->r) * wInv;
			dest[1] = (w - this->g) * wInv;
			dest[2] = (w - this->b) * wInv;
			dest[3] = 1.0f - w;
		}

		void writeCMYK(unsigned char * const dest) const
		{
			float bufferAux[4];

			writeCMYK(bufferAux);
			Color::convertColor(dest, bufferAux, true);
		}

		void addColorWeighted(const Color &color, const float &weight)
		{
			__m128 temp;

			temp = _mm_mul_ps(_mm_loadu_ps(&color.r), _mm_load_ps1(&weight));
			_mm_storeu_ps(&r, _mm_add_ps(_mm_loadu_ps(&r), temp));
		}

		void toGrayscale()
		{
			float newLum = r * 0.3019607843f;
			newLum += g * 0.5921568627f;
			newLum += b * 0.1098039215f;

			this->r = newLum;
			this->g = newLum;
			this->b = newLum;
		}

		void toGrayscale(const float &rWeight, const float &gWeight, const float &bWeight)
		{
			float newLum = r * rWeight;
			newLum += g * gWeight;
			newLum += b * bWeight;

			this->r = newLum;
			this->g = newLum;
			this->b = newLum;
		}

		void negativeRGB()
		{
			float alphaAux = this->a;
			_mm_storeu_ps(&r, _mm_sub_ps(Math::SIMD::fOne, _mm_loadu_ps(&r)));
			this->a = alphaAux;
		}

		void negativeRGB(const float &value)
		{
			float alphaAux = this->a;
			_mm_storeu_ps(&r, _mm_sub_ps(_mm_set_ps1(value), _mm_loadu_ps(&r)));
			this->a = alphaAux;
		}

		void swapRB()
		{
			float temp = this->r;
			this->r = this->b;
			this->b = temp;
		}

		void weightRGB(const float weight)
		{
			this->r *= weight;
			this->g *= weight;
			this->b *= weight;
		}

		void mad(const float opMul, const float opAdd)
		{
			__m128 pixelVal, valMul, valAdd;

			pixelVal = _mm_loadu_ps(&r);
			valMul = _mm_load_ps1(&opMul);
			valAdd = _mm_load_ps1(&opAdd);

			pixelVal = _mm_mul_ps(pixelVal, valMul);
			_mm_storeu_ps(&r, _mm_add_ps(pixelVal, valAdd));
		}

		void clamp()
		{
			_mm_storeu_ps(&r, _mm_min_ps(_mm_max_ps(_mm_loadu_ps(&r), _mm_setzero_ps()), Math::SIMD::fOne));
		}

		void clamp(const float &min, const float &max)
		{
			_mm_storeu_ps(&r, _mm_min_ps(_mm_max_ps(_mm_loadu_ps(&r), _mm_load_ps1(&min)), _mm_load_ps1(&max)));
		}

		void calcInterpolate(const Color &from, const Color &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(&from.r), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(&to.r), _mm_load_ps1(&t)));
			_mm_storeu_ps(&r, tmp);
		}

		void calcInterpolate(const float * const from, const float * const to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(from), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to), _mm_load_ps1(&t)));
			_mm_storeu_ps(&r, tmp);
		}

		void calcInterpolate(const Color &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(&r), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(&to.r), _mm_load_ps1(&t)));
			_mm_storeu_ps(&r, tmp);
		}
	};

} //HorseRadish
