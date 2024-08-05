#include "script/ScriptGlue.h"
#include "script/ScriptEngine.h"

#include "ui/UUID.h"
#include "Events/KeyCodes.h"
#include "Events/input.h"

#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/utils/SceneUtils.h"
#include "project/Project.h"
#include "ui/UI.h"
#include "ui/imnodes.h"

#include "physics/Physics2D.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"

#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>

#include "renderer/Framebuffer.h"

namespace Shado {

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;

#define SHADO_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Shado.InternalCalls::" #Name, Name)
#define SHADO_ADD_UI_INTERNAL_CALL(Name) mono_add_internal_call("Shado.Editor.UI::" #Name, Name)
#define SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(Name) mono_add_internal_call("Shado.Editor.NodeEditor::" #Name, Name)

#pragma region  NativeLog
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
#pragma endregion

#pragma region  Entity
	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, std::string("Entity ID ") + std::to_string(entityID) + " is invalid"); // <-- Do no use HasComponent and/or AddComponent in the constructor of a script

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SHADO_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "");
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static void Entity_RemoveComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, std::string("Entity ID ") + std::to_string(entityID) + " is invalid");

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

	static void Entity_Destroy(UUID entityID) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		scene->destroyEntity(entity);
	}

	static uint64_t Entity_CreateEntityId() {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		
		// Copy given entity
		Entity entity = scene->createEntity();
		return entity.getUUID();
		//ScriptComponent& script = entity.addComponent<ScriptComponent>();
		//
		//SHADO_CORE_ASSERT(obj, "");

		//Ref<ScriptClass> klass = CreateRef<ScriptClass>(mono_object_get_class(obj));
		//Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(klass, obj);
		//script.ClassName = instance->GetScriptClass()->GetClassFullName();
		//*obj_id = (uint64_t)entity.getUUID();
		//ScriptEngine::OnCreateEntity(entity, instance);
	}

	static void Entity_InvokeScriptEngineCreate(MonoObject* monoObject, uint64_t entityId) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityId);
		SHADO_CORE_ASSERT(entity, "");

		ScriptComponent& script = entity.addComponent<ScriptComponent>();
		Ref<ScriptClass> klass = CreateRef<ScriptClass>(mono_object_get_class(monoObject));
		Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(klass, monoObject);
		script.ClassName = instance->GetScriptClass()->GetClassFullName();

		ScriptEngine::OnCreateEntity(entity, instance);
	}

	static void Entity_GetChildren(UUID entityId, MonoArray** childrenIds, MonoReflectionType* arrayType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityId);
		SHADO_CORE_ASSERT(entity, "");

		auto children = entity.getChildren();

		 *childrenIds = mono_array_new(
		 	ScriptEngine::GetAppDomain(),
		 	mono_class_from_mono_type(mono_reflection_type_get_type(arrayType)),
		 	children.size());
		 for (int i = 0; i < children.size(); i++) {
		 	mono_array_set(*childrenIds, uint64_t, i, children[i].getUUID());
		 }
	}
#pragma endregion

#pragma region TagComponent
	/**
	* Tag
	*/
	static void TagComponent_GetTag(UUID entityID, MonoString** outName) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "Scene is nullptr");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "Invalid entity ID {0}", entityID);

		*outName = ScriptEngine::NewString(entity.getComponent<TagComponent>().tag.c_str());
	}

	static void TagComponent_SetTag(UUID entityID, MonoString** refName) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "Scene is nullptr");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "Invalid entity ID {0}", entityID);

		entity.getComponent<TagComponent>().tag = ScriptEngine::MonoStrToUT8(*refName);
	}
#pragma endregion

#pragma region TransformComponent
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

	static uint64_t TransformComponent_GetParentId(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		return entity.getComponent<TransformComponent>().parentId;
	}

	static void TransformComponent_SetParentId(UUID entityID, uint64_t newParentId)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		Entity parent = scene->getEntityById(newParentId);
		SHADO_CORE_ASSERT(parent, "");

		entity.getComponent<TransformComponent>().parentId = parent.getUUID();
	}
#pragma endregion

#pragma region SpriteRendererComponent
	/**
	* Sprite Renderer
	*/
	static void SpriteRendererComponent_GetColour(UUID entityID, glm::vec4* refColour, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = ScriptEngine::MonoStrToUT8(klass);
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

		std::string klassName = ScriptEngine::MonoStrToUT8(klass);
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
		std::string klassName = ScriptEngine::MonoStrToUT8(klass);
		if (klassName == "SpriteRendererComponent") {
			Ref<Texture2D> ref = entity.getComponent<SpriteRendererComponent>().texture;
			ptr = ref ? nullptr : ref.Raw();
		} else if (klassName == "CircleRendererComponent") {
			Ref<Texture2D> ref = entity.getComponent<CircleRendererComponent>().texture;
			ptr = ref ? nullptr : ref.Raw();
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

		std::string klassName = ScriptEngine::MonoStrToUT8(klass);
		if (klassName == "SpriteRendererComponent") {
			Ref<Texture2D> texture(texturePtr);
			texture.Leak();
			entity.getComponent<SpriteRendererComponent>().texture = texture;
		} else if (klassName == "CircleRendererComponent") {
			Ref<Texture2D> texture(texturePtr);
			texture.Leak();
			entity.getComponent<CircleRendererComponent>().texture = texture;
		} else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}

	static void SpriteRendererComponent_GetTilingFactor(UUID entityID, float* outTiling, MonoString* klass)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string klassName = ScriptEngine::MonoStrToUT8(klass);
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

		std::string klassName = ScriptEngine::MonoStrToUT8(klass);
		if (klassName == "SpriteRendererComponent")
			entity.getComponent<SpriteRendererComponent>().tilingFactor = *refTiling;
		else if (klassName == "CircleRendererComponent")
			entity.getComponent<CircleRendererComponent>().tilingFactor = *refTiling;
		else
			SHADO_ERROR("Unknown Sprite class {0}", klassName);
	}
#pragma endregion

#pragma region CircleRendererComponent
	/**
	* Cricle Renderer
	*/
	static void CircleRendererComponent_GetFloatValue(UUID entityID, MonoString* fieldName, float* value)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		std::string field = ScriptEngine::MonoStrToUT8(fieldName);
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

		std::string field = ScriptEngine::MonoStrToUT8(fieldName);
		if (field == "fade")
			entity.getComponent<CircleRendererComponent>().fade = *value;
		else if (field == "thickness")
			entity.getComponent<CircleRendererComponent>().thickness = *value;
		else
			SHADO_ERROR("Unknown field name {0}", field);
	}
#pragma endregion

#pragma region LineRendererComponent
static void LineRendererComponent_GetTarget(uint64_t entityID, glm::vec3* outTarget)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	SHADO_CORE_ASSERT(scene, "");
	Entity entity = scene->getEntityById(entityID);
	SHADO_CORE_ASSERT(entity, "");

	*outTarget = entity.getComponent<LineRendererComponent>().target;
}

static void LineRendererComponent_SetTarget(uint64_t entityID, glm::vec3* target)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	SHADO_CORE_ASSERT(scene, "");
	Entity entity = scene->getEntityById(entityID);
	SHADO_CORE_ASSERT(entity, "");

	entity.getComponent<LineRendererComponent>().target = *target;
}

static void LineRendererComponent_GetColour(uint64_t entityID, glm::vec4* outColour)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	SHADO_CORE_ASSERT(scene, "");
	Entity entity = scene->getEntityById(entityID);
	SHADO_CORE_ASSERT(entity, "");

	*outColour = entity.getComponent<LineRendererComponent>().color;
}

static void LineRendererComponent_SetColour(uint64_t entityID, glm::vec4* colour)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	SHADO_CORE_ASSERT(scene, "");
	Entity entity = scene->getEntityById(entityID);
	SHADO_CORE_ASSERT(entity, "");

	entity.getComponent<LineRendererComponent>().color = *colour;
}
	
#pragma endregion 
	
#pragma region RigidBody2DComponent
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
		rb2d.type = bodyType;
		b2Body* body = (b2Body*)rb2d.runtimeBody;
		
		if (body != nullptr) {
			body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
		}
	}

	/**
	* BoxCollider2DComponent
	*/
	enum class BoxCollider2DComponent_FieldToQuery
	{
		Density = 0,
		Friction, Restitution, RestitutionThreshold,
		Offset, Size,
		CircleCollider, BoxCollider
	};

	static void BoxCollider2DComponent_GetVec2(UUID entityID, BoxCollider2DComponent_FieldToQuery field, glm::vec2* outValue, BoxCollider2DComponent_FieldToQuery componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		BoxCollider2DComponent& bc = (componentType == BoxCollider2DComponent_FieldToQuery::BoxCollider
				? entity.getComponent<BoxCollider2DComponent>() : 
				  (BoxCollider2DComponent&)entity.getComponent<CircleCollider2DComponent>()
			);
		
		switch (field) {
			case BoxCollider2DComponent_FieldToQuery::Offset: 
				*outValue = bc.offset;
				break;
			case BoxCollider2DComponent_FieldToQuery::Size:
				*outValue = bc.size;
				break;
			default:
				SHADO_CORE_ERROR("Invalid BoxCollider2DComponent query field {0}", (int)field);
				break;
		}
	}

	static void BoxCollider2DComponent_SetVec2(UUID entityID, BoxCollider2DComponent_FieldToQuery field, glm::vec2* refValue, BoxCollider2DComponent_FieldToQuery componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		BoxCollider2DComponent& bc = (componentType == BoxCollider2DComponent_FieldToQuery::BoxCollider
			? entity.getComponent<BoxCollider2DComponent>() :
			(BoxCollider2DComponent&)entity.getComponent<CircleCollider2DComponent>()
			);

		switch (field) {
		case BoxCollider2DComponent_FieldToQuery::Offset:
			bc.offset = *refValue;
			break;
		case BoxCollider2DComponent_FieldToQuery::Size:
			bc.size = *refValue;
			break;
		default:
			SHADO_CORE_ERROR("Invalid BoxCollider2DComponent query field {0}", (int)field);
			break;
		}
	}

	static void BoxCollider2DComponent_GetFloat(UUID entityID, BoxCollider2DComponent_FieldToQuery field, float* value, BoxCollider2DComponent_FieldToQuery componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		BoxCollider2DComponent& bc = (componentType == BoxCollider2DComponent_FieldToQuery::BoxCollider
			? entity.getComponent<BoxCollider2DComponent>() :
			(BoxCollider2DComponent&)entity.getComponent<CircleCollider2DComponent>()
			);

		switch (field) {
		case BoxCollider2DComponent_FieldToQuery::Density:
			*value = bc.density;
			break;
		case BoxCollider2DComponent_FieldToQuery::Friction:
			*value = bc.friction;
			break;
		case BoxCollider2DComponent_FieldToQuery::Restitution:
			*value = bc.restitution;
			break;
		case BoxCollider2DComponent_FieldToQuery::RestitutionThreshold:
			*value = bc.restitutionThreshold;
			break;
		default:
			SHADO_CORE_ERROR("Invalid BoxCollider2DComponent query field {0}", (int)field);
			break;
		}
	}

	static void BoxCollider2DComponent_SetFloat(UUID entityID, BoxCollider2DComponent_FieldToQuery field, float* value, BoxCollider2DComponent_FieldToQuery componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		BoxCollider2DComponent& bc = (componentType == BoxCollider2DComponent_FieldToQuery::BoxCollider
			? entity.getComponent<BoxCollider2DComponent>() :
			(BoxCollider2DComponent&)entity.getComponent<CircleCollider2DComponent>()
			);

		switch (field) {
		case BoxCollider2DComponent_FieldToQuery::Density:
			bc.density = *value;
			break;
		case BoxCollider2DComponent_FieldToQuery::Friction:
			bc.friction = *value;
			break;
		case BoxCollider2DComponent_FieldToQuery::Restitution:
			bc.restitution = *value;
			break;
		case BoxCollider2DComponent_FieldToQuery::RestitutionThreshold:
			bc.restitutionThreshold = *value;
			break;
		default:
			SHADO_CORE_ERROR("Invalid BoxCollider2DComponent query field {0}", (int)field);
			break;
		}
	}
#pragma endregion

#pragma region CameraComponent
	/**
	 * Camera component
	 */
	static void CameraComponent_GetPrimary(UUID entityID, bool* primary) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		*primary = entity.getComponent<CameraComponent>().primary;
	}

	static void CameraComponent_SetPrimary(UUID entityID, bool* primary) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<CameraComponent>().primary = *primary;
	}

	static void CameraComponent_GetType(UUID entityID, CameraComponent::Type* type) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		*type = entity.getComponent<CameraComponent>().type;
	}

	static void CameraComponent_SetType(UUID entityID, CameraComponent::Type* type) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<CameraComponent>().setType(*type);
	}

	static void CameraComponent_SetViewport(UUID entityID, uint32_t width, uint32_t height) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		entity.getComponent<CameraComponent>().setViewportSize(width, height);
	}
#pragma endregion

#pragma region ScriptComponent 

	static MonoString* ScriptComponent_GetClassName(uint64_t entityID) {
		Scene* scene = ScriptEngine::GetSceneContext();
		SHADO_CORE_ASSERT(scene, "");
		Entity entity = scene->getEntityById(entityID);
		SHADO_CORE_ASSERT(entity, "");

		ScriptComponent& scriptComponent = entity.getComponent<ScriptComponent>();
		return ScriptEngine::NewString(scriptComponent.ClassName.c_str());
	}

#pragma endregion

#pragma region Input
	/**
	* Input
	*/
	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::isKeyPressed(keycode);
	}

	static void Input_GetMousePos(glm::vec2* result)
	{
		auto [x, y] = Input::getMousePosition();
		result->x = x;
		result->y = y;
	}
#pragma endregion

#pragma region Application
	static void Application_Close() {
		Application::get().close();
	}

	static void Application_GetImGuiWindowSize(MonoString* windowName, glm::vec2* result) {
		// Small hack to get around ImGui not having a way to get a given window size
		ImGuiContext* context = ImGui::GetCurrentContext();
		ImGuiWindow* currentWin = ImGui::GetCurrentWindow();
		auto* targetWin = ImGui::FindWindowByName(ScriptEngine::MonoStrToUT8(windowName).c_str());

		context->CurrentWindow = targetWin;
		*result = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
		
		context->CurrentWindow = currentWin;
	}

	// TODO: maybe use a mutex?
	static bool Application_IsImGuiWindowHovered(MonoString* windowName) {
		ImGuiContext* context = ImGui::GetCurrentContext();
		auto* currentWin = ImGui::GetCurrentWindow();
		auto* targetWin = ImGui::FindWindowByName(ScriptEngine::MonoStrToUT8(windowName).c_str());

		context->CurrentWindow = targetWin;
		bool result = ImGui::IsWindowHovered();
		context->CurrentWindow = currentWin;

		return result;		
	}
#pragma endregion

#pragma region Window
	static void Window_GetSize(glm::vec2* result) {
		*result = { Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight() };
	}

	static void Window_SetSize(glm::vec2 size) {
		Application::get().getWindow().resize(size.x, size.y);
	}
	
	static int Window_Mode() {
		return (int)Application::get().getWindow().getMode();
	}

	static void Window_SetMode(int mode) {
		Application::get().getWindow().setMode((WindowMode)mode);
	}

	static MonoString* Window_GetTitle() {
		return ScriptEngine::NewString(Application::get().getWindow().getTitle().c_str());
	}
	
	static void Window_SetTitle(MonoString* title) {
		Application::get().getWindow().setTitle(ScriptEngine::MonoStrToUT8(title));
	}

	static bool Window_GetVSync() {
		return Application::get().getWindow().isVSync();
	}
	
	static void Window_SetVSync(bool enabled) {
		Application::get().getWindow().setVSync(enabled);
	}

	static void Window_GetPosition(glm::vec2* result) {
		result->x = Application::get().getWindow().getPosX();
		result->y = Application::get().getWindow().getPosY();
	}

	static void Window_SetOpacity(float opacity) {
		Application::get().getWindow().setOpacity(opacity);
	}	

#pragma endregion

#pragma region Framebuffer
	static uint32 Framebuffer_GetColorAttachmentRendererID(Framebuffer* nativePtr, uint32_t index) {
		return nativePtr->getColorAttachmentRendererID(index);
	}

	static Framebuffer* Framebuffer_Create(uint32_t width, uint32_t height, uint32_t samples, MonoArray* attachments, bool swapChainTarget) {
		FramebufferSpecification spec;
		spec.Width = width;
		spec.Height = height;
		spec.Samples = samples;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,  FramebufferTextureFormat::DEPTH24STENCIL8 };
		spec.SwapChainTarget = swapChainTarget;

		//uint32_t* attachmentsData = mono_array_addr(attachments, uint32_t, 0);
		//for (uint32_t i = 0; i < mono_array_length(attachments); i++)
		//	spec.Attachments.Attachments.push_back((FramebufferTextureFormat)attachmentsData[i]);
		
		return snew(Framebuffer) Framebuffer(spec);
	}

	static void Framebuffer_Invalidate(Framebuffer* nativePtr) {
		nativePtr->invalidate();
	}

	static void Framebuffer_Bind(Framebuffer* nativePtr) {
		nativePtr->bind();
	}

	static void Framebuffer_Unbind(Framebuffer* nativePtr) {
		nativePtr->unbind();
	}

	static void Framebuffer_ClearAttachment(Framebuffer* nativePtr, uint32_t attachmentIndex, int value) {
		nativePtr->clearAttachment(attachmentIndex, value);
	}

	static void Framebuffer_Resize(Framebuffer* nativePtr, uint32_t width, uint32_t height) {
		nativePtr->resize(width, height);
	}

	static void Framebuffer_Destroy(Framebuffer* nativePtr) {
		sdelete(nativePtr);
	}
#pragma endregion 
	
#pragma region Scene
	static MonoArray* Scene_GetAllEntities() {
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene) {
			SHADO_CORE_WARN("Scene is null! ScriptEngine context is not set when editor mode. Consider setting it up in the future?");
			return nullptr;
		}

		auto view = scene->getRegistry().view<ScriptComponent>();
		MonoArray* result = mono_array_new(ScriptEngine::GetAppDomain(), ScriptEngine::GetEntityClassType().GetMonoClass(), view.size());

		int i = 0;
		for (auto entity : view) {
			Entity e = { entity, scene };
			UUID uuid = e.getUUID();
			MonoObject* obj = ScriptEngine::GetEntityScriptInstance(uuid)->GetManagedObject();
			mono_array_set(result, MonoObject*, i, obj);
			i += 1;
		}

		return result;
	}
#pragma endregion

#pragma region Texture2D
	/**
	* Texture2D
	*/
	static void Texture2D_Create(MonoString* filepath, Texture2D** ptr) {
		std::filesystem::path path = ScriptEngine::MonoStrToUT8(filepath);

		if (!path.is_absolute())
			path = Project::GetProjectDirectory() / path;

		// Load textre
		Texture2D* texture = snew(Texture2D) Texture2D(path.string());
		*ptr = texture;
	}

	static void Texture2D_Destroy(Texture2D* ptr) {
		// TODO? maybe need a flag in the class that checks if the object was constructed by C# or not
		//delete ptr;
	}

	static void Texture2D_Reset(Texture2D* ptr, MonoString* filepath, Texture2D** newHandle) {
		sdelete(ptr);
		*newHandle = snew(Texture2D) Texture2D(ScriptEngine::MonoStrToUT8(filepath));
	}
#pragma endregion

#pragma region Shader
	/**
	 * Shader
	 */
	static void Shader_CreateShader(MonoString* filepath, Shader** native) {
		std::filesystem::path path = ScriptEngine::MonoStrToUT8(filepath);

		if (!path.is_absolute())
			path = Project::GetProjectDirectory() / path;

		// Generate shader
		*native = snew(Shader) Shader(path.string());
	}
	static void Shader_SetInt(Shader* native, MonoString* name, int* value) {
		if (native)
			native->setInt(ScriptEngine::MonoStrToUT8(name), *value);
	}
	static void Shader_SetIntArray(Shader* native, MonoString* name, MonoArray* value) {
		if (native) {
			native->setIntArray(mono_string_to_utf8(name),
				mono_array_addr(value, int, 0),
				mono_array_length(value)
			);
		}
	}
	static void Shader_SetFloat(Shader* native, MonoString* name, float* value) {
		if (native)
			native->setFloat(mono_string_to_utf8(name), *value);
	}
	static void Shader_SetFloat3(Shader* native, MonoString* name, glm::vec3* value) {
		if (native)
			native->setFloat3(mono_string_to_utf8(name), *value);
	}
	static void Shader_SetFloat4(Shader* native, MonoString* name, glm::vec4* value) {
		if (native)
			native->setFloat4(ScriptEngine::MonoStrToUT8(name), *value);
	}
	static void Shader_Reset(Shader* oldNative, MonoString* filepath, Shader** newNative) {
		sdelete( oldNative);
		Shader_CreateShader(filepath, newNative);
	}
	static void Shader_Destroy(Shader* oldNative) {
		sdelete( oldNative);
	}
#pragma endregion
	
#pragma region Log
	/**
	 * Log
	 */
	enum class Log_Type {
		Info = 0, Warn, Error, Critical, Trace
	};
	static void Log_Log(MonoString* message, Log_Type type) {
		std::string msg = ScriptEngine::MonoStrToUT8(message);

		switch (type)
		{
		case Shado::Log_Type::Info:
			SHADO_INFO(msg);
			break;
		case Shado::Log_Type::Warn:
			SHADO_WARN(msg);
			break;
		case Shado::Log_Type::Error:
			SHADO_ERROR(msg);
			break;
		case Shado::Log_Type::Critical:
			SHADO_CRITICAL(msg);
			break;
		case Shado::Log_Type::Trace:
			SHADO_TRACE(msg);
			break;
		default:
			SHADO_CORE_ERROR("Unknown log type {0} for message {1}", (int)type, msg);
			break;
		}
	}
#pragma endregion

#pragma region Renderer
	/**
	 * Renderer Glue
	 */
	static void Renderer_DrawQuad(glm::vec3* pos, glm::vec3* scale, glm::vec4* colour) {
		Renderer2D::DrawQuad(*pos, *scale, *colour);
	}

	static void Renderer_DrawRotatedQuad(glm::vec3* pos, glm::vec3* scale, glm::vec3* rotation, glm::vec4* colour) {
		Renderer2D::DrawRotatedQuad(*pos, *scale, *rotation, *colour);
	}

	static void Renderer_DrawLine(glm::vec3* p0, glm::vec3* p1, glm::vec4* colour) {
		Renderer2D::DrawLine(*p0, *p1, *colour);
	}

	static void Renderer_DrawQuadShader(glm::vec3* pos, glm::vec3* scale, glm::vec4* colour, Shader* shader) {
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), *pos)
			* glm::scale(glm::mat4(1.0f), *scale);

		Renderer2D::DrawQuad(transform, *shader, *colour);
	}
#pragma endregion

#pragma region UI
	/**
	 * UI
	 */
	static void Begin(MonoString* label) {
		ImGui::Begin(ScriptEngine::MonoStrToUT8(label).c_str());
	}

	static void End() {
		ImGui::End();
	}
	
	static void Text(MonoString* str) {
		ImGui::Text(ScriptEngine::MonoStrToUT8(str).c_str());
	}

	static void Image_Native(Texture2D* ptr, glm::vec2 dim, glm::vec2 uv0, glm::vec2 uv1) {
		ImGui::Image((ImTextureID)ptr->getRendererID(), { dim.x, dim.y }, { uv0.x, uv0.y }, { uv1.x, uv1.y});
	}

	static void Image_Framebuffer_Native(uint32_t fbColorAttachmentRendererId, uint32_t width, uint32_t height) {
		ImGui::Image((ImTextureID)fbColorAttachmentRendererId, { (float)width, (float)height }, { 0, 1 }, { 1, 0 });
	}
	
	static bool Button_Native(MonoString* str, glm::vec2 size) {
		return ImGui::Button(ScriptEngine::MonoStrToUT8(str).c_str(), {size.x, size.y});
	}

	static void Separator() {
		ImGui::Separator();
	}

	static bool InputText(MonoString* label, MonoString** value) {
		const std::string labelStr = ScriptEngine::MonoStrToUT8(label);
		std::string valueStr = ScriptEngine::MonoStrToUT8(*value);
		const std::string oldValue = valueStr;

		bool isUsed = UI::InputTextControl(labelStr, valueStr);
		if (oldValue != valueStr)
			*value = ScriptEngine::NewString(valueStr.c_str());
		return isUsed;
	}

	static bool InputInt(MonoString* label, int* value, float speed, int min, int max, MonoString* format) {
		const std::string labelStr = ScriptEngine::MonoStrToUT8(label);
		
		return ImGui::DragInt(labelStr.c_str(), value, speed, min, max, ScriptEngine::MonoStrToUT8(format).c_str());
	}
	
	static bool InputFloat(MonoString* label, float* value, float speed, float min, float max, MonoString* format) {
		const std::string labelStr = ScriptEngine::MonoStrToUT8(label);
		
		return ImGui::DragFloat(labelStr.c_str(), value, speed, min, max, ScriptEngine::MonoStrToUT8(format).c_str());
	}

	static unsigned int GetId(MonoString* label) {
		return ImGui::GetID(ScriptEngine::MonoStrToUT8(label).c_str());
	}

	static void SetFocus(unsigned int id) {
		ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
	}

	static bool InputTextFileChoose_Native(MonoString* labelStr, MonoString* textStr, MonoArray* extension, MonoString** outPath, UI::FileChooserType type) {
	
		std::string label = ScriptEngine::MonoStrToUT8(labelStr);
		std::string text = ScriptEngine::MonoStrToUT8(textStr);
		std::vector<std::string> extensionCpp;

		for (int i = 0; i < mono_array_length(extension); i++) {
			MonoString* ext = mono_array_get(extension, MonoString*, i);
			extensionCpp.push_back(ScriptEngine::MonoStrToUT8(ext));
		}
		
		// TODO: If UI doesn't work propery maybe issue with the ID? typeid()
		bool hasChanged = false;
		std::string pathChanged = "";
		UI::InputTextWithChooseFile(label, text, extensionCpp,
			(int)&label[0],
			[&hasChanged, &pathChanged](std::string path) {
				hasChanged = true;
				pathChanged = path;
			}, type
		);

		if (hasChanged)
			*outPath = ScriptEngine::NewString(pathChanged.c_str());
		else
			*outPath = nullptr;

		return hasChanged;
	}

	static MonoString* OpenFileDialog_Native(MonoString* filter, UI::FileChooserType type) {
		std::string result;
		const char* filterStr = ScriptEngine::MonoStrToUT8(filter).c_str();

		// Build filter
		switch (type)
		{
		case Shado::UI::FileChooserType::Open:
			result = FileDialogs::openFile(filterStr);
			break;
		case Shado::UI::FileChooserType::Save:
			result = FileDialogs::saveFile(filterStr);
			break;
		case Shado::UI::FileChooserType::Folder:
			result = FileDialogs::chooseFolder();
			break;
		default:
			SHADO_CORE_ERROR("Invalid file chooser dialog type {0}", (int)type);
			break;
		}

		return result.empty() ? nullptr : ScriptEngine::NewString(result.c_str());
	}

	static void Indent(float x) {
		ImGui::Indent(x);
	}

	static void Unindent(float x) {
		ImGui::Unindent(x);
	}

	static void NewLine() {
		ImGui::NewLine();
	}

	static void ShowDemoWindow() {
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}

	static void ShowMetricsWindow() {
		static bool show = true;
		ImGui::ShowMetricsWindow(&show);
	}

	static void BeginGroup() {
		ImGui::BeginGroup();
	}

	static void EndGroup() {
		ImGui::EndGroup();
	}

	static bool BeginChild(MonoString* id, glm::vec2 size, bool border, int flags) {
		return ImGui::BeginChild(ScriptEngine::MonoStrToUT8(id).c_str(), {size.x, size.y}, border, (ImGuiWindowFlags)flags);
	}

	static void EndChild() {
		ImGui::EndChild();
	}

	static void GetContentRegionAvail_Native(glm::vec2* result) {
		*result = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
	}

	static bool BeginMenu(MonoString* label, bool enabled) {
		return ImGui::BeginMenu(ScriptEngine::MonoStrToUT8(label).c_str(), enabled);
	}

	static void EndMenu() {
		ImGui::EndMenu();
	}

	static bool MenuItem(MonoString* label, MonoString* shortcut, bool selected, bool enabled) {
		return ImGui::MenuItem(
			ScriptEngine::MonoStrToUT8(label).c_str(),
			shortcut == nullptr ? nullptr : ScriptEngine::MonoStrToUT8(shortcut).c_str(), selected, enabled);
	}
#pragma endregion

#pragma region NodeEditor
	//static void BeginNodeEditor() {
	//	static bool init = false;
	//	if (!init) {
	//		ImNodes::SetCurrentContext(ImNodes::CreateContext());
	//		ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));
	//		init = true;
	//	}
	//	ImNodes::BeginNodeEditor();
	//}

	//static void EndNodeEditor() {
	//	ImNodes::EndNodeEditor();
	//}

	//static void BeginNode(int id) {
	//	ImNodes::BeginNode(id);
	//}

	//static void EndNode() {
	//	ImNodes::EndNode();
	//}

	//static void BeginNodeTitleBar() {
	//	ImNodes::BeginNodeTitleBar();
	//}

	//static void EndNodeTitleBar() {
	//	ImNodes::EndNodeTitleBar();
	//}

	//static void BeginInputAttribute(int id) {
	//	ImNodes::BeginInputAttribute(id);
	//}

	//static void EndInputAttribute() {
	//	ImNodes::EndInputAttribute();
	//}

	//static void BeginOutputAttribute(int id) {
	//	ImNodes::BeginOutputAttribute(id);
	//}

	//static void EndOutputAttribute() {
	//	ImNodes::EndOutputAttribute();
	//}

	static bool IsLinkCreated(int* startAttr, int* endAttr)
	{
		return ImNodes::IsLinkCreated(startAttr, endAttr);
	}
#pragma endregion

	/**
	 *
	 */
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
		{
			SHADO_ADD_INTERNAL_CALL(NativeLog);
			SHADO_ADD_INTERNAL_CALL(NativeLog_Vector);
			SHADO_ADD_INTERNAL_CALL(NativeLog_VectorDot);

			SHADO_ADD_INTERNAL_CALL(GetScriptInstance);
		}
		
		{
			SHADO_ADD_INTERNAL_CALL(Entity_HasComponent);
			SHADO_ADD_INTERNAL_CALL(Entity_RemoveComponent);
			SHADO_ADD_INTERNAL_CALL(Entity_AddComponent);
			SHADO_ADD_INTERNAL_CALL(Entity_FindEntityByName);
			SHADO_ADD_INTERNAL_CALL(Entity_Destroy);
			SHADO_ADD_INTERNAL_CALL(Entity_CreateEntityId);
			SHADO_ADD_INTERNAL_CALL(Entity_InvokeScriptEngineCreate);
			SHADO_ADD_INTERNAL_CALL(Entity_GetChildren);
		}
			
		{
			SHADO_ADD_INTERNAL_CALL(TagComponent_GetTag);
			SHADO_ADD_INTERNAL_CALL(TagComponent_SetTag);
		}

		{
			SHADO_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_GetScale);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_SetScale);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_GetParentId);
			SHADO_ADD_INTERNAL_CALL(TransformComponent_SetParentId);
		}

		{
			SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColour);
			SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColour);
			SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
			SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
			SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTilingFactor);
			SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTilingFactor);
		}

		{
			SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_GetFloatValue);
			SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_SetFloatValue);
		}

		{
			SHADO_ADD_INTERNAL_CALL(LineRendererComponent_GetTarget);
			SHADO_ADD_INTERNAL_CALL(LineRendererComponent_SetTarget);
			SHADO_ADD_INTERNAL_CALL(LineRendererComponent_GetColour);
			SHADO_ADD_INTERNAL_CALL(LineRendererComponent_SetColour);
		}
		
		{
			SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
			SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
			SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
			SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
			SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
		}

		{
			SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetVec2);
			SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetVec2);
			SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetFloat);
			SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetFloat);
		}

		{
			SHADO_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
			SHADO_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);
			SHADO_ADD_INTERNAL_CALL(CameraComponent_GetType);
			SHADO_ADD_INTERNAL_CALL(CameraComponent_SetType);
			SHADO_ADD_INTERNAL_CALL(CameraComponent_SetViewport);
		}

		{
			SHADO_ADD_INTERNAL_CALL(ScriptComponent_GetClassName);
		}

		{
			SHADO_ADD_INTERNAL_CALL(Input_IsKeyDown);
			SHADO_ADD_INTERNAL_CALL(Input_GetMousePos);
		}

		{
			SHADO_ADD_INTERNAL_CALL(Application_Close);
			SHADO_ADD_INTERNAL_CALL(Application_GetImGuiWindowSize);
			SHADO_ADD_INTERNAL_CALL(Application_IsImGuiWindowHovered);
		}

		{
			SHADO_ADD_INTERNAL_CALL(Window_GetSize);
			SHADO_ADD_INTERNAL_CALL(Window_SetSize);
			SHADO_ADD_INTERNAL_CALL(Window_Mode);
			SHADO_ADD_INTERNAL_CALL(Window_SetMode);
			SHADO_ADD_INTERNAL_CALL(Window_GetTitle);
			SHADO_ADD_INTERNAL_CALL(Window_SetTitle);
			SHADO_ADD_INTERNAL_CALL(Window_GetVSync);
			SHADO_ADD_INTERNAL_CALL(Window_SetVSync);
			SHADO_ADD_INTERNAL_CALL(Window_GetPosition);
			SHADO_ADD_INTERNAL_CALL(Window_SetOpacity);
		}

		// Framebuffer
		{
			SHADO_ADD_INTERNAL_CALL(Framebuffer_GetColorAttachmentRendererID);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_Create);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_Invalidate);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_Bind);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_Unbind);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_ClearAttachment);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_Resize);
			SHADO_ADD_INTERNAL_CALL(Framebuffer_Destroy);
		}
		
		{
			SHADO_ADD_INTERNAL_CALL(Texture2D_Create);
			SHADO_ADD_INTERNAL_CALL(Texture2D_Destroy);
			SHADO_ADD_INTERNAL_CALL(Texture2D_Reset);
		}
		{
			SHADO_ADD_INTERNAL_CALL(Shader_CreateShader);
			SHADO_ADD_INTERNAL_CALL(Shader_SetInt);
			SHADO_ADD_INTERNAL_CALL(Shader_SetIntArray);
			SHADO_ADD_INTERNAL_CALL(Shader_SetFloat);
			SHADO_ADD_INTERNAL_CALL(Shader_SetFloat3);
			SHADO_ADD_INTERNAL_CALL(Shader_SetFloat4);
			SHADO_ADD_INTERNAL_CALL(Shader_Reset);
			SHADO_ADD_INTERNAL_CALL(Shader_Destroy);
		}
		{
			SHADO_ADD_INTERNAL_CALL(Log_Log);
		}

		{
			SHADO_ADD_INTERNAL_CALL(Renderer_DrawQuad);
			SHADO_ADD_INTERNAL_CALL(Renderer_DrawRotatedQuad);
			SHADO_ADD_INTERNAL_CALL(Renderer_DrawLine);
			SHADO_ADD_INTERNAL_CALL(Renderer_DrawQuadShader);
		}

		// Scene
		{
			SHADO_ADD_INTERNAL_CALL(Scene_GetAllEntities);
		}

		// UI
		{
			SHADO_ADD_UI_INTERNAL_CALL(Begin);
			SHADO_ADD_UI_INTERNAL_CALL(End);
			SHADO_ADD_UI_INTERNAL_CALL(Text);
			SHADO_ADD_UI_INTERNAL_CALL(InputText);
			SHADO_ADD_UI_INTERNAL_CALL(InputInt);
			SHADO_ADD_UI_INTERNAL_CALL(InputFloat);
			SHADO_ADD_UI_INTERNAL_CALL(Image_Native);
			SHADO_ADD_UI_INTERNAL_CALL(Image_Framebuffer_Native);
			SHADO_ADD_UI_INTERNAL_CALL(Separator);
			SHADO_ADD_UI_INTERNAL_CALL(Button_Native);
			SHADO_ADD_UI_INTERNAL_CALL(InputTextFileChoose_Native);
			SHADO_ADD_UI_INTERNAL_CALL(OpenFileDialog_Native);
			SHADO_ADD_UI_INTERNAL_CALL(Indent);
			SHADO_ADD_UI_INTERNAL_CALL(Unindent);
			SHADO_ADD_UI_INTERNAL_CALL(GetId);
			SHADO_ADD_UI_INTERNAL_CALL(SetFocus);
			SHADO_ADD_UI_INTERNAL_CALL(NewLine);
			SHADO_ADD_UI_INTERNAL_CALL(ShowDemoWindow);
			SHADO_ADD_UI_INTERNAL_CALL(ShowMetricsWindow);
			SHADO_ADD_UI_INTERNAL_CALL(BeginGroup);
			SHADO_ADD_UI_INTERNAL_CALL(EndGroup);
			SHADO_ADD_UI_INTERNAL_CALL(BeginChild);
			SHADO_ADD_UI_INTERNAL_CALL(EndChild);
			SHADO_ADD_UI_INTERNAL_CALL(GetContentRegionAvail_Native);
			SHADO_ADD_UI_INTERNAL_CALL(BeginMenu);
			SHADO_ADD_UI_INTERNAL_CALL(EndMenu);
			SHADO_ADD_UI_INTERNAL_CALL(MenuItem);
		}

		{
			{
				using namespace ImNodes;
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(BeginNodeEditor);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(EndNodeEditor);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(BeginNode);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(EndNode);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(BeginNodeTitleBar);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(EndNodeTitleBar);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(BeginInputAttribute);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(EndInputAttribute);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(BeginOutputAttribute);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(EndOutputAttribute);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(Link);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(IsNodeHovered);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(IsLinkHovered);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(IsPinHovered);
				SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(IsLinkDestroyed);
			}
			SHADO_ADD_NODE_EDITOR_INTERNAL_CALL(IsLinkCreated);			
		}
	}
}
