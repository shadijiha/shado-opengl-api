#include "script/ScriptGlue.h"
#include "script/ScriptEngine.h"

#include "ui/UUID.h"
#include "Events/KeyCodes.h"
#include "Events/input.h"

#include "scene/Scene.h"
#include "scene/Entity.h"
#include "project/Project.h"
#include "ui/UI.h"

#include "physics/Physics2D.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"

#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>

namespace Shado {

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;

#define SHADO_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Shado.InternalCalls::" #Name, Name)
#define SHADO_ADD_UI_INTERNAL_CALL(Name) mono_add_internal_call("Shado.Editor.UI::" #Name, Name)

	static void NativeLog(MonoString* string, int parameter)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		SHADO_INFO("{0}, {1}", str, parameter);
		std::cout << str << ", " << parameter << std::endl;
	}

	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		SHADO_CORE_WARN("Value: {0}", *parameter);
		*outResult = glm::normalize(*parameter);
	}

	static float NativeLog_VectorDot(glm::vec3* parameter)
	{
		SHADO_CORE_WARN("Value: {0}", *parameter);
		return glm::dot(*parameter, *parameter);
	}

	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SHADO_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "");
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static void Entity_RemoveComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SHADO_CORE_ASSERT(s_EntityRemoveComponentFuncs.find(managedType) != s_EntityRemoveComponentFuncs.end(), "");
		s_EntityRemoveComponentFuncs.at(managedType)(entity);
	}

	static void Entity_AddComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SHADO_CORE_ASSERT(s_EntityAddComponentFuncs.find(managedType) != s_EntityAddComponentFuncs.end(), "");
		s_EntityAddComponentFuncs.at(managedType)(entity);
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->findEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.getUUID();
	}

	/**
	* Tag
	*/
	static void TagComponent_GetTag(UUID entityID, MonoString** outName) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		*outName = ScriptEngine::NewString(entity.getComponent<TagComponent>().tag.c_str());
	}

	static void TagComponent_SetTag(UUID entityID, MonoString** refName) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<TagComponent>().tag = mono_string_to_utf8(*refName);
	}

	/**
	* Transform
	*/
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		*outTranslation = entity.getComponent<TransformComponent>().position;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<TransformComponent>().position = *translation;
	}

	static void TransformComponent_GetRotation(UUID entityID, glm::vec3* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		*outRotation = entity.getComponent<TransformComponent>().rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<TransformComponent>().rotation = *rotation;
	}

	static void TransformComponent_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		*outScale = entity.getComponent<TransformComponent>().scale;
	}

	static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<TransformComponent>().scale = *scale;
	}

	/**
	* Sprite Renderer
	*/
	static void SpriteRendererComponent_GetColour(UUID entityID, glm::vec4* refColour, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = mono_string_to_utf8(klass);
		if (klassName == "SpriteRendererComponent")
			*refColour = entity.getComponent<SpriteRendererComponent>().color;
		else if (klassName == "CircleRendererComponent")
			*refColour = entity.getComponent<CircleRendererComponent>().color;
		else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}

	static void SpriteRendererComponent_SetColour(UUID entityID, glm::vec4* refColour, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = mono_string_to_utf8(klass);
		if (klassName == "SpriteRendererComponent")
			entity.getComponent<SpriteRendererComponent>().color = *refColour;
		else if (klassName == "CircleRendererComponent")
			entity.getComponent<CircleRendererComponent>().color = *refColour;
		else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}

	static MonoObject* SpriteRendererComponent_GetTexture(UUID entityID, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		Texture2D* ptr = nullptr;
		std::string klassName = mono_string_to_utf8(klass);
		if (klassName == "SpriteRendererComponent") {
			Ref<Texture2D> ref = entity.getComponent<SpriteRendererComponent>().texture;
			ptr = ref == nullptr ? nullptr : ref.get();
		} else if (klassName == "CircleRendererComponent") {
			Ref<Texture2D> ref = entity.getComponent<CircleRendererComponent>().texture;
			ptr = ref == nullptr ? nullptr : ref.get();
		} else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);

		if (ptr == nullptr)
			return nullptr;

		// Otherwise construct the C# object
		ScriptClass klassType = ScriptClass("Shado", "Texture2D");
		MonoMethod* ctor = klassType.GetMethod(".ctor", 2);
		MonoObject* instance = klassType.Instantiate();

		void* param[] = {
			ptr,
			ScriptEngine::NewString((ptr->getFilePath()).c_str())
		};
		klassType.InvokeMethod(instance, ctor, param);

		return instance;
	}

	static void SpriteRendererComponent_SetTexture(UUID entityID, Texture2D* texturePtr, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = mono_string_to_utf8(klass);
		if (klassName == "SpriteRendererComponent") {
			entity.getComponent<SpriteRendererComponent>().texture = Ref<Texture2D>(texturePtr);
		} else if (klassName == "CircleRendererComponent") {
			entity.getComponent<CircleRendererComponent>().texture = Ref<Texture2D>(texturePtr);
		} else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}

	static void SpriteRendererComponent_GetTilingFactor(UUID entityID, float* outTiling, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = mono_string_to_utf8(klass);
		if (klassName == "SpriteRendererComponent")
			*outTiling = entity.getComponent<SpriteRendererComponent>().tilingFactor;
		else if (klassName == "CircleRendererComponent")
			*outTiling = entity.getComponent<CircleRendererComponent>().tilingFactor;
		else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}

	static void SpriteRendererComponent_SetTilingFactor(UUID entityID, float* refTiling, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = mono_string_to_utf8(klass);
		if (klassName == "SpriteRendererComponent")
			entity.getComponent<SpriteRendererComponent>().tilingFactor = *refTiling;
		else if (klassName == "CircleRendererComponent")
			entity.getComponent<CircleRendererComponent>().tilingFactor = *refTiling;
		else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}

	/**
	* Cricle Renderer
	*/
	static void CircleRendererComponent_GetFloatValue(UUID entityID, MonoString* fieldName, float* value)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string field = mono_string_to_utf8(fieldName);
		if (field == "fade")
			*value = entity.getComponent<CircleRendererComponent>().fade;
		else if (field == "thickness")
			*value = entity.getComponent<CircleRendererComponent>().thickness;
		else
			SHADO_ERROR("Unknown field name {0}", field);
	}

	static void CircleRendererComponent_SetFloatValue(UUID entityID, MonoString* fieldName, float* value)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string field = mono_string_to_utf8(fieldName);
		if (field == "fade")
			entity.getComponent<CircleRendererComponent>().fade = *value;
		else if (field == "thickness")
			entity.getComponent<CircleRendererComponent>().thickness = *value;
		else
			SHADO_ERROR("Unknown field name {0}", field);
	}

	/**
	* Rigidbody2D
	*/
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		auto& rb2d = entity.getComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.runtimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		auto& rb2d = entity.getComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.runtimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		auto& rb2d = entity.getComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.runtimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static RigidBody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		auto& rb2d = entity.getComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.runtimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, RigidBody2DComponent::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		auto& rb2d = entity.getComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.runtimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	/**
	* Input
	*/
	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::isKeyPressed(keycode);
	}

	/**
	* Texture2D
	*/
	static MonoObject* Texture2D_Create(MonoString* filepath) {
		std::filesystem::path path = std::string(mono_string_to_utf8(filepath));

		if (!path.is_absolute())
			path = Project::GetProjectDirectory() / path;

		// Load textre
		Texture2D* texture = new Texture2D(path.string());

		// Create the C# Texture2D object instance
		ScriptClass klass = ScriptClass("Shado", "Texture2D", true);
		MonoMethod* ctor = klass.GetMethod(".ctor", 2);
		void* param[] = {
			&texture,
			filepath
		};
		MonoObject* instance = klass.Instantiate(ctor, param);
		
		klass.InvokeMethod(instance, ctor, param);

		return instance;
	}

	static void Texture2D_Destroy(Texture2D* ptr) {
		// TODO? maybe need a flag in the class that checks if the object was constructed by C# or not
		//delete ptr;
	}


	/**
	 * UI
	 */
	static void Text(MonoString* str) {
		ImGui::Text(mono_string_to_utf8(str));
	}

	static void Image_Native(Texture2D* ptr, glm::vec2 dim, glm::vec2 uv0, glm::vec2 uv1) {
		ImGui::Image((ImTextureID)ptr->getRendererID(), { dim.x, dim.y }, { uv0.x, uv0.y }, { uv1.x, uv1.y});
	}

	static bool Button_Native(MonoString* str, glm::vec2 size) {
		return ImGui::Button(mono_string_to_utf8(str), {size.x, size.y});
	}

	static void Seperator(MonoString* str) {
		ImGui::Text(mono_string_to_utf8(str));
	}

	static bool InputTextFileChoose_Native(MonoString* labelStr, MonoString* textStr, MonoArray* extension, MonoString** outPath) {
	
		std::string label = mono_string_to_utf8(labelStr);
		std::string text = mono_string_to_utf8(textStr);
		std::vector<std::string> extensionCpp;

		for (int i = 0; i < mono_array_length(extension); i++) {
			MonoString* ext = mono_array_get(extension, MonoString*, i);
			extensionCpp.push_back(mono_string_to_utf8(ext));
		}
		
		// TODO: If UI doesn't work propery maybe issue with the ID? typeid()
		bool hasChanged = false;
		std::string pathChanged = "";
		UI::InputTextWithChooseFile(label, text, extensionCpp,
			typeid(labelStr).hash_code(),
			[&hasChanged, &pathChanged](std::string path) {
				hasChanged = true;
				pathChanged = path;
			}
		);

		if (hasChanged)
			*outPath = ScriptEngine::NewString(pathChanged.c_str());
		else
			*outPath = nullptr;

		return hasChanged;
	}

	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Shado.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					SHADO_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] =	[](Entity entity) { return entity.hasComponent<Component>(); };
				s_EntityRemoveComponentFuncs[managedType] = [](Entity entity) {return entity.removeComponent<Component>(); };
				s_EntityAddComponentFuncs[managedType] = [](Entity entity) {return entity.addComponent<Component>(); };
			}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		SHADO_ADD_INTERNAL_CALL(NativeLog);
		SHADO_ADD_INTERNAL_CALL(NativeLog_Vector);
		SHADO_ADD_INTERNAL_CALL(NativeLog_VectorDot);

		SHADO_ADD_INTERNAL_CALL(GetScriptInstance);

		SHADO_ADD_INTERNAL_CALL(Entity_HasComponent);
		SHADO_ADD_INTERNAL_CALL(Entity_RemoveComponent);
		SHADO_ADD_INTERNAL_CALL(Entity_AddComponent);
		SHADO_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		SHADO_ADD_INTERNAL_CALL(TagComponent_GetTag);
		SHADO_ADD_INTERNAL_CALL(TagComponent_SetTag);

		SHADO_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		SHADO_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		SHADO_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		SHADO_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		SHADO_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		SHADO_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColour);
		SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColour);
		SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
		SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
		SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTilingFactor);
		SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTilingFactor);

		SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_GetFloatValue);
		SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_SetFloatValue);

		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

		SHADO_ADD_INTERNAL_CALL(Input_IsKeyDown);

		SHADO_ADD_INTERNAL_CALL(Texture2D_Create);
		SHADO_ADD_INTERNAL_CALL(Texture2D_Destroy);

		SHADO_ADD_UI_INTERNAL_CALL(Text);
		SHADO_ADD_UI_INTERNAL_CALL(Image_Native);
		SHADO_ADD_UI_INTERNAL_CALL(Seperator);
		SHADO_ADD_UI_INTERNAL_CALL(Button_Native);
		SHADO_ADD_UI_INTERNAL_CALL(InputTextFileChoose_Native);
	}

}