#pragma once
#include <cstdint>
#include <atomic>
#include <unordered_map>

#define snew(Type) new(Shado::Memory::Heap<Type>())
#define sdelete(ptr) Shado::Memory::Free(ptr)

extern "C" {
	double glfwGetTime();
}

namespace Shado {

	class Memory {
	public:
		template<typename T>
		inline static T* Heap(const char* label = "Engine") {
			total_allocated += sizeof(T);
			total_alive += sizeof(T);
			memory_history.emplace_back(glfwGetTime(), total_alive);
			memory_labels[label] += sizeof(T);
			return (T*)malloc(sizeof(T));
		}

		template<typename T>
		inline static T* Heap(uint32_t count, const char* label = "Engine") {
			total_allocated += sizeof(T) * count;
			total_alive += sizeof(T) * count;
			T* ptr = (T*)std::malloc(sizeof(T) * count);
			live_array_refs[ptr] = count;
			memory_labels[label] += sizeof(T) * count;
			return ptr;
		}

		inline static void* HeapRaw(size_t size, const char* label = "Engine") {
			total_allocated += size;
			total_alive += size;
			void* ptr = std::malloc(size);
			live_array_refs[ptr] = size;
			memory_history.emplace_back(glfwGetTime(), total_alive);
			memory_labels[label] += size;
			return ptr;
		}

		inline static void* ReallocRaw(void* block, size_t size, const char* label = "Engine") {
			uint32_t old_block_size = live_array_refs.find(block) != live_array_refs.end() ? live_array_refs[block] : 0;
			total_allocated += size;
			total_alive += size - old_block_size;
			memory_history.emplace_back(glfwGetTime(), total_alive);
			void* ptr = std::realloc(block, size);
			live_array_refs.erase(block);
			live_array_refs[ptr] = size;
			memory_labels[label] += size - old_block_size;
			return ptr;
		}

		template<typename T>
		inline static void Free(T* ptr, bool array = false) {
			if (array) {
				uint32_t count =
					live_array_refs.find(ptr) != live_array_refs.end() ? live_array_refs[ptr] : 1;
				live_array_refs.erase(ptr);
				total_alive -= sizeof(T) * count;
				delete[] ptr;
			}
			else {
				total_alive -= sizeof(T);
				delete ptr;
			}
			memory_history.emplace_back(glfwGetTime(), total_alive);
		}

		inline static void FreeRaw(void* ptr, const char* label = "Engine") {
			size_t size = live_array_refs.find(ptr) != live_array_refs.end() ? live_array_refs[ptr] : 0;
			live_array_refs.erase(ptr);
			total_alive -= size;
			free(ptr);
			memory_history.emplace_back(glfwGetTime(), total_alive);
		}

		inline static void* CallocRaw(size_t count, size_t size, const char* label = "Engine") {
			return std::calloc(count, size);
		}

		template<typename T>
		inline static T* Stack() {
			return (T*)alloca(sizeof(T));
		}

		inline static size_t GetTotalAllocated() { return total_allocated; }
		inline static size_t GetTotalAlive() { return total_alive; }
		inline static const std::vector<std::pair<float, size_t>>& GetMemoryHistory() {
			// If more than 50 entries, remove until 50
			if (memory_history.size() > 50)
				memory_history.erase(memory_history.begin(), memory_history.begin() + (memory_history.size() - 50));
			return memory_history;
		}
		inline static const std::unordered_map<const char*, size_t>& GetMemoryLabels() { return memory_labels; }

	private:
		inline static size_t total_allocated = 0;
		inline static size_t total_alive = 0;
		inline static std::unordered_map<void*, uint32_t> live_array_refs;
		inline static std::vector<std::pair<float, size_t>> memory_history;
		inline static std::unordered_map<const char*, size_t> memory_labels;
	};

	class RefCounted
	{
	public:
		void IncRefCount() const
		{
			++m_RefCount;
		}
		void DecRefCount() const
		{
			--m_RefCount;
		}

		uint32_t GetRefCount() const { return m_RefCount.load(); }

	private:
		mutable std::atomic<uint32_t> m_RefCount = 0;
	};

	namespace RefUtils {
		void AddToLiveReferences(void* instance);
		void RemoveFromLiveReferences(void* instance);
		bool IsLive(void* instance);
	}

	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Instance(nullptr)
		{
		}

		Ref(std::nullptr_t n)
			: m_Instance(nullptr)
		{
		}

		Ref(T* instance)
			: m_Instance(instance)
		{
			static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

			IncRef();
		}

		template<typename T2>
		Ref(const Ref<T2>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncRef();
		}

		template<typename T2>
		Ref(Ref<T2>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		static Ref<T> CopyWithoutIncrement(const Ref<T>& other)
		{
			Ref<T> result = nullptr;
			result->m_Instance = other.m_Instance;
			return result;
		}

		~Ref()
		{
			DecRef();
		}

		Ref(const Ref<T>& other)
			: m_Instance(other.m_Instance)
		{
			IncRef();
		}

		Ref& operator=(std::nullptr_t)
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		Ref& operator=(const Ref<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(const Ref<T2>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(Ref<T2>&& other)
		{
			DecRef();

			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		operator bool() { return m_Instance != nullptr; }
		operator bool() const { return m_Instance != nullptr; }

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		T* Raw() { return  m_Instance; }
		const T* Raw() const { return  m_Instance; }

		void Reset(T* instance = nullptr)
		{
			DecRef();
			m_Instance = instance;
		}
		void Leak() {
			m_Instance->IncRefCount();
		}

		template<typename T2>
		Ref<T2> As() const
		{
			return Ref<T2>(*this);
		}

		template<typename... Args>
		static Ref<T> Create(Args&&... args)
		{
#if HZ_TRACK_MEMORY
			return Ref<T>(new(typeid(T).name()) T(std::forward<Args>(args)...));
#else
			return Ref<T>(new T(std::forward<Args>(args)...));
			return Ref<T>(snew(T) T(std::forward<Args>(args)...));
#endif
		}

		bool operator==(const Ref<T>& other) const
		{
			return m_Instance == other.m_Instance;
		}

		bool operator!=(const Ref<T>& other) const
		{
			return !(*this == other);
		}

		bool EqualsObject(const Ref<T>& other)
		{
			if (!m_Instance || !other.m_Instance)
				return false;

			return *m_Instance == *other.m_Instance;
		}
	private:
		void IncRef() const
		{
			if (m_Instance)
			{
				m_Instance->IncRefCount();
				RefUtils::AddToLiveReferences((void*)m_Instance);
			}
		}

		void DecRef() const
		{
			if (m_Instance)
			{
				m_Instance->DecRefCount();
				if (m_Instance->GetRefCount() == 0)
				{
					sdelete(m_Instance);
					RefUtils::RemoveFromLiveReferences((void*)m_Instance);
					m_Instance = nullptr;
				}
			}
		}

		template<class T2>
		friend class Ref;
		mutable T* m_Instance;
	};

	template<typename T>
	class WeakRef
	{
	public:
		WeakRef() = default;

		WeakRef(Ref<T> ref)
		{
			m_Instance = ref.Raw();
		}

		WeakRef(T* instance)
		{
			m_Instance = instance;
		}

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		bool IsValid() const { return m_Instance ? RefUtils::IsLive(m_Instance) : false; }
		operator bool() const { return IsValid(); }
	private:
		T* m_Instance = nullptr;
	};

}