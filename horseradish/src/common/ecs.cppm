module;

#include <cassert>
#include <cstdint>

export module core:ecs;

import std;

export namespace hr
{
    class ECSRepository
    {
        template<class, class>
        struct TupleTypeConcat;

        template<class... First, class... Second>
        struct TupleTypeConcat<std::tuple<First...>, std::tuple<Second...>>
        {
            using type = std::tuple<First..., Second...>;
        };

        class FactoryComponentTypeId
        {
            static std::size_t identifier() noexcept
            {
                static std::size_t value{1};
                return value++;
            }

        public:
            template<typename>
            static std::size_t type() noexcept
            {
                static const std::size_t value = identifier();
                return value;
            }
        };

        class FactoryIndexTypeId
        {
            static std::size_t identifier() noexcept
            {
                static std::size_t value{1};
                return value++;
            }

        public:
            template<typename>
            static std::size_t type() noexcept
            {
                static const std::size_t value = identifier();
                return value;
            }
        };

        template<class TKey>
        class ComponentStorageBase
        {
        public:
            virtual ~ComponentStorageBase() = default;

            virtual void removeKey(const TKey& key) = 0;
        };

        template<class TKey, class TValue, class THash = std::hash<TKey>, size_t TPageSize = 4096>
        class ComponentStorage final: public ComponentStorageBase<TKey>
        {
            template<class... TComponents>
            friend class View;

            static_assert(TPageSize && ((TPageSize & (TPageSize - 1)) == 0), "size must be power of two");
            static_assert(sizeof(TKey) < TPageSize, "page size must be bigger than key size");
            static_assert(!std::is_empty_v<TKey>, "no point in storing an empty key");

            static constexpr size_t KeysPerPage = TPageSize / sizeof(TKey);

            std::vector<TKey> mDenseKeys;
            std::vector<TValue> mDenseValues;
            std::vector<std::unique_ptr<std::array<size_t, KeysPerPage>>> mReverse;
            TValue mEmptyValue{};

            static std::pair<size_t, size_t> indices(const TKey& key) noexcept
            {
                auto id = THash()(key);
                return {static_cast<size_t>(id / KeysPerPage), static_cast<size_t>(id & (KeysPerPage - 1))};
            }

            bool indexDense(const TKey& key, size_t& index) const noexcept
            {
                auto [page, offset] = indices(key);
                if ((page >= mReverse.size()) || !mReverse[page] || ((mReverse[page]->operator[](offset)) <= 0))
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
            ComponentStorage() = default;
            ComponentStorage(const ComponentStorage&) = delete;
            ComponentStorage& operator=(const ComponentStorage&) = delete;
            ComponentStorage(ComponentStorage&&) = default;
            ComponentStorage& operator=(ComponentStorage&&) = default;
            ~ComponentStorage() = default;

            void reserve(size_t newSize)
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                mDenseKeys.reserve(newSize);
                if constexpr (!std::is_empty_v<TValue>)
                    mDenseValues.reserve(newSize);
            }

            void shrinkToFit()
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if (mDenseKeys.empty())
                    mReverse.clear();

                mDenseKeys.shrink_to_fit();
                mReverse.shrink_to_fit();

                if constexpr (!std::is_empty_v<TValue>)
                    mDenseValues.shrink_to_fit();
            }

            size_t size() const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                return mDenseKeys.size();
            }

            bool empty() const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                return mDenseKeys.empty();
            }

            bool has(const TKey& key) const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                [[maybe_unused]] size_t index;
                return indexDense(key, index);
            }

            TValue& value(const TKey& key) noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (std::is_empty_v<TValue>)
                {
                    return mEmptyValue;
                }
                else
                {
                    size_t index;
                    if (!indexDense(key, index))
                        return mEmptyValue;

                    return mDenseValues[index];
                }
            }

            const TValue& value(const TKey& key) const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (std::is_empty_v<TValue>)
                {
                    return mEmptyValue;
                }
                else
                {
                    size_t index;
                    if (!indexDense(key, index))
                        return mEmptyValue;

                    return mDenseValues[index];
                }
            }

            template<class TCallback>
            void forEachKey(TCallback&& cb) const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                for (const auto& key : mDenseKeys)
                    cb(key);
            }

            template<class TCallback>
            void forEach(TCallback&& cb) noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (std::is_empty_v<TValue>)
                {
                    auto count = mDenseKeys.size();
                    auto walkerKeys = mDenseKeys.data();

                    for (; count > 0; --count, ++walkerKeys)
                        cb(static_cast<const TKey&>(*walkerKeys), TValue{});
                }
                else
                {
                    auto count = mDenseKeys.size();
                    auto walkerKeys = mDenseKeys.data();
                    auto walkerValues = mDenseValues.data();

                    for (; count > 0; --count, ++walkerKeys, ++walkerValues)
                        cb(static_cast<const TKey&>(*walkerKeys), *walkerValues);
                }
            }

            template<class TCallback>
            void forEach(TCallback&& cb) const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (std::is_empty_v<TValue>)
                {
                    auto count = mDenseKeys.size();
                    auto walkerKeys = mDenseKeys.data();

                    for (; count > 0; --count, ++walkerKeys)
                        cb(*walkerKeys, mEmptyValue);
                }
                else
                {
                    auto count = mDenseKeys.size();
                    auto walkerKeys = mDenseKeys.data();
                    auto walkerValues = mDenseValues.data();

                    for (; count > 0; --count, ++walkerKeys, ++walkerValues)
                        cb(*walkerKeys, *walkerValues);
                }
            }

            std::span<const TKey> rawKeys() const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                return {mDenseKeys.data(), mDenseKeys.size()};
            }

            std::span<TValue> raw() noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (std::is_empty_v<TValue>)
                    return {};
                else
                    return {mDenseValues.data(), mDenseValues.size()};
            }

            std::span<const TValue> raw() const noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (std::is_empty_v<TValue>)
                    return {};
                else
                    return {mDenseValues.data(), mDenseValues.size()};
            }

            void add(TKey key, TValue value)
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                auto [page, offset] = indices(key);

                assurePage(page);
                auto& pageRef = *mReverse[page];

                if (pageRef[offset] == 0)
                {
                    pageRef[offset] = mDenseKeys.size() + 1;
                    mDenseKeys.push_back(std::move(key));
                    if constexpr (!std::is_empty_v<TValue>)
                        mDenseValues.push_back(std::move(value));
                }
                else
                {
                    mDenseKeys[pageRef[offset] - 1] = std::move(key);
                    if constexpr (!std::is_empty_v<TValue>)
                        mDenseValues[pageRef[offset] - 1] = std::move(value);
                }
            }

            bool update(TKey key, TValue value) noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if constexpr (!std::is_empty_v<TValue>)
                {
                    return false;
                }
                else
                {
                    size_t index;
                    if (!indexDense(key, index))
                        return false;

                    mDenseValues[index] = std::move(value);
                    return true;
                }
            }

            void remove(const TKey& key) noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if (!has(key))
                    return;

                auto indicesRemove = indices(key);
                auto indicesLast = indices(mDenseKeys.back());

                auto& pageRefRemove = *mReverse[indicesRemove.first];
                auto& pageRefLast = *mReverse[indicesLast.first];

                std::swap(mDenseKeys[pageRefRemove[indicesRemove.second] - 1], mDenseKeys[pageRefLast[indicesLast.second] - 1]);
                if constexpr (!std::is_empty_v<TValue>)
                    std::swap(mDenseValues[pageRefRemove[indicesRemove.second] - 1], mDenseValues[pageRefLast[indicesLast.second] - 1]);

                pageRefLast[indicesLast.second] = pageRefRemove[indicesRemove.second];
                pageRefRemove[indicesRemove.second] = 0;

                mDenseKeys.pop_back();

                if constexpr (!std::is_empty_v<TValue>)
                    mDenseValues.pop_back();
            }

            bool changeKeyIndex(const TKey& key, size_t newIndex) noexcept
            {
                assert(std::is_empty_v<TValue> || (mDenseKeys.size() == mDenseValues.size()));

                if (!has(key) || (newIndex >= mDenseKeys.size()))
                    return false;

                auto indicesCurrent = indices(key);
                auto indicesNew = indices(mDenseKeys[newIndex]);
                if ((indicesCurrent.first == indicesNew.first) && (indicesCurrent.second == indicesNew.second))
                    return true;

                auto& pageRefCurrent = *mReverse[indicesCurrent.first];
                auto& pageRefNew = *mReverse[indicesNew.first];

                std::swap(mDenseKeys[pageRefCurrent[indicesCurrent.second] - 1], mDenseKeys[pageRefNew[indicesNew.second] - 1]);
                if constexpr (!std::is_empty_v<TValue>)
                    std::swap(mDenseValues[pageRefCurrent[indicesCurrent.second] - 1], mDenseValues[pageRefNew[indicesNew.second] - 1]);

                std::swap(pageRefNew[indicesNew.second], pageRefCurrent[indicesCurrent.second]);

                return true;
            }

            void removeKey(const TKey& key) override
            {
                remove(key);
            }
        };

        template<class TKey>
        class IndexBase
        {
        public:
            virtual ~IndexBase() = default;

            virtual bool processNewKey(const TKey& key) = 0;
            virtual bool processDeletedKey(const TKey& key) = 0;
        };

        template<class TKey, class TKeyHash, class TIndexA, class TComponentB>
        class Index: public IndexBase<TKey>
        {
            friend class Index;

            template<class TComponent>
            using StorageType = ComponentStorage<TKey, TComponent, TKeyHash>;

            std::vector<TKey> mKeys;
            TIndexA* mIndexBase{nullptr};
            IndexBase<TKey>* mIndexParent{nullptr};
            StorageType<TComponentB>* mStorageB{nullptr};

            template<class TCallback, std::size_t... TIdx>
            void forEach(TCallback&& cb, std::index_sequence<TIdx...>) noexcept
            {
                static_assert(std::tuple_size_v<decltype(mIndexBase->raw())> == sizeof...(TIdx));

                auto ptrsA = mIndexBase->raw(); // std::tuple<*...>
                auto ptrB = mStorageB->raw();

                auto count = mKeys.size();
                for (decltype(count) i = 0; i < count; ++i)
                {
                    cb(static_cast<const TKey&>(mKeys[i]), (*std::get<TIdx>(ptrsA))..., *ptrB);
                    ((++std::get<TIdx>(ptrsA)), ...);
                    ++ptrB;
                }
            }

            template<class TCallback, std::size_t... TIdx>
            void forEach(TCallback&& cb, std::index_sequence<TIdx...>) const noexcept
            {
                static_assert(std::tuple_size_v<decltype(mIndexBase->raw())> == sizeof...(TIdx));

                auto ptrsA = mIndexBase->raw(); // std::tuple<*...>
                auto ptrB = mStorageB->raw();

                auto count = mKeys.size();
                for (decltype(count) i = 0; i < count; ++i)
                {
                    cb(static_cast<const TKey&>(mKeys[i]), (*std::get<TIdx>(ptrsA))..., *ptrB);
                    ((++std::get<TIdx>(ptrsA)), ...);
                    ++ptrB;
                }
            }

        public:
            using TypeTuple = typename TupleTypeConcat<typename TIndexA::TypeTuple, std::tuple<TComponentB>>::type;

        public:
            Index(TIndexA* indexBase, StorageType<TComponentB>* storageB)
              : mIndexBase{indexBase}, mStorageB{storageB}
            {
                mIndexBase->mIndexParent = this;

                if (mIndexBase->size() < mStorageB->size())
                {
                    mIndexBase->forEachKey(
                      [this](const auto& key)
                      {
                          if (!mStorageB->has(key))
                              return;

                          mKeys.push_back(key);
                      });
                }
                else
                {
                    mStorageB->forEachKey(
                      [this](const auto& key)
                      {
                          if (!mIndexBase->has(key))
                              return;

                          mKeys.push_back(key);
                      });
                }

                size_t curIndex = 0;
                for (const auto& key : mKeys)
                {
                    mIndexBase->changeKeyIndex(key, curIndex);
                    mStorageB->changeKeyIndex(key, curIndex);
                    ++curIndex;
                }
            }

            ~Index()
            {
                mIndexBase->mIndexParent = nullptr;
            }

            size_t size() const noexcept
            {
                return mKeys.size();
            }

            bool has(const TKey& key) const noexcept
            {
                return (mIndexBase->has(key) && mStorageB->has(key));
            }

            bool hasParent() const noexcept
            {
                return mIndexParent;
            }

            template<class TCallback>
            void forEachKey(TCallback&& cb) const noexcept
            {
                for (const auto& key : mKeys)
                    cb(key);
            }

            template<class TCallback>
            void forEach(TCallback&& cb) noexcept
            {
                forEach(std::forward<TCallback>(cb), std::make_integer_sequence<size_t, std::tuple_size_v<TIndexA::TypeTuple>>{});
            }

            template<class TCallback>
            void forEach(TCallback&& cb) const noexcept
            {
                forEach(std::forward<TCallback>(cb), std::make_integer_sequence<size_t, std::tuple_size_v<TIndexA::TypeTuple>>{});
            }

            std::span<const TKey> rawKeys() const noexcept
            {
                return mKeys.data();
            }

            auto raw() noexcept
            {
                return std::tuple_cat(mIndexBase->raw(), std::make_tuple(mStorageB->raw()));
            }

            auto raw() const noexcept
            {
                return std::tuple_cat(mIndexBase->raw(), std::make_tuple(mStorageB->raw()));
            }

            bool changeKeyIndex(const TKey& key, size_t newIndex) noexcept
            {
                assert(has(key) && (newIndex < mKeys.size()));

                if (!has(key) || (newIndex >= mKeys.size()))
                    return false;

                if (!mIndexBase->changeKeyIndex(key, newIndex))
                    return false;

                if (!mStorageB->changeKeyIndex(key, newIndex))
                    return false;

                auto it = std::find(std::begin(mKeys), std::end(mKeys), key);
                assert(it != mKeys.end());
                std::swap(*it, mKeys[newIndex]);

                return true;
            }

            bool processNewKey(const TKey& key) override
            {
                if (!has(key))
                    return false;

                mKeys.push_back(key);
                mIndexBase->changeKeyIndex(key, mKeys.size() - 1);
                mStorageB->changeKeyIndex(key, mKeys.size() - 1);

                if (mIndexParent)
                    return mIndexParent->processNewKey(key);

                return true;
            }

            bool processDeletedKey(const TKey& key) override
            {
                if (mIndexParent && mIndexParent->processDeletedKey(key)) // the change index request must come from the top most parent with the index
                    return true;

                if (!has(key))
                    return false;

                if (mKeys.back() != key)
                    changeKeyIndex(key, mKeys.size() - 1);

                mKeys.pop_back();

                return true;
            }
        };

        template<class TKey, class TKeyHash, class TComponent>
        class Index<TKey, TKeyHash, TComponent, void>: public IndexBase<TKey>
        {
            friend class Index;

            template<class TComponent>
            using StorageType = ComponentStorage<TKey, TComponent, TKeyHash>;

            IndexBase<TKey>* mIndexParent{nullptr};
            StorageType<TComponent>* mStorage{nullptr};

        public:
            using TypeTuple = std::tuple<TComponent>;

        public:
            Index(StorageType<TComponent>* storage)
              : mStorage{storage}
            {}

            size_t size() const noexcept
            {
                return mStorage->size();
            }

            bool has(const TKey& key) const noexcept
            {
                return mStorage->has(key);
            }

            bool hasParent() const noexcept
            {
                return mIndexParent;
            }

            template<class TCallback>
            void forEachKey(TCallback&& cb) const noexcept
            {
                mStorage->forEachKey(std::forward<TCallback>(cb));
            }

            template<class TCallback>
            void forEach(TCallback&& cb) noexcept
            {
                mStorage->forEach(std::forward<TCallback>(cb));
            }

            template<class TCallback>
            void forEach(TCallback&& cb) const noexcept
            {
                mStorage->forEach(std::forward<TCallback>(cb));
            }

            std::span<const TKey> rawKeys() const noexcept
            {
                return mStorage->rawKeys();
            }

            std::tuple<TComponent*> raw() noexcept
            {
                return {mStorage->raw()};
            }

            std::tuple<const TComponent*> raw() const noexcept
            {
                return {mStorage->raw()};
            }

            bool changeKeyIndex(const TKey& key, size_t curIndex) noexcept
            {
                return mStorage->changeKeyIndex(key, curIndex);
            }

            bool processNewKey(const TKey& key) override
            {
                // the parent index will take care of everything
                if (mIndexParent)
                    return mIndexParent->processNewKey(key);

                return true;
            }

            bool processDeletedKey(const TKey& key) override
            {
                if (mIndexParent)
                    mIndexParent->processDeletedKey(key); // the change index request must come from the top most parent with the index

                return true;
            }
        };

        template<class TKey, class TKeyHash, class TBaseIndex, class TComponent = void, class... TComponents>
        struct IndexPathImpl
        {
            using Type = typename IndexPathImpl<TKey, TKeyHash, Index<TKey, TKeyHash, TBaseIndex, TComponent>, TComponents...>::Type;
        };

        template<class TKey, class TKeyHash, class TBaseIndex>
        struct IndexPathImpl<TKey, TKeyHash, TBaseIndex, void>
        {
            using Type = TBaseIndex;
        };

        template<class TKey, class TKeyHash, class TFirst, class... TOthers>
        struct IndexPath
        {
            using Type = typename IndexPathImpl<TKey, TKeyHash, Index<TKey, TKeyHash, TFirst, void>, TOthers...>::Type;
        };

    public:
        class EntityId
        {
        public:
            struct Hash
            {
                size_t operator()(EntityId enttId) const noexcept
                {
                    return static_cast<size_t>(enttId.id());
                }
            };

        public:
            constexpr EntityId() = default;
            ~EntityId() noexcept = default;

            constexpr EntityId(const EntityId&) = default;
            constexpr EntityId& operator=(const EntityId&) = default;
            constexpr EntityId(EntityId&&) = default;
            constexpr EntityId& operator=(EntityId&&) = default;

            constexpr explicit EntityId(uint32_t id, uint32_t version) noexcept
              : m_data{((version & 0xFFF) << 20) | (id & 0xFFFFF)}
            {}

            constexpr explicit operator bool() const noexcept
            {
                return (m_data != 0);
            }

            bool operator==(const EntityId& rhs) const noexcept
            {
                return (m_data == rhs.m_data);
            }

            bool operator!=(const EntityId& rhs) const noexcept
            {
                return !operator==(rhs);
            }

            constexpr uint32_t id() const noexcept
            {
                return static_cast<uint32_t>(m_data & 0xFFFFF);
            }

            constexpr uint32_t version() const noexcept
            {
                return static_cast<uint32_t>((m_data >> 20) & 0xFFF);
            }

        private:
            uint32_t m_data{0};
        };

        template<class... TComponents>
        class View
        {
            friend class ECSRepository;

            template<class TComponent>
            using StorageType = ComponentStorage<EntityId, TComponent, EntityId::Hash>;

            const std::tuple<ComponentStorage<EntityId, TComponents, EntityId::Hash>*...> mPools;

            const std::vector<EntityId>* candidates() const noexcept
            {
                return std::min({(&(std::get<StorageType<TComponents>*>(mPools)->mDenseKeys))...}, [](const auto lhs, const auto rhs) { return (lhs->size() < rhs->size()); });
            }

        private:
            View(StorageType<TComponents>*... data)
              : mPools{data...}
            {}

        public:
            class Iterator
            {
                const View& mView;
                size_t mOffset{0};
                const std::vector<EntityId>* mCandidates{nullptr};

            public:
                // iterator traits
                using iterator_category = std::forward_iterator_tag;
                using value_type = std::tuple<EntityId, std::tuple<const TComponents&...>>;
                using difference_type = ptrdiff_t;
                using pointer = const value_type*;
                using reference = const value_type&;

            private:
                void moveToNextValid() noexcept
                {
                    for (; mOffset < mCandidates->size(); ++mOffset)
                    {
                        const auto& entt = mCandidates->operator[](mOffset);
                        if ((std::get<StorageType<TComponents>*>(mView.mPools)->has(entt) && ...))
                            break;
                    }
                }

            public:
                explicit constexpr Iterator(const View& view) noexcept
                  : mView{view}
                {
                    mCandidates = mView.candidates();
                    mOffset = mCandidates->size();
                }

                explicit constexpr Iterator(const View& view, size_t offset) noexcept
                  : mView{view}, mOffset{offset}
                {
                    mCandidates = mView.candidates();
                    assert(mCandidates && (mOffset < mCandidates->size()));

                    moveToNextValid();
                }

                bool operator==(const Iterator& other) const noexcept
                {
                    return (mOffset == other.mOffset);
                }

                bool operator!=(const Iterator& other) const noexcept
                {
                    return (mOffset != other.mOffset);
                }

                Iterator& operator++()
                {
                    ++mOffset;
                    moveToNextValid();
                    return *this;
                }

                Iterator operator++(int)
                {
                    Iterator tmp(*this);
                    operator++();
                    return tmp;
                }

                std::tuple<EntityId, std::tuple<const TComponents&...>> operator*() const noexcept
                {
                    const auto& entt = mCandidates->operator[](mOffset);
                    return {entt, {std::get<StorageType<TComponents>*>(mView.mPools)->value(entt)...}};
                };
            };

        public:
            bool has(const EntityId& enttId) const noexcept
            {
                return ((std::get<StorageType<TComponents>*>(mPools)->has(enttId) && ...));
            }

            template<class TCallback>
            void forEach(TCallback&& cb) const noexcept
            {
                auto entities = candidates();

                for (const auto& entt : *entities)
                {
                    if ((std::get<StorageType<TComponents>*>(mPools)->has(entt) && ...))
                    {
                        cb(entt, std::get<StorageType<TComponents>*>(mPools)->value(entt)...);
                    }
                }
            }

            using const_iterator = Iterator;

            const_iterator begin() const noexcept
            {
                return Iterator(*this, 0);
            }

            const_iterator end() const noexcept
            {
                return Iterator(*this);
            }
        };

        template<class TComponent>
        class View<TComponent>
        {
            friend class ECSRepository;

            ComponentStorage<EntityId, TComponent, EntityId::Hash>* mData;

        private:
            View(ComponentStorage<EntityId, TComponent, EntityId::Hash>* data)
              : mData{data}
            {}

        public:
            class Iterator
            {
                const EntityId* mWalkerKeys;
                const TComponent* mWalkerData;

            public:
                // iterator traits
                using iterator_category = std::forward_iterator_tag;
                using value_type = std::tuple<EntityId, const TComponent&>;
                using difference_type = ptrdiff_t;
                using pointer = const value_type*;
                using reference = const value_type&;

            public:
                explicit constexpr Iterator(const View& view) noexcept
                {
                    auto count = view.mData->size();
                    mWalkerKeys = view.mData->rawKeys() + count;
                    mWalkerData = view.mData->raw() + count;
                }

                explicit constexpr Iterator(const View& view, size_t offset) noexcept
                {
                    auto count = view.mData->size();
                    assert(offset < count);
                    if (offset > count)
                        offset = count;

                    mWalkerKeys = view.mData->rawKeys() + offset;
                    mWalkerData = view.mData->raw() + offset;
                }

                bool operator==(const Iterator& other) const noexcept
                {
                    return (mWalkerKeys == other.mWalkerKeys);
                }

                bool operator!=(const Iterator& other) const noexcept
                {
                    return (mWalkerKeys != other.mWalkerKeys);
                }

                Iterator& operator++()
                {
                    ++mWalkerKeys;
                    ++mWalkerData;
                    return *this;
                }

                Iterator operator++(int)
                {
                    Iterator tmp(*this);
                    operator++();
                    return tmp;
                }

                std::tuple<EntityId, const TComponent&> operator*() const noexcept
                {
                    return {*mWalkerKeys, *mWalkerData};
                };
            };

        public:
            size_t size() const noexcept
            {
                return mData->size();
            }

            bool has(const EntityId& enttId) const noexcept
            {
                return mData->has(enttId);
            }

            const TComponent& value(const EntityId& enttId) const noexcept
            {
                return mData->value(enttId);
            }

            template<class TCallback>
            void forEachKey(TCallback&& cb) const noexcept
            {
                mData->forEachKey(std::forward<TCallback>(cb));
            }

            template<class TCallback>
            void forEach(TCallback&& cb) const noexcept
            {
                mData->forEach(std::forward<TCallback>(cb));
            }

            const TComponent* raw() const noexcept
            {
                return mData->raw();
            }

            using const_iterator = Iterator;

            const_iterator begin() const noexcept
            {
                return Iterator(*this, 0);
            }

            const_iterator end() const noexcept
            {
                return Iterator(*this);
            }
        };

        template<class... TComponents>
        class ViewIndexed
        {
            friend class ECSRepository;

            using Index = typename IndexPath<EntityId, EntityId::Hash, TComponents...>::Type;

            Index* mIndex;

        private:
            ViewIndexed(Index* index)
              : mIndex{index}
            {}

        public:
            class Iterator
            {
                size_t mOffset{0};
                const EntityId* mWalkerKeys{nullptr};
                std::tuple<const TComponents*...> mWalkerDatas;

            public:
                // iterator traits
                using iterator_category = std::forward_iterator_tag;
                using value_type = std::tuple<EntityId, std::tuple<const TComponents&...>>;
                using difference_type = ptrdiff_t;
                using pointer = const value_type*;
                using reference = const value_type&;

            private:
                template<class... TComponents>
                static std::tuple<const TComponents&...> getTupleRefs(std::tuple<const TComponents*...> tuplePointers, size_t index)
                {
                    return {static_cast<const TComponents&>(std::get<const TComponents*>(tuplePointers)[index])...};
                }

            public:
                explicit constexpr Iterator(const ViewIndexed& view) noexcept
                  : mWalkerKeys{view.mIndex->rawKeys()}, mWalkerDatas{view.mIndex->raw()}
                {
                    mOffset = view.mIndex->size();
                }

                explicit constexpr Iterator(const ViewIndexed& view, size_t offset) noexcept
                  : mOffset{offset}, mWalkerKeys{view.mIndex->rawKeys()}, mWalkerDatas{view.mIndex->raw()}
                {
                    auto count = view.mIndex->size();
                    assert(mOffset < count);
                    if (mOffset > count)
                        mOffset = count;
                }

                bool operator==(const Iterator& other) const noexcept
                {
                    return (mOffset == other.mOffset);
                }

                bool operator!=(const Iterator& other) const noexcept
                {
                    return (mOffset != other.mOffset);
                }

                Iterator& operator++()
                {
                    ++mOffset;
                    return *this;
                }

                Iterator operator++(int)
                {
                    Iterator tmp(*this);
                    operator++();
                    return tmp;
                }

                std::tuple<EntityId, std::tuple<const TComponents&...>> operator*() const noexcept
                {
                    return {mWalkerKeys[mOffset], Iterator::getTupleRefs(mWalkerDatas, mOffset)};
                };
            };

        public:
            size_t size() const noexcept
            {
                return (mIndex ? mIndex->size() : 0);
            }

            bool has(const EntityId& enttId) const noexcept
            {
                return (mIndex ? mIndex->has(enttId) : false);
            }

            template<class TCallback>
            void forEachKey(TCallback&& cb) const noexcept
            {
                if (!mIndex)
                    return;

                mIndex->forEachKey(std::forward<TCallback>(cb));
            }

            template<class TCallback>
            void forEach(TCallback&& cb) const noexcept
            {
                if (!mIndex)
                    return;

                mIndex->forEach(std::forward<TCallback>(cb));
            }

            auto raw() const noexcept
            {
                return mIndex->raw();
            }

            using const_iterator = Iterator;

            const_iterator begin() const noexcept
            {
                return Iterator(*this, 0);
            }

            const_iterator end() const noexcept
            {
                return Iterator(*this);
            }
        };

    private:
        struct ComponentData final
        {
            size_t id{0};
            size_t indexOwnerId{0};
            IndexBase<EntityId>* indexOwner{nullptr};
            std::unique_ptr<ComponentStorageBase<EntityId>> storage;
        };

        struct IndexData final
        {
            size_t id{0};
            std::unique_ptr<IndexBase<EntityId>> index;
        };

    private:
        template<class TComponent>
        ComponentStorage<EntityId, TComponent, EntityId::Hash>& assureComponent()
        {
            auto cId = FactoryComponentTypeId::type<TComponent>() - 1;
            if (cId >= mComponents.size())
            {
                mComponents.resize(cId + 1);
                mComponents[cId].id = cId + 1;
                mComponents[cId].storage = std::make_unique<ComponentStorage<EntityId, TComponent, EntityId::Hash>>();
            }

            assert(std::all_of(mComponents.begin(), mComponents.end(), [](const auto& component) { return (component.id > 0); }));

            return static_cast<ComponentStorage<EntityId, TComponent, EntityId::Hash>&>(*mComponents[cId].storage);
        }

        template<class TComponent>
        ComponentData& assureComponentData() noexcept
        {
            assureComponent<TComponent>();

            auto cId = FactoryComponentTypeId::type<TComponent>() - 1;

            assert(cId < mComponents.size());
            return mComponents[cId];
        }

        template<class TComponent>
        ComponentStorage<EntityId, TComponent, EntityId::Hash>* retrieveComponent() const noexcept
        {
            auto cId = FactoryComponentTypeId::type<TComponent>() - 1;
            if ((cId >= mComponents.size()) || (mComponents[cId].id <= 0))
                return nullptr;

            return static_cast<ComponentStorage<EntityId, TComponent, EntityId::Hash>*>(mComponents[cId].storage.get());
        }

        template<class TIndexA, class TComponentB, class... TComponents>
        bool index(TIndexA* indexA)
        {
            // force creation if it doesn't exist
            auto& cStorageB = assureComponentData<TComponentB>();

            // our target index id
            auto iId = FactoryIndexTypeId::type<TupleTypeConcat<TIndexA::TypeTuple, std::tuple<TComponentB>>::type>() - 1;

            // if the index is already owned by someone else
            if (cStorageB.indexOwner && ((cStorageB.indexOwnerId - 1) != iId))
                return false;

            // if we don't have a corresponding index, create one
            if (!cStorageB.indexOwner)
            {
                assert((iId >= mIndices.size()) || !mIndices[iId].index);

                std::unique_ptr<Index<EntityId, EntityId::Hash, TIndexA, TComponentB>> newIndex;
                {
                    auto storageB = static_cast<ComponentStorage<EntityId, TComponentB, EntityId::Hash>*>(cStorageB.storage.get());
                    newIndex = std::make_unique<Index<EntityId, EntityId::Hash, TIndexA, TComponentB>>(indexA, storageB);
                }

                cStorageB.indexOwner = newIndex.get();
                cStorageB.indexOwnerId = iId + 1;

                mIndices.resize(iId + 1);
                mIndices[iId].id = iId + 1;
                mIndices[iId].index = std::move(newIndex);
            }

            // build the rest of the index
            if constexpr (sizeof...(TComponents) > 0)
            {
                auto curIndex = reinterpret_cast<Index<EntityId, EntityId::Hash, TIndexA, TComponentB>*>(mIndices[iId].index.get());
                return index<Index<EntityId, EntityId::Hash, TIndexA, TComponentB>, TComponents...>(curIndex);
            }

            return true;
        }

    private:
        struct
        {
            std::vector<EntityId> ids;
            std::size_t numDeleted{0};
            EntityId nextDeleted{0, 0};
        } mEntities;

        std::vector<ComponentData> mComponents;
        std::vector<IndexData> mIndices;

    public:
        ECSRepository() = default;
        ECSRepository(const ECSRepository&) = delete;
        ECSRepository& operator=(const ECSRepository&) = delete;
        ECSRepository(ECSRepository&&) = default;
        ECSRepository& operator=(ECSRepository&&) = default;
        ~ECSRepository() = default;

        EntityId gen()
        {
            if (mEntities.numDeleted <= 0)
            {
                EntityId enttId{static_cast<uint32_t>( mEntities.ids.size() + 1), 0};
                mEntities.ids.push_back(enttId);

                return enttId;
            }

            mEntities.numDeleted--;

            auto enttId = mEntities.nextDeleted;
            std::swap(mEntities.nextDeleted, mEntities.ids[mEntities.nextDeleted.id() - 1]);

            return enttId;
        }

        void destroy(EntityId enttId) noexcept
        {
            auto index = enttId.id() - 1;
            if (!enttId || (index >= mEntities.ids.size()))
                return;

            // remove from storage
            remove(enttId);

            // recycle the entity
            {
                auto& oldEnttId = mEntities.ids[index];
                oldEnttId = EntityId{oldEnttId.id(), oldEnttId.version() + 1};

                std::swap(oldEnttId, mEntities.nextDeleted);

                mEntities.numDeleted++;
            }
        }

        void reserve(size_t numEntities)
        {
            mEntities.ids.reserve(numEntities);
        }

        bool isValid(EntityId enttId) const noexcept
        {
            if (!enttId)
                return false;

            auto index = enttId.id() - 1;
            return ((index < mEntities.ids.size()) && (enttId.id() == mEntities.ids[index].id()) && (enttId.version() == mEntities.ids[index].version()));
        }

        template<class TComponent, class... TComponentArgs>
        void assign(EntityId enttId, TComponentArgs&&... componentArgs)
        {
            // add data
            {
                auto& cStorage = assureComponent<TComponent>();

                if constexpr (sizeof...(componentArgs) <= 0)
                    cStorage.add(enttId, TComponent{});
                else
                    cStorage.add(enttId, TComponent{std::forward<TComponentArgs>(componentArgs)...});
            }

            // updates indices
            {
                auto& component = assureComponentData<TComponent>();
                if (component.indexOwner)
                    component.indexOwner->processNewKey(enttId);
            }
        }

        template<class TComponent>
        void update(EntityId enttId, TComponent&& value)
        {
            auto cStorage = retrieveComponent<TComponent>();
            if (cStorage)
                cStorage->update(enttId, std::forward<TComponent>(value));
        }

        template<class TComponent>
        void reserve(size_t numValues)
        {
            auto cStorage = retrieveComponent<TComponent>();
            if (cStorage)
                cStorage->reserve(numValues);
        }

        void remove(EntityId enttId) noexcept
        {
            for (auto& component : mComponents)
            {
                if (component.indexOwner)
                    component.indexOwner->processDeletedKey(enttId);

                component.storage->removeKey(enttId);
            }
        }

        template<class TComponent>
        void remove(EntityId enttId) noexcept
        {
            auto& component = assureComponentData<TComponent>();

            if (component.indexOwner)
                component.indexOwner->processDeletedKey(enttId);

            component.storage->removeKey(enttId);
        }

        template<class TComponent>
        bool has() const noexcept
        {
            auto cStorage = retrieveComponent<TComponent>();
            return cStorage;
        }

        template<class TComponent>
        bool has(EntityId enttId) const noexcept
        {
            auto cStorage = retrieveComponent<TComponent>();
            return (cStorage && cStorage->has(enttId));
        }

        template<class TComponent, class TCallback>
        void forEach(TCallback&& cb) const noexcept
        {
            auto cStorage = retrieveComponent<TComponent>();
            if (!cStorage)
                return;

            cStorage->forEach(cb);
        }

        template<class TComponentA, class... TComponents>
        bool index()
        {
            // force creation if it doesn't exist
            auto& cStorageA = assureComponentData<TComponentA>();

            // our target index id
            auto iId = FactoryIndexTypeId::type<std::tuple<TComponentA>>() - 1;

            // if the index is already owned by someone else
            if (cStorageA.indexOwner && ((cStorageA.indexOwnerId - 1) != iId))
                return false;

            // if we don't have a corresponding index, create one
            if (!cStorageA.indexOwner)
            {
                assert((iId >= mIndices.size()) || !mIndices[iId].index);

                // the first index is actually just a proxy for the storage

                std::unique_ptr<Index<EntityId, EntityId::Hash, TComponentA, void>> newIndex;
                {
                    auto storageA = static_cast<ComponentStorage<EntityId, TComponentA, EntityId::Hash>*>(cStorageA.storage.get());
                    newIndex = std::make_unique<Index<EntityId, EntityId::Hash, TComponentA, void>>(storageA);
                }

                cStorageA.indexOwner = newIndex.get();
                cStorageA.indexOwnerId = iId + 1;

                mIndices.resize(iId + 1);
                mIndices[iId].id = iId + 1;
                mIndices[iId].index = std::move(newIndex);
            }

            // build the rest of the index
            if constexpr (sizeof...(TComponents) > 0)
            {
                auto curIndex = reinterpret_cast<Index<EntityId, EntityId::Hash, TComponentA, void>*>(mIndices[iId].index.get());
                if (!index<Index<EntityId, EntityId::Hash, TComponentA, void>, TComponents...>(curIndex))
                    return false;
            }

            return true;
        }

        template<class... TComponents>
        bool indexRemove()
        {
            static_assert(sizeof...(TComponents) >= 1, "Indices must have at least one component");

            auto iId = FactoryIndexTypeId::type<std::tuple<TComponents...>>() - 1;
            if ((iId >= mIndices.size()) || !mIndices[iId].index)
                return true; // already removed

            auto index = reinterpret_cast<typename IndexPath<EntityId, EntityId::Hash, TComponents...>::Type*>(mIndices[iId].index.get());
            if (index->hasParent())
                return false; // can't destroy if it's in use

            for (auto& component : mComponents)
            {
                if (component.indexOwner != index)
                    continue;

                component.indexOwnerId = 0;
                component.indexOwner = nullptr;
                break;
            }

            mIndices[iId].id = 0;
            mIndices[iId].index = nullptr; // will destroy index

            return true;
        }

        template<class... TComponents>
        View<TComponents...> view()
        {
            static_assert(sizeof...(TComponents) >= 1, "Views must have at least one component");

            return {&assureComponent<TComponents>()...};
        }

        template<class... TComponents>
        ViewIndexed<TComponents...> viewIndexed() const noexcept
        {
            static_assert(sizeof...(TComponents) >= 1, "Indexed views must have at least one component");

            auto iId = FactoryIndexTypeId::type<std::tuple<TComponents...>>() - 1;
            if ((iId >= mIndices.size()) || !mIndices[iId].index)
                return {nullptr};

            return {reinterpret_cast<typename IndexPath<EntityId, EntityId::Hash, TComponents...>::Type*>(mIndices[iId].index.get())};
        }
    };
}
