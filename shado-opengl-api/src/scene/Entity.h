#pragma once
#include "debug/Debug.h"
#include "Scene.h"
#include "entt.hpp"

namespace Shado {

	class Entity {
	public:
		Entity();
		Entity(entt::entity handle, Scene* scene);
		Entity(entt::entity handle, entt::registry* registry);
		Entity(const Entity&) = default;
		~Entity();

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			SHADO_CORE_ASSERT(isValid() && !hasComponent<T>(), "Entity already has component Or is not valid!");
			return getRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T, typename ... Args>
		T& addOrReplaceComponent(Args&& ... args) {
			SHADO_CORE_ASSERT(isValid(), "Invalid entity!")
			return getRegistry().emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& getComponent() const {
			SHADO_CORE_ASSERT(isValid() && hasComponent<T>(), "Entity does not have component or is not valid!");
			return getRegistry().get<T>(m_EntityHandle);
		}

		template<typename T>
		bool hasComponent() const {
			//SHADO_CORE_ASSERT(isValid(), "Invalid entity!")
			return getRegistry().any_of<T>(m_EntityHandle);
		}

		template<typename T>
		void removeComponent() {
			SHADO_CORE_ASSERT(isValid() && hasComponent<T>(), "Entity does not have component or is invalid!");
			getRegistry().remove<T>(m_EntityHandle);
		}

		bool isValid()	const {
			return m_EntityHandle != entt::null && getRegistry().valid(m_EntityHandle);
		}

		bool isChild() const;

		UUID getUUID() const;

		const Scene& getScene() const { return *m_Scene; }
		void setScene(Scene* scene) { m_Scene = scene; }
		
		std::vector<Entity> getChildren() const;

		operator bool()		const { return isValid(); }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

		bool operator==(const Entity& other)	const	{ return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other)	const	{ return !operator==(other); }
	protected:
		entt::registry& getRegistry() const {
			return m_Scene != nullptr ? m_Scene->m_Registry : *m_Registry;
		}

	protected:
		entt::entity m_EntityHandle{entt::null};
		Scene* m_Scene;
		entt::registry* m_Registry;
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
