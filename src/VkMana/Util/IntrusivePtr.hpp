#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

namespace VkMana
{
	class SingleThreadedCounter
	{
	public:
		inline void AddRef() { m_count++; }
		inline bool Release() { return --m_count == 0; }

	private:
		uint32_t m_count = 1;
	};

	class MultiThreadedCounter
	{
	public:
		MultiThreadedCounter() { m_count.store(1, std::memory_order_relaxed); }

		inline void AddRef() { m_count.fetch_add(1, std::memory_order_relaxed); }
		inline bool Release()
		{
			auto result = m_count.fetch_sub(1, std::memory_order_acq_rel);
			return result == 0;
		}

	private:
		std::atomic_uint32_t m_count;
	};

	template <typename T>
	class IntrusivePtr;

	template <typename T, typename Deleter = std::default_delete<T>, typename ReferenceOps = SingleThreadedCounter>
	class IntrusivePtrEnabled
	{
	public:
		using IntrusivePtrType = IntrusivePtr<T>;
		using EnabledBase = T;
		using EnabledDeleter = Deleter;
		using EnabledReferenceOps = ReferenceOps;

		IntrusivePtrEnabled() = default;
		IntrusivePtrEnabled(const IntrusivePtrEnabled&) = delete;
		void operator=(const IntrusivePtrEnabled&) = delete;

		void ReleaseReference()
		{
			if (m_refCounter.Release())
				Deleter()(static_cast<T*>(this));
		}

		void AddReference() { m_refCounter.AddRef(); }

	protected:
		auto ReferenceFromThis() -> IntrusivePtr<T>;

	private:
		ReferenceOps m_refCounter;
	};

	template <typename T>
	class IntrusivePtr
	{
	public:
		template <typename U>
		friend class IntrusivePtr;

		IntrusivePtr() = default;
		IntrusivePtr(std::nullptr_t)
			: m_data(nullptr)
		{
		}
		explicit IntrusivePtr(T* handle)
			: m_data(handle)
		{
		}

		IntrusivePtr(const IntrusivePtr& other) { *this = other; }

		template <typename U>
		explicit IntrusivePtr(const IntrusivePtr<U>& other)
		{
			*this = other;
		}

		template <typename U>
		explicit IntrusivePtr(IntrusivePtr<U>&& other) noexcept
		{
			*this = std::move(other);
		}

		IntrusivePtr(IntrusivePtr&& other) noexcept { *this = std::move(other); }

		~IntrusivePtr() { Reset(); }

		auto Get() -> T* { return m_data; }
		auto Get() const -> const T* { return m_data; }

		void Reset()
		{
			using ReferenceBase = IntrusivePtrEnabled<typename T::EnabledBase, typename T::EnabledDeleter, typename T::EnabledReferenceOps>;

			// Static up-cast here to avoid potential issues with multiple intrusive inheritance.
			// Also makes sure that the pointer type actually inherits from this type.
			if (m_data)
				static_cast<ReferenceBase*>(m_data)->ReleaseReference();
			m_data = nullptr;
		}

		auto Release() & -> T*
		{
			T* ret = m_data;
			m_data = nullptr;
			return ret;
		}

		auto Release() && -> T*
		{
			T* ret = m_data;
			m_data = nullptr;
			return ret;
		}

		auto operator*() -> T& { return *m_data; }
		auto operator*() const -> const T& { return *m_data; }
		auto operator->() -> T* { return m_data; }
		auto operator->() const -> const T* { return m_data; }
		explicit operator bool() const { return m_data != nullptr; }
		bool operator==(const IntrusivePtr& other) const { return m_data == other.m_data; }
		bool operator!=(const IntrusivePtr& other) const { return *this != other; }

		template <typename U>
		auto operator=(const IntrusivePtr<U>& other) -> IntrusivePtr&
		{
			static_assert(std::is_base_of<T, U>::value, "Cannot safely assign down-casted intrusive pointers.");

			using ReferenceBase = IntrusivePtrEnabled<typename T::EnabledBase, typename T::EnabledDeleter, typename T::EnabledReferenceOps>;

			Reset();
			m_data = static_cast<T*>(other.m_data);

			// Static up-cast here to avoid potential issues with multiple intrusive inheritance.
			// Also makes sure that the pointer type actually inherits from this type.
			if (m_data)
				static_cast<ReferenceBase*>(m_data)->AddReference();
			m_data = nullptr;
		}

		auto operator=(const IntrusivePtr& other) -> IntrusivePtr&
		{
			using ReferenceBase = IntrusivePtrEnabled<typename T::EnabledBase, typename T::EnabledDeleter, typename T::EnabledReferenceOps>;

			if (this != &other)
			{
				Reset();
				m_data = other.m_data;

				if (m_data)
					static_cast<ReferenceBase*>(m_data)->AddReference();
			}
			return *this;
		}

		template <typename U>
		auto operator=(IntrusivePtr<U>&& other) noexcept -> IntrusivePtr&
		{
			Reset();
			m_data = other.m_data;
			other.m_data = nullptr;
			return *this;
		}

		auto operator=(IntrusivePtr&& other) noexcept -> IntrusivePtr&
		{
			if (this != &other)
			{
				Reset();
				m_data = other.m_data;
				other.m_data = nullptr;
			}
			return *this;
		}

	private:
		T* m_data = nullptr;
	};

	template <typename T, typename Deleter, typename ReferenceOps>
	auto IntrusivePtrEnabled<T, Deleter, ReferenceOps>::ReferenceFromThis() -> IntrusivePtr<T>
	{
		AddReference();
		return IntrusivePtr<T>(static_cast<T*>(this));
	}

	template <typename T>
	using ThreadSafeIntrusivePtrEnabled = IntrusivePtrEnabled<T, std::default_delete<T>, MultiThreadedCounter>;

} // namespace VkMana
