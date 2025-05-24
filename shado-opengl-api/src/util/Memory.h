#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#define snew(Type) new(Shado::Memory::Heap<Type>(std::string("Engine - ") + typeid(Type).name()))
#define sdelete(ptr) Shado::Memory::Free(ptr)

extern "C" {
double glfwGetTime();
}

namespace Shado {
    class Memory {
    public:
        template <typename T>
        inline static T* Heap(const std::string& label = "Engine") {
            return static_cast<T*>(HeapRaw(sizeof(T), label));
        }

        template <typename T>
        inline static T* Heap(uint64_t count, const std::string& label = "Engine") {
            return static_cast<T*>(HeapRaw(sizeof(T) * count, label));
        }

        static void* HeapRaw(size_t size, const std::string& label = "Engine");

        static void* ReallocRaw(void* block, size_t size, const std::string& label = "Engine");

        template <typename T>
        inline static void Free(T* ptr, const std::string& label = "Engine") {
            FreeRaw(static_cast<void*>(ptr), label);
        }

        static void FreeRaw(void* ptr, const std::string& label = "Engine");

        static void* CallocRaw(size_t count, size_t size, const std::string& label = "Engine");

        template <typename T>
        inline static T* Stack() {
            return static_cast<T*>(alloca(sizeof(T)));
        }

        static size_t GetTotalAllocated() { return total_allocated; }
        static size_t GetTotalAlive() { return total_alive; }

        static const std::vector<std::pair<float, size_t>>& GetMemoryHistory();

        static const std::unordered_map<std::string, size_t>& GetMemoryLabels();

    private:
        inline static size_t total_allocated = 0;
        inline static size_t total_alive = 0;
        inline static std::unordered_map<void*, size_t> live_array_refs;
        inline static std::vector<std::pair<float, size_t>> memory_history;
        inline static std::unordered_map<std::string, size_t> memory_labels;
    };

    class RefCounted {
    public:
        void IncRefCount() const {
            ++m_RefCount;
        }

        void DecRefCount() const {
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

    template <typename T>
    class Ref {
    public:
        Ref()
            : m_Instance(nullptr) {}

        Ref(std::nullptr_t n)
            : m_Instance(nullptr) {}

        Ref(T* instance)
            : m_Instance(instance) {
            static_assert(std::is_base_of_v<RefCounted, T>, "Class is not RefCounted!");

            IncRef();
        }

        template <typename T2>
        Ref(const Ref<T2>& other) {
            m_Instance = static_cast<T*>(other.m_Instance);
            IncRef();
        }

        template <typename T2>
        Ref(Ref<T2>&& other) {
            m_Instance = static_cast<T*>(other.m_Instance);
            other.m_Instance = nullptr;
        }

        static Ref<T> CopyWithoutIncrement(const Ref<T>& other) {
            Ref<T> result = nullptr;
            result->m_Instance = other.m_Instance;
            return result;
        }

        ~Ref() {
            DecRef();
        }

        Ref(const Ref<T>& other)
            : m_Instance(other.m_Instance) {
            IncRef();
        }

        Ref& operator=(std::nullptr_t) {
            DecRef();
            m_Instance = nullptr;
            return *this;
        }

        Ref& operator=(const Ref<T>& other) {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template <typename T2>
        Ref& operator=(const Ref<T2>& other) {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template <typename T2>
        Ref& operator=(Ref<T2>&& other) {
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

        T* Raw() { return m_Instance; }
        const T* Raw() const { return m_Instance; }

        void Reset(T* instance = nullptr) {
            DecRef();
            m_Instance = instance;
        }

        void Leak() {
            m_Instance->IncRefCount();
        }

        template <typename T2>
        Ref<T2> As() const {
            return Ref<T2>(*this);
        }

        template <typename... Args>
        static Ref<T> Create(Args&&... args) {
#if HZ_TRACK_MEMORY
			return Ref<T>(new(typeid(T).name()) T(std::forward<Args>(args)...));
#else
            return Ref<T>(new T(std::forward<Args>(args)...));
            return Ref<T>(snew(T) T(std::forward<Args>(args)...));
#endif
        }

        bool operator==(const Ref<T>& other) const {
            return m_Instance == other.m_Instance;
        }

        bool operator!=(const Ref<T>& other) const {
            return !(*this == other);
        }

        bool EqualsObject(const Ref<T>& other) {
            if (!m_Instance || !other.m_Instance)
                return false;

            return *m_Instance == *other.m_Instance;
        }

    private:
        void IncRef() const {
            if (m_Instance) {
                m_Instance->IncRefCount();
                RefUtils::AddToLiveReferences(static_cast<void*>(m_Instance));
            }
        }

        void DecRef() const {
            if (m_Instance) {
                m_Instance->DecRefCount();
                if (m_Instance->GetRefCount() == 0) {
                    sdelete(m_Instance);
                    RefUtils::RemoveFromLiveReferences(static_cast<void*>(m_Instance));
                    m_Instance = nullptr;
                }
            }
        }

        template <class T2>
        friend class Ref;
        mutable T* m_Instance;
    };

    template <typename T>
    class WeakRef {
    public:
        WeakRef() = default;

        WeakRef(Ref<T> ref) {
            m_Instance = ref.Raw();
        }

        WeakRef(T* instance) {
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

    /****************** Costum smart ptr ********************/
    template <typename T>
    using ScopedRef = std::unique_ptr<T>;

    template <typename T, typename... Args>
    ScopedRef<T> CreateScoped(Args&&... args) {
        return ScopedRef<T>(snew(T) T(std::forward<Args>(args)...));
    }

    template <typename T, typename... Args>
    Ref<T> CreateRef(Args&&... args) {
        return Ref(snew(T) T(std::forward<Args>(args)...));
    }
}
