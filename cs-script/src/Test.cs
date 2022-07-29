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
        private Entity camera;
        private List<Entity> entities = new List<Entity>();
        private bool flag = false;
        protected override void OnCreate() {
            camera = Scene.GetPrimaryCameraEntity();

            Debug.Info("Transform {0}", Transform.Position);

            if (!HasComponent<SpriteRendererComponent>())
                AddComponent<SpriteRendererComponent>();

            SpriteRendererComponent component = GetComponent<SpriteRendererComponent>();
            component.Color = new Vector4(0.3f, 1.0f, 0.7f, 1.0f);

            Texture2D texture = new Texture2D(@"assets\riven.png");
            for (int i = 1; i < 4; i++) { 
                Entity entity = Entity.Create();
                entity.GetComponent<TransformComponent>().Position = new Vector3(i * 2, 0.0f, 0.0f);
                var sprite = entity.AddComponent<SpriteRendererComponent>();
                //sprite.Color = new Vector4((i + 0.1f) / 3.0f, 0.0f, 0.3f * (i - 0.5f), 1.0f);
                sprite.Texture = texture;

                var rb = entity.AddComponent<RigidBody2DComponent>();
                rb.Type = RigidBody2DComponent.BodyType.DYNAMIC;

                var bc = entity.AddComponent<BoxCollider2DComponent>();
                bc.Restitution = 0.7f;
 
                entities.Add(entity);
            }
        }

        protected override void OnUpdate(float dt)
        {
            if (!flag && Input.IsKeyPressed(KeyCode.Space)) {

                Texture2D texture = entities[0].GetComponent<SpriteRendererComponent>().Texture;
                if (texture != null)
                    Debug.Info(texture);

                entities[0].Destroy();
                entities.RemoveAt(0);
                flag = true;
            }

            if (camera != null)
            {
                if (Input.IsKeyPressed(KeyCode.D))
                {
                    camera.Transform.Position += new Vector3(0.05f, 0.0f, 0.0f);
                }
                else if (Input.IsKeyPressed(KeyCode.A))
                {
                    camera.Transform.Position -= new Vector3(0.05f, 0.0f, 0.0f);
                }

                if (Input.IsKeyPressed(KeyCode.W))
                {
                    camera.Transform.Position += new Vector3(0.0f, 0.05f, 0.0f);
                }
                else if (Input.IsKeyPressed(KeyCode.S))
                {
                    camera.Transform.Position -= new Vector3(0.0f, 0.05f, 0.0f);
                }
            }


            Renderer.DrawQuad(Vector3.Zero, Vector3.One, Vector3.Zero);
        }

        protected override void OnDestroyed()
        {
            Debug.Info("{0} destroyed!", Id);
        }
    }
}
