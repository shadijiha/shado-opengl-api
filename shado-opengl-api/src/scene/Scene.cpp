#include "Scene.h"
#include "Components.h"
#include "renderer/Renderer2D.h"
#include "Entity.h"
#include <box2d/b2_world.h>

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

namespace Shado {

	Scene::Scene() {
	}

	Scene::~Scene() {
	}

	Entity Scene::createEntity(const std::string& name) {
		return createEntityWithUUID(name, UUID());
	}

	Entity Scene::createEntityWithUUID(const std::string& name, Shado::UUID uuid) {
		entt::entity id = m_Registry.create();
		Entity entity = { id, this };

		entity.addComponent<IDComponent>().id = uuid;
		entity.addComponent<TransformComponent>();

		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? std::string("Entity ") + std::to_string((uint64_t)uuid) : name;

		return entity;
	}

	void Scene::destroyEntity(Entity entity) {
		m_Registry.destroy(entity);
	}

	void Scene::onRuntimeStart() {
		// TODO make the physics adjustable
		m_World = new b2World({ 0.0f, -9.8f });

		auto view = m_Registry.view<RigidBody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };

			auto& transform = entity.getComponent<TransformComponent>();
			auto& rb2D = entity.getComponent<RigidBody2DComponent>();
			

			b2BodyDef bodyDef;
			bodyDef.type = (b2BodyType)rb2D.type;	// TODO : maybe change this
			bodyDef.fixedRotation = rb2D.fixedRotation;
			bodyDef.position.Set(transform.position.x, transform.position.y);
			bodyDef.angle = transform.rotation.z;

			b2Body* body = m_World->CreateBody(&bodyDef);
			rb2D.runtimeBody = body;

			if (entity.hasComponent<BoxCollider2DComponent>()) {
				auto& collider = entity.getComponent<BoxCollider2DComponent>();

				b2PolygonShape polygonShape;
				polygonShape.SetAsBox(transform.scale.x * collider.size.x, transform.scale.y * collider.size.y);

				b2FixtureDef fixtureDef;
				fixtureDef.restitution = collider.restitution;
				fixtureDef.restitutionThreshold = collider.restitutionThreshold;
				fixtureDef.friction = collider.friction;
				fixtureDef.density = collider.density;
				fixtureDef.shape = &polygonShape;

				b2Fixture* fixture = body->CreateFixture(&fixtureDef);
				collider.runtimeFixture = fixture;
			}			
		}
	}

	void Scene::onRuntimeStop() {
		delete m_World;
		m_World = nullptr;
	}

	void Scene::onUpdateRuntime(TimeStep ts) {
		// Update script
		{
			m_Registry.view<NativeScriptComponent>().each([this, ts](auto entity, NativeScriptComponent& nsc) {
				// TODO: mave to onScenePlay
				if (!nsc.script) {
					nsc.script = nsc.instantiateScript();
					nsc.script->m_EntityHandle = entity;
					nsc.script->m_Scene = this;
					nsc.script->onCreate();					
				}

				nsc.script->onUpdate(ts);
			});
		}

		// Update physics
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_World->Step(ts, velocityIterations, positionIterations);

		// Get transforms from box 2d
		auto view = m_Registry.view<RigidBody2DComponent>();
		for(auto e : view) {
			Entity entity = { e, this };

			auto& transform = entity.getComponent<TransformComponent>();
			auto& rb2D = entity.getComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rb2D.runtimeBody;
			transform.position.x = body->GetPosition().x;
			transform.position.y = body->GetPosition().y;
			transform.rotation.z = body->GetAngle();
		}
	}

	void Scene::onDrawRuntime() {

		// Render 2D: Cameras
		Camera* primaryCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			// Loop through ortho cameras
			auto group = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : group) {
				auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.primary) {
					primaryCamera = camera.camera.get();
					cameraTransform = transform.getTransform();
					break;
				}
			}
		}

		
		if (primaryCamera) {
			Renderer2D::BeginScene(*primaryCamera, cameraTransform);

			// Render stuff
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.getTransform(), sprite, (int)entity);
			}

			Renderer2D::EndScene();
		}	
	}

	void Scene::onUpdateEditor(TimeStep ts, EditorCamera& camera) {
	}

	void Scene::onDrawEditor(EditorCamera& camera) {

		Renderer2D::BeginScene(camera);

		// Render stuff
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group) {
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawSprite(transform.getTransform(), sprite, (int)entity);
		}

		Renderer2D::EndScene();
	}

	void Scene::onViewportResize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize cams
		// Loop through ortho cameras
		auto orthCams = m_Registry.view<CameraComponent>();
		for (auto entity : orthCams) {
			auto& camera = orthCams.get<CameraComponent>(entity);

			if (!camera.fixedAspectRatio) {
				camera.setViewportSize(width, height);
			}
		}
	}

	Entity Scene::getPrimaryCameraEntity() {
		auto cams = m_Registry.view<CameraComponent>();
		for (auto entity : cams) {
			const auto& camera = cams.get<CameraComponent>(entity);

			if (camera.primary)
				return { entity, this };			
		}
		return {};
	}
}
