#pragma once
#include <glm/detail/type_quat.hpp>
#include "util/Util.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Shado {

	class Log
	{
	public:
		static void init();

		static Ref<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};


}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

#ifdef SHADO_ENABLE_ASSERTS
#define SHADO_ASSERT(x, ...) { if(!(x)) { SHADO_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define SHADO_CORE_ASSERT(x, ...) { if(!(x)) { SHADO_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define SHADO_ASSERT(x, ...)
#define SHADO_CORE_ASSERT(x, ...)
#endif

// Core log macros
// Core log macros
#if 1
#define SHADO_CORE_TRACE(...)    ::Shado::Log::getCoreLogger()->trace(__VA_ARGS__)
#define SHADO_CORE_INFO(...)     ::Shado::Log::getCoreLogger()->info(__VA_ARGS__)
#define SHADO_CORE_WARN(...)     ::Shado::Log::getCoreLogger()->warn(__VA_ARGS__)
#define SHADO_CORE_ERROR(...)    ::Shado::Log::getCoreLogger()->error(__VA_ARGS__)
#define SHADO_CORE_CRITICAL(...) ::Shado::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SHADO_TRACE(...)         ::Shado::Log::getClientLogger()->trace(__VA_ARGS__)
#define SHADO_INFO(...)          ::Shado::Log::getClientLogger()->info(__VA_ARGS__)
#define SHADO_WARN(...)          ::Shado::Log::getClientLogger()->warn(__VA_ARGS__)
#define SHADO_ERROR(...)         ::Shado::Log::getClientLogger()->error(__VA_ARGS__)
#define SHADO_CRITICAL(...)      ::Shado::Log::getClientLogger()->critical(__VA_ARGS__)

#else
#define SHADO_CORE_TRACE(...)    spdlog::trace(__VA_ARGS__)
#define SHADO_CORE_INFO(...)	 spdlog::info(__VA_ARGS__)
#define SHADO_CORE_WARN(...)     spdlog::warn(__VA_ARGS__)
#define SHADO_CORE_ERROR(...)    spdlog::error(__VA_ARGS__)
#define SHADO_CORE_CRITICAL(...) spdlog::critical(__VA_ARGS__)

// Client log macros
#define SHADO_TRACE(...)         spdlog::trace(__VA_ARGS__)
#define SHADO_INFO(...)          spdlog::info(__VA_ARGS__)
#define SHADO_WARN(...)          spdlog::warn(__VA_ARGS__)
#define SHADO_ERROR(...)         spdlog::error(__VA_ARGS__)
#define SHADO_CRITICAL(...)      spdlog::critical(__VA_ARGS__)
#endif

