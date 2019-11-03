#include "stringUtils.hpp"

#include <libs/fmt/format.h>

#include <memory>
#include <codecvt>

namespace hr
{
	namespace
	{
		// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
		// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

		constexpr uint8_t utf8d[] = {
			// The first part of the table maps bytes to character classes that
			// to reduce the size of the transition table and create bitmasks.
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
			 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			 8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
			10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

			// The second part is a transition table that maps a combination
			// of a state of the automaton and a character class to a state.
			 0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
			12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
			12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
			12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
			12,36,12,12,12,12,12,12,12,12,12,12,
		};

		uint32_t decode(uint32_t* state, uint32_t* codep, uint32_t byte)
		{
			uint32_t type = utf8d[byte];

			*codep = (*state != UTF8_ACCEPT) ?
				(byte & 0x3fu) | (*codep << 6) :
				(0xff >> type) & (byte);

			*state = utf8d[256 + *state + type];
			return *state;
		}
	}

	namespace
	{
		const char* skipLeftWhitespace(const char* str)
		{
			while (std::memchr(" \t\n\r", *str, 4))
				++str;
			return str;
		}

		const char* skipRightWhitespace(const char* end)
		{
			while (std::memchr(" \t\n\r", end[-1], 4))
				--end;
			return end;
		}

		size_t ucodepointUTF8Size(const char32_t codepoint)
		{
			if (codepoint < 0x80)
				return 1;
			if (codepoint < 0x800)
				return 2;
			if (codepoint < 0x10000)
				return 3;
			if (codepoint < 0x110000)
				return 4;
			return 0;
		}

		size_t ucodepointUTF8Encode(const char32_t codepoint, char* const out)
		{
			if (codepoint < 0x80)
			{
				out[0] = static_cast<char>(codepoint);
				return 1;
			}

			if (codepoint < 0x800)
			{
				out[0] = static_cast<char>((codepoint >> 6) | 0xC0);
				out[1] = static_cast<char>((codepoint & 0x3F) | 0x80);
				return 2;
			}

			if (codepoint < 0x10000)
			{
				out[0] = static_cast<char>((codepoint >> 12) | 0xE0);
				out[1] = static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
				out[2] = static_cast<char>((codepoint & 0x3F) | 0x80);
				return 3;
			}

			if (codepoint < 0x110000)
			{
				out[0] = static_cast<char>((codepoint >> 18) | 0xF0);
				out[1] = static_cast<char>(((codepoint >> 12) & 0x3F) | 0x80);
				out[2] = static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
				out[3] = static_cast<char>((codepoint & 0x3F) | 0x80);
				return 4;
			}

			return 0;
		}

		size_t ucodepointUTF16Encode(char32_t codepoint, char16_t* const out)
		{
			if (codepoint <= 0xFFFF)
			{
				//UTF-16 surrogate values are illegal in UTF-32 (0xFFFF or 0xFFFE are both reserved values)
				if ((codepoint >= 0xD800) && (codepoint <= 0xDFFF))
					return 0;

				out[0] = static_cast<char16_t>(codepoint);
				return 1;
			}

			if (codepoint > 0x0010FFFF) //max legal UTF32
				return 0;

			//target is a character in range 0xFFFF - 0x10FFFF
			out[0] = static_cast<char16_t>(((codepoint - 0x0010000UL) >> 10) + 0xD800);
			out[1] = static_cast<char16_t>(((codepoint - 0x0010000UL) & 0x3FFUL) + 0xDC00);
			return 2;
		}

		size_t ucodepointUTF16Decode(const char16_t* in, size_t count, char32_t& codepoint)
		{
			if (count <= 0)
				return 0;

			auto uc = static_cast<char32_t>(*in++);
			if ((uc - 0xd800) >= 2048) //isn't a surrogate
			{
				codepoint = uc;
				return 1; //just read 1 element
			}

			if (count < 2)
				return 0;

			if ((uc & 0xfffffc00) != 0xd800) //must be high surrogate (uc = high)
				return 0;

			auto low = static_cast<char32_t>(*in);
			if ((low & 0xfffffc00) != 0xdc00) //must be low surrogate
				return 0;

			codepoint = (uc << 10) + low - 0x35fdc00;
			return 2;
		}
	}

	StringUtils::utf8Wrapper::utf8Iterator& StringUtils::utf8Wrapper::utf8Iterator::operator++()
	{
		if (mIt == mItEnd)
			return *this;

		if (mIt < mItNext) //already moved forward (see operator*)
		{
			mIt = mItNext;
			return *this;
		}

		uint32_t codepoint;
		uint32_t state = UTF8_ACCEPT;

		while (mIt != mItEnd)
		{
			if (!decode(&state, &codepoint, *mIt))
				break;
			++mIt;
		}

		if (state != UTF8_ACCEPT)
			mIt = mItNext = mItEnd; //invalid
		else
			++mIt;

		return *this;
	}

	char32_t StringUtils::utf8Wrapper::utf8Iterator::operator*() noexcept
	{
		if (mIt == mItEnd)
			return 0;

		mItNext = mIt;

		uint32_t codepoint = 0;
		uint32_t state = UTF8_ACCEPT;

		while (mItNext != mItEnd)
		{
			if (!decode(&state, &codepoint, *mItNext))
				break;
			++mItNext;
		}

		if (state != UTF8_ACCEPT)
		{
			mIt = mItNext = mItEnd; //invalid
			return 0;
		}

		++mItNext;
		return static_cast<char32_t>(codepoint);
	}

	StringUtils::utf8Wrapper::utf8Wrapper(std::string_view str) noexcept
		: mStr{ std::move(str) }
	{ }

	StringUtils::utf8Wrapper::utf8Wrapper(std::string_view str, size_t skipCodepoints)
		: mStr{ std::move(str) }
	{
		if (skipCodepoints <= 0)
			return;

		uint32_t codepoint;
		uint32_t state = UTF8_ACCEPT;

		auto s = reinterpret_cast<const uint8_t*>(mStr.data());
		for (; *s; ++s)
		{
			if (decode(&state, &codepoint, *s))
				continue;

			skipCodepoints--;
			if (skipCodepoints <= 0)
				break;
		}

		if ((state != UTF8_ACCEPT) || (*s == '\0'))
		{
			mStr = std::string_view{};
			return;
		}

		s++;
		mStr = std::string_view(reinterpret_cast<const char*>(s), mStr.length() - (s - reinterpret_cast<const uint8_t*>(mStr.data())));
	}

	StringUtils::utf16Wrapper::utf16Iterator& StringUtils::utf16Wrapper::utf16Iterator::operator++()
	{
		if (mIt == mItEnd)
			return *this;

		if (mIt < mItNext) //already moved forward (see operator*)
		{
			mIt = mItNext;
			return *this;
		}

		char32_t codepoint;
		auto count = ucodepointUTF16Decode(mItNext, mItEnd - mItNext, codepoint);
		if (count == 0)
			mIt = mItNext = mItEnd; //invalid
		else
			mIt += count;

		return *this;
	}

	char32_t StringUtils::utf16Wrapper::utf16Iterator::operator*() noexcept
	{
		if (mIt == mItEnd)
			return 0;

		mItNext = mIt;

		char32_t codepoint;
		mItNext += ucodepointUTF16Decode(mItNext, mItEnd - mItNext, codepoint);
		if (mItNext == mIt)
		{
			mIt = mItNext = mItEnd; //invalid
			return 0;
		}

		return codepoint;
	}

	StringUtils::utf16Wrapper::utf16Wrapper(std::u16string_view str) noexcept
		: mStr{ std::move(str) }
	{ }

	bool StringUtils::isValidUTF8(std::string_view str)
	{
		if (str.empty())
			return true;

		uint32_t codepoint;
		uint32_t state = UTF8_ACCEPT;

		auto s = reinterpret_cast<const uint8_t*>(str.data());
		while (*s)
			decode(&state, &codepoint, *s++);

		return (state == UTF8_ACCEPT);
	}

	size_t StringUtils::countUTF8Codepoints(std::string_view str)
	{
		if (str.empty())
			return 0;

		size_t count = 0;
		uint32_t codepoint;
		uint32_t state = UTF8_ACCEPT;

		for (auto s = reinterpret_cast<const uint8_t*>(str.data()); *s; ++s)
		{
			if (!decode(&state, &codepoint, *s))
				count++;
		}

		if (state != UTF8_ACCEPT)
			return 0; //invalid UTF8

		return count;
	}

	void StringUtils::conv2UTF8(std::wstring_view str, std::string& strUTF8)
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t)) //absurd way of checking if we are in windows or not...
		{
			conv2UTF8(std::u16string_view{ reinterpret_cast<const char16_t*>(str.data()), str.size() }, strUTF8);
			return;
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			conv2UTF8(std::u32string_view{ reinterpret_cast<const char32_t*>(str.data()), str.size() }, strUTF8);
			return;
		}
	}

	void StringUtils::conv2UTF8(char32_t codepoint, std::string& strUTF8)
	{
		strUTF8.reserve(strUTF8.size() + 4);

		char tmpBuffer[4];
		auto numBytes = ucodepointUTF8Encode(codepoint, tmpBuffer);

		strUTF8.append(tmpBuffer, numBytes);
	}

	void StringUtils::conv2UTF8(std::u16string_view strUTF16, std::string& strUTF8)
	{
		char tmpBuffer[4];
		for (const auto& codepoint : StringUtils::utf16Wrapper(strUTF16))
		{
			auto numBytes = ucodepointUTF8Encode(codepoint, tmpBuffer);
			strUTF8.append(tmpBuffer, numBytes);
		}
	}

	void StringUtils::conv2UTF8(std::u32string_view strUTF32, std::string& strUTF8)
	{
		if (strUTF32.empty())
			return;

		strUTF8.reserve(strUTF8.size() + (strUTF32.size() * 2));

		char tmpBuffer[4];
		for (const auto& codepoint : strUTF32)
		{
			auto numBytes = ucodepointUTF8Encode(codepoint, tmpBuffer);
			strUTF8.append(tmpBuffer, numBytes);
		}
	}

	std::string StringUtils::conv2UTF8(std::wstring_view str)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(str, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(char32_t codepoint)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(codepoint, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(std::u16string_view strUTF16)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(strUTF16, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(std::u32string_view strUTF32)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(strUTF32, strUTF8);
		return strUTF8;
	}

	std::u16string StringUtils::conv2UTF16(std::string_view strUTF8)
	{
		if (strUTF8.empty())
			return {};

		std::u16string str;
		str.reserve(strUTF8.size());

		for (const auto& codepoint : StringUtils::utf8Wrapper(strUTF8))
		{
			char16_t out[2];
			auto bytes = ucodepointUTF16Encode(codepoint, out);
			if (bytes <= 0)
				return {}; //illegal input

			str.push_back(out[0]);
			if (bytes == 2)
				str.push_back(out[1]);
		}

		return str;
	}

	std::u32string StringUtils::conv2UTF32(std::string_view strUTF8)
	{
		if (strUTF8.empty())
			return {};

		std::u32string str;
		str.reserve(strUTF8.size());

		for (const auto& codepoint : StringUtils::utf8Wrapper(strUTF8))
			str.push_back(codepoint);

		return str;
	}

	std::wstring StringUtils::conv2Native(std::string_view strUTF8)
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t)) //absurd way of checking if we are in windows or not...
		{
			std::wstring str;

			for (const auto& codepoint : StringUtils::utf8Wrapper(strUTF8))
			{
				char16_t out[2];
				auto bytes = ucodepointUTF16Encode(codepoint, out);
				if (bytes <= 0)
					return {}; //illegal input

				str.push_back(static_cast<wchar_t>(out[0]));
				if (bytes == 2)
					str.push_back(static_cast<wchar_t>(out[1]));
			}

			return str;
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			std::wstring str;

			for (const auto& codepoint : StringUtils::utf8Wrapper(strUTF8))
				str.push_back(static_cast<wchar_t>(codepoint));

			return str;
		}
		else
		{
			return {};
		}
	}

	bool StringUtils::endsWith(std::string_view str, std::string_view ending)
	{
		return (str.size() >= ending.size()) && equal(ending.rbegin(), ending.rend(), str.rbegin());
	}

	void StringUtils::closeAt(std::string& str, size_t pos)
	{
		str = StringUtils::closeAtCopy(str, pos);
	}

	std::string StringUtils::closeAtCopy(std::string_view str, size_t pos)
	{
		if (pos == 0)
			return std::string();

		std::string newStr;
		newStr.reserve(str.size());

		char tmpBuffer[4];
		for (const auto& codepoint : StringUtils::utf8Wrapper(str))
		{
			if ((pos--) == 0)
				break;

			auto numBytes = ucodepointUTF8Encode(codepoint, tmpBuffer);
			newStr.append(tmpBuffer, numBytes);
		}

		return newStr;
	}

	void StringUtils::trim(std::string& str)
	{
		if (str.empty())
			return;

		str.assign(skipLeftWhitespace(str.c_str()), skipRightWhitespace(str.c_str() + str.length()));
	}

	std::string StringUtils::trimCopy(std::string_view str)
	{
		if (str.empty())
			return std::string();

		return std::string(skipLeftWhitespace(str.data()), skipRightWhitespace(str.data() + str.length()));
	}

	void StringUtils::erase(std::string& str, const char32_t codepoint)
	{
		str = StringUtils::eraseCopy(str, codepoint);
	}

	std::string StringUtils::eraseCopy(std::string_view str, const char32_t codepoint)
	{
		std::string newStr;
		newStr.reserve(str.size());

		char tmpBuffer[4];
		for (const auto& curCodepoint : StringUtils::utf8Wrapper(str))
		{
			if (curCodepoint == codepoint)
				continue;

			auto numBytes = ucodepointUTF8Encode(curCodepoint, tmpBuffer);
			newStr.append(tmpBuffer, numBytes);
		}

		return newStr;
	}

	void StringUtils::replace(std::string& str, const char32_t codepointOld, const char32_t codepointNew)
	{
		str = StringUtils::replaceCopy(str, codepointOld, codepointNew);
	}

	std::string StringUtils::replaceCopy(std::string_view str, const char32_t codepointOld, const char32_t codepointNew)
	{
		std::string newStr;
		newStr.reserve(str.size());

		char tmpBuffer[4];
		for (const auto& curCodepoint : StringUtils::utf8Wrapper(str))
		{
			auto numBytes = ucodepointUTF8Encode((curCodepoint == codepointOld) ? codepointNew : curCodepoint, tmpBuffer);
			newStr.append(tmpBuffer, numBytes);
		}

		return newStr;
	}

	void StringUtils::replace(std::string& str, std::string_view replaceOldStr, std::string_view replaceNewStr)
	{
		str = StringUtils::replaceCopy(str, replaceOldStr, replaceNewStr);
	}

	std::string StringUtils::replaceCopy(std::string_view str, std::string_view replaceOldStr, std::string_view replaceNewStr)
	{
		if (str.empty() || replaceOldStr.empty())
			return {};

		std::string newStr{ str };

		std::string::size_type pos;
		while ((pos = str.find_first_of(replaceOldStr, 0)) != std::string::npos)
			newStr.replace(pos, pos + replaceOldStr.size(), replaceNewStr);

		return newStr;
	}

	char32_t StringUtils::getUnicodeAt(std::string_view str, size_t index)
	{
		if (str.empty())
			return 0;

		size_t curIndex = 0;
		for (const auto& codepoint : StringUtils::utf8Wrapper(str))
		{
			if (curIndex == index)
				return codepoint;

			curIndex++;
		}

		return 0;
	}

	void StringUtils::reverse(std::string& str)
	{
		auto reversed = StringUtils::reverseCopy(str);
		std::swap(reversed, str);
	}

	std::string StringUtils::reverseCopy(std::string_view str)
	{
		if (str.empty())
			return std::string();

		std::unique_ptr<char[]> buffer(new char[str.size() + 1]);
		auto walker = buffer.get() + str.size();

		*walker = '\0';
		for (const auto& codepoint : StringUtils::utf8Wrapper(str))
		{
			walker -= ucodepointUTF8Size(codepoint);
			ucodepointUTF8Encode(codepoint, walker);
		}

		return std::string(walker);
	}

	std::string StringUtils::formatDuration(std::chrono::milliseconds duration)
	{
		auto miliseconds = duration.count();
		if (miliseconds < 1000)
			return fmt::format("{} ms", miliseconds);

		double remaining = static_cast<double>(miliseconds) * 0.001;
		if (remaining < 60.0)
			return fmt::format("{:.3f} seconds", remaining);

		remaining = static_cast<double>(remaining) * 0.01666666666666666666666666666667;
		if (remaining < 60.0)
			return fmt::format("{:.3f} minutes", remaining);

		remaining = static_cast<double>(remaining) * 0.01666666666666666666666666666667;
		if (remaining < 60.0)
			return fmt::format("{:.3f} hours", remaining);

		remaining = static_cast<double>(remaining) * 0.04166666666666666666666666666667;
		return fmt::format("{:.3f} days", remaining);
	}

	std::string StringUtils::formatSize(uint64_t bytes)
	{
		if (bytes < 1024)
			return fmt::format("{} bytes", bytes);

		double remaining = static_cast<double>(bytes) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{:.3f} KB", remaining);

		remaining = static_cast<double>(remaining) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{:.3f} MB", remaining);

		remaining = static_cast<double>(remaining) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{:.3f} GB", remaining);

		remaining = static_cast<double>(remaining) * 0.0009765625;
		return fmt::format("{:.3f} TB", remaining);
	}

	std::string StringUtils::formatSize(int64_t bytes)
	{
		auto sign = "";

		if (bytes < 0)
		{
			sign = "-";
			bytes = -bytes;
		}

		if (bytes < 1024)
			return fmt::format("{}{} bytes", sign, bytes);

		double remaining = static_cast<double>(bytes) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{}{:.3f} KB", sign, remaining);

		remaining = static_cast<double>(remaining) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{}{:.3f} MB", sign, remaining);

		remaining = static_cast<double>(remaining) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{}{:.3f} GB", sign, remaining);

		remaining = static_cast<double>(remaining) * 0.0009765625;
		return fmt::format("{}{:.3f} TB", sign, remaining);
	}
}