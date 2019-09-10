#pragma once

#include "sparseMap.hpp"

#include <vector>
#include <type_traits>

namespace hr
{
	class ECSRepository
	{
		class FactoryTypeId
		{
			static std::size_t identifier() noexcept
			{
				static std::size_t value{ 1 };
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

		class ComponentStorageBase
		{
		public:
			virtual ~ComponentStorageBase() = default;
		};

		template<class TKey, class TValue, class THash = std::hash<TKey>, size_t TPageSize = 4096>
		class ComponentStorage final : public ComponentStorageBase
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

			template<class TCallback>
			void forEach(TCallback&& cb) noexcept
			{
				assert(mDenseKeys.size() == mDenseValues.size());

				auto count = mDenseKeys.size();
				for (decltype(count) i = 0; i < count; i++)
					cb(static_cast<const TKey&>(mDenseKeys[i]), mDenseValues[i]);
			}

			template<class TCallback>
			void forEach(TCallback&& cb) const noexcept
			{
				assert(mDenseKeys.size() == mDenseValues.size());

				auto count = mDenseKeys.size();
				for (decltype(count) i = 0; i < count; i++)
					cb(mDenseKeys[i], mDenseValues[i]);
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

			void remove(const TKey& key) noexcept
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

		struct ComponentData final
		{
			size_t id{ 0 };
			std::unique_ptr<ComponentStorageBase> storage;
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
			constexpr EntityId(const EntityId&) = default;
			constexpr EntityId& operator=(const EntityId&) = default;
			constexpr EntityId(EntityId&&) = default;
			constexpr EntityId& operator=(EntityId&&) = default;
			~EntityId() = default;

			constexpr explicit EntityId(std::uint32_t id, std::uint32_t version) noexcept
				: m_data{ ((version & 0xFFF) << 20) | (id & 0xFFFFF) }
			{ }

			constexpr explicit operator bool() const noexcept
			{
				return (m_data != 0);
			}

			constexpr std::uint32_t id() const noexcept
			{
				return static_cast<std::uint32_t>(m_data & 0xFFFFF);
			}

			constexpr std::uint32_t version() const noexcept
			{
				return static_cast<std::uint32_t>((m_data >> 20) & 0xFFF);
			}

		private:
			std::uint32_t m_data{ 0 };
		};

		template<class... TComponents>
		class View
		{
			friend class ECSRepository;

			template<class TComponent>
			using StorageType = ComponentStorage<EntityId, TComponent, EntityId::Hash>;

			const std::tuple<ComponentStorage<EntityId, TComponents, EntityId::Hash>*...> mPools;

		private:
			View(StorageType<TComponents>*... data)
				: mPools{ data... }
			{ }

		public:
			size_t size() const noexcept
			{
				return std::tuple_size<std::tuple<ComponentStorage<EntityId, TComponents, EntityId::Hash>*...>>::value;
			}

			template<class TComponent>
			size_t size() const noexcept
			{
				return std::get<StorageType<TComponent>*>(mPools)->size();
			}
		};

	private:

		template<class TComponent>
		ComponentStorage<EntityId, TComponent, EntityId::Hash>& assureComponent()
		{
			auto cId = FactoryTypeId::type<TComponent>() - 1;
			if (cId >= mComponents.size())
			{
				mComponents.resize(cId + 1);
				mComponents[cId].id = cId + 1;
				mComponents[cId].storage = std::make_unique<ComponentStorage<EntityId, TComponent, EntityId::Hash>>();
			}

			return static_cast<ComponentStorage<EntityId, TComponent, EntityId::Hash>&>(*mComponents[cId].storage);
		}

		template<class TComponent>
		ComponentStorage<EntityId, TComponent, EntityId::Hash>* retrieveComponent() const noexcept
		{
			auto cId = FactoryTypeId::type<TComponent>() - 1;
			if ((cId >= mComponents.size()) || (mComponents[cId].id <= 0))
				return nullptr;

			return static_cast<ComponentStorage<EntityId, TComponent, EntityId::Hash>*>(mComponents[cId].storage.get());
		}

	private:
		struct {
			std::vector<EntityId> ids;
			std::size_t numDeleted{ 0 };
			EntityId nextDeleted{ 0, 0 };
		} mEntities;

		std::vector<ComponentData> mComponents;

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
				EntityId enttId{ mEntities.ids.size() + 1, 0 };
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

			auto& oldEnttId = mEntities.ids[index];
			oldEnttId = EntityId{ oldEnttId.id(), oldEnttId.version() + 1 };

			std::swap(oldEnttId, mEntities.nextDeleted);

			mEntities.numDeleted++;
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
			auto& cStorage = assureComponent<TComponent>();

			if constexpr (sizeof...(componentArgs) <= 0)
				cStorage.add(enttId, TComponent{});
			else
				cStorage.add(enttId, TComponent{ std::forward<TComponentArgs>(componentArgs)... });
		}

		template<class TComponent>
		void reserve(size_t numValues)
		{
			auto cStorage = retrieveComponent<TComponent>();
			if (cStorage)
				cStorage->reserve(numValues);
		}

		template<class TComponent>
		void remove(EntityId enttId) noexcept
		{
			auto cStorage = retrieveComponent<TComponent>();
			if (cStorage)
				cStorage->remove(enttId);
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

		template<class... TComponents>
		View<TComponents...> view() noexcept
		{
			return { &assureComponent<TComponents>()... };
		}

	};
}