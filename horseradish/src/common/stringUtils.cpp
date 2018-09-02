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

		size_t ucodepointUTF8Size(const unsigned int codepoint)
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

		size_t ucodepointUTF8Encode(const unsigned int codepoint, char* const outBuffer)
		{
			if (codepoint < 0x80)
			{
				outBuffer[0] = static_cast<char>(codepoint);
				return 1;
			}

			if (codepoint < 0x800)
			{
				outBuffer[0] = static_cast<char>((codepoint >> 6) | 0xC0);
				outBuffer[1] = static_cast<char>((codepoint & 0x3F) | 0x80);
				return 2;
			}

			if (codepoint < 0x10000)
			{
				outBuffer[0] = static_cast<char>((codepoint >> 12) | 0xE0);
				outBuffer[1] = static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
				outBuffer[2] = static_cast<char>((codepoint & 0x3F) | 0x80);
				return 3;
			}

			if (codepoint < 0x110000)
			{
				outBuffer[0] = static_cast<char>((codepoint >> 18) | 0xF0);
				outBuffer[1] = static_cast<char>(((codepoint >> 12) & 0x3F) | 0x80);
				outBuffer[2] = static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
				outBuffer[3] = static_cast<char>((codepoint & 0x3F) | 0x80);
				return 4;
			}

			outBuffer[0] = '\0';
			return 0;
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

	StringUtils::utf8Wrapper::utf8Iterator StringUtils::utf8Wrapper::utf8Iterator::operator++(int)
	{
		utf8Iterator tmp(*this);
		operator++();
		return tmp;
	}

	unsigned int StringUtils::utf8Wrapper::utf8Iterator::operator*()
	{
		if (mIt == mItEnd)
			return 0;

		mItNext = mIt;

		uint32_t codepoint;
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
		return static_cast<unsigned int>(codepoint);
	}

	StringUtils::utf8Wrapper::utf8Wrapper(std::string_view str)
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

	void StringUtils::conv2UTF8(const std::wstring& strUTF16, std::string& strUTF8)
	{
		if (strUTF16.empty())
			return;

		strUTF8.append(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(strUTF16));
	}

	void StringUtils::conv2UTF8(const wchar_t* const strUTF16, std::string& strUTF8)
	{
		if (!strUTF16)
			return;

		strUTF8.append(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(strUTF16));
	}

	void StringUtils::conv2UTF8(const unsigned int ucodepoint, std::string& strUTF8)
	{
		strUTF8.reserve(strUTF8.size() + 4);

		char tmpBuffer[4];
		auto numBytes = ucodepointUTF8Encode(ucodepoint, tmpBuffer);

		strUTF8.append(tmpBuffer, numBytes);
	}

	void StringUtils::conv2UTF8(const std::vector<unsigned int>& ucodepoints, std::string& strUTF8)
	{
		if (ucodepoints.empty())
			return;

		strUTF8.reserve(strUTF8.size() + (ucodepoints.size() * 2));

		char tmpBuffer[4];
		for (const auto& codepoint : ucodepoints)
		{
			auto numBytes = ucodepointUTF8Encode(codepoint, tmpBuffer);
			strUTF8.append(tmpBuffer, numBytes);
		}
	}

	std::string StringUtils::conv2UTF8(const std::wstring& strUTF16)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(strUTF16, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(const wchar_t* const strUTF16)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(strUTF16, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(const unsigned int ucodepoint)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(ucodepoint, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(const std::vector<unsigned int>& ucodepoints)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(ucodepoints, strUTF8);
		return strUTF8;
	}

	std::wstring StringUtils::conv2UTF16(const std::string& strUTF8)
	{
		if (strUTF8.empty())
			return std::wstring();

		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().from_bytes(strUTF8);
	}

	std::wstring StringUtils::conv2UTF16(const char* const strUTF8)
	{
		if (!strUTF8)
			return std::wstring();

		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().from_bytes(strUTF8);
	}

	std::vector<unsigned int> StringUtils::conv2Unicode(const std::string& strUTF8)
	{
		if (strUTF8.empty())
			return std::vector<unsigned int>();

		std::vector<unsigned int> vec;
		vec.reserve(strUTF8.size());

		for (const auto& curUnicode : StringUtils::utf8Wrapper(strUTF8))
			vec.push_back(curUnicode);

		return vec;
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

	void StringUtils::erase(std::string& str, const unsigned int codepoint)
	{
		str = StringUtils::eraseCopy(str, codepoint);
	}

	std::string StringUtils::eraseCopy(std::string_view str, const unsigned int codepoint)
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

	void StringUtils::replace(std::string& str, const unsigned int codepointOld, const unsigned int codepointNew)
	{
		str = StringUtils::replaceCopy(str, codepointOld, codepointNew);
	}

	std::string StringUtils::replaceCopy(std::string_view str, const unsigned int codepointOld, const unsigned int codepointNew)
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

	void StringUtils::replace(std::string& str, const std::string& replaceOldStr, const std::string& replaceNewStr)
	{
		str = StringUtils::replaceCopy(str, replaceOldStr, replaceNewStr);
	}

	std::string StringUtils::replaceCopy(const std::string& str, const std::string& replaceOldStr, const std::string& replaceNewStr)
	{
		if (str.empty() || replaceOldStr.empty())
			return str;

		std::string newStr = str;

		std::string::size_type pos;
		while ((pos = str.find_first_of(replaceOldStr, 0)) != std::string::npos)
			newStr.replace(pos, pos + replaceOldStr.size(), replaceNewStr);

		return newStr;
	}

	unsigned int StringUtils::getUnicodeAt(const std::string& str, size_t index)
	{
		if (str.empty())
			return 0;

		unsigned int curIndex = 0;
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

	std::string StringUtils::reverseCopy(const std::string& str)
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
			return fmt::format("{0} ms", miliseconds);

		double remaining = static_cast<double>(miliseconds) * 0.001;
		if (remaining < 60.0)
			return fmt::format("{0:.3f} seconds", remaining);

		remaining = static_cast<double>(remaining) * 0.01666666666666666666666666666667;
		if (remaining < 60.0)
			return fmt::format("{0:.3f} minutes", remaining);

		remaining = static_cast<double>(remaining) * 0.01666666666666666666666666666667;
		if (remaining < 60.0)
			return fmt::format("{0:.3f} hours", remaining);

		remaining = static_cast<double>(remaining) * 0.04166666666666666666666666666667;
		return fmt::format("{0:.3f} days", remaining);
	}

	std::string StringUtils::formatSize(size_t bytes)
	{
		if (bytes < 1024)
			return fmt::format("{0} bytes", bytes);

		double remaining = static_cast<double>(bytes) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{0:.3f} KB", remaining);

		remaining = static_cast<double>(remaining)* 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{0:.3f} MB", remaining);

		remaining = static_cast<double>(remaining)* 0.0009765625;
		return fmt::format("{0:.3f} GB", remaining);
	}
}