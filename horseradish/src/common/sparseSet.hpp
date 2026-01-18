#pragma once

#include <array>
#include <functional>
#include <type_traits>
#include <vector>

namespace hr
{
    /**
     * @brief Basic sparse packed set implementation
     *
     * A sparse packed set is typically only useful to store (unsigned) integers, but we can go around
     * that limitation (and store other types of data) by finding a way to get a unique id for each element.
     * We do that by requiring that the hash used on the data to be a perfect hash. In other words, the value
     * returned by the hash function uniquely identifies the data.
     *
     * @note
     * By design, there's no inherent order for how TKeys are stored
     *
     */
    template<class TKey, class THash = std::hash<TKey>, size_t TPageSize = 4096>
    class SparsePackedSet final
    {
        static_assert(TPageSize && ((TPageSize & (TPageSize - 1)) == 0), "size must be power of two");
        static_assert(sizeof(TKey) < TPageSize);
        static_assert(!std::is_empty_v<TKey>, "no point in storing an empty key");

        static constexpr size_t KeysPerPage = TPageSize / sizeof(TKey);

        std::vector<TKey> mDense;
        std::vector<std::unique_ptr<std::array<size_t, KeysPerPage>>> mReverse;

        static std::pair<size_t, size_t> indices(const TKey& key) noexcept
        {
            auto id = THash()(key);
            return {static_cast<size_t>(id / KeysPerPage), static_cast<size_t>(id & (KeysPerPage - 1))};
        }

        bool indexDense(const TKey& key, size_t& index) const noexcept
        {
            auto [page, offset] = indices(key);
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
            friend class SparsePackedSet;

            const TKey* mIt{nullptr};

            constexpr explicit Iterator(const TKey* start) noexcept
              : mIt{start}
            {}

        public:
            // iterator traits
            using iterator_category = std::random_access_iterator_tag;
            using value_type = TKey;
            using difference_type = ptrdiff_t;
            using pointer = const TKey*;
            using reference = const TKey&;

        public:
            bool operator==(const Iterator& other) const noexcept
            {
                return (mIt == other.mIt);
            }

            bool operator!=(const Iterator& other) const noexcept
            {
                return (mIt != other.mIt);
            }

            Iterator& operator++() noexcept
            {
                mIt++;
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
                mIt--;
                return *this;
            }

            Iterator operator--(int) const noexcept
            {
                Iterator tmp(*this);
                operator--();
                return tmp;
            }

            const TKey* operator->() const noexcept
            {
                return mIt;
            }

            const TKey& operator*() const noexcept
            {
                return *mIt;
            }
        };

        using const_iterator = Iterator;

    public:
        SparsePackedSet() = default;
        SparsePackedSet(const SparsePackedSet&) = delete;
        SparsePackedSet& operator=(const SparsePackedSet&) = delete;
        SparsePackedSet(SparsePackedSet&&) = default;
        SparsePackedSet& operator=(SparsePackedSet&&) = default;
        virtual ~SparsePackedSet() = default;

        const_iterator begin() const noexcept
        {
            return const_iterator{mDense.data()};
        }

        const_iterator end() const noexcept
        {
            return const_iterator{mDense.data() + mDense.size()};
        }

        void reserve(size_t newSize)
        {
            mDense.reserve(newSize);
        }

        void shrinkToFit()
        {
            if (mDense.empty())
                mReverse.clear();

            mDense.shrink_to_fit();
            mReverse.shrink_to_fit();
        }

        size_t size() const noexcept
        {
            return mDense.size();
        }

        bool empty() const noexcept
        {
            return mDense.empty();
        }

        const TKey* data() const noexcept
        {
            return mDense.data();
        }

        bool has(const TKey& key) const noexcept
        {
            [[maybe_unused]] size_t index;
            return indexDense(key, index);
        }

        const_iterator find(const TKey& key) const noexcept
        {
            size_t index;
            if (!indexDense(key, index))
                return end();

            return const_iterator{mDense.data() + index};
        }

        void add(TKey key)
        {
            auto [page, offset] = indices(key);

            assurePage(page);
            auto& pageRef = *mReverse[page];

            if (pageRef[offset] == 0)
            {
                pageRef[offset] = mDense.size() + 1;
                mDense.push_back(std::move(key));
            }
            else
            {
                mDense[pageRef[offset] - 1] = std::move(key);
            }
        }

        void remove(const TKey& key)
        {
            if (!has(key))
                return;

            auto indicesRemove = indices(key);
            auto indicesLast = indices(mDense.back());

            auto& pageRefRemove = *mReverse[indicesRemove.first];
            auto& pageRefLast = *mReverse[indicesLast.first];

            std::swap(mDense[pageRefRemove[indicesRemove.second] - 1], mDense[pageRefLast[indicesLast.second] - 1]);
            pageRefLast[indicesLast.second] = pageRefRemove[indicesRemove.second];
            pageRefRemove[indicesRemove.second] = 0;

            mDense.pop_back();
        }
    };
}
