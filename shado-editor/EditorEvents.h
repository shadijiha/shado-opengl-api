#pragma once
#include <string>
#include <sstream>

#include <glm/glm.hpp>

#include "Events/Event.h"

namespace Shado {
	class EditorGuizmosStartEvent : public Event {
	public:
		EditorGuizmosStartEvent(const glm::mat4& transform) : m_Transform(transform) {
		}

		inline const glm::mat4& getTransform() const { return m_Transform; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "EditorGuizmosStart";
			return ss.str();
		}

		EVENT_CLASS_TYPE(EditorGuizmosStart)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:
		glm::mat4 m_Transform;
	};

	class EditorEntityChanged : public Event {
	public:
		enum class ChangeType {
			ENTITY_ADDED, ENTITY_REMOVED, ENTITY_MODIFIED, ENTITY_PARENT_CHANGED,
			COMPONENT_ADDED, COMPONENT_REMOVED,
		};
		EditorEntityChanged(ChangeType type, Entity target)
			: m_ModificationType(type), m_Target(target)
		{
		}

		std::string toString() const override {
			std::stringstream ss;
			ss << "EditorEntityModified {type: " << (int)m_ModificationType << ", target: " << m_Target.getUUID() << "}";
			return ss.str();
		}

		EVENT_CLASS_TYPE(EditorEntityChanged)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)
	private:
		ChangeType m_ModificationType;
		Entity m_Target;
	};
}