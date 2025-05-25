#include "Memory.h"
#include <mutex>
#include <unordered_set>
#include <debug/Debug.h>

#include "Application.h"
#include "debug/Profile.h"

namespace Shado { namespace {
        std::unordered_set<void*>* CreateLiveReferencesMap() {
            auto* map = snew(std::unordered_set<void*>) std::unordered_set<void*>();
            Application::get().AddOnDestroyedCallback([map]() { sdelete(map); });
            return map;
        }

        std::unordered_set<void*>* s_LiveReferences = CreateLiveReferencesMap();
        std::mutex s_LiveReferenceMutex;
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
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            SHADO_CORE_ASSERT(instance, "");
            s_LiveReferences->insert(instance);
        }

        void RemoveFromLiveReferences(void* instance) {
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            // TODO: fix teardown bug
            //if (!IsLive(instance))
            //    SHADO_CORE_WARN("Attempting to remove a live reference instance that is not in the s_LiveReferences map");
            //else
            //    s_LiveReferences->erase(instance);
        }

        bool IsLive(void* instance) {
            return s_LiveReferences && instance && s_LiveReferences->contains(instance);
        }
    }}
