#pragma once
#include <glm/vec2.hpp>

#include "box2d/b2_body.h"
#include "Texture2D.h"
#include "util/Util.h"

namespace Shado {
	enum class EntityType {
		STATIC = 0,
		KINEMATIC,
		DYNAMIC
	};

	struct EntityDefinition {
		std::string name = "Untitled Entity";
		EntityType type = EntityType::STATIC;

		glm::vec3 position = {0, 0, 0};
		glm::vec2 scale = {1, 1};

		Ref<Texture2D> texture = nullptr;
		uint32_t tillingfactor = 1;
		Color color = Color::WHITE;

		float density = 1.0f;
		float friction = 0.3f;
	};
	
	class Entity {
	public:
		Entity(const EntityDefinition& definition, b2World& world);
		~Entity();

		void draw() const;
		void destroy();

		Entity& setName(const std::string& name);
		Entity& setTexture(Ref<Texture2D> texture);
		Entity& setTexture(const std::string& path);
		Entity& setTillingFactor(uint32_t tillingfactor);
		Entity& setColor(const Color& color);
		Entity& setType(const EntityType& type);

		uint64_t getId()			const	{ return id; }
		std::string getName()		const	{ return name; }
		Ref<Texture2D> getTexture() const	{ return texture; }
		uint32_t getTillingFactor() const	{ return tilingfactor; }
		Color	getColor()			const	{ return color; }
		b2Body* getNativeBody()				{ return body; }
		
	private:
		uint64_t id;
		std::string name;

		glm::vec2 scale;
		float z;
		
		Ref<Texture2D> texture;
		uint32_t tilingfactor;
		Color color;

		b2Body* body;
	};
}
