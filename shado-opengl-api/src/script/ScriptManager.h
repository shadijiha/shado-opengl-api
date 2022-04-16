#pragma once
#include <list>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string>

namespace Shado {

	class ScriptManager {
	public:
		static void init(const std::string& path);
		static void shutdown();

		static void addInternalCall(const std::string& methodSignature, const void* func);

		static void invokeStaticMethod(const std::string& methodSignature, void** args = nullptr);
		static void invokeInstanceMethod(const std::string& methodSignature, MonoObject* obj, void** args = nullptr);
	
		static MonoObject* createObject(const std::string& klass, const std::string& constructorSignature = "", void** args = nullptr);

		static std::list<MonoClass*> getAssemblyClassList();

	private:
		static MonoClass* getClass(const std::string& namesace, const std::string& klass);
		static MonoMethod* getMethod(const std::string& methodSignature);
	
	private:
		inline static MonoDomain* domain;
		inline static MonoAssembly* assembly;
		inline static MonoImage* image;
		inline static std::string s_namespace = "Shado";
	};
}