#pragma once

#include <array>
#include <vector>
#include <cassert>
#include <functional>
#include <type_traits>

namespace hr
{
	/**
	 * @brief Basic sparse packed map implementation
	 *
	 * This is exactly like SparsePackedSet, but with an aditional array to store values.
	 *
	 * @note
	 * By design, there's no inherent order for how TKeys and/or TValues are stored
	 *
	 */
	template<class TKey, class TValue, class THash = std::hash<TKey>, size_t TPageSize = 4096>
	class SparsePackedMap final
	{
		static_assert(TPageSize && ((TPageSize & (TPageSize - 1)) == 0), "size must be power of two");
		static_assert(sizeof(TKey) < TPageSize);
		static_assert(!std::is_empty_v<TKey>, "no point in storing an empty key");
		static_assert(!std::is_empty_v<TValue>, "no point in storing an empty value");

		static constexpr size_t KeysPerPage = TPageSize / sizeof(TKey);

		std::vector<TKey> mDenseKeys;
		std::vector<TValue> mDenseValues;
		std::vector<std::unique_ptr<std::array<size_t, KeysPerPage>>> mReverse;
		TValue mEmptyValue{};

		static std::pair<size_t, size_t> indices(const TKey& key) noexcept
		{
			auto id = THash()(key);
			return { static_cast<size_t>(id / KeysPerPage), static_cast<size_t>(id & (KeysPerPage - 1)) };
		}

		bool indexDense(const TKey& key, size_t& index) const noexcept
		{
			auto[page, offset] = indices(key);
			if ((page >= mReverse.size()) || !mReverse[page] || (mReverse[page]->operator[](offset) <= 0))
				return false;

			index = (mReverse[page]->operator[](offset)) - 1;
			return true;
		}

		void assurePage(size_t pageIndex)
		{
			if (pageIndex >= mReverse.size())
				mReverse.resize(pageIndex + 1);

			if (mReverse[pageIndex])
				return;

			mReverse[pageIndex] = std::make_unique<std::array<size_t, KeysPerPage>>();
			std::memset(mReverse[pageIndex]->data(), 0, sizeof(size_t) * KeysPerPage);
		}

	public:
		class Iterator
		{
			friend class SparsePackedMap;

			const TKey *mItKey{ nullptr };
			const TValue *mItValue{ nullptr };

			constexpr explicit Iterator(const TKey* startKey, const TValue* startValue) noexcept
				: mItKey{ startKey }, mItValue{ startValue }
			{ }

		public:
			// iterator traits
			using iterator_category = std::random_access_iterator_tag;
			using value_type = TKey;
			using difference_type = ptrdiff_t;
			using pointer = const std::pair<const TKey&, const TValue&>;
			using reference = const std::pair<const TKey&, const TValue&>;

		public:
			bool operator==(const Iterator& other) const noexcept
			{
				assert((mItKey == other.mItKey) == (mItValue == other.mItValue));
				return (mItKey == other.mItKey);
			}

			bool operator!=(const Iterator& other) const noexcept
			{
				assert((mItKey == other.mItKey) == (mItValue == other.mItValue));
				return (mItKey != other.mItKey);
			}

			Iterator& operator++() noexcept
			{
				mItKey++;
				mItValue++;
				return *this;
			}

			Iterator operator++(int) const noexcept
			{
				Iterator tmp(*this);
				operator++();
				return tmp;
			}

			Iterator& operator--() noexcept
			{
				mItKey--;
				mItValue--;
				return *this;
			}

			Iterator operator--(int) const noexcept
			{
				Iterator tmp(*this);
				operator--();
				return tmp;
			}

			const std::pair<const TKey&, const TValue&> operator->() const noexcept
			{
				return { *mItKey, *mItValue };
			}

			const std::pair<const TKey&, const TValue&> operator*() const noexcept
			{
				return { *mItKey, *mItValue };
			}
		};

		using const_iterator = Iterator;

	public:
		SparsePackedMap() = default;
		SparsePackedMap(const SparsePackedMap&) = delete;
		SparsePackedMap& operator=(const SparsePackedMap&) = delete;
		SparsePackedMap(SparsePackedMap&&) = default;
		SparsePackedMap& operator=(SparsePackedMap&&) = default;
		~SparsePackedMap() = default;

		const_iterator begin() const noexcept
		{
			return const_iterator{ mDenseKeys.data(), mDenseValues.data() };
		}

		const_iterator end() const noexcept
		{
			return const_iterator{ mDenseKeys.data() + mDenseKeys.size(), mDenseValues.data() + mDenseValues.size() };
		}

		void reserve(size_t newSize)
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			mDenseKeys.reserve(newSize);
			mDenseValues.reserve(newSize);
		}

		void shrinkToFit()
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			if (mDenseKeys.empty())
				mReverse.clear();

			mDenseKeys.shrink_to_fit();
			mDenseValues.shrink_to_fit();
			mReverse.shrink_to_fit();
		}

		size_t size() const noexcept
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			return mDenseKeys.size();
		}

		bool empty() const noexcept
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			return mDenseKeys.empty();
		}
		
		bool has(const TKey& key) const noexcept
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			[[maybe_unused]] size_t index;
			return indexDense(key, index);
		}

		const_iterator find(const TKey& key) const noexcept
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			size_t index;
			if (!indexDense(key, index))
				return end();

			return const_iterator{ mDenseKeys.data() + index, mDenseValues.data() + index };
		}

		TValue& value(const TKey& key) noexcept
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			size_t index;
			if (!indexDense(key, index))
				return mEmptyValue;

			return mDenseValues[index];
		}

		const TValue& value(const TKey& key) const noexcept
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			size_t index;
			if (!indexDense(key, index))
				return mEmptyValue;

			return mDenseValues[index];
		}

		void add(TKey key, TValue value)
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			auto[page, offset] = indices(key);
			
			assurePage(page);
			auto& pageRef = *mReverse[page];

			if (pageRef[offset] == 0)
			{
				pageRef[offset] = mDenseKeys.size() + 1;
				mDenseKeys.push_back(std::move(key));
				mDenseValues.push_back(std::move(value));
			}
			else
			{
				mDenseKeys[pageRef[offset] - 1] = std::move(key);
				mDenseValues[pageRef[offset] - 1] = std::move(value);
			}
		}

		void remove(const TKey& key)
		{
			assert(mDenseKeys.size() == mDenseValues.size());

			if (!has(key))
				return;

			auto indicesRemove = indices(key);
			auto indicesLast = indices(mDenseKeys.back());

			auto& pageRefRemove = *mReverse[indicesRemove.first];
			auto& pageRefLast = *mReverse[indicesLast.first];

			std::swap(mDenseKeys[pageRefRemove[indicesRemove.second] - 1], mDenseKeys[pageRefLast[indicesLast.second] - 1]);
			std::swap(mDenseValues[pageRefRemove[indicesRemove.second] - 1], mDenseValues[pageRefLast[indicesLast.second] - 1]);
			pageRefLast[indicesLast.second] = pageRefRemove[indicesRemove.second];
			pageRefRemove[indicesRemove.second] = 0;

			mDenseKeys.pop_back();
			mDenseValues.pop_back();
		}
	};
}