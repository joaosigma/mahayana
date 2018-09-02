#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <iterator>

namespace hr
{
	class StringUtils
	{
	public:
		class utf8Wrapper
		{
		public:
			class utf8Iterator : public std::iterator<std::forward_iterator_tag, unsigned int>
			{
				const uint8_t *mIt, *mItNext, *mItEnd;

			public:
				explicit constexpr utf8Iterator(const uint8_t* start, const uint8_t* end)
					: mIt{ start }, mItNext{ start }, mItEnd{ end }
				{ }

				bool operator==(const utf8Iterator& other)
				{
					return (mIt == other.mIt);
				}

				bool operator!=(const utf8Iterator& other)
				{
					return (mIt != other.mIt);
				}

				utf8Iterator& operator++();
				utf8Iterator operator++(int);

				unsigned int operator*();
			};

		public:
			typedef utf8Iterator const_iterator;

			explicit utf8Wrapper(std::string_view str);
			explicit utf8Wrapper(std::string_view str, size_t skipCodepoints);

			const_iterator begin() const
			{
				auto start = reinterpret_cast<const uint8_t*>(mStr.data());
				auto end = start + mStr.size();
				return utf8Iterator(start, end);
			}

			const_iterator end() const
			{
				auto start = reinterpret_cast<const uint8_t*>(mStr.data());
				auto end = start + mStr.size();
				return utf8Iterator(end, end);
			}

		private:
			std::string_view mStr;
		};

	public:
		static bool isValidUTF8(std::string_view str);
		static size_t countUTF8Codepoints(std::string_view str);

		static void conv2UTF8(const std::wstring& strUTF16, std::string& strUTF8);
		static void conv2UTF8(const wchar_t* const strUTF16, std::string& strUTF8);
		static void conv2UTF8(const unsigned int ucodepoint, std::string& strUTF8);
		static void conv2UTF8(const std::vector<unsigned int>& ucodepoints, std::string& strUTF8);
		static std::string conv2UTF8(const std::wstring& strUTF16);
		static std::string conv2UTF8(const wchar_t* const strUTF16);
		static std::string conv2UTF8(const unsigned int ucodepoint);
		static std::string conv2UTF8(const std::vector<unsigned int>& ucodepoints);

		static std::wstring conv2UTF16(const std::string& strUTF8);
		static std::wstring conv2UTF16(const char* const strUTF8);
		static std::vector<unsigned int> conv2Unicode(const std::string& strUTF8);

		static bool endsWith(std::string_view str, std::string_view ending);

		static void closeAt(std::string& str, size_t pos);
		static std::string closeAtCopy(std::string_view str, size_t pos);

		static void trim(std::string& str);
		static std::string trimCopy(std::string_view str);

		static void erase(std::string& str, const unsigned int codepoint);
		static std::string eraseCopy(std::string_view str, const unsigned int codepoint);

		static void replace(std::string& str, const unsigned int codepointOld, const unsigned int codepointNew);
		static std::string replaceCopy(std::string_view str, const unsigned int codepointOld, const unsigned int codepointNew);

		static void replace(std::string& str, const std::string& replaceOldStr, const std::string& replaceNewStr);
		static std::string replaceCopy(const std::string& str, const std::string& replaceOldStr, const std::string& replaceNewStr);

		static unsigned int getUnicodeAt(const std::string& str, size_t index);

		static void reverse(std::string& str);
		static std::string reverseCopy(const std::string& str);

		static std::string formatDuration(std::chrono::milliseconds duration);
		static std::string formatSize(size_t bytes);
	};
}
