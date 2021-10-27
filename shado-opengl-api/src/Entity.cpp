#include "Entity.h"
#include "box2d/b2_world.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "Renderer2D.h"


namespace Shado {

	Entity::Entity(const EntityDefinition& definition, b2World& world)
		: name(definition.name), scale(definition.scale), tilingfactor(definition.tillingfactor), texture(definition.texture), color(definition.color)
	{
		id = rand();
		z = definition.position.z;

		// Create box2D body
		b2BodyDef bodyDef;
		bodyDef.type = (b2BodyType)definition.type;
		bodyDef.position.Set(definition.position.x, definition.position.y);
		
		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(definition.scale.x / 2.0f, definition.scale.y / 2.0f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.density = definition.density;
		fixtureDef.friction = definition.friction;

		b2Body* body = world.CreateBody(&bodyDef);
		body->CreateFixture(&fixtureDef);

		this->body = body;
	}

	Entity::~Entity() {}

	void Entity::draw() const {
		const b2Vec2& position = body->GetPosition();
		float angle = body->GetAngle();
		
		if (texture != nullptr) {
			Renderer2D::DrawRotatedQuad({ position.x, position.y, z }, scale, angle, texture);
		} else
		{
			Renderer2D::DrawRotatedQuad({ position.x, position.y, z }, scale, angle, color);
		}
	}

	void Entity::destroy() {
		body->GetWorld()->DestroyBody(body);
	}

	Entity& Entity::setName(const std::string& name) {
		this->name = name;
		return *this;
	}

	Entity& Entity::setTexture(Ref<Texture2D> texture) {
		this->texture = texture;
		return *this;
	}

	Entity& Entity::setTexture(const std::string& path) {
		texture = CreateRef<Texture2D>(path);
		return *this;
	}

	Entity& Entity::setTillingFactor(uint32_t factor) {
		this->tilingfactor = factor;
		return *this;
	}

	Entity& Entity::setColor(const Color& color) {
		this->color = color;
		return *this;
	}

	Entity& Entity::setType(const EntityType& type) {
		body->SetType((b2BodyType)type);

		return *this;
	}
}
