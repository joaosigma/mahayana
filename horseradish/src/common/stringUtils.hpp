#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <iterator>

namespace HorseRadish
{
	class StringUtils
	{
	public:
		class utf8Wrapper
		{
		public:
			class utf8Iterator : public std::iterator<std::forward_iterator_tag, unsigned int>
			{
			public:
				utf8Iterator(const std::string& str)
				{
					mStrIt = str.begin();
					mStrItEnd = str.end();
				}

				utf8Iterator(const std::string& str, std::string::const_iterator it)
					: mStrIt(it)
				{
					mStrItEnd = str.end();
				}

				utf8Iterator(const utf8Iterator& it)
				{
					mStrIt = it.mStrIt;
					mStrItEnd = it.mStrItEnd;
					mStrItNext = it.mStrItNext;
				}

				bool operator==(const utf8Iterator& other)
				{
					return (mStrIt == other.mStrIt);
				}

				bool operator!=(const utf8Iterator& other)
				{
					return (mStrIt != other.mStrIt);
				}

				utf8Iterator& operator++()
				{
					if (mStrIt == mStrItEnd)
						return *this;

					if (mStrIt < mStrItNext)
					{
						mStrIt = mStrItNext;
						return *this;
					}

					do {
						mStrIt++;
					} while ((mStrIt != mStrItEnd) && !utf8Iterator::isutf8(*mStrIt));

					return *this;
				}

				utf8Iterator operator++(int)
				{
					utf8Iterator tmp(*this);
					operator++();
					return tmp;
				}

				unsigned int operator*()
				{
					if (mStrIt == mStrItEnd)
						return 0;

					mStrItNext = mStrIt;

					unsigned int finalChar = 0, bytesRead = 0;
					do {
						finalChar <<= 6;
						finalChar += static_cast<unsigned char>(*mStrItNext);

						bytesRead++;
						mStrItNext++;
					} while ((mStrItNext != mStrItEnd) && !utf8Iterator::isutf8(*mStrItNext));

					finalChar -= utf8Iterator::offsetsFromUTF8[bytesRead - 1];
					return finalChar;
				}

			private:
				static const unsigned __int32 offsetsFromUTF8[6];

				static bool isutf8(const char &value)
				{
					return ((value & 0xC0) != 0x80);
				}

			private:
				std::string::const_iterator mStrIt, mStrItNext, mStrItEnd;
			};

		public:
			typedef utf8Iterator const_iterator;

			utf8Wrapper(const std::string& str)
				: mStr(str)
			{
			}

			const_iterator begin() const
			{
				return utf8Iterator(mStr);
			}

			const_iterator end() const
			{
				return utf8Iterator(mStr, mStr.end());
			}

		private:
			const std::string& mStr;
		};

	private:
		static const char* findFirst(const char* str)
		{
			while (memchr(" \t\n\r", *str, 4)) 
				++str;
			return str;
		}

		static const char* findLast(const char* end)
		{
			while (memchr(" \t\n\r", end[-1], 4))
				--end;
			return end;
		}

		static unsigned int unicodeUTF8Size(const unsigned int unicodeChar)
		{
			if (unicodeChar < 0x80)
				return 1;
			if (unicodeChar < 0x800)
				return 2;
			if (unicodeChar < 0x10000)
				return 3;
			if (unicodeChar < 0x110000)
				return 4;
			return 0;
		}

		static unsigned int unicodeUTF8(const unsigned int unicodeChar, char* const outBuffer)
		{
			if (unicodeChar < 0x80)
			{
				outBuffer[0] = (char)unicodeChar;
				return 1;
			}

			if (unicodeChar < 0x800)
			{
				outBuffer[0] = (unicodeChar >> 6) | 0xC0;
				outBuffer[1] = (unicodeChar & 0x3F) | 0x80;
				return 2;
			}

			if (unicodeChar < 0x10000)
			{
				outBuffer[0] = (unicodeChar >> 12) | 0xE0;
				outBuffer[1] = ((unicodeChar >> 6) & 0x3F) | 0x80;
				outBuffer[2] = (unicodeChar & 0x3F) | 0x80;
				return 3;
			}

			if (unicodeChar < 0x110000)
			{
				outBuffer[0] = (unicodeChar >> 18) | 0xF0;
				outBuffer[1] = ((unicodeChar >> 12) & 0x3F) | 0x80;
				outBuffer[2] = ((unicodeChar >> 6) & 0x3F) | 0x80;
				outBuffer[3] = (unicodeChar & 0x3F) | 0x80;
				return 4;
			}

			outBuffer[0] = '\0';
			return 0;
		}

	public:
		static std::string conv2UTF8(const std::wstring& strUTF16);
		static std::string conv2UTF8(const wchar_t* const strUTF16);
		static std::string conv2UTF8(const unsigned int charUnicode);
		static std::string conv2UTF8(const std::vector<unsigned int>& strUnicode);

		static std::wstring conv2UTF16(const std::string& strUTF8);
		static std::wstring conv2UTF16(const char* const strUTF8);
		static std::vector<unsigned int> conv2Unicode(const std::string& strUTF8);

		static std::string trimCopy(const std::string& str);
		static void trim(std::string& str);

		static void replace(std::string& str, const unsigned int unicodeCharOld, const unsigned int unicodeCharNew);
		static std::string replaceCopy(const std::string& str, const unsigned int replaceOldChar, const unsigned int replaceNewChar);

		static void replace(std::string& str, const std::string& replaceOldStr, const std::string& replaceNewStr);
		static std::string replaceCopy(const std::string& str, const std::string& replaceOldStr, const std::string& replaceNewStr);

		static unsigned int getUnicodeAt(const std::string& str, const unsigned int strIndex);

		static void reverse(std::string& str);
		static std::string reverseCopy(const std::string& str);

		static std::string formatDuration(std::chrono::milliseconds duration);
		static std::string formatSize(unsigned int bytes);
	};
}
