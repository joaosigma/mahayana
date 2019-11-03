#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace hr
{
	class StringUtils
	{
	public:
		class utf8Wrapper
		{
		public:
			class utf8Iterator
			{
				const uint8_t *mIt, *mItNext, *mItEnd;

			public:
				// iterator traits
				using iterator_category = std::forward_iterator_tag;
				using value_type = char32_t;
				using difference_type = ptrdiff_t;
				using pointer = const char32_t*;
				using reference = const char32_t&;

			public:
				explicit constexpr utf8Iterator(const uint8_t* start, const uint8_t* end) noexcept
					: mIt{ start }, mItNext{ start }, mItEnd{ end }
				{ }

				bool operator==(const utf8Iterator& other) const noexcept
				{
					return (mIt == other.mIt);
				}

				bool operator!=(const utf8Iterator& other) const noexcept
				{
					return (mIt != other.mIt);
				}

				utf8Iterator& operator++();

				utf8Iterator operator++(int)
				{
					utf8Iterator tmp(*this);
					operator++();
					return tmp;
				}

				char32_t operator*() noexcept;
			};

		public:
			using const_iterator = utf8Iterator;

			explicit utf8Wrapper(std::string_view str) noexcept;
			explicit utf8Wrapper(std::string_view str, size_t skipCodepoints);

			const_iterator begin() const noexcept
			{
				auto start = reinterpret_cast<const uint8_t*>(mStr.data());
				auto end = start + mStr.size();
				return const_iterator(start, end);
			}

			const_iterator end() const noexcept
			{
				auto start = reinterpret_cast<const uint8_t*>(mStr.data());
				auto end = start + mStr.size();
				return const_iterator(end, end);
			}

		private:
			std::string_view mStr;
		};

		class utf16Wrapper
		{
		public:
			class utf16Iterator
			{
				const char16_t *mIt, *mItNext, *mItEnd;

			public:
				// iterator traits
				using iterator_category = std::forward_iterator_tag;
				using value_type = char32_t;
				using difference_type = ptrdiff_t;
				using pointer = const char32_t*;
				using reference = const char32_t&;

			public:
				explicit constexpr utf16Iterator(const char16_t* start, const char16_t* end) noexcept
					: mIt{ start }, mItNext{ start }, mItEnd{ end }
				{ }

				bool operator==(const utf16Iterator& other) const noexcept
				{
					return (mIt == other.mIt);
				}

				bool operator!=(const utf16Iterator& other) const noexcept
				{
					return (mIt != other.mIt);
				}

				utf16Iterator& operator++();

				utf16Iterator operator++(int)
				{
					utf16Iterator tmp(*this);
					operator++();
					return tmp;
				}

				char32_t operator*() noexcept;
			};

		public:
			using const_iterator = utf16Iterator;

			explicit utf16Wrapper(std::u16string_view str) noexcept;

			const_iterator begin() const noexcept
			{
				auto start = reinterpret_cast<const char16_t*>(mStr.data());
				auto end = start + mStr.size();
				return const_iterator(start, end);
			}

			const_iterator end() const noexcept
			{
				auto start = reinterpret_cast<const char16_t*>(mStr.data());
				auto end = start + mStr.size();
				return const_iterator(end, end);
			}

		private:
			std::u16string_view mStr;
		};

	public:
		static bool isValidUTF8(std::string_view str);
		static size_t countUTF8Codepoints(std::string_view str);

		static void conv2UTF8(std::wstring_view str, std::string& strUTF8);
		static void conv2UTF8(char32_t codepoint, std::string& strUTF8);
		static void conv2UTF8(std::u16string_view strUTF16, std::string& strUTF8);
		static void conv2UTF8(std::u32string_view strUTF32, std::string& strUTF8);

		static std::string conv2UTF8(std::wstring_view str);
		static std::string conv2UTF8(char32_t codepoint);
		static std::string conv2UTF8(std::u16string_view strUTF16);
		static std::string conv2UTF8(std::u32string_view strUTF32);

		static std::u16string conv2UTF16(std::string_view strUTF8);
		static std::u32string conv2UTF32(std::string_view strUTF8);
		static std::wstring conv2Native(std::string_view strUTF8);

		static bool endsWith(std::string_view str, std::string_view ending);

		static void closeAt(std::string& str, size_t pos);
		static std::string closeAtCopy(std::string_view str, size_t pos);

		static void trim(std::string& str);
		static std::string trimCopy(std::string_view str);

		static void erase(std::string& str, const char32_t codepoint);
		static std::string eraseCopy(std::string_view str, const char32_t codepoint);

		static void replace(std::string& str, const char32_t codepointOld, const char32_t codepointNew);
		static std::string replaceCopy(std::string_view str, const char32_t codepointOld, const char32_t codepointNew);

		static void replace(std::string& str, std::string_view replaceOldStr, std::string_view replaceNewStr);
		static std::string replaceCopy(std::string_view str, std::string_view replaceOldStr, std::string_view replaceNewStr);

		static char32_t getUnicodeAt(std::string_view str, size_t index);

		static void reverse(std::string& str);
		static std::string reverseCopy(std::string_view str);

		static std::string formatDuration(std::chrono::milliseconds duration);
		static std::string formatSize(uint64_t bytes);
		static std::string formatSize(int64_t bytes);
	};
}
