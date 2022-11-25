#pragma once

#include <string_view>
#include <optional>
#include <memory>
#include <span>
#include <vector>
#include <cstddef>

namespace hr::serialize
{
	namespace traits
	{
		namespace detail
		{
			template <typename T> struct is_std_vector : std::false_type{};
			template <typename... TArgs> struct is_std_vector<std::vector<TArgs...>> : std::true_type{};

			template <typename T> struct is_std_shared_ptr : std::false_type{};
			template <typename... TArgs> struct is_std_shared_ptr<std::shared_ptr<TArgs...>> : std::true_type{};

			template <typename T> struct is_std_unique_ptr : std::false_type{};
			template <typename... TArgs> struct is_std_unique_ptr<std::shared_ptr<TArgs...>> : std::true_type{};

			template <typename T> struct is_std_optional : std::false_type{};
			template <typename... TArgs> struct is_std_optional<std::shared_ptr<TArgs...>> : std::true_type{};
		};

		template <typename T>
		struct is_std_vector {
			static constexpr bool const value = detail::is_std_vector<std::decay_t<T>>::value;
		};

		template <typename T>
		struct is_std_shared_ptr {
			static constexpr bool const value = detail::is_std_shared_ptr<std::decay_t<T>>::value;
		};

		template <typename T>
		struct is_std_unique_ptr {
			static constexpr bool const value = detail::is_std_unique_ptr<std::decay_t<T>>::value;
		};

		template <typename T>
		struct is_std_optional {
			static constexpr bool const value = detail::is_std_optional<std::decay_t<T>>::value;
		};
	}

	template<class T>
	struct NamedParam
	{
		std::string_view name;
		const T& param;
	};

	struct Access
	{
		template<typename T>
		static NamedParam<T> namedParam(std::string_view name, const T& param)
		{
			return NamedParam<T>{ .name = name, .param = param};
		}

		template<class TArchive, class TClass>
		static void serialize(TArchive& writer, const TClass& t)
		{
			t.serialize<TArchive>(writer);
		}

		template<class TArchive, class TClass>
		static void deserialize(TArchive& reader, TClass& t)
		{
			t.deserialize<TArchive>(reader);
		}

		template<class TArchive, class TClass>
		static std::shared_ptr<TClass> deserializeShared(TArchive& reader)
		{
			return TClass::template deserializeNew<TArchive>(reader);
		}

		template<class TArchive, class TClass>
		static std::shared_ptr<TClass> deserializeUnique(TArchive& reader)
		{
			return TClass::template deserializeUnique<TArchive>(reader);
		}
	};

	template<class TArchive>
	struct Archive
	{
		template<typename T>
		T readFixed()
		{
			return static_cast<TArchive*>(this)->template readFixed<T>();
		}

		bool readBytes(std::span<std::byte> buffer)
		{
			return static_cast<TArchive*>(this)->readBytes(buffer);
		}

		void writeArrayBegin(size_t numElements)
		{
			static_cast<TArchive*>(this)->writeArrayBegin(numElements);
		}

		void writeArrayEnd()
		{
			static_cast<TArchive*>(this)->writeArrayEnd();
		}

		void writeObjectBegin()
		{
			static_cast<TArchive*>(this)->writeObjectBegin();
		}

		void writeObjectEnd()
		{
			static_cast<TArchive*>(this)->writeObjectEnd();
		}

		void writeObjectField(std::string_view name)
		{
			static_cast<TArchive*>(this)->writeObjectField(name);
		}

		void writePtrState(const bool valid)
		{
			static_cast<TArchive*>(this)->writePtrState(valid);
		}

		template<typename T>
		void writeFixed(const T val)
		{
			static_cast<TArchive*>(this)->template writeFixed<T>(val);
		}

		void writeString(std::string_view str)
		{
			static_cast<TArchive*>(this)->writeString(str);
		}

		void writeBuffer(std::span<const std::byte> buffer)
		{
			static_cast<TArchive*>(this)->writeBuffer(buffer);
		}
	};

	template<class TArchiveImpl>
	class ArchiveReader
	{
	public:
		explicit ArchiveReader(TArchiveImpl& archiver) noexcept
		  : m_archive{ archiver }
		{ }

		const TArchiveImpl& archive() const noexcept
		{
			return m_archive;
		}

		template<typename T>
		ArchiveReader& operator >> (T& value)
		{
			if constexpr (std::is_enum_v<T>)
			{
				value = static_cast<T>(m_archive.readFixed<typename std::underlying_type<T>::type>());
			}
			else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
			{
				value = m_archive.readFixed<T>();
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				value.clear();

				auto size = m_archive.readFixed<uint32_t>();
				if (size > 0)
				{
					value.resize(size);
					m_archive.readBytes(std::as_writable_bytes(std::span{ value }));
				}
			}
			else if constexpr (traits::is_std_vector<T>::value)
			{
				value.clear();

				auto numElements = m_archive.readFixed<uint32_t>();
				if (numElements > 0)
				{
					if constexpr (TArchiveImpl::isBinary() && std::is_trivial_v<typename T::value_type>)
					{
						value.resize(numElements);
						m_archive.readBytes(std::as_writable_bytes(std::span{ value }));
					}
					else
					{
						value.reserve(numElements);
						for (; numElements > 0; numElements--)
						{
							typename T::value_type elem;
							(*this) >> elem;
							value.push_back(std::move(elem));
						}
					}
				}
			}
			else if constexpr (traits::is_std_shared_ptr<T>::value)
			{
				auto hasData = m_archive.readFixed<uint8_t>();
				value = (hasData != 0) ? Access::deserializeShared<ArchiveReader<TArchiveImpl>, T::element_type>(*this) : std::shared_ptr<T>{};
			}
			else if constexpr (traits::is_std_unique_ptr<T>::value)
			{
				auto hasData = m_archive.readFixed<uint8_t>();
				value = (hasData != 0) ? Access::deserializeUnique<ArchiveReader<TArchiveImpl>, T::element_type>(*this) : std::unique_ptr<T>{};
			}
			else if constexpr (traits::is_std_optional<T>::value)
			{
				if (auto hasData = m_archive.readFixed<uint8_t>(); hasData != 0)
					(*this) >> *value;
				else
					value = std::nullopt_t;
			}
			else
			{
				Access::deserialize<ArchiveReader<TArchiveImpl>, T>(*this, value);
			}

			return *this;
		}

	private:
		TArchiveImpl& m_archive;
	};

	template<class TArchiveImpl>
	class ArchiveWriter
	{
	public:
		ArchiveWriter(TArchiveImpl& archiver) noexcept
		  : m_archive{ archiver }
		{ }

		const TArchiveImpl& archive() const noexcept
		{
			return m_archive;
		}

		template<typename T>
		ArchiveWriter& operator<<(const NamedParam<T>& namedParam)
		{
			m_archive.writeObjectField(namedParam.name);
			return ((*this) << namedParam.param);
		}

		template<typename T, typename std::enable_if<!std::is_enum<T>::value>::type* = nullptr>
		ArchiveWriter& operator<<(const T& value)
		{
			m_archive.writeObjectBegin();
			Access::serialize<ArchiveWriter<TArchiveImpl>, T>(*this, value);
			m_archive.writeObjectEnd();

			return *this;
		}

		template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
		ArchiveWriter& operator<<(const T& enumValue)
		{
			m_archive.template writeFixed<typename std::underlying_type<T>::type>(static_cast<typename std::underlying_type<T>::type>(enumValue));
			return *this;
		}

		ArchiveWriter& operator<<(const char& value)
		{
			m_archive.template writeFixed<char>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const int8_t& value)
		{
			m_archive.template writeFixed<int8_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const uint8_t& value)
		{
			m_archive.template writeFixed<uint8_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const int16_t& value)
		{
			m_archive.template writeFixed<int16_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const uint16_t& value)
		{
			m_archive.template writeFixed<uint16_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const int32_t& value)
		{
			m_archive.template writeFixed<int32_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const uint32_t& value)
		{
			m_archive.template writeFixed<uint32_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const int64_t& value)
		{
			m_archive.template writeFixed<int64_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const uint64_t& value)
		{
			m_archive.template writeFixed<uint64_t>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const float& value)
		{
			m_archive.template writeFixed<float>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const double& value)
		{
			m_archive.template writeFixed<double>(value);
			return *this;
		}

		ArchiveWriter& operator<<(const bool& value)
		{
			m_archive.writeBool(value);
			return *this;
		}

		ArchiveWriter& operator<<(const std::string& str)
		{
			m_archive.writeString(str);
			return *this;
		}

		ArchiveWriter& operator<<(std::string_view str)
		{
			m_archive.writeString(str);
			return *this;
		}

		template<typename T>
		ArchiveWriter& operator<<(const std::vector<T>& vec)
		{
			m_archive.writeArrayBegin(vec.size());

			if constexpr (TArchiveImpl::isBinary() && std::is_trivial_v<T>)
			{
				m_archive.writeBytes(std::as_bytes(std::span{vec}));
			}
			else
			{
				for (const auto& element : vec)
					(*this) << element;
			}

			m_archive.writeArrayEnd();
			return *this;
		}

		template<typename T>
		ArchiveWriter& operator<<(const std::shared_ptr<T>& ptr)
		{
			m_archive.writePtrState(static_cast<bool>(ptr));
			if (ptr) (*this) << *ptr;
			return *this;
		}

		template<typename T>
		ArchiveWriter& operator<<(const std::unique_ptr<T>& ptr)
		{
			m_archive.writePtrState(static_cast<bool>(ptr));
			if (ptr) (*this) << *ptr;
			return *this;
		}

		template<typename T>
		ArchiveWriter& operator<<(const std::optional<T>& optional)
		{
			m_archive.writePtrState(static_cast<bool>(optional));
			if (optional) (*this) << *optional;
			return *this;
		}

	private:
		TArchiveImpl& m_archive;
	};
}
