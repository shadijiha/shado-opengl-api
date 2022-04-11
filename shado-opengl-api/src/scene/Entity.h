#pragma once
#include "Debug.h"
#include "Scene.h"
#include "entt.hpp"

namespace Shado {

	class Entity {
	public:
		Entity();
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity&) = default;
		~Entity();

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			SHADO_CORE_ASSERT(!hasComponent<T>(), "Entity already has component!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& getComponent() {
			SHADO_CORE_ASSERT(hasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool hasComponent() {
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}

		template<typename T>
		void removeComponent() {
			SHADO_CORE_ASSERT(hasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		operator bool()		const { return m_EntityHandle != entt::null; }
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
