#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <span>
#include <vector>
#include <cstddef>

namespace hr::serialize
{
	template<class T>
	struct NamedParam
	{
		std::string_view name;
		const T& param;
	};

	template<class T>
	struct Support
	{ };

	namespace traits
	{
		namespace detail
		{
			template <typename T> struct is_std_span : std::false_type{};
			template <typename T> struct is_std_span<std::span<T>> : std::true_type{};
			template <typename T> struct is_std_span<const std::span<T>> : std::true_type{};
			template <typename T, size_t N> struct is_std_span<std::span<T, N>> : std::true_type{};
			template <typename T, size_t N> struct is_std_span<const std::span<T, N>> : std::true_type{};

			template <typename T> struct is_std_vector : std::false_type{};
			template <typename... TArgs> struct is_std_vector<std::vector<TArgs...>> : std::true_type{};

			template <typename T> struct is_std_shared_ptr : std::false_type{};
			template <typename... TArgs> struct is_std_shared_ptr<std::shared_ptr<TArgs...>> : std::true_type{};

			template <typename T> struct is_std_unique_ptr : std::false_type{};
			template <typename... TArgs> struct is_std_unique_ptr<std::unique_ptr<TArgs...>> : std::true_type{};

			template <typename T> struct is_std_optional : std::false_type{};
			template <typename... TArgs> struct is_std_optional<std::optional<TArgs...>> : std::true_type{};

			template <typename T> struct is_named_param : std::false_type{};
			template <typename... TArgs> struct is_named_param<NamedParam<TArgs...>> : std::true_type{};
		};

		template<typename T>
		struct is_std_span
		{
			static constexpr bool const value = detail::is_std_span<std::decay_t<T>>::value;
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

		template<typename T>
		struct is_named_param
		{
			static constexpr bool const value = detail::is_named_param<std::decay_t<T>>::value;
		};
	}

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
		
		bool skipBytes(size_t size)
		{
			return static_cast<TArchive*>(this)->skipBytes(size);
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
		ArchiveReader& operator>>(const T& value)
		{
			static_assert(!std::is_same_v<T, size_t>); //because size_t size varies between architectures

			if constexpr (traits::is_std_span<T>::value)
			{
				auto numElements = m_archive.readFixed<uint32_t>();
				if (value.size() < numElements)
				{
					//skip values
					for (; numElements > 0; numElements--)
					{
						typename T::value_type elem;
						(*this) >> elem;
					}
				}
				else if (numElements > 0)
				{
					if constexpr (TArchiveImpl::isBinary() && std::is_trivial_v<typename T::value_type>)
					{
						m_archive.readBytes(std::as_writable_bytes(value));
					}
					else
					{
						size_t curIndex{0};
						for (; numElements > 0; numElements--)
							(*this) >> value[curIndex++];
					}
				}
			}
			else
			{
				(Support<typename std::decay<T>::type>{}).load(*this, value);
			}

			return *this;
		}

		template<typename T>
		ArchiveReader& operator>> (T& value)
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
			else if constexpr (traits::is_std_span<T>::value)
			{
				auto numElements = m_archive.readFixed<uint32_t>();
				if (value.size() < numElements)
				{
					//skip values
					for (; numElements > 0; numElements--)
					{
						typename T::value_type elem;
						(*this) >> elem;
					}
				}
				else if (numElements > 0)
				{
					if constexpr (TArchiveImpl::isBinary() && std::is_trivial_v<typename T::value_type>)
					{
						m_archive.readBytes(std::as_writable_bytes(value));
					}
					else
					{
						size_t curIndex{ 0 };
						for (; numElements > 0; numElements--)
							(*this) >> value[curIndex++];
					}
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
				value = (hasData != 0) ? (Support<typename std::decay<T>::type>{}).loadShared(*this) : std::shared_ptr<T>{};
			}
			else if constexpr (traits::is_std_unique_ptr<T>::value)
			{
				auto hasData = m_archive.readFixed<uint8_t>();
				value = (hasData != 0) ? (Support<typename std::decay<T>::type>{}).loadUnique(*this) : std::unique_ptr<T>{};
			}
			else if constexpr (traits::is_std_optional<T>::value)
			{
				if (auto hasData = m_archive.readFixed<uint8_t>(); hasData != 0)
					(*this) >> *value;
				else
                    value = std::nullopt_t{};
			}
			else
			{
				(Support<typename std::decay<T>::type>{}).load(*this, value);
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
		ArchiveWriter& operator<<(const T& value)
		{
			static_assert(!std::is_same_v<T, size_t>); //because size_t size varies between architectures

			if constexpr (traits::is_named_param<T>::value)
			{
				m_archive.writeObjectField(value.name);
				(*this) << value.param;
			}
			else if constexpr (std::is_enum_v<T>)
			{
				m_archive.template writeFixed<typename std::underlying_type<T>::type>(static_cast<typename std::underlying_type<T>::type>(value));
			}
			else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
			{
				m_archive.template writeFixed<T>(value);
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
			{
				m_archive.writeString(value);
			}
			else if constexpr (traits::is_std_span<T>::value)
			{
				m_archive.writeArrayBegin(value.size());

				if constexpr (TArchiveImpl::isBinary() && std::is_trivial_v<typename T::value_type>)
				{
					m_archive.writeBytes(std::as_bytes(value));
				}
				else
				{
					for (const auto& element : value)
						(*this) << element;
				}

				m_archive.writeArrayEnd();
			}
			else if constexpr (traits::is_std_vector<T>::value)
			{
				m_archive.writeArrayBegin(value.size());

				if constexpr (TArchiveImpl::isBinary() && std::is_trivial_v<typename T::value_type>)
				{
					m_archive.writeBytes(std::as_bytes(std::span{value}));
				}
				else
				{
					for (const auto& element : value)
						(*this) << element;
				}

				m_archive.writeArrayEnd();
			}
			else if constexpr (traits::is_std_shared_ptr<T>::value || traits::is_std_unique_ptr<T>::value || traits::is_std_optional<T>::value)
			{
				m_archive.writePtrState(static_cast<bool>(value));
				if (value) (*this) << *value;
			}
			else
			{
				m_archive.writeObjectBegin();
				(Support<typename std::decay<T>::type>{}).save(*this, value);
				m_archive.writeObjectEnd();
			}

			return *this;
		}

	private:
		TArchiveImpl& m_archive;
	};
}
