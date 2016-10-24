#include "types.hpp"

#include <cstring>

namespace HorseRadish
{
	bool operator == (const hData128& a, const hData128& b)
	{
		return (memcmp(a.ui8, b.ui8, sizeof(a.ui8)) == 0);
	}

	bool operator != (const hData128& a, const hData128& b)
	{
		return (memcmp(a.ui8, b.ui8, sizeof(a.ui8)) != 0);
	}

	bool operator < (const hData128& a, const hData128& b)
	{
		return (memcmp(a.ui8, b.ui8, sizeof(a.ui8)) < 0);
	}

	bool operator > (const hData128& a, const hData128& b)
	{
		return (memcmp(a.ui8, b.ui8, sizeof(a.ui8)) > 0);
	}

	bool operator <= (const hData128& a, const hData128& b)
	{
		return (memcmp(a.ui8, b.ui8, sizeof(a.ui8)) <= 0);
	}

	bool operator >= (const hData128& a, const hData128& b)
	{
		return (memcmp(a.ui8, b.ui8, sizeof(a.ui8)) >= 0);
	}
}