#pragma once
#include <list>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string>
#include <unordered_map>
#include <sstream>
namespace Shado {

	class ScriptClassInstance;

	struct ScriptFieldDesc {
		enum class Visibility {
			PRIVATE, PROTECTED_AND_INTERNAL, INTERNAL, PROTECTED, PUBLIC
		};

		std::string name;
		MonoType* type;
		MonoClassField* field;
		Visibility visibility;
	};

	struct ScriptClassDesc {
		std::string name_space;
		std::string name;
		MonoClass* klass;
		std::unordered_map<std::string, ScriptFieldDesc> fields;
		std::unordered_map<std::string, MonoMethod*> methodes;		

		std::string toString() const {
			std::stringstream ss;
			ss << "class " << name_space << "::" << name << std::endl;

			ss << "\t fields:\n";
			for (const auto [name, field] : fields) {
				ss << "\t\t" << name << std::endl;
			}

			ss << "\t methods:\n";
			for (const auto [name, method] : methodes) {
				ss << "\t\t" << name << std::endl;
			}
			ss << std::endl;
			return ss.str();
		}
	};
	
	class ScriptManager {
	public:
		static void init(const std::string& path);
		static void shutdown();
		static void reload(const std::string& path = assemblyPathFallback);

		static void addInternalCall(const std::string& methodSignature, const void* func);

		static void invokeStaticMethod(const std::string& methodSignature, void** args = nullptr);
		static void invokeInstanceMethod(const std::string& methodSignature, MonoObject* obj, void** args = nullptr);
	
		static ScriptClassInstance createObject(const ScriptClassDesc& desc, const std::string& constructorSignature = "", void** args = nullptr);

		static std::list<ScriptClassDesc> getAssemblyClassList();

		static void setAssemblyDefaultPath(const std::string& path)	{ assemblyPathFallback = path; }
		static bool hasValidDefautDLL()								{ return !assemblyPathFallback.empty(); }
	private:
		static MonoClass* getClass(const std::string& namesace, const std::string& klass);
		static MonoMethod* getMethod(const std::string& methodSignature);
	
	private:
		inline static MonoDomain* domain;
		inline static MonoAssembly* assembly;
		inline static MonoImage* image;
		inline static std::string s_namespace = "Shado";
		inline static std::string assemblyPathFallback;

	};
	
	class ScriptClassInstance {
	public:
		ScriptClassInstance(MonoDomain* domain, MonoImage* image, ScriptClassDesc desc, const std::string& ctorSignature = "", void** args = nullptr);
		~ScriptClassInstance();

		MonoMethod* getMethod(const std::string& nameOrSignature, bool includesNamespace = false);
		ScriptFieldDesc getField(const std::string& name);

		MonoObject* invokeMethod(MonoMethod* method, void** args = nullptr);

		MonoObject* getFieldValue(const ScriptFieldDesc& field);
		MonoObject* getFieldValue(const std::string& name);

		/**
		 * Use this to unbox premitive types from C# script
		 */
		template<typename T>
		T unboxAndCastTo(MonoObject* obj);

	private:
		std::string getFullSignature(const std::string& methodNameAndArgs);
	private:
		ScriptClassDesc description;
		MonoImage* image;
		MonoObject* instance;
		MonoDomain* domain;
	};

	template <typename T>
	T ScriptClassInstance::unboxAndCastTo(MonoObject* obj) {
		return *(T*)mono_object_unbox(obj);
	}
}
