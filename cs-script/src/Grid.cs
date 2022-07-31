using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Shado;
using Shado.math;

namespace Sandbox
{
    public class Grid : Entity
    {
        List<Entity> entities = new List<Entity>();

        protected override void OnCreate()
        {

            for (float y = -5.0f; y < 5.0f; y += 0.5f)
            {
                for (float x = -5.0f; x < 5.0f; x += 0.5f)
                {
                    Entity entity = Create();
                    entity.Transform.Position = Transform.Position + new Vector3(x, y, 0);
                    entity.Transform.Scale = new Vector3(0.45f, 0.45f, 0);
                    entity.Tag = $"Grid cell {x}, {y}";

                    var sprite = entity.AddComponent<SpriteRendererComponent>();
                    sprite.Color = new Vector4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f);

                    var rb = entity.AddComponent<RigidBody2DComponent>();
                    rb.Type = RigidBody2DComponent.BodyType.DYNAMIC;

                    entity.AddComponent<BoxCollider2DComponent>();

                    entities.Add(entity);
                }
             }
        }

        protected override void OnUpdate(float dt)
        {
           
        }
    }
}
