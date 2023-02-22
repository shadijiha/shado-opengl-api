#include "script/ScriptGlue.h"
#include "script/ScriptEngine.h"

#include "ui/UUID.h"
#include "Events/KeyCodes.h"
#include "Events/input.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

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

#define SHADO_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Shado.InternalCalls::" #Name, Name)

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

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::isKeyPressed(keycode);
	}

	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Hazel.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					SHADO_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
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
		SHADO_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		SHADO_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		SHADO_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

		SHADO_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

}