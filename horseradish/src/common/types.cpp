#include "types.hpp"

#include "math.hpp"

#include <limits>
#include <cstring>

namespace hr::types
{
	namespace
	{
		constexpr float shortScaleTo = static_cast<float>(std::numeric_limits<int16_t>::max());
		constexpr float shortScaleFrom = 1.0f / static_cast<float>(std::numeric_limits<int16_t>::max());
		
		constexpr float ushortScaleTo = static_cast<float>(std::numeric_limits<uint16_t>::max());
		constexpr float ushortScaleFrom = 1.0f / static_cast<float>(std::numeric_limits<uint16_t>::max());
	}

	namespace detail
	{
		void pack(const float* const in, int16_t* const out, size_t numValues)
		{
			for (size_t i = 0; i < numValues; i++)
				out[i] = static_cast<int16_t>(Math::fClamp(in[i], -1.0f, 1.0f) * shortScaleTo);
		}

		void pack(const float* const in, uint16_t* const out, size_t numValues)
		{
			for (size_t i = 0; i < numValues; i++)
				out[i] = static_cast<uint16_t>(Math::fClamp(in[i], 0.0f, 1.0f) * ushortScaleTo);
		}

		void unpack(const int16_t* const in, float* const out, size_t numValues)
		{
			for (size_t i = 0; i < numValues; i++)
				out[i] = static_cast<float>(in[i])* shortScaleFrom;
		}

		void unpack(const uint16_t* const in, float* const out, size_t numValues)
		{
			for (size_t i = 0; i < numValues; i++)
				out[i] = static_cast<float>(in[i])* ushortScaleFrom;
		}
	}

	// hData128
	bool operator == (const hData128& a, const hData128& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) == 0);
	}

	bool operator != (const hData128& a, const hData128& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) != 0);
	}

	bool operator < (const hData128& a, const hData128& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) < 0);
	}

	bool operator > (const hData128& a, const hData128& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) > 0);
	}

	bool operator <= (const hData128& a, const hData128& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) <= 0);
	}

	bool operator >= (const hData128& a, const hData128& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) >= 0);
	}

	// hData256
	bool operator == (const hData256& a, const hData256& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) == 0);
	}

	bool operator != (const hData256& a, const hData256& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) != 0);
	}

	bool operator < (const hData256& a, const hData256& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) < 0);
	}

	bool operator > (const hData256& a, const hData256& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) > 0);
	}

	bool operator <= (const hData256& a, const hData256& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) <= 0);
	}

	bool operator >= (const hData256& a, const hData256& b)
	{
		return (std::memcmp(a.ui8, b.ui8, sizeof(a.ui8)) >= 0);
	}
}