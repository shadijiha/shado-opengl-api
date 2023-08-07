#include <unordered_set>
#include <mutex>
#include <debug/Debug.h>
#include "Memory.h"
namespace Shado {

	static std::unordered_set<void*> s_LiveReferences;
	static std::mutex s_LiveReferenceMutex;

	namespace RefUtils {

		void AddToLiveReferences(void* instance)
		{
			std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
			SHADO_CORE_ASSERT(instance, "");
			s_LiveReferences.insert(instance);
		}

		void RemoveFromLiveReferences(void* instance)
		{
			std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
			SHADO_CORE_ASSERT(instance, "");
			SHADO_CORE_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end(), "");
			s_LiveReferences.erase(instance);
		}

		bool IsLive(void* instance)
		{
			SHADO_CORE_ASSERT(instance, "");
			return s_LiveReferences.find(instance) != s_LiveReferences.end();
		}
	}


}