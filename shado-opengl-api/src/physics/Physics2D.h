#pragma once
#include "scene/Components.h"
#include "box2d/b2_body.h"

namespace Shado {

	namespace Utils {

		inline b2BodyType Rigidbody2DTypeToBox2DBody(RigidBody2DComponent::BodyType bodyType)
		{
			switch (bodyType)
			{
			case RigidBody2DComponent::BodyType::STATIC:    return b2_staticBody;
			case RigidBody2DComponent::BodyType::DYNAMIC:   return b2_dynamicBody;
			case RigidBody2DComponent::BodyType::KINEMATIC: return b2_kinematicBody;
			}

			SHADO_CORE_ASSERT(false, "Unknown body type");
			return b2_staticBody;
		}

		inline RigidBody2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
		{
			switch (bodyType)
			{
			case b2_staticBody:    return RigidBody2DComponent::BodyType::STATIC;
			case b2_dynamicBody:   return RigidBody2DComponent::BodyType::DYNAMIC;
			case b2_kinematicBody: return RigidBody2DComponent::BodyType::KINEMATIC;
			}

			SHADO_CORE_ASSERT(false, "Unknown body type");
			return RigidBody2DComponent::BodyType::STATIC;
		}

	}

}