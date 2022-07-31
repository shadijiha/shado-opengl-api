#pragma once
#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <sstream>

#include "scene/Entity.h"

struct _MonoObject;
struct _MonoClass;
struct _MonoMethod;
struct _MonoDomain;
struct _MonoException;
struct _MonoImage;
struct _MonoAssembly;
struct _MonoImage;
struct _MonoType;
struct _MonoClassField;
struct _MonoArray;

typedef struct _MonoClass MonoClass;
typedef struct _MonoImage MonoImage;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoArray MonoArray;

typedef struct _MonoObject MonoObject;
typedef struct _MonoException MonoException;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoType MonoType;
typedef struct _MonoClassField MonoClassField;

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
		MonoClass* parent;
		MonoClass* klass;
		std::unordered_map<std::string, ScriptFieldDesc> fields;
		std::unordered_map<std::string, MonoMethod*> methodes;		

		std::string toString() const {
			std::stringstream ss;
			ss << "class " << name_space << "::" << name << " extends " << fromMonoClass(parent).name << std::endl;

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

		static ScriptClassDesc fromMonoClass(MonoClass* klass);
	};
	
	class ScriptManager {
	public:
		static void init(const std::string& path);
		static void shutdown();
		static void reload(const std::string& path = assemblyPathFallback);
		static void requestThreadsDump();

		static void addInternalCall(const std::string& methodSignature, const void* func);

		static void invokeStaticMethod(const std::string& methodSignature, void** args = nullptr);
		static void invokeInstanceMethod(const std::string& methodSignature, MonoObject* obj, void** args = nullptr);
	
		static ScriptClassInstance createObject(const ScriptClassDesc& desc, const std::string& constructorSignature = "", void** args = nullptr);
		static ScriptClassInstance createEntity(const ScriptClassDesc& desc, UUID id, Scene* scene);
		static MonoArray* createArray(const ScriptClassDesc& klass, uint32_t size);

		static std::list<ScriptClassDesc> getAssemblyClassList();
		static std::list<ScriptClassDesc> getChildrenOf(const std::string& parentName);
		static ScriptClassDesc getClassByName(const std::string& name);
		static bool hasClass(const std::string& name, const std::string& name_space = "");

		static void setAssemblyDefaultPath(const std::string& path)	{ assemblyPathFallback = path; }
		static bool hasValidDefautDLL()								{ return !assemblyPathFallback.empty(); }
		static std::string getDLLPath()								{ return assemblyPathFallback; }
		static bool hasInit() { return image != nullptr && domain != nullptr; }

		static MonoImage* getImage() { return image; }
	private:
		static MonoClass* getClass(const std::string& namesace, const std::string& klass);
		static MonoMethod* getMethod(const std::string& methodSignature);

		static void setUpAllInternalCalls();

	private:
		inline static MonoDomain* domain;
		inline static MonoAssembly* assembly;
		inline static MonoImage* image;
		inline static std::string s_namespace = "Shado";
		inline static std::string assemblyPathFallback;


		struct ComponentsCallbacks {
			std::function<void(Entity)> addComponent;
			std::function<bool(Entity)> hasComponent;
			std::function<void(Entity)> removeComponent;

			ComponentsCallbacks(std::function<void(Entity)> f1, std::function<bool(Entity)> f2, std::function<void(Entity)> f3)
				: addComponent(f1), hasComponent(f2), removeComponent(f3) {}
			ComponentsCallbacks(){}
		};
		inline static std::unordered_map<std::string, ComponentsCallbacks> registedComponents;

	};
	
	class ScriptClassInstance {
	public:
		ScriptClassInstance(MonoDomain* domain, MonoImage* image, ScriptClassDesc desc, const std::string& ctorSignature = "", void** args = nullptr);
		ScriptClassInstance() = default;
		~ScriptClassInstance();

		MonoMethod* getMethod(const std::string& nameOrSignature, bool includesNamespace = false);
		ScriptFieldDesc getField(const std::string& name);

		MonoObject* invokeMethod(MonoMethod* method, void** args = nullptr);
		MonoObject* invokeMethod(MonoMethod* method, void* arg) {
			void* args[] = {
				arg
			};
			return invokeMethod(method, (void**)args);
		}

		MonoObject* getFieldValue(const ScriptFieldDesc& field);
		MonoObject* getFieldValue(const std::string& name);

		void setFieldValue(const ScriptFieldDesc& field, void* value);
		void setFieldValue(const std::string& name, void* value);

		const ScriptClassDesc& getDescription() const { return description; }

		/**
		 * Use this to unbox premitive types from C# script
		 */
		//template<typename T>
		void* unbox(MonoObject* obj);

		MonoObject* getNative() { return instance; }

	private:
		std::string getFullSignature(const std::string& methodNameAndArgs);
	private:
		ScriptClassDesc description;
		MonoImage* image;
		MonoObject* instance;
		MonoDomain* domain;

		friend class ScriptManager;
	};
}
