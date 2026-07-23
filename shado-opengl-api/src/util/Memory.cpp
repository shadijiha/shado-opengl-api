#include "Memory.h"
#include <mutex>
#include <unordered_set>
#include <debug/Debug.h>

#include "Application.h"
#include "debug/Profile.h"

namespace Shado { namespace {
        // Lazily-created leak-tracking set.
        //
        // This intentionally does NOT construct the Application (previously it
        // called Application::get() and registered a destroy callback). Memory
        // allocations happen both during C++ static initialization and during
        // the Application's own construction, so touching Application::get()
        // here caused the entire engine (window, scripting host, ...) to be
        // constructed during static initialization before main() ran — and
        // could recurse into Application::get() mid-construction.
        std::unordered_set<void*>* GetLiveReferences() {
            static auto* s_LiveReferences = new std::unordered_set<void*>();
            return s_LiveReferences;
        }

        // Intentionally leaked (never destroyed). Static Ref<> objects can be
        // destroyed during process exit (__cxa_finalize), after this
        // translation unit's statics would otherwise have been torn down.
        // Locking a destroyed std::mutex throws "mutex lock failed: Invalid
        // argument" from a noexcept destructor, which calls std::terminate.
        // Leaking keeps the mutex valid for the entire program lifetime.
        std::mutex& LiveRefMutex() {
            static std::mutex* s_mutex = new std::mutex();
            return *s_mutex;
        }
    }

    ////////////////////////////////
    //////// Memory class //////////
    ////////////////////////////////

    void* Memory::HeapRaw(size_t size, const std::string& label) {
        SHADO_PROFILE_FUNCTION();
    
        total_allocated += size;
        total_alive += size;
        void* ptr = std::malloc(size);
        live_array_refs[ptr] = size;
        memory_history.emplace_back(glfwGetTime(), total_alive);
        memory_labels[label] += size;
        return ptr;
    }

    void* Memory::ReallocRaw(void* block, size_t size, const std::string& label) {
        SHADO_PROFILE_FUNCTION();
        uint32_t old_block_size = live_array_refs.contains(block) ? live_array_refs[block] : 0;
        total_allocated += size;
        total_alive += size - old_block_size;
        memory_history.emplace_back(glfwGetTime(), total_alive);
        void* ptr = std::realloc(block, size);
        live_array_refs.erase(block);
        live_array_refs[ptr] = size;
        memory_labels[label] += size - old_block_size;
        return ptr;
    }

    void Memory::FreeRaw(void* ptr, const std::string& label) {
        SHADO_PROFILE_FUNCTION();
        size_t size = live_array_refs.contains(ptr) ? live_array_refs[ptr] : 0;
        live_array_refs.erase(ptr);
        total_alive -= size;
        std::free(ptr);
        memory_history.emplace_back(glfwGetTime(), total_alive);
    }

    void* Memory::CallocRaw(size_t count, size_t size, const std::string& label) {
        return std::calloc(count, size);
    }

    const std::vector<std::pair<float, size_t>>& Memory::GetMemoryHistory() {
        SHADO_PROFILE_FUNCTION();
        
        // If more than max entries, remove until max
        constexpr int maxDataPoints = 10240;
        if (memory_history.size() > maxDataPoints)
            memory_history.erase(memory_history.begin(),
                                 memory_history.begin() + (memory_history.size() - maxDataPoints));
        return memory_history;
    }

    const std::unordered_map<std::string, size_t>& Memory::GetMemoryLabels() {
        return memory_labels;
    }

    namespace RefUtils {
        void AddToLiveReferences(void* instance) {
            std::scoped_lock lock(LiveRefMutex());
            SHADO_CORE_ASSERT(instance, "");
            GetLiveReferences()->insert(instance);
        }

        void RemoveFromLiveReferences(void* instance) {
            std::scoped_lock lock(LiveRefMutex());
            // TODO: fix teardown bug
            //if (!IsLive(instance))
            //    SHADO_CORE_WARN("Attempting to remove a live reference instance that is not in the s_LiveReferences map");
            //else
            //    s_LiveReferences->erase(instance);
        }

        bool IsLive(void* instance) {
            return instance && GetLiveReferences()->contains(instance);
        }
    }}
