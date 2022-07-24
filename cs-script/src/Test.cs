using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Shado.math;

namespace Shado
{
    public class Test : Entity
    {

        protected override void OnCreate() {
            Debug.Info("Hehexd slol {0}", Id);

            if (!HasComponent<SpriteRendererComponent>())
                AddComponent<SpriteRendererComponent>();

            SpriteRendererComponent component = GetComponent<SpriteRendererComponent>();
            component.Color = new Vector4(0.3f, 1.0f, 0.7f, 1.0f);

            Texture2D texture = new Texture2D(@"assets\riven.png");
            for (int i = 0; i < 3; i++) { 
                Entity entity = Entity.Create();
                entity.GetComponent<TransformComponent>().Position = new Vector3(i * 2, 0.0f, 0.0f);
                var sprite = entity.AddComponent<SpriteRendererComponent>();
                sprite.Color = new Vector4((i + 0.1f) / 3.0f, 0.0f, 0.3f * (i - 0.5f), 1.0f);
                sprite.Texture = texture;

                var rb = entity.AddComponent<RigidBody2DComponent>();
                rb.Type = RigidBody2DComponent.BodyType.DYNAMIC;

                var bc = entity.AddComponent<BoxCollider2DComponent>();
                bc.Restitution = 0.7f;
                Debug.Warn(sprite.Color);
            }
        }

        protected override void OnUpdate(float dt)
        {
            //Debug.Info(Input.GetMousePos());
        }

        protected override void OnDestroyed()
        {
            Debug.Info("{0} destroyed!", Id);
        }
    }
}
