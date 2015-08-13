#include "stringUtils.hpp"

#include <libs\cppformat\format.h>

#include <memory>
#include <codecvt>

namespace HorseRadish
{
	const unsigned __int32 StringUtils::utf8Wrapper::utf8Iterator::offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

	void StringUtils::conv2UTF8(const std::wstring& strUTF16, std::string& strUTF8)
	{
		if (strUTF16.empty())
			return;

		strUTF8.append(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(strUTF16));
	}

	void StringUtils::conv2UTF8(const wchar_t* const strUTF16, std::string& strUTF8)
	{
		if (strUTF16 == nullptr)
			return;

		strUTF8.append(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(strUTF16));
	}

	void StringUtils::conv2UTF8(const unsigned int charUnicode, std::string& strUTF8)
	{
		strUTF8.reserve(strUTF8.size() + 4);

		char tmpBuffer[4];

		auto numBytes = StringUtils::unicodeUTF8(charUnicode, tmpBuffer);
		strUTF8.append(tmpBuffer, numBytes);
	}

	void StringUtils::conv2UTF8(const std::vector<unsigned int>& strUnicode, std::string& strUTF8)
	{
		if (strUnicode.empty())
			return;

		strUTF8.reserve(strUTF8.size() + (strUnicode.size() * 2));

		char tmpBuffer[4];
		for (const auto& curUnicode : strUnicode)
		{
			auto numBytes = StringUtils::unicodeUTF8(curUnicode, tmpBuffer);
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

	std::string StringUtils::conv2UTF8(const unsigned int charUnicode)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(charUnicode, strUTF8);
		return strUTF8;
	}

	std::string StringUtils::conv2UTF8(const std::vector<unsigned int>& strUnicode)
	{
		std::string strUTF8;
		StringUtils::conv2UTF8(strUnicode, strUTF8);
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
		if (strUTF8 == nullptr)
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

	bool StringUtils::endsWith(const std::string& str, const std::string& ending)
	{
		return (str.size() >= ending.size()) && equal(ending.rbegin(), ending.rend(), str.rbegin());
	}

	void StringUtils::closeAt(std::string& str, unsigned int pos)
	{
		str = StringUtils::closeAtCopy(str, pos);
	}

	std::string StringUtils::closeAtCopy(const std::string& str, unsigned int pos)
	{
		if (pos == 0)
			return std::string();

		std::string newStr;
		newStr.reserve(str.size());

		char tmpBuffer[4];
		for (const auto& curChar : StringUtils::utf8Wrapper(str))
		{
			if ((pos--) == 0)
				break;

			auto numBytes = StringUtils::unicodeUTF8(curChar, tmpBuffer);
			newStr.append(tmpBuffer, numBytes);
		}

		return newStr;
	}

	void StringUtils::trim(std::string& str)
	{
		if (str.empty())
			return;

		str.assign(StringUtils::findFirst(str.c_str()), StringUtils::findLast(str.c_str() + str.length()));
	}

	std::string StringUtils::trimCopy(const std::string& str)
	{
		if (str.empty())
			return std::string();

		return std::string(StringUtils::findFirst(str.c_str()), StringUtils::findLast(str.c_str() + str.length()));
	}

	void StringUtils::replace(std::string& str, const unsigned int unicodeCharOld, const unsigned int unicodeCharNew)
	{
		str = StringUtils::replaceCopy(str, unicodeCharOld, unicodeCharNew);
	}

	std::string StringUtils::replaceCopy(const std::string& str, const unsigned int replaceOldChar, const unsigned int replaceNewChar)
	{
		std::string newStr;
		newStr.reserve(str.size());

		char tmpBuffer[4];
		for (const auto& curChar : StringUtils::utf8Wrapper(str))
		{
			auto numBytes = StringUtils::unicodeUTF8((curChar == replaceOldChar) ? replaceNewChar : curChar, tmpBuffer);
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

	unsigned int StringUtils::getUnicodeAt(const std::string& str, const unsigned int strIndex)
	{
		if (str.empty())
			return 0;

		unsigned int curIndex = 0;
		for (const auto& curUnicode : StringUtils::utf8Wrapper(str))
		{
			if (curIndex == strIndex)
				return curUnicode;

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
		for (const auto& curUnicode : StringUtils::utf8Wrapper(str))
		{
			walker -= StringUtils::unicodeUTF8Size(curUnicode);
			StringUtils::unicodeUTF8(curUnicode, walker);
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
			return fmt::format("{0:.3} seconds", remaining);

		remaining = static_cast<double>(remaining) * 0.01666666666666666666666666666667;
		if (remaining < 60.0)
			return fmt::format("{0:.3} minutes", remaining);

		remaining = static_cast<double>(remaining) * 0.01666666666666666666666666666667;
		if (remaining < 60.0)
			return fmt::format("{0:.3} hours", remaining);

		remaining = static_cast<double>(remaining) * 0.04166666666666666666666666666667;
		return fmt::format("{0:.3} days", remaining);
	}

	std::string StringUtils::formatSize(unsigned int bytes)
	{
		if (bytes < 1024)
			return fmt::format("{0} bytes", bytes);

		double remaining = static_cast<double>(bytes) * 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{0:.3} KB", remaining);

		remaining = static_cast<double>(remaining)* 0.0009765625;
		if (remaining < 1024.0)
			return fmt::format("{0:.3} MB", remaining);

		remaining = static_cast<double>(remaining)* 0.0009765625;
		return fmt::format("{0:.3} GB", remaining);
	}
}