#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"
#include "mono/metadata/exception.h"
#include <FileWatch.h>

#include "Application.h"
#include "debug/Profile.h"
#include "../util/Buffer.h"
#include "../util/FileSystem.h"

#include "../Project/Project.h"
#include "scene/Components.h"

namespace Shado {

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },

		{ "Shado.Vector2", ScriptFieldType::Vector2 },
		{ "Shado.Vector3", ScriptFieldType::Vector3 },
		{ "Shado.Vector4", ScriptFieldType::Vector4 },
		{ "Shado.Colour", ScriptFieldType::Colour },

		{ "Shado.Entity", ScriptFieldType::Entity },
		{ "Shado.Prefab", ScriptFieldType::Prefab },
	};

	namespace Utils {

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), pdbFileData.Size());
					SHADO_CORE_INFO("Loaded PDB {}", pdbPath);
				}
			}

			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				SHADO_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
			{
				SHADO_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}
	}

	static ScriptEngineData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_Data->AssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_Data->AssemblyReloadPending = true;

			Application::get().SubmitToMainThread([]()
				{
					s_Data->AppAssemblyFileWatcher.reset();
					ScriptEngine::ReloadAssembly();
				});
		}
	}

	void ScriptEngine::Init()
	{
		if (s_Data != nullptr) {
			Shutdown();
		}

		s_Data = snew(ScriptEngineData) ScriptEngineData();

		InitMono();
		ScriptGlue::RegisterFunctions();

		auto& projConfig = Project::GetActive()->GetConfig();

		// TODO: These aseembly pathes are terrible and need to re-evaludate them
		bool status = LoadAssembly(Project::GetProjectDirectory() / "bin/Release-windows-x86_64" / projConfig.Name / "Shado-script-core.dll");
		if (!status)
		{
			SHADO_CORE_ERROR("[ScriptEngine] Could not load Shado-ScriptCore assembly.");
			return;
		}

		auto scriptModulePath = Project::GetProjectDirectory() / projConfig.ScriptModulePath;

		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			SHADO_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
			return;
		}

		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_Data->EntityClass = ScriptClass("Shado", "Entity", true);
		s_Data->EditorClass = ScriptClass("Shado.Editor", "Editor", true);
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	void ScriptEngine::InitMono()
	{
		// set allocator
		auto* allocator = snew(MonoAllocatorVTable) MonoAllocatorVTable();
		allocator->malloc =		[](size_t size)				{ return Memory::HeapRaw(size, "C#"); };
		allocator->realloc =	[](void* ptr, size_t size)	{ return Memory::ReallocRaw(ptr, size, "C#"); };
		allocator->free =		[](void* ptr)				{ Memory::FreeRaw(ptr, "C#"); };
		allocator->calloc =		[](size_t count, size_t size) { return Memory::CallocRaw(count, size, "C#"); };
		mono_set_allocator_vtable(allocator);
		mono_set_assemblies_path("mono/lib");

		if (s_Data->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		SHADO_CORE_ASSERT(rootDomain, "");

		// Store the root domain pointer
		s_Data->RootDomain = rootDomain;

		if (s_Data->EnableDebugging)
			mono_debug_domain_create(s_Data->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_debug_cleanup();

		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyFilepath = filepath;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (s_Data->CoreAssembly == nullptr)
			return false;

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (s_Data->AppAssembly == nullptr)
			return false;

		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);

		s_Data->AppAssemblyFileWatcher = std::make_shared<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
		s_Data->AssemblyReloadPending = false;
		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);

		LoadAssembly(s_Data->CoreAssemblyFilepath);
		LoadAppAssembly(s_Data->AppAssemblyFilepath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_Data->EntityClass = ScriptClass("Shado", "Entity", true);
		s_Data->EditorClass = ScriptClass("Shado.Editor", "Editor", true);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		if (!s_Data) {
			SHADO_CORE_WARN("No C# DLL was provided");
			return;
		}
		s_Data->SceneContext = scene;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.getComponent<ScriptComponent>();
		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			UUID entityID = entity.getUUID();

			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity, true);
			s_Data->EntityInstances[entityID] = instance;

			// Copy field values
			if (s_Data->EntityScriptFields.find(entityID) != s_Data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnCreateEntity(Entity entity, Ref<ScriptInstance> managed)
	{
		const auto& sc = entity.getComponent<ScriptComponent>();
		SHADO_CORE_ASSERT(ScriptEngine::EntityClassExists(sc.ClassName), "");

		UUID entityID = entity.getUUID();
		s_Data->EntityInstances[entityID] = managed;

		// Copy field values
		if (s_Data->EntityScriptFields.find(entityID) != s_Data->EntityScriptFields.end())
		{
			const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(entityID);
			for (const auto& [name, fieldInstance] : fieldMap)
				managed->SetFieldValueInternal(name,fieldInstance.m_Buffer);
		}

		managed->InvokeOnCreate();
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, TimeStep ts)
	{
		UUID entityUUID = entity.getUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnUpdate((float)ts);
		}
		else
		{
			SHADO_CORE_ERROR("Could not find ScriptInstance for entity {}", entityUUID);
		}
	}

	// Called with the scene.OnDrawRuntime
	void ScriptEngine::OnDrawEntity(Entity entity)
	{
		UUID entityUUID = entity.getUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnDraw();
		} else
		{
			SHADO_CORE_ERROR("Could not find ScriptInstance for entity {}", entityUUID);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_Data->EntityInstances.find(entityID);
		if (it == s_Data->EntityInstances.end())
			return nullptr;

		return it->second;
	}

	Ref<ScriptInstance> ScriptEngine::CreateEntityScriptInstance(Ref<ScriptClass> klass, Entity e)
	{
		Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(klass, e);
		s_Data->EntityInstances[e.getUUID()] = instance;
		return instance;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (s_Data->EntityClasses.find(name) == s_Data->EntityClasses.end())
			return nullptr;

		return s_Data->EntityClasses.at(name);
	}

	void ScriptEngine::OnRuntimeStop()
	{
		if (!s_Data)
			return;

		// Destroy all script instances
		for (auto& [uuid, instance] : s_Data->EntityInstances)
			instance->InvokeOnDestroy();

		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		SHADO_CORE_ASSERT(entity, "");

		UUID entityID = entity.getUUID();
		return s_Data->EntityScriptFields[entityID];
	}

	void ScriptEngine::Helper_ProcessFields(MonoClass* monoClass, const char* className, Ref<ScriptClass> scriptClass) {
		int fieldCount = mono_class_num_fields(monoClass);

		SHADO_CORE_WARN("{} has {} fields:", className, fieldCount);
		void* iterator = nullptr;
		while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
		{
			const char* fieldName = mono_field_get_name(field);
			uint32_t flags = mono_field_get_flags(field);
			if (flags & FIELD_ATTRIBUTE_PUBLIC)
			{
				MonoType* type = mono_field_get_type(field);
				ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
				SHADO_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

				scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
			}
		}
	}

	void ScriptEngine::Helper_ProcessClass(const MonoTableInfo* typeDefinitionsTable, int32_t i, MonoClass* entityClass, MonoClass* editorClass, MonoImage* image) {

		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

		const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* className = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		std::string fullName;
		if (strlen(nameSpace) != 0)
			fullName = fmt::format("{}.{}", nameSpace, className);
		else
			fullName = className;

		MonoClass* monoClass = mono_class_from_name(image, nameSpace, className);

		if (monoClass == nullptr)
			return;

		if (monoClass == entityClass)
			return;

		if (monoClass == editorClass)
			return;

		bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
		if (isEntity) {

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;


			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.
			Helper_ProcessFields(monoClass, className, scriptClass);
		}

		bool isEditor = mono_class_is_subclass_of(monoClass, editorClass, false);
		if (isEditor) {
			bool isCore = image == s_Data->CoreAssemblyImage;
			SHADO_CORE_INFO("{0}, {1}", fullName, isCore);

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className, isCore);

			Helper_ProcessFields(monoClass, className, scriptClass);

			// Get an instance of the class
			Ref<ScriptInstance> scriptInstance = CreateRef<ScriptInstance>(scriptClass, true);

			// Call GetTargetType to get the type for
			MonoString* typeFor = (MonoString*)scriptClass->InvokeMethod(
				scriptInstance->GetManagedObject(),
				//scriptClass->GetMethod("GetTargetType", 0)
				mono_class_get_method_from_name(editorClass, "GetTargetType", 0)
			);

			SHADO_CORE_INFO("{0} is for {1}", fullName, typeFor == nullptr ? "null!" : mono_string_to_utf8(typeFor));
			if (typeFor != nullptr) {
				std::string typeForStr = mono_string_to_utf8(typeFor);
				s_Data->EditorClasses[typeForStr] = scriptClass;
				s_Data->EditorInstances[typeForStr] = scriptInstance;
			}
		}
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		// Process AppAssembly
		s_Data->EntityClasses.clear();

		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Shado", "Entity");
		MonoClass* editorClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Shado.Editor", "Editor");
		
		{
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
			for (int32_t i = 0; i < numTypes; i++)
			{
				Helper_ProcessClass(typeDefinitionsTable, i, entityClass, editorClass, s_Data->AppAssemblyImage);
			}
		}

		// Process Core Assembly
		{
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->CoreAssemblyImage, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
			for (int32_t i = 0; i < numTypes; i++)
			{
				Helper_ProcessClass(typeDefinitionsTable, i, entityClass, editorClass, s_Data->CoreAssemblyImage);
			}
		}

		//auto& entityClasses = s_Data->EntityClasses;
		//mono_field_get_value()
	}

	MonoString* ScriptEngine::NewString(const char* str)
	{
		// Cache string
		static std::unordered_map<const char*, MonoString*> s_StringCache;
		if (s_StringCache.find(str) != s_StringCache.end())
			return s_StringCache[str];

		auto* csStr = mono_string_new(s_Data->AppDomain, str);
		s_StringCache[str] = csStr;
		return csStr;
	}

	std::string ScriptEngine::MonoStrToUT8(MonoString* str)
	{
		if (!str) {
			return "null";
		}
		char* ptr = mono_string_to_utf8(str);
		std::string result(ptr);
		mono_free(ptr);
		return result;
	}

	ScriptInstance ScriptEngine::InstanceFromRawObject(MonoObject* object) {
		return ScriptInstance(CreateRef<ScriptClass>(mono_object_get_class(object)), object);
	}

	const ScriptEngineData& ScriptEngine::GetData() {
		return *s_Data;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}


	void ScriptEngine::DrawCustomEditorForFieldRunning(const ScriptField& field, Ref<ScriptInstance> scriptInstance, const std::string& name)
	{
		for (auto [typeFor, editorInstance] : s_Data->EditorInstances) {
			// If we find custom editor, then invoke the OnEditorDraw func
			std::string temp = mono_type_get_name(mono_field_get_type(field.ClassField));
			if (typeFor == temp) {

				// Set the target
				MonoObject* scriptInstanceManagedObject = scriptInstance->GetManagedObject();
				if (scriptInstanceManagedObject == nullptr)
					continue;
				
				MonoObject* target = mono_field_get_value_object(GetAppDomain(), field.ClassField, scriptInstanceManagedObject);//scriptInstance->GetFieldValue<MonoObject*>(name);
				uint32_t targetGCHandle = mono_gchandle_new_weakref(target, false);
				mono_field_set_value(
					editorInstance->GetManagedObject(),
					mono_class_get_field_from_name(s_Data->EditorClass.m_MonoClass, "monoGCHandle"),
					&targetGCHandle
				);

				// Set the fieldName
				mono_field_set_value(editorInstance->GetManagedObject(),
					mono_class_get_field_from_name(s_Data->EditorClass.m_MonoClass, "fieldName"),
					ScriptEngine::NewString(name.c_str())
				);

				if (target == nullptr)
					continue;

				MonoMethod* method = editorInstance->GetScriptClass()->GetMethod("OnEditorDraw", 0);
				MonoObject* object = editorInstance->GetManagedObject();
				if (method && object) {
					editorInstance->GetScriptClass()->InvokeMethod(
						object,
						method
					);
				}
			}
		}
	}

	void ScriptEngine::InvokeCustomEditorEvents(Event& event)
	{
		static struct CSEvent {
			/* General event */
			int32_t categoryFlags;
			int32_t type;
			bool handled;

			/* Window event */
			uint32_t width;
			uint32_t height;

			/* Mouse events */
			float x = 0;
			float y = 0;
			int button = 0;

			/* Key events */
			KeyCode keycode = (KeyCode)-1;
			int32_t repeatCount = 0;
		};

		for (auto [typeFor, editorInstance] : s_Data->EditorInstances) {
			// If we find custom editor, then invoke the OnEditorDraw func
			MonoMethod* onEventMethod = editorInstance->GetScriptClass()->GetMethod("OnEvent", 1);

			if (onEventMethod)
			{
				// Helper function
				auto is = [&event](std::initializer_list<EventType> types) {
					if (!&event)
						return false;

					for (auto type : types)
						if (type == event.getEventType())
							return true;
					return false;
				};

				CSEvent e; 
				/* Event */
				e.categoryFlags = event.getCategoryFlags();
				e.type = (int32_t)event.getEventType();
				e.handled = event.isHandled();
			
				/* Window event*/
				e.width = is({ EventType::WindowResize }) ? ((WindowResizeEvent&)event).getWidth() : 0;
				e.height = is({ EventType::WindowResize }) ? ((WindowResizeEvent&)event).getHeight() : 0;
				
				/* mouse events*/
				e.x = is({ EventType::MouseMoved }) ? ((MouseMovedEvent&)event).getX() : 0;
				e.y = is({ EventType::MouseMoved }) ? ((MouseMovedEvent&)event).getY() : 0;
				e.button = is({ EventType::MouseButtonPressed, EventType::MouseButtonReleased }) ? ((MouseButtonEvent&)event).getMouseButton() : 0;

				/* Key events */
				e.keycode = is({ EventType::KeyPressed, EventType::KeyReleased, EventType::KeyTyped }) ? ((KeyEvent&)event).getKeyCode() : (KeyCode)-1;
				e.repeatCount = is({ EventType::KeyPressed }) ? ((KeyPressedEvent&)event).getRepeatCount() : 0;
				
				void* params[] = { &e };

				editorInstance->GetScriptClass()->InvokeMethod(
					editorInstance->GetManagedObject(),
					onEventMethod,
					params
				);
				//event.setHandled(e.handled);
			}			
		}
	}

	void ScriptEngine::DrawCustomEditorForFieldStopped(const std::string& fieldName, const ScriptField& field, Entity entity, Ref<ScriptClass> klass, bool wasSet) {
		for (auto [typeFor, editorInstance] : s_Data->EditorInstances) {
			// If we find custom editor, then invoke the OnEditorDraw func
			std::string temp = mono_type_get_name(mono_field_get_type(field.ClassField));
			if (typeFor == temp) {

				// Get the script instance
				Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.getUUID());
				if (!scriptInstance) {
					// If script instance is not found, create one
					scriptInstance = CreateRef<ScriptInstance>(klass, entity);
					s_Data->EntityInstances[entity.getUUID()] = scriptInstance;
				}

				// Set the target
				MonoObject* target = scriptInstance->GetFieldValue<MonoObject*>(fieldName);
				MonoObject* managedObject = editorInstance->GetManagedObject();
				if (target == nullptr || managedObject == nullptr)
					continue;
				
				uint32_t targetGCHandle = mono_gchandle_new_weakref(target, false);
				mono_field_set_value(
					managedObject,
					mono_class_get_field_from_name(s_Data->EditorClass.m_MonoClass, "monoGCHandle"),
					&targetGCHandle
				);

				// Set the fieldName
				mono_field_set_value(managedObject,
					mono_class_get_field_from_name(s_Data->EditorClass.m_MonoClass, "fieldName"),
					ScriptEngine::NewString(fieldName.c_str())
				);

				

				MonoMethod* method = editorInstance->GetScriptClass()->GetMethod("OnEditorDraw", 0);
				if (method) {
					editorInstance->GetScriptClass()->InvokeMethod(
						managedObject,
						method
					);
				}

				if (!wasSet) {
					//ScriptFieldInstance& fieldInstance = ScriptEngine::GetScriptFieldMap(entity)[fieldName];
					//fieldInstance.Field = field;
					//fieldInstance.SetValue(target);
				}
			}
		}		
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		if (s_Data->EntityInstances.find(uuid) != s_Data->EntityInstances.end())
			return s_Data->EntityInstances.at(uuid)->GetManagedObject();

		return nullptr;
		//SHADO_CORE_ASSERT(s_Data->EntityInstances.find(uuid) != s_Data->EntityInstances.end(), "");
	}

	MonoDomain* ScriptEngine::GetAppDomain()
	{
		return s_Data->AppDomain;
	}

	MonoDomain* ScriptEngine::GetRootDomain()
	{
		return s_Data->RootDomain;
	}

	const ScriptClass& ScriptEngine::GetEntityClassType()
	{
		return s_Data->EntityClass;
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass, MonoMethod* ctor, void** args)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		if (ctor == nullptr && args == nullptr)
			mono_runtime_object_init(instance);
		else
			mono_runtime_invoke(ctor, instance, args, nullptr);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage,
			m_ClassNamespace.c_str(),
			m_ClassName.c_str());
	}

	ScriptClass::ScriptClass(MonoClass* klass)
		: m_MonoClass(klass)
	{
		m_ClassNamespace = mono_class_get_namespace(klass);
		m_ClassName = mono_class_get_name(klass);
	}
		
	MonoObject* ScriptClass::Instantiate(MonoMethod* ctor, void** args)
	{
		return ScriptEngine::InstantiateClass(m_MonoClass, ctor, args);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = NULL;
		MonoObject* result = nullptr;
		
		if (method && instance) {
			result = mono_runtime_invoke(method, instance, params, &exception);
		} else if (method && !instance) {
			const char* methodName = mono_method_get_name(method);
			SHADO_CORE_ERROR("Attempting to call {0} on null object {1}", methodName, m_ClassName);
		}

		if (exception) {
			bool info = true;
			void* params[] = {
				&info
			};

			MonoString* str = (MonoString*)mono_runtime_invoke(
				mono_class_get_method_from_name(mono_get_exception_class(),
					"ToString", 0),
				exception, nullptr, nullptr
			);
			std::string message = mono_string_to_utf8(str);
			SHADO_CORE_ERROR("C# Threw and Exception: {0}", message);
		}

		return result;
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity, bool handleStrongRef)
		: m_ScriptClass(scriptClass)
	{
		//m_Instance = scriptClass->Instantiate();
		MonoObject* scriptInstance = scriptClass->Instantiate();
		if (handleStrongRef)
			m_GCHandle = mono_gchandle_new(scriptInstance, false);
		else
			m_GCHandle = mono_gchandle_new_weakref(scriptInstance, false);
		m_HandleStrongRef = handleStrongRef;

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		m_OnDrawMethod = scriptClass->GetMethod("OnDraw", 0);
		m_OnDestroyedMethod = scriptClass->GetMethod("OnDestroy", 0);

		// Call Entity constructor
		{
			UUID entityID = entity.getUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(GetManagedObject(), m_Constructor, &param);
		}
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, bool handleStrongRef)
		: m_ScriptClass(scriptClass), m_HandleStrongRef(handleStrongRef)
	{
		//m_Instance = scriptClass->Instantiate();
		MonoObject* scriptInstance = scriptClass->Instantiate();

		if (handleStrongRef)
			m_GCHandle = mono_gchandle_new(scriptInstance, false);
		else
			m_GCHandle = mono_gchandle_new_weakref(scriptInstance, false);
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, MonoObject* object, bool handleStrongRef)
		: m_ScriptClass(scriptClass) //m_Instance(object)
	{
		//m_GCHandle = mono_gchandle_new_weakref(object, false);
		if (handleStrongRef)
			m_GCHandle = mono_gchandle_new(object, false);
		else
			m_GCHandle = mono_gchandle_new_weakref(object, false);
		m_HandleStrongRef = handleStrongRef;
		
		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 0);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		m_OnDrawMethod = scriptClass->GetMethod("OnDraw", 0);
		m_OnDestroyedMethod = scriptClass->GetMethod("OnDestroy", 0);
	}

	ScriptInstance::~ScriptInstance() {
		if (m_HandleStrongRef)
			mono_gchandle_free(m_GCHandle);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(GetManagedObject(), m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(GetManagedObject(), m_OnUpdateMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnDraw() {
		if (m_OnDrawMethod)
		{
			m_ScriptClass->InvokeMethod(GetManagedObject(), m_OnDrawMethod);
		}
	}

	void ScriptInstance::InvokeOnDestroy() {
		if (m_OnDestroyedMethod)
		{
			m_ScriptClass->InvokeMethod(GetManagedObject(), m_OnDestroyedMethod);
		}
	}

	MonoObject* ScriptInstance::GetManagedObject() const {
		return mono_gchandle_get_target(m_GCHandle);
	}

	uint32_t ScriptInstance::GetGCHandle() const {
		return m_GCHandle;
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;

		MonoObject* instance = GetManagedObject();
		if (!instance)
			return false;
		
		mono_field_get_value(instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		MonoObject* instance = GetManagedObject();
		if (!instance)
			return false;
		
		mono_field_set_value(instance, field.ClassField, (void*)value);
		return true;
	}

}
