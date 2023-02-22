#pragma once
#include "debug/Debug.h"
#include "Scene.h"
#include "entt.hpp"
#include "Components.h"

namespace Shado {

	class Entity {
	public:
		Entity();
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity&) = default;
		~Entity();

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			SHADO_CORE_ASSERT(isValid() && !hasComponent<T>(), "Entity already has component Or is not valid!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T, typename ... Args>
		T& addOrReplaceComponent(Args&& ... args) {
			SHADO_CORE_ASSERT(isValid(), "Invalid entity!")
			return m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& getComponent() {
			SHADO_CORE_ASSERT(isValid() && hasComponent<T>(), "Entity does not have component or is not valid!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool hasComponent() {
			//SHADO_CORE_ASSERT(isValid(), "Invalid entity!")
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}

		template<typename T>
		void removeComponent() {
			SHADO_CORE_ASSERT(isValid() && hasComponent<T>(), "Entity does not have component or is invalid!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		bool isValid()	const {
			return m_EntityHandle != entt::null && m_Scene->m_Registry.valid(m_EntityHandle);
		}

		UUID getUUID() {
			return getComponent<IDComponent>().id;
		}

		operator bool()		const { return isValid(); }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

		bool operator==(const Entity& other)	const	{ return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other)	const	{ return !operator==(other); }
	protected:
		entt::entity m_EntityHandle{entt::null};
		Scene* m_Scene;
	};


	class ScriptableEntity : public Entity {
	public:
		virtual ~ScriptableEntity()	{}

	protected:
		virtual void onCreate() {}
		virtual void onUpdate(TimeStep ts) {}
		virtual void onDestroyed() {}

		friend class Scene;
	};
}
