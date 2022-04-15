#pragma once

#define ASSERT(x) if (!(x))	__debugbreak();
#define glCall(x) GLClearError();\
					x;\
					ASSERT(GLCheckError(#x, __FILE__, __LINE__))

#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

void GLClearError();
bool GLCheckError(const char* function, const char* file, int line);

namespace Shado {

	class Log
	{
	public:
		static void init();

		static std::shared_ptr<spdlog::logger>& getCoreLogger() { return singleton.s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& getClientLogger() { return singleton.s_ClientLogger; }
	private:
		void initSingleton();

		static Log singleton;
		std::shared_ptr<spdlog::logger> s_CoreLogger;
		std::shared_ptr<spdlog::logger> s_ClientLogger;
		std::vector<spdlog::sink_ptr> logSinks;

		bool hasInit;
	};


}

#ifdef SHADO_ENABLE_ASSERTS
#define SHADO_ASSERT(x, ...) { if(!(x)) { SHADO_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define SHADO_CORE_ASSERT(x, ...) { if(!(x)) { SHADO_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define SHADO_ASSERT(x, ...)
#define SHADO_CORE_ASSERT(x, ...)
#endif

// Core log macros
#if 0	// There are problems with Spdlogger instances
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


